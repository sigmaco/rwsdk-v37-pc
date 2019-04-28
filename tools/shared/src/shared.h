
/*
 * \ingroup rwstool
 * \page rwstooloverview RenderWare Stream Tool Overview.
 *
 * A set of CLI tools to allow the user to manipulate a RenderWare Stream
 * binary file.
 *
 * Stream Tools contains:
 *
 *  <li> \ref rwsjoin - Multiple stream merge tool.
 *  <li> \ref rwsplit - Stream splitting tool.
 *  <li> \ref rwspitxd - Stream PI Texture dictionary tool.
 *  <li> \ref rwsedit - Stream editing tool.
 */

#ifndef TOOLS_SHARED_H
#define TOOLS_SHARED_H

#ifdef    __cplusplus
extern              "C"
{
#endif                          /* __cplusplus */

typedef struct ToolAPI ToolAPI;

typedef enum ToolOptionParamType ToolOptionParamType;

enum ToolOptionParamType
{
    ToolParamTypeBoolE,
    ToolParamTypeImplicitBoolE,  /* if the switch is present, implies var=true */
    ToolParamTypeIntE,
    ToolParamTypeRealE,
    ToolParamTypeStringE,
    ToolParamTypeString2E,
    ToolParamTypeString3E,
    ToolParamTypeString4E,
    ToolParamTypeStringArrayE, /* Constant length string array, data is ptr
                                * to ToolParamTypeStringArray            */
    ToolParamTypeSListE,     /* ptr to ptr to rwslist of ptr-to-char in
                              * command line */
    ToolParamTypeUser        /* data pts to a ToolOptionParamHandlerFn */
};

typedef struct ToolOption ToolOption;

typedef int
(ToolOptionParamHandlerFn)(ToolAPI *api, int argc, char* argv[]);

typedef void
(ToolOptionHelpFn)(ToolAPI *api, ToolOption *option);

struct ToolOption
{
    const RwChar *longName;
    const RwChar *shortName;
    const RwChar *argDefn;
    ToolOptionParamType paramType;
    void *data;
    const RwChar *helpText;
    ToolOptionHelpFn *helpFn;
};

typedef struct ToolParamTypeStringArray ToolParamTypeStringArray;

struct ToolParamTypeStringArray
{
    RwUInt32 size;
    RwChar **strings;
};

typedef RwInt32 (ToolUserStartupFn)(ToolAPI *api, RwChar *currentPath);
typedef RwInt32 (ToolUserAfterOptionsFn)(ToolAPI *api);
typedef RwInt32 (ToolUserProcessFileFn)(ToolAPI *api, RwChar *filename);
typedef RwInt32 (ToolUserShutdownFn)(ToolAPI *api);
typedef RwInt32 (ToolUserPluginAttachFn)(ToolAPI *api);

typedef RwInt32 (ToolStartupFn)(ToolAPI *api, RwChar *currentPath);
typedef RwInt32 (ToolProcessOptionFn)(ToolAPI *api, int argc, char *argv[]);
typedef RwInt32 (ToolAfterOptionsFn)(ToolAPI *api);
typedef RwInt32 (ToolShutdownFn)(ToolAPI *api);
typedef RwInt32 (ToolProcessFileFn)(ToolAPI *api, RwChar *filename);
            /* 0=success; non-zero will stop processing files and pass */
            /* retVal to calling process */
typedef void (ToolHelpFn)(ToolAPI *api);
typedef void (ToolReportFn)(ToolAPI *api, const RwChar *message);
typedef void (ToolLogFn)(ToolAPI *api, const RwChar *message);
typedef void (ToolWarningFn)(ToolAPI *api, const RwChar *message);
typedef void (ToolErrorFn)(ToolAPI *api, const RwChar *message);
typedef void (ToolProgressFn)(ToolAPI *api, RwReal totalProgressPercent);

struct ToolAPI
{
    const RwChar *appName;
    RwChar *copyrightMessage;
    const RwChar *appHelp;

    ToolOption *options;
    RwUInt32 numOptions;

    /* Callbacks for user to override without needing to chain original */
    ToolUserStartupFn *userStartupFn;
    ToolUserAfterOptionsFn *userAfterOptionsFn;
    ToolUserProcessFileFn *userProcessFileFn;
    ToolUserShutdownFn *userShutdownFn;
    ToolUserPluginAttachFn *userPluginAttachFn;

    /* Outer-level functions used by ToolAPIExecute */
    ToolStartupFn *startupFn;
    ToolProcessOptionFn *processOptionFn;
    ToolAfterOptionsFn *afterOptionsFn;
    ToolProcessFileFn *processFileFn;
    ToolShutdownFn *shutdownFn;

    /* Callbacks for relaying progress / events */
    ToolHelpFn *helpFn;
    ToolReportFn *reportFn;
    ToolLogFn *logFn;
    ToolWarningFn *warningFn;
    ToolErrorFn *errorFn;
    ToolProgressFn *progressFn;

    /* Internal progress tracking */
    RwReal progress;
    RwReal reportedProgress;
    RwUInt32 nFiles;

};

extern ToolStartupFn DefaultToolAPIStartupFn;
extern ToolProcessOptionFn DefaultToolAPIProcessOptionFn;
extern ToolAfterOptionsFn DefaultToolAPIAfterOptionsFn;
extern ToolProcessFileFn DefaultToolAPIProcessFileFn;
extern ToolShutdownFn DefaultToolAPIShutdownFn;
extern ToolHelpFn DefaultToolAPIHelpFn;
extern ToolLogFn DefaultToolAPIReportFn;
extern ToolLogFn DefaultToolAPILogFn;
extern ToolWarningFn DefaultToolAPIWarningFn;
extern ToolErrorFn DefaultToolAPIErrorFn;
extern ToolProgressFn DefaultToolAPIProgressFn;

extern ToolAPI DefaultToolAPI;

/* Exposed API */

extern RwInt32
ToolAPIInitAPI(ToolAPI *api, const RwChar *appName,
               const RwChar *appHelp,
               ToolOption *options, RwUInt32 numOptions);

extern RwInt32
ToolAPIShutdownAPI(ToolAPI *api);

/* Main access point to tools functions */
extern RwInt32
ToolAPIExecute(ToolAPI *api, int argc, char* argv[]);

/*
 * Reporting functions
 * Call these inside of ToolProcessFileFn etc to pass strings back to caller
 */

/* Report a general output message */
extern void
ToolAPIReport(ToolAPI *api, const RwChar *message);

/* Report an internal diagnostic message (progress etc) */
extern void
ToolAPIReportLog(ToolAPI *api, const RwChar *message);

/* Report that something's possibly gone wrong */
extern void
ToolAPIReportWarning(ToolAPI *api, const RwChar *message);

/* Report that something's definitely gone wrong, and program will fail
 * because of it */
extern void
ToolAPIReportError(ToolAPI *api, const RwChar *message);

/* Make a guess of how far the program is through execution */
extern void
ToolAPIReportProgress(ToolAPI *api, RwReal deltaProgressPercentGuess);
                /* deltaProgressPercentGuess is just a guess of progress
                 * since last call to this fn */

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* TOOLS_SHARED_H */

