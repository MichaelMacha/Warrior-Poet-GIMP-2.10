#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <gtk/gtk.h>
#include <gegl-0.4/gegl.h>
#include <gegl-0.4/gegl-buffer.h>

#define PLUGIN_NAME "region-operations-example"

//function prototypes
static void query (void);
static void run (
    const char          *name,
    int                 nparams,
    const GimpParam     *param,
    int                 *n_returns,
    GimpParam           **v_returns);

typedef struct {
    double source_x;
    double source_y;
    double source_width;
    double source_height;

    double sink_x;
    double sink_y;
    double sink_width;
    double sink_height;
} MyParameters;

static MyParameters myparms =
{
    32,
    32,
    32,
    32,
    
    0,
    0,
    32,
    32    
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
      "source-x",
      "Source x origin"
    },
    {
      GIMP_PDB_FLOAT,
      "source-y",
      "Source y origin"
    },
    {
      GIMP_PDB_FLOAT,
      "source-width",
      "Width of source"
    },
    {
      GIMP_PDB_FLOAT,
      "source-height",
      "Height of source"
    },
    {
      GIMP_PDB_FLOAT,
      "sink-x",
      "Sink x origin"
    },
    {
      GIMP_PDB_FLOAT,
      "sink-y",
      "Sink y origin"
    },
    {
      GIMP_PDB_FLOAT,
      "sink-width",
      "Width of sink"
    },
    {
      GIMP_PDB_FLOAT,
      "sink-height",
      "Height of sink"
    },
  };

  gimp_install_procedure (
    PLUGIN_NAME,
    "Regional operations example",
    "Demonstration of using GEGL to manipulate entire buffers at once..",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "_Region Example...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PLUGIN_NAME,
                               "<Image>/Filters/Misc/Region"); 
}

static void flip_region(gint32 image, gint32 drawable) {
    gegl_init(NULL, NULL);
    babl_init();
    
    GeglBuffer *buffer = gimp_drawable_get_buffer(drawable);
    GeglBuffer *shadow = gimp_drawable_get_shadow_buffer(drawable);
    
    GeglRectangle destRect = {myparms.sink_x, myparms.sink_y, myparms.sink_width, myparms.sink_height};
    GeglRectangle rect = {myparms.source_x, myparms.source_y, myparms.source_width, myparms.source_height};
    
    const Babl *format = gegl_buffer_get_format(buffer);
    GeglBuffer *side_buffer = gegl_buffer_new(&destRect, format);
    
    //fill shadow buffer with unaltered original
    gegl_buffer_copy(buffer,
                    gegl_buffer_get_extent(buffer),
                    GEGL_ABYSS_WHITE,
                    shadow,
                    gegl_buffer_get_extent(buffer));
    gegl_buffer_copy(buffer,
                    gegl_buffer_get_extent(buffer),
                    GEGL_ABYSS_WHITE,
                    side_buffer,
                    gegl_buffer_get_extent(side_buffer));
    
    gegl_render_op(side_buffer,
                   side_buffer,
                   "gegl:reflect",
                   "origin-x", gegl_buffer_get_width(side_buffer)/2.0,
                   "origin-y", gegl_buffer_get_height(side_buffer)/2.0,
                   "x", (gdouble)-1,
                   "y", (gdouble)0,
                   NULL);
    gegl_render_op(side_buffer,
                   side_buffer,
                   "gegl:scale-ratio",
                   "x", (gdouble)(myparms.sink_width/myparms.source_width),
                   "y", (gdouble)(myparms.sink_height/myparms.source_height),
                   NULL);
    
    GeglRectangle scaled_rect = {destRect.x, destRect.y,
      destRect.width / 2.0, destRect.height / 2.0};
      //gegl_buffer_get_width(side_buffer)/2.0,
      //gegl_buffer_get_height(side_buffer)/2.0};
    
    gegl_buffer_copy(side_buffer,
                     &scaled_rect,
                     GEGL_ABYSS_NONE,
                     shadow,
                     &scaled_rect);
    
    g_object_unref(side_buffer);
    g_object_unref(shadow);
    g_object_unref(buffer);
    
    gimp_drawable_merge_shadow(drawable, TRUE);
    
    gimp_drawable_update(drawable, 0, 0,
        gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    
    gimp_displays_flush();
    
    gegl_exit();
}

static gboolean region_dialog(gint32 drawable) {
    gboolean ok_to_run;
    
    GtkWidget *dialog;
    
    gimp_ui_init("region", FALSE);
    
    dialog = gimp_dialog_new("Region", "region",
                                NULL, 0,
                                NULL, PLUGIN_NAME,
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OK, GTK_RESPONSE_OK,
                                NULL);    
    
    GtkWidget *source_label = gtk_label_new_with_mnemonic("S_ource:");
    gtk_widget_show(source_label);
    
    GtkObject *source_x_adj = gtk_adjustment_new(myparms.source_x, 0, G_MAXINT32, 1, 10, 0);
    GtkWidget *source_x_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (source_x_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(source_x_spinner), TRUE);
    gtk_widget_show(source_x_spinner);
    
    GtkObject *source_y_adj = gtk_adjustment_new(myparms.source_y, 0, G_MAXINT32, 1, 10, 0);
    GtkWidget *source_y_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (source_y_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(source_y_spinner), TRUE);
    gtk_widget_show(source_y_spinner);
    
    GtkObject *source_width_adj = gtk_adjustment_new(myparms.source_width, 0, G_MAXINT32, 1, 10, 0);
    GtkWidget *source_width_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (source_width_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(source_width_spinner), TRUE);
    gtk_widget_show(source_width_spinner);
    
    GtkObject *source_height_adj = gtk_adjustment_new(myparms.source_height, 0, G_MAXINT32, 1, 10, 0);
    GtkWidget *source_height_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (source_height_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(source_height_spinner), TRUE);
    gtk_widget_show(source_height_spinner);
    
    GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), source_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), source_x_spinner, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), source_y_spinner, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), source_width_spinner, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), source_height_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    GtkWidget *sink_label = gtk_label_new_with_mnemonic("S_ource:");
    gtk_widget_show(sink_label);
    
    GtkObject *sink_x_adj = gtk_adjustment_new(myparms.sink_x, 0, G_MAXINT32, 1, 10, 0);
    GtkWidget *sink_x_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (sink_x_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(sink_x_spinner), TRUE);
    gtk_widget_show(sink_x_spinner);
    
    GtkObject *sink_y_adj = gtk_adjustment_new(myparms.sink_y, 0, G_MAXINT32, 1, 10, 0);
    GtkWidget *sink_y_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (sink_y_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(sink_y_spinner), TRUE);
    gtk_widget_show(sink_y_spinner);
    
    GtkObject *sink_width_adj = gtk_adjustment_new(myparms.sink_width, 0, G_MAXINT32, 1, 10, 0);
    GtkWidget *sink_width_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (sink_width_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(sink_width_spinner), TRUE);
    gtk_widget_show(sink_width_spinner);
    
    GtkObject *sink_height_adj = gtk_adjustment_new(myparms.sink_height, 0, G_MAXINT32, 1, 10, 0);
    GtkWidget *sink_height_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (sink_height_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(sink_height_spinner), TRUE);
    gtk_widget_show(sink_height_spinner);
    
    hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), sink_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), sink_x_spinner, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), sink_y_spinner, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), sink_width_spinner, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), sink_height_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
        
    gtk_widget_show (dialog);
    
    g_signal_connect(source_x_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.source_x);
    g_signal_connect(source_y_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.source_y);
    g_signal_connect(source_width_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.source_width);
    g_signal_connect(source_height_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.source_height);
    
    g_signal_connect(sink_x_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.sink_x);
    g_signal_connect(sink_y_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.sink_y);
    g_signal_connect(sink_width_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.sink_width);
    g_signal_connect(sink_height_adj, "value_changed", G_CALLBACK (gimp_double_adjustment_update), &myparms.sink_height);
    
    ok_to_run = (gimp_dialog_run (GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);
    
    gtk_widget_destroy (dialog);
    return ok_to_run;

}

static void run (
    const char       *name,
    int               nparams,
    const GimpParam  *param,
    int              *nreturn_vals,
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
    
    switch(run_mode) {
        case GIMP_RUN_INTERACTIVE:
            gimp_get_data(PLUGIN_NAME, &myparms);
            if(!region_dialog(drawable))
                return;
            break;
        case GIMP_RUN_NONINTERACTIVE:
            if(nparams != 11)
                values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
            else {
                myparms.source_x = param[3].data.d_float;
                myparms.source_y = param[4].data.d_float;
                myparms.source_width = param[5].data.d_float;
                myparms.source_height = param[6].data.d_float;
                myparms.sink_x = param[3].data.d_float;
                myparms.sink_y = param[4].data.d_float;
                myparms.sink_width = param[5].data.d_float;
                myparms.sink_height = param[6].data.d_float;
            }
            break;
        case GIMP_RUN_WITH_LAST_VALS:
            gimp_get_data (PLUGIN_NAME, &myparms);
            break;
        default:
            break;
    }
    
    flip_region(image, drawable);
    
    if(run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PLUGIN_NAME, &myparms, sizeof(myparms));
}

MAIN()
