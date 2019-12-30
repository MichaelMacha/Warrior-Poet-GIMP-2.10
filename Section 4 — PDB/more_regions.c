#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <gtk/gtk.h>

#define PROC_NAME "essential-c-plug-in"

//function prototypes
static void query (void);
static void run (
    const char          *name,
    int                 nparams,
    const GimpParam     *param,
    int                 *n_returns,
    GimpParam           **v_returns);

typedef struct {
    int seed;
    int count;
} MyParameters;

static MyParameters myparms =
{
    1,
    100
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
      "seed",
      "Randomization seed"
    },
    {
      GIMP_PDB_INT32,
      "count",
      "Displacement count"
    }
  };

  gimp_install_procedure (
    PROC_NAME,
    "Bare metal!",
    "Example of essential plug-in written in a compiled language.",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "More _Regions...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PROC_NAME,
                               "<Image>/Filters/Misc/PDB"); 
}

gboolean more_regions_dialog(gint32 drawable) {
    gboolean ok_to_run;
    
    GtkWidget *dialog;
    
    gimp_ui_init(PROC_NAME, FALSE);
    
    dialog = gimp_dialog_new("More Regions", "more_regions",
                                NULL, 0,
                                NULL, PROC_NAME,
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
    
    GtkWidget *count_label = gtk_label_new_with_mnemonic("Displacement _Count:");
    gtk_widget_show(count_label);
    
    GtkObject *count_adj = gtk_adjustment_new(myparms.count, G_MININT32, G_MAXINT32, 1, 10, 0);
    GtkWidget *count_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (count_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(count_spinner), TRUE);
    gtk_widget_show(count_spinner);
    
    hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), count_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), count_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    
    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    gtk_widget_show (dialog);
    
    g_signal_connect(seed_adj, "value_changed", G_CALLBACK (gimp_int_adjustment_update), &myparms.seed);
    g_signal_connect(count_adj, "value_changed", G_CALLBACK (gimp_int_adjustment_update), &myparms.count);
    
    ok_to_run = (gimp_dialog_run (GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);
    
    gtk_widget_destroy (dialog);
    return ok_to_run;
}

static double uniform(double maximum) {
    return (rand() / (RAND_MAX + 1.0)) * maximum;
}

static GimpPDBStatusType more_regions(GimpRunMode run_mode, gint32 image, gint32 drawable) {
    srand(myparms.seed);
    
    gint n_params;
    GimpParam *return_vals;
    GimpPDBStatusType success = GIMP_PDB_SUCCESS;
    char *plugin = "region-operations-example";
    
    double width = (double)gimp_drawable_width(drawable);
    double height = (double)gimp_drawable_height(drawable);
    
    gboolean is_thawed = gimp_image_undo_is_enabled(image);
    gimp_drawable_merge_shadow(drawable, TRUE);
    
    if(is_thawed) {
        gimp_image_undo_group_start(image);
        gimp_image_undo_freeze(image);
    }
    
    for(int i = 0; i < myparms.count; i++) {
        gimp_progress_set_text_printf("Displacing region %d of %d...", i, myparms.count);
        gimp_progress_update((double)i/myparms.count);
        
        double source_x = uniform(width);
        double source_y = uniform(height);
        double source_width = uniform(width - source_x);
        double source_height = uniform(height - source_y);
        
        double sink_x = uniform(width);
        double sink_y = uniform(height);
        double sink_width = uniform(width - sink_x);
        double sink_height = uniform(height - sink_y);
        
        return_vals = gimp_run_procedure(
            plugin,
            &n_params,
            GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
            GIMP_PDB_IMAGE, image,
            GIMP_PDB_DRAWABLE, drawable,
            GIMP_PDB_FLOAT, source_x,
            GIMP_PDB_FLOAT, source_y,
            GIMP_PDB_FLOAT, source_width,
            GIMP_PDB_FLOAT, source_height,
            GIMP_PDB_FLOAT, sink_x,
            GIMP_PDB_FLOAT, sink_y,
            GIMP_PDB_FLOAT, sink_width,
            GIMP_PDB_FLOAT, sink_height,
            GIMP_PDB_END);
        
        if(return_vals[0].data.d_int32 != GIMP_PDB_SUCCESS) {
            success = GIMP_PDB_CALLING_ERROR;
            break;
        }
        
        gimp_destroy_params(return_vals, n_params);
    }
    
    if(is_thawed) {
        gimp_image_undo_thaw(image);
        gimp_image_undo_group_end(image);
    }
    //gimp_drawable_merge_shadow(drawable, TRUE);
    
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
    
    *nreturn_vals = 1;
    *return_vals  = values;

    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = GIMP_PDB_SUCCESS;
    
    GimpRunMode run_mode = param[0].data.d_int32;
    gint32 image = param[1].data.d_image;
    gint32 drawable = param[2].data.d_drawable;
    
    switch(run_mode) {
    case GIMP_RUN_INTERACTIVE:
        gimp_get_data(PROC_NAME, &myparms);
        if(!more_regions_dialog(drawable))
            return;
        break;
    case GIMP_RUN_NONINTERACTIVE:
            if(nparams != 5) {
                values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
            } else {
                myparms.seed = param[3].data.d_int32;
                myparms.count = param[4].data.d_int32;
            }
        break;
    case GIMP_RUN_WITH_LAST_VALS:
            gimp_get_data (PROC_NAME, &myparms);
        break;
    default:
        break;
    }
    
    values[0].data.d_status = more_regions(run_mode, image, drawable);
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    gimp_displays_flush();
    
    if(run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PROC_NAME, &myparms, sizeof(myparms));

}

MAIN()
