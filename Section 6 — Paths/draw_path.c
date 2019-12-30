#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <gtk/gtk.h>
#include <gegl-0.4/gegl.h>
#include <math.h>

#define PROC_NAME "draw-path"

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

typedef struct {
    gint32 path;
} MyParameters;

static MyParameters myparms =
{
    0
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
      GIMP_PDB_VECTORS,
      "path",
      "Path to trace"
    }
  };

  gimp_install_procedure (
    PROC_NAME,
    "Draw Path",
    "Draw a Basic Path",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "_Draw Path...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PROC_NAME,
                               "<Image>/Filters/Misc/Path"); 
}

static void update_path_to_value (GtkWidget  *widget,
                              gpointer *data) {
    gimp_int_combo_box_get_active(GIMP_INT_COMBO_BOX(widget), &myparms.path);
}

static gboolean draw_path_dialog(gint32 drawable) {
    gboolean ok_to_run;
    
    GtkWidget *dialog;
    
    gimp_ui_init("draw_path", FALSE);
    
    dialog = gimp_dialog_new("Draw a Path", "draw_path",
                                NULL, 0,
                                NULL, PROC_NAME,
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OK, GTK_RESPONSE_OK,
                                NULL);    
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    GtkWidget *path_dropdown = gimp_vectors_combo_box_new (NULL, NULL);
    gtk_widget_show(path_dropdown);
    
    gtk_box_pack_start(GTK_BOX(content), path_dropdown, TRUE, TRUE, 0);
    
    gimp_int_combo_box_connect(GIMP_INT_COMBO_BOX(path_dropdown), myparms.path, G_CALLBACK(update_path_to_value), NULL);
    
    ok_to_run = (gimp_dialog_run (GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);
    
    gtk_widget_destroy (dialog);
    
    return ok_to_run;
}

static GimpPDBStatusType vectors_demo(gint32 image, gint32 drawable) {
    GimpPDBStatusType success = GIMP_PDB_SUCCESS;
    
    GeglBuffer *buffer = gimp_drawable_get_buffer(drawable);
    GeglBuffer *shadow = gimp_drawable_get_shadow_buffer(drawable);
    
    //left commented, for the sake of aiding clarity
    /*
    gegl_buffer_copy(buffer, gegl_buffer_get_extent(buffer), GEGL_ABYSS_NONE, shadow, gegl_buffer_get_extent(shadow));

    //Create new vector
    gint32 vector = gimp_vectors_new(image, "My New Vector");
        
    /*#define COORD_COUNT 24
    double points[COORD_COUNT] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 100.0, 0.0, 100.0, 0.0, 100.0, 
        100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 
        100.0, 0.0, 100.0, 0.0, 100.0, 0.0};
    gint32 stroke = gimp_vectors_stroke_new_from_points(
        vector,
        GIMP_VECTORS_STROKE_TYPE_BEZIER,
        COORD_COUNT,
        points,
        TRUE);
    #undef COORD_COUNT
    
    gimp_image_insert_vectors(
        image,
        vector,
        0,
        -1);
    
    gdouble x, y, slope;
    gboolean valid = TRUE;
    /*for(int i = 0; i < 3; i++) {
        gimp_vectors_stroke_get_point_at_dist(
            vector, 
            stroke,
            110.0 * i,
            0.1,
            &x,
            &y,
            &slope,
            &valid);
        g_message("%f, %f, %f, %s\n", x, y, slope, valid ? "Valid" : "Invalid");
    }*/
    
    gimp_context_push();
    {
        gimp_context_set_gradient("Full saturation spectrum CCW");
        
        gint stroke_count;
        gint *strokes = gimp_vectors_get_strokes(myparms.path,
            &stroke_count);
        for(int i = 0; i < stroke_count; i++) {
            int n_points;
            gdouble *points;
            gboolean closed;
            
            double *curve_points;
            int n_curve_points;
            
            curve_points = gimp_vectors_stroke_interpolate(myparms.path, strokes[i], 1.0, &n_curve_points, &closed);
            
            gimp_paintbrush(
                drawable,
                1000.0,
                n_curve_points,
                curve_points,
                GIMP_PAINT_CONSTANT,
                100.0);

        }
        
        gegl_buffer_flush(shadow);
        
        g_object_unref(shadow);
        g_object_unref(buffer);
    }
    gimp_context_pop();
    
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
            if(!draw_path_dialog(drawable))
                return;
            break;
        case GIMP_RUN_NONINTERACTIVE:
            if(nparams != 4)
                values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
            else
                myparms.path = param[3].data.d_int32;
            break;
        case GIMP_RUN_WITH_LAST_VALS:
            gimp_get_data (PROC_NAME, &myparms);
            break;
        default:
            break;
    }
    
    gegl_init(NULL, NULL);
    babl_init();
    
    values[0].data.d_status = vectors_demo(image, drawable);
    
    gegl_exit();
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    gimp_displays_flush();

    if(run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PROC_NAME, &myparms, sizeof(myparms));
}

MAIN()
