
#include <stdio.h>

#include <rwcore.h>
#include "shared.h"

RwSList *g_AddFilesList = NULL;
RwChar  *g_Buffer = NULL;
RwUInt32 g_BufferSize = 0;

ToolOption ToolOptions[] = {
    { "add", "a", " <file>", ToolParamTypeSListE, &g_AddFilesList, "Add file", NULL }
};

RwUInt32 NumToolOptions = sizeof(ToolOptions) / sizeof(ToolOption);

RwChar          JoinFileMsg[80];

/*
 *
 */

static RwInt32
JoinFile(ToolAPI *api, RwStream *outStream, RwChar *inFilename, RwChar *outFilename)
{
    RwStream        *inStream;
    RwInt32         err;

    err = 0;

    inStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, inFilename);

    if (inStream)
    {
        RwChunkHeaderInfo       chunkInfo;

        while ((RwStreamReadChunkHeaderInfo(inStream, &chunkInfo)) && (!err))
        {
            /* Check if we need to resize the buffer. */
            if (chunkInfo.length > g_BufferSize)
            {
                RwChar  *tmpBuf;

                if (g_Buffer)
                {
                    tmpBuf = RwRealloc(g_Buffer, chunkInfo.length, 0);
                }
                else
                {
                    tmpBuf = RwMalloc(chunkInfo.length, 0);
                }

                if (tmpBuf != NULL)
                {
                    g_Buffer = tmpBuf;
                    g_BufferSize = chunkInfo.length;
                }
                else
                {
                    /* Failed to grow the buffer. Stop processing and
                     * raise an error.
                     */
                    sprintf(JoinFileMsg, "File <%s> : Failed to create buffer.", outFilename);

                    ToolAPIReportError(api, JoinFileMsg);

                    err = 1;
                }
            }

            if (!err)
            {
                /* Read into a buffer. */
                if (RwStreamRead(inStream, g_Buffer, chunkInfo.length) == chunkInfo.length)
                {
                    /* Write out the header and data. */
                    _rwStreamWriteVersionedChunkHeader(outStream,
                        chunkInfo.type, chunkInfo.length, chunkInfo.version, chunkInfo.buildNum);

                    RwStreamWrite(outStream, g_Buffer, chunkInfo.length);
                }
                else
                {
                    /* Error during reading. Stop and exit. */
                    sprintf(JoinFileMsg, "File <%s> : Error during reading file <%s>.", outFilename, inFilename);

                    ToolAPIReportError(api, JoinFileMsg);

                    err = 1;
                }
            }
        }

        RwStreamClose(inStream, NULL);

        /* Log the file as processed. */
        sprintf(JoinFileMsg, "File <%s> : File <%s> appended.\n", outFilename, inFilename);

        ToolAPIReport(api, JoinFileMsg);
    }
    else
    {
        /* Failed to open the file. Raise a warning but continue on to the next file. */
        sprintf(JoinFileMsg, "File <%s> : File <%s> not found.", outFilename, inFilename);

        ToolAPIReportWarning(api, JoinFileMsg);
    }

    return (err);
}

/*
 *
 */

static RwInt32
JoinFileProcessFile(ToolAPI *api, RwChar *outFilename)
{
    RwStream        *stream;
    RwInt32         err;
    RwChar          *inFilename;

    /* Attempt to open the output stream. */
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, outFilename);

    if (stream)
    {
        RwInt32         i, num;

        /* Query the global file list for number of files to append. */
        num = _rwSListGetNumEntries(g_AddFilesList);

        i = 0;
        err = 0;

        while ((i < num) && (!err))
        {
            /* Grab the filename and append it to the output stream. */
            inFilename = *(RwChar **) _rwSListGetEntry(g_AddFilesList, i);

            err = JoinFile(api, stream, inFilename, outFilename);

            i++;
        }

        RwStreamClose(stream, NULL);
    }
    else
    {
        /* Failed to create output file. */
        sprintf(JoinFileMsg, "File <%s> : File failed to open.", outFilename);

        ToolAPIReportError(api, JoinFileMsg);
    }

    return (err);
}

/*
 *
 */

static RwInt32
JoinFileStartup(ToolAPI *api __RWUNUSED__, RwChar *currentPath __RWUNUSED__)
{
    g_AddFilesList = rwSListCreate(sizeof(char *), 0);
    if (g_AddFilesList == NULL)
        return (1);

    g_Buffer = NULL;
    g_BufferSize = 0;


    return 0;
}

static RwInt32
JoinFileShutdown(ToolAPI *api __RWUNUSED__)
{
    if (g_AddFilesList)
        rwSListDestroy(g_AddFilesList);
    g_AddFilesList = NULL;

    if (g_Buffer)
        RwFree(g_Buffer);

    g_Buffer = NULL;
    g_BufferSize = 0;

    return 0;
}

/*
 *
 */

int main(int argc, char* argv[])
{
    ToolAPI api;
    RwInt32 err;

    ToolAPIInitAPI(&api, "File Append", "Tool to join two or more files",
                   ToolOptions, NumToolOptions);

    api.userStartupFn = JoinFileStartup;
    api.userProcessFileFn = JoinFileProcessFile;
    api.userShutdownFn = JoinFileShutdown;

    err = ToolAPIExecute(&api, argc, argv);

    ToolAPIShutdownAPI(&api);

    return (err);
}

