#include <stdio.h>
#include <string.h>
#include <ctype.h>

#if (defined(RENDERWARE))
/* Need for OutputDebugString */
#include <windows.h>
#endif /* (defined(RENDERWARE)) */

#include <rwcore.h>
#include <rtfsyst.h>
#include "shared.h"

#define MAX_NB_FILES_PER_FS (5)

/* Global variables */
RwBool g_ToolOptionHelpRequired = FALSE;
RwBool g_ToolOptionVerbose = FALSE;

static ToolOption HelpOption =
     { "help", "h", "", ToolParamTypeImplicitBoolE,
        &g_ToolOptionHelpRequired,  "Print this help message", NULL };
static ToolOption VerboseOption =
     { "verbose", "v", "", ToolParamTypeImplicitBoolE,
        &g_ToolOptionVerbose,  "Print out log messages while processing", NULL };

/* Default API block */
ToolAPI DefaultToolAPI = {
    NULL,
    { "Copyright (c) 2003 Criterion Software Ltd." },
    NULL,
    NULL,
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    DefaultToolAPIStartupFn,
    DefaultToolAPIProcessOptionFn,
    DefaultToolAPIAfterOptionsFn,
    DefaultToolAPIProcessFileFn,
    DefaultToolAPIShutdownFn,
    DefaultToolAPIHelpFn,
    DefaultToolAPIReportFn,
    DefaultToolAPILogFn,
    DefaultToolAPIWarningFn,
    DefaultToolAPIErrorFn,
    DefaultToolAPIProgressFn,
    0.0f, 0.0f, 0
};

#if (defined(RENDERWARE))
static void
DebugMessageHandler(RwDebugType type __RWUNUSED__, const RwChar *str)
{
    OutputDebugString(str);
    OutputDebugString(RWSTRING("\n"));

    return;
}
#endif /* (defined(RENDERWARE)) */

RwBool
InstallFileSystem(void)
{
    RwChar      curDirBuffer[_MAX_PATH];
    RwUInt32    retValue;
    RtFileSystem *wfs, *unc;

    RwUInt32 drivesMask;
    RwInt32 drive;
    RwChar  fsName[2];
    
    /* get the current directory */
    retValue = GetCurrentDirectory(_MAX_PATH, curDirBuffer);
    if (0 == retValue || retValue > _MAX_PATH)
    {
        return FALSE;
    }

    /* Register a unc file system */
    
    /* tag a \ to the end of the current directory */
    /* only fails if the buffer size is exhausted */
    rwstrcat( curDirBuffer, "\\" );

    /** Setup the file system manager */
    if ((unc = RtWinFSystemInit(MAX_NB_FILES_PER_FS, curDirBuffer, "unc")) != NULL)
    {
        /* Now actually register the file system */
        if (RtFSManagerRegister(unc) == FALSE)
        {
            return (FALSE);
        }
    }
    else
    {
        return (FALSE);
    }
    
    /* Now register local file systems */
    
    CharUpper(curDirBuffer);

    /* 
     * loop through all logical drives and register a file system for each
     * valid one.
     * Start at 2: this removes a:
     */
    drivesMask = GetLogicalDrives();

    for( drive = 2, drivesMask >>= 1; drivesMask != 0; drive++, drivesMask >>= 1)
    {
        if (drivesMask & 0x01)
        {
            RwInt32 ret;
            RwChar  deviceName[4];

            deviceName[0] = drive + 'A' - 1;
            deviceName[1] = ':';
            deviceName[2] = '\\';   /* Needed for Win98 */
            deviceName[3] = '\0';

            ret = GetDriveType(deviceName);

            if ( ret != DRIVE_UNKNOWN && ret != DRIVE_NO_ROOT_DIR )
            {
                /* Fix device name */
                deviceName[2] = '\0';

                fsName[0] = deviceName[0];
                fsName[1] = '\0';

                /** Setup the file system manager */
                if ((wfs = RtWinFSystemInit(MAX_NB_FILES_PER_FS, deviceName, fsName)) != NULL)
                {
                    /* Now actually register the file system */
                    if (RtFSManagerRegister(wfs) == FALSE)
                    {
                        return (FALSE);
                    }
                    else
                    {
                        /* Set the unc file system as default if we have a unc path */
                        if (curDirBuffer[1] != ':')
                        {
                            RtFSManagerSetDefaultFileSystem(unc);
                        }
                        else if (deviceName[0] == curDirBuffer[0])
                        {
                            RtFSManagerSetDefaultFileSystem(wfs);
                        }
                    }
                }
                else
                {
                    return (FALSE);
                }
            }
        }
    }

    return (TRUE);
}

/* Default callbacks */
RwInt32
DefaultToolAPIStartupFn(ToolAPI *api, RwChar *currentPath)
{
#if (defined(RENDERWARE) || defined(RENDERWARE_ALLPLUGINS))
     /* Startup RenderWare */
    if (RwEngineInit(NULL, 0, (2 * 1024 * 1024)))
    {
        RwEngineOpenParams params;

        RwDebugSetHandler(DebugMessageHandler);

        /* Attach all plugins */
        if (api->userPluginAttachFn)
        {
            if ((api->userPluginAttachFn)(api))
            {
                return 255;
            }
        }

        params.displayID = NULL;

        if (RwEngineOpen(&params))
        {
            RwEngineSetSubSystem(0);

            if (RwEngineStart())
            {
                // Initialise the file system
                if (RtFSManagerOpen(RTFSMAN_UNLIMITED_NUM_FS) != FALSE)
                {
                    if (!InstallFileSystem())
                    {
                        return 0;
                    }
                }
                else
                {
                    return 0;
                }
                
                if (api->userStartupFn)
                {
                    return (api->userStartupFn)(api, currentPath);
                }
                else
                {
                    return 0;
                }
            }

            RwEngineClose();
        }

        RwEngineTerm();
    }

    return 255;
#else   /* (defined(RENDERWARE) || defined(RENDERWARE_ALLPLUGINS)) */
    return 0;
#endif  /* (defined(RENDERWARE) || defined(RENDERWARE_ALLPLUGINS)) */
}

static ToolOption *
MatchOption(ToolOption *options, RwUInt32 count, RwChar *name)
{
    if (options)
    {
        RwUInt32 i=0;
        ToolOption *option = options;
        while (i<count)
        {
            if  (    (0 == strcmp(option->shortName, &name[1]))
                  || (    ('-' == name[1])
                       && (0 == strcmp(option->longName, &name[2]))
                     )
                )
            {
                return option;
            }
            ++option;
            ++i;
        }
    }

    return (ToolOption*)NULL;;
}

RwInt32
DefaultToolAPIProcessOptionFn(ToolAPI *api, int argc, char *argv[])
{
    ToolOption *option;
    RwInt32 numRequiredParams;

    if (   ! (option = MatchOption(&HelpOption, 1, argv[0]))
        && ! (option = MatchOption(&VerboseOption, 1, argv[0]))
        && ! (option = MatchOption(api->options, api->numOptions, argv[0]))
       )
    {
        /* No option matched */
        return 0;
    }

    /* Custom handler? */
    if (ToolParamTypeUser == option->paramType)
    {
        ToolOptionParamHandlerFn *handler
          = (ToolOptionParamHandlerFn *)option->data;
        return (*handler)(api, argc, &argv[0]);
    }

    /* Validate */
    {
        switch (option->paramType)
        {
            case ToolParamTypeImplicitBoolE:
                numRequiredParams=1;
                break;
            case ToolParamTypeString2E:
                numRequiredParams=3;
                break;
            case ToolParamTypeString3E:
                numRequiredParams=4;
                break;
            case ToolParamTypeString4E:
                numRequiredParams=5;
                break;
            case ToolParamTypeStringArrayE:
                numRequiredParams = ((ToolParamTypeStringArray *)option->data)
                                ->size + 1;
                break;
            default:
                numRequiredParams=2;
        }

        if (argc<numRequiredParams)
        {
            ToolAPIReportError(api, "Not enough arguments for option");
            return 0;
        }
    }

    /* Assign */
    switch (option->paramType)
    {
        case ToolParamTypeBoolE:
            {
                char c=argv[1][0];
                switch (c)
                {
                    case '1':
                    case 't':
                    case 'T':
                    case 'y':
                    case 'Y':
                        *((RwBool *)option->data) = TRUE;
                        break;
                    case '0':
                    case 'f':
                    case 'F':
                    case 'n':
                    case 'N':
                        *((RwBool *)option->data) = FALSE;
                        break;
                    default:
                        ToolAPIReportError(api, "Unrecognized parameter value");
                        return 0;
                }
            }
            break;
        case ToolParamTypeImplicitBoolE:
            *((RwBool *)option->data) = TRUE;
            break;
        case ToolParamTypeIntE:
            *((RwInt32 *)option->data) = strtol(argv[1], NULL, 0);
            break;
        case ToolParamTypeStringE:
            {
                RwChar **data=(RwChar **)option->data;
                data[0] = argv[1];
            }
            break;
        case ToolParamTypeString2E:
            {
                RwChar **data=(RwChar **)option->data;
                data[0] = argv[1];
                data[1] = argv[2];
            }
            break;
        case ToolParamTypeString3E:
            {
                RwChar **data=(RwChar **)option->data;
                data[0] = argv[1];
                data[1] = argv[2];
                data[2] = argv[3];
            }
            break;
        case ToolParamTypeString4E:
            {
                RwChar **data=(RwChar **)option->data;
                data[0] = argv[1];
                data[1] = argv[2];
                data[2] = argv[3];
            }
            break;
        case ToolParamTypeStringArrayE:
            {
                ToolParamTypeStringArray *array
                = (ToolParamTypeStringArray *)option->data;
                array->strings = &argv[1];
            }
            break;
        case ToolParamTypeSListE:
            {
                RwSList *stringList = *((RwSList **)option->data);
                RwChar **entry = (RwChar **)_rwSListGetNewEntry(stringList, 0);
                *entry = argv[1];
            }
            break;
        case ToolParamTypeRealE:
                *((RwReal *)option->data) = (RwReal) atof(argv[1]);
            break;
        default:
            break;
    }

    return numRequiredParams;
}

RwInt32
DefaultToolAPIAfterOptionsFn(ToolAPI *api)
{
    if (g_ToolOptionHelpRequired)
    {
        (api->helpFn)(api);
        return 255;
    }
    if (api->userAfterOptionsFn)
    {
        return (api->userAfterOptionsFn)(api);
    }
    return 0;
}

RwInt32
DefaultToolAPIProcessFileFn(ToolAPI *api, RwChar *filename)
{
    ToolAPIReport(api, "Processing file ");
    ToolAPIReport(api, filename);
    ToolAPIReport(api, "\n");

    if (api->userProcessFileFn)
    {
        return (api->userProcessFileFn)(api, filename);
    }

    return 0;
}

RwInt32
DefaultToolAPIShutdownFn(ToolAPI *api)
{
    RwInt32 retVal = 0;

    if (api->userShutdownFn)
    {
        retVal = (api->userShutdownFn)(api);
    }
#if (defined(RENDERWARE) || defined(RENDERWARE_ALLPLUGINS))
    /* Shutdown RenderWare */
    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();
#endif  /* (defined(RENDERWARE) || defined(RENDERWARE_ALLPLUGINS)) */

    return retVal;
}

#define MAX_LINE 79

static void
PrintN(RwUInt32 n, RwChar c)
{
    while(n--)
    {
        putc(' ', stdout);
    };
}

static void
PrettyPrint(const RwChar *text, RwUInt32 firstLineStart,
            RwUInt32 followingLinesStart)
{
    RwUInt32 currentLineStart = firstLineStart;
    RwBool firstLine = TRUE;

    while (*text)
    {
        if (!firstLine && *text)
        {
            /* indent */
            PrintN(followingLinesStart, ' ');
        }

        /* Skip past whitespace */
        while (isspace(*text))
        {
           ++text;
        }


        {
            const RwChar *endLine = text;
            const RwChar *endNextWord = text;

            /* Drag the endline past the firstword on the line */
            while (*endLine && !isspace(*endLine) && (endLine - text + currentLineStart < MAX_LINE))
            {
                ++endLine;
            }

            endNextWord = endLine;
            /* While the end of the next word isn't past the end of the line, move
             * the endline pointer there */
            while (*endNextWord && (endNextWord - text + currentLineStart < MAX_LINE))
            {
                ++endNextWord;
                if (!*endNextWord || isspace(*endNextWord))
                {
                    endLine = endNextWord;
                }
            }

            /* We now have a full line to print */
            {
                RwChar line[MAX_LINE+1];
                strncpy(line, text, endLine-text);
                line[endLine-text]='\0';
                puts(line);
            }

            text = endLine;
            currentLineStart = followingLinesStart;

            firstLine = FALSE;
        }
    }
}

void
DefaultToolAPIOptionHelpFn(ToolAPI *api, ToolOption *option)
{
    RwChar buffer[80];
    RwChar buffer2[80];
    sprintf(buffer, "-%s%s, --%s%s", option->shortName, option->argDefn, option->longName, option->argDefn);
    sprintf(buffer2, "    %-16s : ", buffer);
    ToolAPIReport(api, buffer2);
    if (strlen(buffer2)>23)
    {
        ToolAPIReport(api, "\n                       ");
    }
    PrettyPrint(option->helpText, 23, 23);
}

void
DefaultToolAPIHelpFn(ToolAPI *api)
{
    RwUInt32 i;
    RwChar buffer[200];

    sprintf(buffer, "%s  %s\n", api->appName, api->copyrightMessage);
    ToolAPIReport(api, buffer);
    sprintf(buffer, "   ");
    ToolAPIReport(api, buffer);
    PrettyPrint(api->appHelp, 3, 3);
    sprintf(buffer, "Usage: %s [options] [files] ...\n", api->appName);
    ToolAPIReport(api, buffer);
    DefaultToolAPIOptionHelpFn(api, &HelpOption);
    DefaultToolAPIOptionHelpFn(api, &VerboseOption);
    for (i=0; i<api->numOptions; ++i)
    {
        ToolOption *option = &api->options[i];
        ToolOptionHelpFn *optionHelpFn = option->helpFn;
        if (optionHelpFn)
        {
            (option->helpFn)(api, option);
        }
        else
        {
            DefaultToolAPIOptionHelpFn(api, option);
        }
    }
}

void
DefaultToolAPIReportFn(ToolAPI *api, const RwChar *message)
{
    printf("%s", message);
}

void
DefaultToolAPILogFn(ToolAPI *api, const RwChar *message)
{
    if (g_ToolOptionVerbose)
    {
        printf("%s\n", message);
    }
}

void
DefaultToolAPIWarningFn(ToolAPI *api, const RwChar *message)
{
    fprintf(stderr, "%s\n", message);
}

void DefaultToolAPIErrorFn(ToolAPI *api, const RwChar *message)
{
    fprintf(stderr, "%s\n", message);
}

void
DefaultToolAPIProgressFn(ToolAPI *api, RwReal totalProgressPercent)
{
    RwChar buffer[50];
    sprintf(buffer, "%3.2f%% complete", totalProgressPercent);
    DefaultToolAPILogFn(api, buffer);
}

/* Exposed API */
RwInt32
ToolAPIInitAPI(ToolAPI *api, const RwChar *appName,
               const RwChar *appHelp,
               ToolOption *options, RwUInt32 numOptions)
{
    memcpy(api, &DefaultToolAPI, sizeof(ToolAPI));
    api->appName = appName;
    api->appHelp = appHelp;
    api->options = options;
    api->numOptions = numOptions;

    return 0;
}

RwInt32
ToolAPIShutdownAPI(ToolAPI *api)
{
    return 0;
}

/* Main access point to tools functions */
RwInt32
ToolAPIExecute(ToolAPI *api, int argc, char* argv[])
{
    RwInt32 programResult = 0;
    RwInt32 i = 1;
    RwInt32 afterOptionsResult = 0;

    if (0 != (api->startupFn)(api, argv[0]))
    {
        ToolAPIReportError(api, "Error initializing tool");
        return 255;
    }

    /* Initial progress */
    api->progress = 0.0f;
    api->reportedProgress = 0.0f;
    (api->progressFn)(api, api->progress);


    /* No arguments provided; user needs help */
    if (1>=argc)
    {
        g_ToolOptionHelpRequired = 1;
    }

    /* Process options */
    while (i<argc)
    {
        if ('-'==argv[i][0])
        {
            RwUInt32 argsProcessed
                = (api->processOptionFn)(api, argc-i, &argv[i]);

            if (argsProcessed)
            {
                i+=argsProcessed;
            }
            else
            {
                ToolAPIReportError(api, "Unrecognized argument");
                g_ToolOptionHelpRequired = 1;
                break;
            }

            /* Update progress guesstimate */
            api->progress = (i + 1.0f)/argc*100.0f;
            (api->progressFn)(api, api->progress);
        }
        else
        {
            /* We've hit the start of the files */
            break;
        }
    }

    /* Now know how many files there are */
    api->nFiles = argc-i;

    /* Processed the options; do post-options processing */
    if ( 0 != (afterOptionsResult = (api->afterOptionsFn)(api)))
    {
        /* an error! */
        return afterOptionsResult;
    }

    /* Process files */
    while (i<argc)
    {
        RwChar *filename = argv[i];
        RwInt32 processFileResult;

        processFileResult = (api->processFileFn)(api, filename);

        if (0!=processFileResult)
        {
            ToolAPIReportError(api, "Error processing file");
            (api->shutdownFn)(api);
            return processFileResult;
        }

        /* Update progress guesstimate */
        api->progress = (i + 1.0f)/argc * 100.0f;
        (api->progressFn)(api, api->progress);

        /* next file */
        ++i;
    }

    /* Big success! */
    return (api->shutdownFn)(api);
}

/* Call these inside of ToolProcessFileFn etc to pass strings back to caller */
void
ToolAPIReport(ToolAPI *api, const RwChar *message)
{
    (api->reportFn)(api, message);
}

void
ToolAPIReportLog(ToolAPI *api, const RwChar *message)
{
    (api->logFn)(api, message);
}

void
ToolAPIReportWarning(ToolAPI *api, const RwChar *message)
{
    (api->warningFn)(api, message);
}

void
ToolAPIReportError(ToolAPI *api, const RwChar *message)
{
    (api->errorFn)(api, message);
}

extern void
ToolAPIReportProgress(ToolAPI *api, RwReal deltaProgressPercentGuess)
                /* deltaProgressPercentGuess is just a guess of progress
                 * since last call to this fn */
{
    RwUInt32 nFiles = api->nFiles;
    if (nFiles<1)
    {
        nFiles=1;
    }

    api->reportedProgress += deltaProgressPercentGuess/nFiles;

    if (deltaProgressPercentGuess>0)
    {
        RwReal projectedDeltaProgress =
              (api->progress / api->reportedProgress)
            * (deltaProgressPercentGuess / nFiles);
        api->reportedProgress += projectedDeltaProgress;
        if (api->reportedProgress > 99.999f)
        {
            api->reportedProgress = 99.999f;
        }
    }
    (api->progressFn)(api, api->progress);
}

