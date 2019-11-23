#include <libgimp/gimp.h>
#include <gtk/gtk.h>

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
    "essential-c-plug-in",
    "Bare metal!",
    "Example of essential plug-in written in a compiled language.",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "Essential _C...",
    "*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args),
    0,
    args,
    NULL);

    gimp_plugin_menu_register ("essential-c-plug-in",
                               "<Image>/Filters/Misc"); 
}

static void run (
    const char      *name,
    int             nparams,
    const GimpParam *param,
    int             *nreturn_vals,
    GimpParam       **return_vals)
{
    static GimpParam  values[1];
    //GimpPDBStatusType status = GIMP_PDB_SUCCESS;
    GimpRunMode       run_mode;
    
    *nreturn_vals = 1;
    *return_vals  = values;

    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = GIMP_PDB_SUCCESS;
    
    g_message("Your plug-in says hello!\n");
    /* Getting run_mode - we won't display a dialog if 
     * we are in NONINTERACTIVE mode */
    /*run_mode = param[0].data.d_int32;

    if (run_mode != GIMP_RUN_NONINTERACTIVE)
      g_message("Hello, world!\n");*/
}

MAIN()
