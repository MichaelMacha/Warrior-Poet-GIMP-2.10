#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <gtk/gtk.h>

#define PLUGIN_NAME "tiles-example"

//function prototypes
static void query (void);
static void run (
    const char          *name,
    int                 nparams,
    const GimpParam     *param,
    int                 *n_returns,
    GimpParam           **v_returns);


//Navigation structure so GIMP knows where to look
GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,    //initialization plug-in
  NULL,    //quitting plug-in
  query,   //plug-in query function
  run,     //and, where the actual work is done!
};

//Parameters
typedef struct {
    int seed;
} MyParameters;

static MyParameters myparms =
{
    1
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
      "seed",
      "Randomization seed for color choice"
    }
  };

  gimp_install_procedure (
    PLUGIN_NAME,
    "Randomly Color Image Tiles",
    "Select a psuedorandom color for each tile of the image, and flood-fill it.",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "Scatter Tile Colors...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PLUGIN_NAME,
                               "<Image>/Filters/Misc/Tiles"); 
}

/*static void color_tiles(gint32 image, gint32 drawable) {
    gegl_init(NULL, NULL);
    babl_init();
    
    GeglBuffer *buffer = gimp_drawable_get_buffer(drawable);
    //const Babl *format = gegl_buffer_get_format(buffer);
    const Babl *parse_format = babl_format("RGB u8");
    GeglRectangle rect = {0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable)};
    
    GeglBufferIterator *gi = gegl_buffer_iterator_new(
        buffer,
        &rect,
        0,                  //mip-map level; don't worry about it for now.
        parse_format,
        GEGL_ACCESS_READWRITE,
        GEGL_ABYSS_NONE,    //corner case presumptions
        1);                 //tile slots
    
    while(gegl_buffer_iterator_next(gi)) {
        GeglRectangle roi = gi->items[0].roi;
        int size = roi.width * roi.height * 3;
        guchar *pixel = (guchar *)gi->items[0].data;
        guchar color[3] = { rand() % 255, rand() % 255, rand() % 255 };
        for(int i = 0; i < size; i += 3) {
            for(int j = 0; j < 3; j++) {
                pixel[i + j] = color[j];
            }
        }
    }
    
    g_object_unref(buffer);
    
    gegl_exit();
}*/

static void shadow_tiles(gint32 image, gint32 drawable) {
    gegl_init(NULL, NULL);
    babl_init();
    
    GeglBuffer *buffer = gimp_drawable_get_shadow_buffer(drawable);
    //const Babl *format = gegl_buffer_get_format(buffer);
    const Babl *parse_format = babl_format("RGB u8");
    GeglRectangle rect = {0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable)};
    
    GeglBufferIterator *gi = gegl_buffer_iterator_new(
        buffer,
        &rect,
        0,                  //mip-map level; don't worry about it for now.
        parse_format,
        GEGL_ACCESS_READWRITE,
        GEGL_ABYSS_NONE,    //corner case presumptions
        1);                 //tile slots
    
    while(gegl_buffer_iterator_next(gi)) {
        GeglRectangle roi = gi->items[0].roi;
        int size = roi.width * roi.height * 3;
        guchar *pixel = (guchar *)gi->items[0].data;
        guchar color[3] = { rand() % 255, rand() % 255, rand() % 255 };
        for(int i = 0; i < size; i += 3) {
            for(int j = 0; j < 3; j++) {
                pixel[i + j] = color[j];
            }
        }
    }
    
    gegl_buffer_flush(buffer);
    gimp_drawable_merge_shadow(drawable, TRUE);
    g_object_unref(buffer);
    
    gegl_exit();
}

gboolean color_tiles_dialog(gint32 drawable) {
    gboolean ok_to_run;
    
    GtkWidget *dialog;
    
    gimp_ui_init("tiles", FALSE);
    
    dialog = gimp_dialog_new("Color Tiles", "tiles",
                                NULL, 0,
                                NULL, PLUGIN_NAME,
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OK, GTK_RESPONSE_OK,
                                NULL);    
    
    GtkWidget *seed_label = gtk_label_new_with_mnemonic("_Random Seed:");
    gtk_widget_show(seed_label);
    
    GtkObject *seed_adj = gtk_adjustment_new(myparms.seed, G_MININT32, G_MAXINT32, 1, 10, 0);
    GtkWidget *seed_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (seed_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(seed_spinner), TRUE);
    gtk_widget_show(seed_spinner);
    
    GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), seed_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), seed_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    gtk_widget_show (dialog);
    
    g_signal_connect(seed_adj, "value_changed", G_CALLBACK (gimp_int_adjustment_update), &myparms.seed);
    
    ok_to_run = (gimp_dialog_run (GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);
    
    gtk_widget_destroy (dialog);
    return ok_to_run;
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
    
    //Grab parameters
    run_mode = param[0].data.d_int32;    
    gint32 image = param[1].data.d_image;
    gint32 drawable = param[2].data.d_drawable;
    
    switch(run_mode){
        case GIMP_RUN_INTERACTIVE:
            gimp_get_data(PLUGIN_NAME, &myparms);
            if(!color_tiles_dialog(drawable))
                return;
            break;
        case GIMP_RUN_NONINTERACTIVE:
            if(nparams != 4)
                values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
            else
                myparms.seed = param[3].data.d_int32;
            break;
        case GIMP_RUN_WITH_LAST_VALS:
            gimp_get_data (PLUGIN_NAME, &myparms);
            break;
        default:
            break;
    }
    
    srand(myparms.seed);
    
    //color_tiles(image, drawable);
    shadow_tiles(image, drawable);
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    
    gimp_displays_flush();
    
    if(run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PLUGIN_NAME, &myparms, sizeof(myparms));

}

MAIN()
