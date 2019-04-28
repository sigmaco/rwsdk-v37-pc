
#include <stdio.h>
#include <string.h>

#include <rwcore.h>
#include "shared.h"


RwChar          SplitFileMsg[80];

typedef struct SplitFileGlobals SplitFileGlobals;

struct SplitFileGlobals
{
    RwInt32     clumpCount,
                atomicCount,
                worldCount,
                textureCount,
                textnativeCount,
                hanimCount,
                dmorphCount,
                txdCount,
                pitxdCount;
};

SplitFileGlobals SplitFileVars;

/*
 *
 */
static RwInt32
SplitFileProcessFile( ToolAPI *api, RwChar *inFilename )
{
    RwStream        *inStream, *outStream;
    RwBool          err, write;
    RwChar          outFilename[80], *buffer, *name;
    RwUInt32        size;

    err = 0;

    SplitFileVars.clumpCount = 0;
    SplitFileVars.atomicCount = 0;
    SplitFileVars.worldCount = 0;
    SplitFileVars.textureCount = 0;
    SplitFileVars.textnativeCount = 0;
    SplitFileVars.hanimCount = 0;
    SplitFileVars.dmorphCount = 0;
    SplitFileVars.txdCount = 0;
    SplitFileVars.pitxdCount = 0;

    buffer = NULL;
    size = 0;

    inStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, inFilename);

    if (inStream)
    {
        RwChunkHeaderInfo       chunkInfo;

        /* Strip the extension from the name. */
        name = strrchr(inFilename, '.');
        *name = 0;

        while ((RwStreamReadChunkHeaderInfo(inStream, &chunkInfo)) && (!err))
        {
            write = TRUE;

            /* Switch between different types of chunks */
            switch (chunkInfo.type)
            {
                case rwID_CLUMP:
                {
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : Found clump.\n", inFilename);
                    ToolAPIReport(api, SplitFileMsg);
                    *name = 0;

                    rwsprintf(outFilename, "%s_clump%02d.dff", inFilename, SplitFileVars.clumpCount);
                    SplitFileVars.clumpCount++;
                    break;
                }

                case rwID_ATOMIC:
                {
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : Found atomic.\n", inFilename);
                    ToolAPIReport(api, SplitFileMsg);
                    *name = 0;

                    rwsprintf(outFilename, "%s_atomic%02d.rws", inFilename, SplitFileVars.atomicCount);
                    SplitFileVars.atomicCount++;
                    break;
                }

                case rwID_WORLD:
                {
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : Found world.\n", inFilename);
                    ToolAPIReport(api, SplitFileMsg);
                    *name = 0;

                    rwsprintf(outFilename, "%s_world%02d.bsp", inFilename, SplitFileVars.worldCount);
                    SplitFileVars.worldCount++;
                    break;
                }

                case rwID_HANIMANIMATION:
                {
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : Found hanim.\n", inFilename);
                    ToolAPIReport(api, SplitFileMsg);
                    *name = 0;

                    rwsprintf(outFilename, "%s_hanim%02d.anm", inFilename, SplitFileVars.hanimCount);
                    SplitFileVars.hanimCount++;
                    break;
                }

                case rwID_DMORPHANIMATION:
                {
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : Found dmorph animation.\n", inFilename);
                    ToolAPIReport(api, SplitFileMsg);
                    *name = 0;

                    rwsprintf(outFilename, "%s_dmorph%02d.dma", inFilename, SplitFileVars.dmorphCount);
                    SplitFileVars.dmorphCount++;
                    break;
                }

                case rwID_TEXTURENATIVE:
                {
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : Found native texture.\n", inFilename);
                    ToolAPIReport(api, SplitFileMsg);
                    *name = 0;

                    rwsprintf(outFilename, "%s_textnative%02d.rws", inFilename, SplitFileVars.textnativeCount);
                    SplitFileVars.textnativeCount++;
                    break;
                }

                case rwID_TEXTURE:
                {
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : Found texture.\n", inFilename);
                    ToolAPIReport(api, SplitFileMsg);
                    *name = 0;

                    rwsprintf(outFilename, "%s_text%02d.rws", inFilename, SplitFileVars.textureCount);
                    SplitFileVars.textureCount++;
                    break;
                }

                case rwID_TEXDICTIONARY:
                {
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : Found texture dictionary.\n", inFilename);
                    ToolAPIReport(api, SplitFileMsg);
                    *name = 0;

                    rwsprintf(outFilename, "%s_txd%02d.txd", inFilename, SplitFileVars.txdCount);
                    SplitFileVars.txdCount++;
                    break;
                }

                case rwID_PITEXDICTIONARY:
                {
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : Found PI texture dictionary.\n", inFilename);
                    ToolAPIReport(api, SplitFileMsg);
                    *name = 0;

                    rwsprintf(outFilename, "%s_pitxd%02d.pitd", inFilename, SplitFileVars.pitxdCount);
                    SplitFileVars.pitxdCount++;
                    break;
                }

                default:
                {
                    write = FALSE;
                    break;
                }
            }

            /* Write out to the appropiate output stream. */
            if (write)
            {
                outStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, outFilename);

                if (outStream == NULL)
                {
                    /* Failed to open a stream. Flag an error and exit. */
                    *name = '.';
                    sprintf(SplitFileMsg, "File <%s> : File <%s> failed to open for writing.", inFilename, outFilename);
                    *name = 0;

                    ToolAPIReportError(api, SplitFileMsg);

                    err = 1;
                }
                else
                {
                    /* Check if we need to resize the buffer. */
                    if (chunkInfo.length > size)
                    {
                        RwChar  *tmpBuf;

                        if (buffer != NULL)
                        {
                            tmpBuf = RwRealloc(buffer, chunkInfo.length, 0);
                        }
                        else
                        {
                            tmpBuf = RwMalloc(chunkInfo.length, 0);
                        }

                        if (tmpBuf != NULL)
                        {
                            buffer = tmpBuf;
                            size = chunkInfo.length;
                        }
                        else
                        {
                            /* Failed to grow the buffer. Stop processing and
                            * raise an error.
                            */
                            *name = '.';
                            sprintf(SplitFileMsg, "File <%s> : Failed to create input buffers.", inFilename);
                            *name = 0;

                            ToolAPIReportError(api, SplitFileMsg);

                            err = 1;
                        }
                    }

                    /* Check if we can do the transfer. */
                    if (!err)
                    {
                        /* Read into a buffer. */
                        if (RwStreamRead(inStream, buffer, chunkInfo.length) == chunkInfo.length)
                        {
                            /* Write out the header and data. */
                            _rwStreamWriteVersionedChunkHeader(outStream,
                                chunkInfo.type, chunkInfo.length, chunkInfo.version, chunkInfo.buildNum);

                            RwStreamWrite(outStream, buffer, chunkInfo.length);

                            /* Close the output stream. */
                            RwStreamClose(outStream, NULL);
                        }
                        else
                        {
                            /* Error during reading. Stop and exit. */
                            *name = '.';
                            sprintf(SplitFileMsg, "File <%s> : Error during reading.", inFilename);
                            *name = 0;

                            ToolAPIReportError(api, SplitFileMsg);

                            err = 1;
                        }
                    }
                }
            }
            else
            {
                /* Skip this chunk */
                RwStreamSkip(inStream, chunkInfo.length);
            }
        }

        /* Close the input stream. */
        RwStreamClose(inStream, NULL);

        /* Restore the extension. */
        *name = '.';
    }
    else
    {
        /* Failed to open the file. Raise an error. */
        sprintf(SplitFileMsg, "File <%s> : File not found.", inFilename);

        ToolAPIReportError(api, SplitFileMsg);

        err = 1;
    }

    /* Clean up */
    if (buffer)
        RwFree(buffer);

    return (err);
}

/*
 *
 */

int main(int argc, char* argv[])
{
    ToolAPI api;
    RwInt32 err;

    ToolAPIInitAPI(&api, "rwssplit", "split a file into muliple components",
                   NULL, 0);

    api.userProcessFileFn = SplitFileProcessFile;

    err = ToolAPIExecute(&api, argc, argv);

    ToolAPIShutdownAPI(&api);

    return(err);
}

