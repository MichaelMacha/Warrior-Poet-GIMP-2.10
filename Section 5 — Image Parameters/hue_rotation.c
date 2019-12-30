#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <gtk/gtk.h>
#include <math.h>

#define PROC_NAME "hue-rotation"

//function prototypes
static void query (void);
static void run (
    const char          *name,
    int                 nparams,
    const GimpParam     *param,
    int                 *n_returns,
    GimpParam           **v_returns);

typedef struct {
    double angle;
} MyParameters;

static MyParameters myparms =
{
    45.0
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
      GIMP_PDB_FLOAT,
      "angle",
      "Angle by which color hue will rotate."
    }
  };

  gimp_install_procedure (
    PROC_NAME,
    "Scalar Hue Rotation",
    "Rotates selection hue by scalar value",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "Hue _Rotation...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PROC_NAME,
                               "<Image>/Filters/Misc/Image Parameters"); 
}

static gboolean hue_rotation_dialog(gint32 drawable) {
    gboolean ok_to_run;
    
    GtkWidget *dialog;
    
    gimp_ui_init("hue_rotation", FALSE);
    
    dialog = gimp_dialog_new("Hue Rotation", "hue_rotation",
                                NULL, 0,
                                NULL, PROC_NAME,
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OK, GTK_RESPONSE_OK,
                                NULL);    
    
    GtkWidget *rotation_label = gtk_label_new_with_mnemonic("_Rotation Angle:");
    gtk_widget_show(rotation_label);
    
    GtkObject *rotation_adj = gtk_adjustment_new(myparms.angle, -360.0, 360.0, 1.0, 15.0, 0);
    GtkWidget *rotation_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (rotation_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(rotation_spinner), TRUE);
    gtk_widget_show(rotation_spinner);
    
    GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), rotation_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), rotation_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    gtk_widget_show (dialog);
    
    g_signal_connect(rotation_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.angle);
    
    ok_to_run = (gimp_dialog_run (GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);
    
    gtk_widget_destroy (dialog);
    return ok_to_run;
}

static GimpPDBStatusType rotate_colors(GimpRunMode run_mode, gint32 image, gint32 drawable) {
    gegl_init(NULL, NULL);
    babl_init();
    
    const Babl *hsl_format = babl_format("HSL double");
    gint x_offset, y_offset;
    gimp_drawable_offsets(drawable, &x_offset, &y_offset);
    const GeglRectangle rect = {x_offset, y_offset, gimp_drawable_width(drawable), gimp_drawable_height(drawable)};
    
    GeglBuffer *buffer = gimp_drawable_get_buffer(drawable);
    GeglBuffer *shadow = gimp_drawable_get_shadow_buffer(drawable);
    GeglBuffer *midpoint = gegl_buffer_new(&rect, hsl_format);
    
    gegl_buffer_copy(buffer, &rect, GEGL_ABYSS_NONE, midpoint, &rect);
    
    GeglBufferIterator *gi = gegl_buffer_iterator_new(
        midpoint,
        &rect,
        0,
        hsl_format,
        GEGL_ACCESS_READWRITE,
        GEGL_ABYSS_NONE,
        1);
    
    const double action_angle = myparms.angle/360.0;
    
    while(gegl_buffer_iterator_next(gi)) {
        double *pixel = (double *)gi->items[0].data;
        GeglRectangle roi = gi->items[0].roi;
        int size = roi.width * roi.height * (8 * 3);
        for(int ix = 0; ix < roi.width; ix++)
            for(int iy = 0; iy < roi.height; iy++) {
                int i = (iy * roi.width + ix) * 3;
                pixel[i + 0] = fmod((action_angle + pixel[i + 0]), 1.0);
            }
    }
    
    gegl_buffer_copy(midpoint, &rect, GEGL_ABYSS_NONE, shadow, &rect);
    
    gimp_drawable_merge_shadow(drawable, TRUE);
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    
    g_object_unref(midpoint);
    g_object_unref(shadow);
    g_object_unref(buffer);
    
    gegl_exit();
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
                myparms.angle = param[3].data.d_float;
            break;
        case GIMP_RUN_WITH_LAST_VALS:
            gimp_get_data (PROC_NAME, &myparms);
            break;
        default:
            break;
    }
    
    values[0].data.d_status = rotate_colors(run_mode, image, drawable);
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    gimp_displays_flush();
    
    if(run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PROC_NAME, &myparms, sizeof(myparms));

}

MAIN()
