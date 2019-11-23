#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <babl/babl.h>
#include <gegl-0.4/gegl.h>
#include <gegl-0.4/gegl-color.h>
#include <gegl-0.4/gegl-buffer-backend.h>

//#include <time.h>
#include <stdlib.h>

//DEBUG
#include <stdio.h>

//function prototypes
static void query (void);
static void run (
    const char          *name,
    int                 nparams,
    const GimpParam     *param,
    int                 *n_returns,
    GimpParam           **v_returns);
static void init (void);

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
      "rectangle count",
      "Number of rectangles to draw"
    },
    {
      GIMP_PDB_INT32,
      "seed value",
      "Seed for random number generator"
    }
  };

  gimp_install_procedure (
    "data-marshalling-example",
    "Example of how to quickly access large chunks of an image.",
    "Demonstrates marshalling large portions of an image for faster processing.",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "_Marshall Data...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register ("data-marshalling-example",
                               "<Image>/Filters/Misc"); 
}

typedef struct {
    int rectangle_count;
    int seed;
} MyParameters;

static MyParameters myparms =
{
    3,  /* rectangle count */
    1   /* random seed */
};

static gboolean marshall_dialog( int drawable ) {
    gboolean ok_to_run;
    
    GtkWidget *dialog;
    
    gimp_ui_init("data_marshalling", FALSE);
    
    dialog = gimp_dialog_new("Marshall Data", "data_marshalling",
                                NULL, 0,
                                NULL, "data-marshalling-example",
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OK, GTK_RESPONSE_OK,
                                NULL);
    
    //Add the label
    GtkWidget *count_label = gtk_label_new_with_mnemonic("_Count:");
    gtk_widget_show(count_label);
    
    //Add the count spinner
    GtkObject *count_adj = gtk_adjustment_new(myparms.rectangle_count, 0, 100, 1, 10, 0);
    GtkWidget *count_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (count_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(count_spinner), TRUE);
    gtk_widget_show(count_spinner);
    
    //grab the content, make an hbox, add label and spinner, add hbox to content
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), count_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), count_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    GtkWidget *seed_label = gtk_label_new_with_mnemonic("_Seed:");
    gtk_widget_show(seed_label);
    
    GtkObject *seed_adj = gtk_adjustment_new(myparms.seed, 0, INT_MAX, 1, 10, 0);
    GtkWidget *seed_spinner = gtk_spin_button_new(GTK_ADJUSTMENT (seed_adj), 2, 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(seed_spinner), TRUE);
    gtk_widget_show(seed_spinner);
    
    hbox = gtk_hbox_new(TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), seed_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), seed_spinner, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(content), hbox, TRUE, TRUE, 0);
    
    //Lastly, before showing our dialog, set up connections to our spin button
    g_signal_connect(count_adj, "value_changed", G_CALLBACK (gimp_int_adjustment_update), &myparms.rectangle_count);
    g_signal_connect(seed_adj, "value_changed", G_CALLBACK (gimp_int_adjustment_update), &myparms.seed);
    
    gtk_widget_show (dialog);
    
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
    //srand(time(NULL));
    
    static GimpParam  values[1];
    
    const Babl *inFormat;
    const Babl *outFormat;
    
    gint32 drawable;
    
    //Set up our return value
    *nreturn_vals = 1;
    *return_vals  = values;
    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = GIMP_PDB_SUCCESS;
    
    //Grab the drawable
    drawable = param[2].data.d_drawable;
    
    //Check to see if we're in an interactive mode
    GimpRunMode run_mode = param[0].data.d_int32;
    
    switch(run_mode){
        case GIMP_RUN_INTERACTIVE:
            printf("Interactive mode\n");
            gimp_get_data("data-marshalling-example", &myparms);
            if(!marshall_dialog(drawable))
                return;
            break;
        case GIMP_RUN_NONINTERACTIVE:
            printf("Non-Interactive mode\n");
            if(nparams != 5)
                values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
            else {
                myparms.rectangle_count = param[3].data.d_int32;
                myparms.seed = param[4].data.d_int32;
                
                if(myparms.rectangle_count < 0)
                    values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
            }
            break;
        case GIMP_RUN_WITH_LAST_VALS:
            printf("Running with last values\n");
            gimp_get_data ("data-marshalling-example", &myparms);
            printf("GIMP_RUN_WITH_LAST_VALS rect count: %d\n", myparms.rectangle_count);
            break;
        default:
            break;
    }
    srand(myparms.seed);
    printf("Rectangle Count: %d\n", myparms.rectangle_count);
    
    babl_init();
    
    //Create the format for our original data
    inFormat = babl_format("RGB u8");   //8-bit unsigned RGB
    
    //get the buffer
    GeglBuffer *buffer = gimp_drawable_get_buffer(drawable);
    
    //grab the final format, which will vary
    outFormat = gegl_buffer_get_format(buffer);
    
    for(int i = 0; i < myparms.rectangle_count; i++) {
        int width = rand() % gimp_drawable_width(drawable);
        int height = rand() % gimp_drawable_height(drawable);
        int orig_x = -width + rand() % (gimp_drawable_width(drawable) + width * 2);
        int orig_y = -height + rand() % (gimp_drawable_height(drawable) + height * 2);
        int bypp = 3;
        
        GeglRectangle rect = {orig_x, orig_y, width, height};
        
        guchar inBuffer[width * height * bypp];
        
        guchar color[3] = { rand() % 255, rand() % 255, rand() % 255 };
        for(int i = 0; i < width * height * bypp; i += bypp) {
            inBuffer[i + 0] = color[0];
            inBuffer[i + 1] = color[1];
            inBuffer[i + 2] = color[2];
        }
        
        //Ignore this?
        //gegl_buffer_set_pixel(buffer, 0, 0, gegl_buffer_get_format(buffer), 0);
        
        //set region of buffer to original data
        gegl_buffer_set(buffer,
                        &rect,
                        0,
                        outFormat,
                        inBuffer,
                        GEGL_AUTO_ROWSTRIDE);
    }
    
    //flush operations on the buffer
    gegl_buffer_flush(buffer);
    
    //TODO: This seems to fail. Figure out why, potentially remove it from the book.
    //merge shadow for the drawable, containing the changes we've made.
    //gimp_drawable_merge_shadow(drawable, TRUE);
    
    //update the relevant region of the drawable.
    //gimp_drawable_update(drawable, rect.x, rect.y, rect.width, rect.height);
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    
    //Flush displays.
    gimp_displays_flush();
    
    printf("Done. 5\n");
    
    if(run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data ("data-marshalling-example", &myparms, sizeof(myparms));
    
    //Old style, now deprecated.
    //gimp_drawable_detach(drawable);
}

MAIN()
