#include <libgimp/gimp.h>
#include <gtk/gtk.h>

#define PROC_NAME "basic-translation"

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
    gdouble dydx;
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
    }
  };

  gimp_install_procedure (
    PROC_NAME,
    "Basic Translation",
    "Example of translation",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "Basic _Translation...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PROC_NAME,
                               "<Image>/Filters/Misc/Distorts"); 
}

static GimpPDBStatusType basic_translation(gint32 image, gint32 drawable) {
    GeglBuffer *buffer = gimp_drawable_get_buffer(drawable);
    GeglBuffer *shadow = gimp_drawable_get_shadow_buffer(drawable);
    
    printf("Buffer: %p Shadow: %p\n", buffer, shadow);
    
    gint width = gegl_buffer_get_width(buffer);
    gint height = gegl_buffer_get_height(buffer);
    
    GeglMatrix3 *mat3 = gegl_matrix3_new();
    gegl_matrix3_identity(mat3);
    mat3->coeff[0][2] = -100;
    mat3->coeff[1][2] = -200;
    double destX = 0.0, destY = 0.0;
    for(double y = 0; y < height; y++)
        for(double x = 0; x < width; x++) {
            //grab segment of buffer, times transform, splice to shadow buffer
            destX = x;
            destY = y;
            gegl_matrix3_transform_point(mat3, &destX, &destY);
            GeglRectangle srcRect = {x, y, 1, 1};
            GeglRectangle destRect = {destX, destY, 1, 1};
            gegl_buffer_copy(buffer, &srcRect, GEGL_ABYSS_NONE, shadow, &destRect);
        }
    g_object_unref(shadow);
    g_object_unref(buffer);
    printf("%s\n", gegl_matrix3_to_string(mat3));
    gimp_drawable_merge_shadow(drawable, TRUE);
    
    return GIMP_PDB_SUCCESS;
}

gboolean distort_dialog() {
    return TRUE;
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
    gint32 image = param[1].data.d_image;
    gint32 drawable = param[2].data.d_drawable;
    
    *nreturn_vals = 1;
    *return_vals  = values;

    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = GIMP_PDB_SUCCESS;
    
        switch(run_mode){
        case GIMP_RUN_INTERACTIVE:
            gimp_get_data(PROC_NAME, &myparms);
            if(!distort_dialog(drawable))
                return;
            break;
        case GIMP_RUN_NONINTERACTIVE:
            if(nparams != 4)
                values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
            else
                myparms.dydx = param[3].data.d_int32;
            break;
        case GIMP_RUN_WITH_LAST_VALS:
            gimp_get_data (PROC_NAME, &myparms);
            break;
        default:
            break;
    }
    
    gegl_init(NULL, NULL);
    babl_init();
    
    values[0].data.d_status = basic_translation(image, drawable);
    
    gegl_exit();
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    gimp_displays_flush();

    if(run_mode == GIMP_RUN_INTERACTIVE)
        gimp_set_data (PROC_NAME, &myparms, sizeof(myparms));
}

MAIN()
