#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <gegl-0.4/gegl.h>
#include <gtk/gtk.h>
#include <math.h>

#define PROC_NAME "hue-rotation-by-image"

//function prototypes
static void query (void);
static void run (
    const char          *name,
    int                 nparams,
    const GimpParam     *param,
    int                 *n_returns,
    GimpParam           **v_returns);

typedef struct {
    double scale_width;
    double scale_height;
    gint32 parameter_drawable;
} MyParameters;

static MyParameters myparms =
{
    1.0,
    1.0,
    0
};


//Navigation structure so GIMP knows where to look
GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,    //initialization plug-in
  NULL,    //quitting plug-in
  query,   //plug-in query function
  run,     //and, where the actual work is done!
};

static void query (void)
{
  static GimpParamDef args[] = {
    {
      GIMP_PDB_INT32,
      "run-mode",
      "Interactive, or non-interactive"
    },
    {
      GIMP_PDB_IMAGE,
      "image",
      "Image of application"
    },
    {
      GIMP_PDB_DRAWABLE,
      "drawable",
      "Drawable surface"
    },
    {
      GIMP_PDB_INT32,
      "image",
      "Luminosity image that controls hue rotation."
    }
  };

  gimp_install_procedure (
    PROC_NAME,
    "Image-Based Hue Rotation",
    "Rotates selection hue by image luminosity",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "Hue Rotation by _Image...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PROC_NAME,
                               "<Image>/Filters/Misc/Image Parameters"); 
}

static void update_image_to_value (GtkWidget  *widget,
                              gpointer *data) {
    gimp_int_combo_box_get_active(GIMP_INT_COMBO_BOX(widget), &myparms.parameter_drawable);
}

static gboolean hue_rotation_dialog(gint32 drawable) {
    gboolean ok_to_run;
    
    GtkWidget *dialog;
    
    gimp_ui_init("hue_rotation_by_image", FALSE);
    
    dialog = gimp_dialog_new("Hue Rotation By Image", "hue_rotation_by_image",
                                NULL, 0,
                                NULL, PROC_NAME,
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OK, GTK_RESPONSE_OK,
                                NULL);    
    
    GtkWidget *rotation_label = gtk_label_new_with_mnemonic("_Rotation Image:");
    gtk_widget_show(rotation_label);
    
    GtkWidget *image_dropdown = gimp_drawable_combo_box_new(NULL, NULL);
    gtk_widget_show(image_dropdown);
    
    GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), rotation_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), image_dropdown, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    GtkWidget *scale_label = gtk_label_new_with_mnemonic("_Scale:");
    gtk_widget_show(scale_label);
    
    GtkObject *width_adj = gtk_adjustment_new(myparms.scale_width, 0, G_MAXDOUBLE, .1, 10, 0);
    GtkWidget *width_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (width_adj), 2, 2);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(width_spinner), TRUE);
    gtk_widget_show(width_spinner);
    
    GtkObject *height_adj = gtk_adjustment_new(myparms.scale_height, 0, G_MAXDOUBLE, .1, 10, 0);
    GtkWidget *height_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (height_adj), 2, 2);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(height_spinner), TRUE);
    gtk_widget_show(height_spinner);

    hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), scale_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), width_spinner, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), height_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    
    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    gtk_widget_show (dialog);
    
    gimp_int_combo_box_connect(GIMP_INT_COMBO_BOX(image_dropdown), myparms.parameter_drawable, G_CALLBACK(update_image_to_value), NULL);
    g_signal_connect(width_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.scale_width);
    g_signal_connect(height_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.scale_height);
    
    ok_to_run = (gimp_dialog_run (GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);
    
    gtk_widget_destroy (dialog);
    return ok_to_run;
}

static GeglBuffer *get_control_buffer(gint32 drawable) {
    gint32 x, y, width, height;
    gimp_drawable_mask_bounds(drawable, &x, &y, &width, &height);
    const GeglRectangle rect = {x, y, width, height};
    
    const Babl *y_format = babl_format("Y double");
    
    GeglBuffer *parameter = gimp_drawable_get_buffer(drawable);
    GeglBuffer *control = gegl_buffer_new(&rect, y_format);
    gegl_buffer_copy(parameter, &rect, GEGL_ABYSS_NONE, control, &rect);
    
    g_object_unref(parameter);
    
    return control;
}

static GimpPDBStatusType rotate_colors(GimpRunMode run_mode, gint32 image, gint32 drawable, gint32 parameter_drawable) {
    GimpPDBStatusType success = GIMP_PDB_SUCCESS;

    gegl_init(NULL, NULL);
    babl_init();
    
    GeglBuffer *buffer = gimp_drawable_get_buffer(drawable);
    const GeglRectangle *rect = gegl_buffer_get_extent(buffer);
    const Babl *hsl_format = babl_format("HSL double");
    
    GeglBuffer *shadow = gimp_drawable_get_shadow_buffer(drawable);
    GeglBuffer *midpoint = gegl_buffer_new(rect, hsl_format);
    
    gegl_buffer_copy(buffer, rect, GEGL_ABYSS_NONE, midpoint, rect);
    
    GeglBufferIterator *gi = gegl_buffer_iterator_new(
        midpoint,
        rect,
        0,
        hsl_format,
        GEGL_ACCESS_READWRITE,
        GEGL_ABYSS_NONE,
        1);
    
    gegl_buffer_flush(midpoint);
    
    GeglBuffer *control = get_control_buffer(parameter_drawable);
    const Babl *control_format = babl_format("Y double");
    
    GeglSampler *sampler = gegl_buffer_sampler_new(
        control,
        control_format,
        GEGL_SAMPLER_LINEAR);
    
    //double action_angle = 0.125; //myparms.angle/360.0;
    double action_angle;
    double d_hue;
    
    GeglBufferMatrix2 scale = {{{myparms.scale_width, 0}, {0, myparms.scale_height}}};
    while(gegl_buffer_iterator_next(gi)) {
        double *pixel = (double *)gi->items[0].data;
        GeglRectangle roi = gi->items[0].roi;
        int size = roi.width * roi.height * (8 * 3);
        for(int ix = 0; ix < roi.width; ix++)
            for(int iy = 0; iy < roi.height; iy++) {
                int i = (iy * roi.width + ix) * 3;
                gegl_sampler_get(sampler,
                    (double)(roi.x + ix),
                    (double)(roi.y + iy),
                    &scale,
                    &d_hue,
                    GEGL_ABYSS_WHITE);
                action_angle = d_hue - 0.5;
                pixel[i + 0] = fmod((action_angle + pixel[i + 0]), 1.0);
            }
    }
    
    gegl_buffer_copy(midpoint, rect, GEGL_ABYSS_NONE, shadow, rect);
    
    gegl_buffer_flush(shadow);
    gimp_drawable_merge_shadow(drawable, TRUE);
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    
    g_object_unref(midpoint);
    g_object_unref(shadow);
    g_object_unref(buffer);
    
    gegl_exit();
    
    return success;
}

static void run (
    const char      *name,
    int             nparams,
    const GimpParam *param,
    int             *nreturn_vals,
    GimpParam       **return_vals)
{
    static GimpParam  values[1];
    GimpRunMode       run_mode;
    
    *nreturn_vals = 1;
    *return_vals  = values;

    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = GIMP_PDB_SUCCESS;
    
    run_mode = param[0].data.d_int32;
    gint32 image = param[1].data.d_image;
    gint32 drawable = param[2].data.d_drawable;
    
    
    switch(run_mode){
        case GIMP_RUN_INTERACTIVE:
            gimp_get_data(PROC_NAME, &myparms);
            if(!hue_rotation_dialog(drawable))
                return;
            break;
        case GIMP_RUN_NONINTERACTIVE:
            if(nparams != 4)
                values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
            else
                myparms.parameter_drawable = param[3].data.d_int32;
            break;
        case GIMP_RUN_WITH_LAST_VALS:
            gimp_get_data (PROC_NAME, &myparms);
            break;
        default:
            break;
    }
    
    gint32 parameter_drawable = myparms.parameter_drawable;
    values[0].data.d_status = rotate_colors(run_mode, image, drawable, parameter_drawable);
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    gimp_displays_flush();
    
    if(run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PROC_NAME, &myparms, sizeof(myparms));

}

MAIN()
