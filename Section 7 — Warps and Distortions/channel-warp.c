#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <gtk/gtk.h>
#include <gegl-0.4/gegl.h>
#include <math.h>

#define PROC_NAME "channel-warp"

//function prototypes
static void query (void);
static void run (
    const char          *name,
    int                 nparams,
    const GimpParam     *param,
    int                 *n_returns,
    GimpParam           **v_returns);

typedef struct {
    GimpRGB control_color;
    double vector[2];
    double origin[2];
} MyParameters;

static MyParameters myparms =
{
    {1.0, 0.0, 0.0, 1.0},
    {2.0, 2.0},
    {0.5, 0.5}
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
      GIMP_PDB_COLOR,
      "control color",
      "Color by which to scale warping"
    },
    {
      GIMP_PDB_FLOAT,
      "x displacement",
      "Horizontal translation degree"
    },
    {
      GIMP_PDB_FLOAT,
      "y displacement",
      "Vertical translation degree"
    }
  };

  gimp_install_procedure (
    PROC_NAME,
    "Color Warp",
    "Warp according to hue angle with a color",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "_Color Warp",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PROC_NAME,
                               "<Image>/Filters/Misc/Distorts"); 
}

static void control_color_adjust(GimpColorSelection *selection) {
    gimp_color_selection_get_color(selection, &myparms.control_color);
}

gboolean plugin_dialog(gint32 drawable) {
    gboolean ok_to_run;
    
    GtkWidget *dialog;
    
    #define NAME "channel-warp"
    gimp_ui_init(NAME, FALSE);
    
    dialog = gimp_dialog_new("Warp by Color Angle", NAME,
                                NULL, 0,
                                NULL, PROC_NAME,
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OK, GTK_RESPONSE_OK,
                                NULL);    
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    GtkWidget *color_label = gtk_label_new("Warp Color:");
    gtk_widget_show(color_label);
    
    GtkWidget *color_chooser = gimp_color_selection_new();
    gimp_color_selection_set_color(GIMP_COLOR_SELECTION(color_chooser), &myparms.control_color);
    gtk_widget_show(color_chooser);
    
    GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), color_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), color_chooser, TRUE, TRUE, 0);
    gtk_widget_show(hbox);

    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    GtkWidget *vector_label = gtk_label_new("Warp Vector:");
    gtk_widget_show(vector_label);
    
    GtkObject *x_adj = gtk_adjustment_new(myparms.vector[0], -G_MAXDOUBLE, G_MAXDOUBLE, .1, 10, 0);
    GtkWidget *x_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (x_adj), 2, 2);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(x_spinner), TRUE);
    gtk_widget_show(x_spinner);
    
    GtkObject *y_adj = gtk_adjustment_new(myparms.vector[1], -G_MAXDOUBLE, G_MAXDOUBLE, .1, 10, 0);
    GtkWidget *y_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (y_adj), 2, 2);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(y_spinner), TRUE);
    gtk_widget_show(y_spinner);
    
    hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), vector_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), x_spinner, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), y_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);

    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    GtkWidget *origin_label = gtk_label_new("Warp Origin:");
    gtk_widget_show(origin_label);
    
    GtkObject *ox_adj = gtk_adjustment_new(myparms.origin[0], 0.0, 1.0, 0.01, 0.1, 0);
    GtkWidget *ox_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (ox_adj), 2, 2);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(ox_spinner), TRUE);
    gtk_widget_show(ox_spinner);
    
    GtkObject *oy_adj = gtk_adjustment_new(myparms.origin[1], 0.0, 1.0, 0.01, 0.1, 0);
    GtkWidget *oy_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (oy_adj), 2, 2);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(oy_spinner), TRUE);
    gtk_widget_show(oy_spinner);
    
    hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), origin_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), ox_spinner, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), oy_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);

    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    gtk_widget_show (dialog);
    
    //connect widgets here
    g_signal_connect(x_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.vector[0]);
    g_signal_connect(y_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.vector[1]);
    g_signal_connect(ox_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.origin[0]);
    g_signal_connect(oy_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.origin[1]);
    g_signal_connect(color_chooser, "color_changed", G_CALLBACK (control_color_adjust), GIMP_COLOR_SELECTION(color_chooser));
    ok_to_run = (gimp_dialog_run (GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);
    
    gtk_widget_destroy (dialog);
    return ok_to_run;
}


static GimpPDBStatusType action(gint32 image, gint32 drawable) {
    //create rgba->hsva fish
    const Babl *hsla = babl_format("HSLA double");
    const Babl *fish = babl_fish(babl_format("RGBA double"), hsla);
    
    //create single-pixel buffer of our GimpRGB color
    const double sample[4];
    
    //convert our sample to HSLA
    babl_process(fish, &myparms.control_color, (void *)sample, 1);
    
    //grab buffer, and shadow
    GeglBuffer *buffer = gimp_drawable_get_buffer(drawable);
    GeglBuffer *shadow = gimp_drawable_get_shadow_buffer(drawable);
    
    GeglMatrix3 *trans = gegl_matrix3_new();
    gegl_matrix3_identity(trans);
    trans->coeff[0][2] = myparms.origin[0] * gegl_buffer_get_width(buffer);
    trans->coeff[1][2] = myparms.origin[1] * gegl_buffer_get_height(buffer);
    
    GeglMatrix3 *inv_trans = gegl_matrix3_copy(trans);
    gegl_matrix3_invert(inv_trans);
    
    GeglMatrix3 *mat3 = gegl_matrix3_new();
    double destX, destY;
    for(double x = 0; x < gimp_drawable_width(drawable); x++) {
        gimp_progress_update(x/gimp_drawable_width(drawable));
        for(double y = 0; y < gimp_drawable_height(drawable); y++) {
            destX = x;
            destY = y;
            
            GeglRectangle srcRect = {x, y, 1, 1};
            
            double magnitude[4];
            
            //nonlinearity
            gegl_buffer_get(buffer, &srcRect, 1, hsla, &magnitude, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
            
            double d_hue = 1.0 - fmod(fabs(magnitude[0] - sample[0]), 0.5) * 2.0;
            
            mat3->coeff[0][0] = myparms.vector[0] * d_hue;
            mat3->coeff[1][1] = myparms.vector[1] * d_hue;
            mat3->coeff[2][2] = 1.0;
            
            gegl_matrix3_transform_point(inv_trans, &destX, &destY);
            gegl_matrix3_transform_point(mat3, &destX, &destY);
            gegl_matrix3_transform_point(trans, &destX, &destY);
            
            //GeglRectangle srcRect = {x, y, 1, 1};
            GeglRectangle destRect = {destX, destY, 1, 1};
            
            gegl_buffer_copy(buffer, &destRect, GEGL_ABYSS_CLAMP, shadow, &srcRect);
        }
    }
            
    gegl_buffer_flush(shadow);
    gimp_drawable_merge_shadow(drawable, TRUE);
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    
    g_object_unref(shadow);
    g_object_unref(buffer);
    
    return GIMP_PDB_SUCCESS;
}

static void run (
    const char      *name,
    int             nparams,
    const GimpParam *param,
    int             *nreturn_vals,
    GimpParam       **return_vals)
{
    static GimpParam  values[1];
    GimpRunMode run_mode = param[0].data.d_int32;
    gint32 image = param[1].data.d_image;
    gint32 drawable = param[2].data.d_drawable;
    
    *nreturn_vals = 1;
    *return_vals  = values;

    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = GIMP_PDB_SUCCESS;
    
    switch(run_mode){
        case GIMP_RUN_INTERACTIVE:
            gimp_get_data(PROC_NAME, &myparms);
            if(!plugin_dialog(drawable))
                return;
            break;
        case GIMP_RUN_NONINTERACTIVE:
            if(nparams != 4)
                values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
            else {
                myparms.control_color.r = param[4].data.d_float;
                myparms.control_color.g = param[5].data.d_float;
                myparms.control_color.b = param[6].data.d_float;
                myparms.control_color.a = 1.0;
                myparms.vector[0] = param[7].data.d_float;
                myparms.vector[1] = param[8].data.d_float;
                myparms.origin[0] = param[9].data.d_float;
                myparms.origin[1] = param[10].data.d_float;
                }
            break;
        case GIMP_RUN_WITH_LAST_VALS:
            gimp_get_data (PROC_NAME, &myparms);
            break;
        default:
            break;
    }
    
    gegl_init(NULL, NULL);
    babl_init();
    
    values[0].data.d_status = action(image, drawable);
    
    gegl_exit();
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    gimp_displays_flush();

    if(run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PROC_NAME, &myparms, sizeof(myparms));
}

MAIN()
