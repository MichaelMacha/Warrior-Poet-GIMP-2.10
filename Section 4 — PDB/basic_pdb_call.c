#include <libgimp/gimp.h>
#include <gtk/gtk.h>

#define PROC_NAME "basic-pdb-call"

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
    "Demo of plug-in calls",
    "Example of calling another registered plug-in internally.",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "PDB _Call...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register (PROC_NAME,
                               "<Image>/Filters/Misc/PDB"); 
}

static GimpPDBStatusType pdb_call(GimpRunMode run_mode, gint32 image, gint32 drawable) {
    gint n_params;
    GimpParam *return_vals;
    
    GimpPDBStatusType success = GIMP_PDB_SUCCESS;
    char *plugin = "script-fu-waves-anima";
    return_vals = gimp_run_procedure(
        plugin,
        &n_params,
        GIMP_PDB_INT32, GIMP_RUN_NONINTERACTIVE,
        GIMP_PDB_IMAGE, image,
        GIMP_PDB_DRAWABLE, drawable,
        GIMP_PDB_FLOAT, 20.0,
        GIMP_PDB_FLOAT, 20.0,
        GIMP_PDB_FLOAT, 3.0,
        GIMP_PDB_INT32, TRUE,
        GIMP_PDB_END);
    
    if(return_vals[0].data.d_int32 != GIMP_PDB_SUCCESS) {
        //g_message("%s failed on call\n", plugin);
        success = GIMP_PDB_CALLING_ERROR;
    }
    //g_message("%d\n", n_params);
    //g_message("%d\n", return_vals[0].data.d_int32);
    
    gimp_destroy_params(return_vals, n_params);
    
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
    
    values[0].data.d_status = pdb_call(run_mode, image, drawable);
    
    gimp_drawable_update(drawable, 0, 0, gimp_drawable_width(drawable), gimp_drawable_height(drawable));
    gimp_displays_flush();
}

MAIN()
