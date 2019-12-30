#include <libgimp/gimp.h>
#include <gtk/gtk.h>

#define PROC_NAME "vectors-demo"

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
    }
  };

  gimp_install_procedure (
    PROC_NAME,
    "Test Path Injector",
    "Create a Basic Path",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "_Basic Path...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PROC_NAME,
                               "<Image>/Filters/Misc/Path"); 
}

static void vectors_demo(gint32 image, gint32 drawable) {
    //Create new vector
    gint32 vector = gimp_vectors_new(image, "My New Vector");
    
    #define COORD_COUNT 24
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
    
    GeglBuffer *buffer = gimp_drawable_get_shadow_buffer(drawable);
    const Babl *format = babl_format("RGB u8");
    GeglSampler *sampler = gegl_buffer_sampler_new(
        buffer,
        format,
        GEGL_SAMPLER_LINEAR);

    
    
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
    
    vectors_demo(image, drawable);
}

MAIN()
