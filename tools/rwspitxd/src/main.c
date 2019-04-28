
#include <stdio.h>
#include <string.h>

#include <rwcore.h>
#include <rtpitexd.h>
#include <rtpng.h>
#include <rtbmp.h>
#include <rpcollis.h>
#include <rpdmorph.h>
#include <rphanim.h>
#include <rtimport.h>
#include <rpmatfx.h>
#include <rpmipkl.h>
#include <rpmorph.h>
#include <rpltmap.h>
#include <rtquat.h>
#include <rpspline.h>
#include <rpskin.h>
#include <rtslerp.h>
#include <rpusrdat.h>
#include <rpworld.h>
#include <rtworld.h>
#include <rtvcat.h>
#include <rpanisot.h>
#include <rppatch.h>
#include <rttoc.h>
#include <rppvs.h>
#include <rtpitexd.h>
#include <rpprtstd.h>
#include <rpprtadv.h>
#include <rt2d.h>
#include <rt2danim.h>
#include <rptoon.h>
#include <rpuvanim.h>
#include <rpnormmap.h>
#include <rpadc.h>

#include "shared.h"


RwSList *g_AddList = NULL;
RwSList *g_RemoveList = NULL;
RwChar  *g_RenameTex[2] = {NULL, NULL};
RwChar  *g_ReplaceTex[2] = {NULL, NULL};
RwSList *g_ExtractList = NULL;
RwChar  *g_AddTxDict = NULL;
RwChar  *g_InFilename = NULL;
RwBool  g_ExtractAll = FALSE;

RwChar *g_TexUAdrName = NULL;
RwChar *g_TexVAdrName = NULL;
RwChar *g_TexFilterName = NULL;

RwBool *g_ListTex = FALSE;

RwTextureAddressMode    g_TexUAdr;
RwTextureAddressMode    g_TexVAdr;
RwTextureFilterMode     g_TexFilter;

#define ADDR_MODE_WRAP_STR "WRAP"
#define ADDR_MODE_MIRROR_STR "MIRROR"
#define ADDR_MODE_BORDER_STR "BORDER"
#define ADDR_MODE_CLAMP_STR "CLAMP"

#define DEFAULT_ADDR_MODE rwTEXTUREADDRESSCLAMP
#define DEFAULT_ADDR_MODE_STR ADDR_MODE_CLAMP_STR

#define FILTER_MODE_NEAREST_STR "NEAREST"
#define FILTER_MODE_LINEAR_STR "LINEAR"
#define FILTER_MODE_MIPNEAREST_STR "MIPNEAREST"
#define FILTER_MODE_MIPLINEAR_STR "MIPLINEAR"
#define FILTER_MODE_LINEARMIPNEAREST_STR "LINEARMIPNEAREST"
#define FILTER_MODE_LINEARMIPLINEAR_STR "LINEARMIPLINEAR"

#define DEFAULT_FILTER_MODE rwFILTERNEAREST
#define DEFAULT_FILTER_MODE_STR FILTER_MODE_NEAREST_STR

RwChar g_AdrModeWrapStr[] = ADDR_MODE_WRAP_STR;
RwChar g_AdrModeMirrorStr[] = ADDR_MODE_MIRROR_STR;
RwChar g_AdrModeBorderStr[] = ADDR_MODE_BORDER_STR;
RwChar g_AdrModeClampStr[] = ADDR_MODE_CLAMP_STR;

RwChar g_FilterModeNearestStr[] = FILTER_MODE_NEAREST_STR;
RwChar g_FilterModeLinearStr[] = FILTER_MODE_LINEAR_STR;
RwChar g_FilterModeMipNearestStr[] = FILTER_MODE_MIPNEAREST_STR;
RwChar g_FilterModeMipLinearStr[] = FILTER_MODE_MIPLINEAR_STR;
RwChar g_FilterModeLinearMipNearestStr[] = FILTER_MODE_LINEARMIPNEAREST_STR;
RwChar g_FilterModeLinearMipLinearStr[] = FILTER_MODE_LINEARMIPLINEAR_STR;

ToolOption ToolOptions[] = {
    { "uadr",       "ua",   " <amode>",  ToolParamTypeStringE,  &g_TexUAdrName,   "U Addressing Mode ("ADDR_MODE_CLAMP_STR ", " ADDR_MODE_MIRROR_STR ", " ADDR_MODE_BORDER_STR " or " ADDR_MODE_WRAP_STR "; default=" DEFAULT_ADDR_MODE_STR ")", NULL },
    { "vadr",       "va",   " <amode>",  ToolParamTypeStringE,  &g_TexVAdrName,   "U Addressing Mode ("ADDR_MODE_CLAMP_STR ", " ADDR_MODE_MIRROR_STR ", " ADDR_MODE_BORDER_STR " or " ADDR_MODE_WRAP_STR "; default=" DEFAULT_ADDR_MODE_STR ")", NULL },
    { "filter",     "fm",   " <fmode>",  ToolParamTypeStringE,  &g_TexFilterName, "Filter Mode ("FILTER_MODE_NEAREST_STR ", " FILTER_MODE_LINEAR_STR ", " FILTER_MODE_MIPNEAREST_STR ", " FILTER_MODE_MIPLINEAR_STR ", " FILTER_MODE_LINEARMIPNEAREST_STR ", " FILTER_MODE_LINEARMIPLINEAR_STR "; default=" DEFAULT_FILTER_MODE_STR ")", NULL },

    { "add",        "a",    " <file>",   ToolParamTypeSListE,  &g_AddList,     "Add texture", NULL },
    { "remove",     "rm",   " <texname>", ToolParamTypeSListE,  &g_RemoveList,  "Remove texture", NULL },
    { "rename",     "rn",   " <oldname> <newname>", ToolParamTypeString2E, g_RenameTex,  "Rename texture", NULL },
    { "replace",    "rp",   " <name> <file>", ToolParamTypeString2E, g_ReplaceTex, "Replace texture", NULL },
    { "extract",    "e",    " <name>", ToolParamTypeSListE,  &g_ExtractList, "Extract texture", NULL },
    { "extractall", "ea",   "", ToolParamTypeImplicitBoolE,  &g_ExtractAll, "Extract all texture", NULL },

    { "list",       "l",    "", ToolParamTypeImplicitBoolE,  &g_ListTex, "List texture", NULL },

    { "merge",      "m",    " <file>", ToolParamTypeStringE,  &g_AddTxDict,  "Add texture dictionary", NULL }
};

RwUInt32 NumToolOptions = sizeof(ToolOptions) / sizeof(ToolOption);

#define DEFAULT_STR_LEN     128

RwInt32         PiTxDMsgSize = 0;
RwChar          *PiTxDMsg = NULL;

ToolAPI         *g_ToolAPI;

RwChar          *g_Buffer;
RwUInt32        g_BufferSize;


/*
 *
 */
static RwChar *
PiTxDResizeBuffer( RwChar * buf, RwInt32 *newSize, RwInt32 *oldSize )
{
    RwChar      *newBuf;

    if (*newSize > *oldSize)
    {
        if (buf)
        {
            newBuf = RwRealloc(buf, *newSize, 0);
        }
        else
        {
            newBuf = RwMalloc(*newSize, 0);
        }

        if (newBuf)
            *oldSize = *newSize;
    }
    else
    {
        newBuf = buf;
    }

    return newBuf;
}

/*
 *
 */
static RwInt32
PiTxDStreamTransfer( ToolAPI *api,
                     RwStream *inStream, RwStream *outStream, RwChar *inFilename )
{
    RwChunkHeaderInfo   chunkInfo;
    RwInt32             err;
    RwChar              *tmpBuf;


    err = 0;

    while (RwStreamReadChunkHeaderInfo(inStream, &chunkInfo) && (!err))
    {
        /* Check if we need to resize the buffer. */
        tmpBuf = PiTxDResizeBuffer(g_Buffer, &chunkInfo.length, &g_BufferSize);

        if (tmpBuf != NULL)
        {
            g_Buffer = tmpBuf;
        }
        else
        {
            /* Failed to grow the buffer. Stop processing and
             * raise an error.
             */
            sprintf(PiTxDMsg, "File <%s> : Failed to create buffer.", inFilename);
            ToolAPIReportError(api, PiTxDMsg);

            err = 1;
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
                sprintf(PiTxDMsg, "File <%s> : Error during reading file.", inFilename);
                ToolAPIReportError(api, PiTxDMsg);

                err = 1;
            }
        }
    }

    return (err);
}


/*
 *
 */

static RwChar *
PiTxDTranslateAdrMode( ToolAPI *api, RwTextureAddressMode mode, RwChar *inFilename )
{
    RwChar  *name;

    name = NULL;

    if (mode == rwTEXTUREADDRESSWRAP)
    {
        name = g_AdrModeWrapStr;
    }
    else if (mode == rwTEXTUREADDRESSMIRROR)
    {
        name = g_AdrModeMirrorStr;
    }
    else if (mode == rwTEXTUREADDRESSBORDER)
    {
        name = g_AdrModeBorderStr;
    }
    else if (mode == rwTEXTUREADDRESSCLAMP)
    {
        name = g_AdrModeClampStr;
    }
    else
    {
        /* Unrecognised mode. Raise an error.  */
        sprintf(PiTxDMsg, "File <%s> : Addressing mode <%d> not recognised.",
            inFilename, mode);
        ToolAPIReportWarning(api, PiTxDMsg);
    }

    return (name);
}

/*
 *
 */

static RwTextureAddressMode
PiTxDTranslateAdrModeStr( ToolAPI *api, RwChar *name, RwChar *inFilename )
{
    RwTextureAddressMode    mode;
    RwInt32                 l;
    RwChar                  *tmpBuf;

    if (name == NULL)
    {
        mode = DEFAULT_ADDR_MODE;

        /* Mode not set. Use default setting.  */
        l = strlen(inFilename) + strlen(DEFAULT_ADDR_MODE_STR) + DEFAULT_STR_LEN;
        tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

        if (tmpBuf != NULL)
        {
            PiTxDMsg = tmpBuf;

            sprintf(PiTxDMsg, "File <%s> : Addressing mode not set. Using " DEFAULT_ADDR_MODE_STR ".",
                inFilename);
            ToolAPIReportWarning(api, PiTxDMsg);
        }
        else
        {
            /* Memory failure. */
            sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
            ToolAPIReportError(api, PiTxDMsg);
        }
    }
    else if (strcmp(name, g_AdrModeWrapStr) == 0)
    {
        mode = rwTEXTUREADDRESSWRAP;
    }
    else if (strcmp(name, g_AdrModeMirrorStr) == 0)
    {
        mode = rwTEXTUREADDRESSMIRROR;
    }
    else if (strcmp(name, g_AdrModeBorderStr) == 0)
    {
        mode = rwTEXTUREADDRESSBORDER;
    }
    else if (strcmp(name, g_AdrModeClampStr) == 0)
    {
        mode = rwTEXTUREADDRESSCLAMP;
    }
    else
    {
        mode = DEFAULT_ADDR_MODE;

        /* Unrecognised mode. Use default setting.  */
        l = strlen(inFilename) + strlen(name) + strlen(DEFAULT_ADDR_MODE_STR) + DEFAULT_STR_LEN;
        tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

        if (tmpBuf != NULL)
        {
            PiTxDMsg = tmpBuf;

            sprintf(PiTxDMsg, "File <%s> : Addressing mode <%s> not recognised. Using " DEFAULT_ADDR_MODE_STR ".",
                inFilename, name);
            ToolAPIReportWarning(api, PiTxDMsg);
        }
        else
        {
            /* Memory failure. */
            sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
            ToolAPIReportError(api, PiTxDMsg);
        }
    }

    return (mode);
}

/*
 *
 */

static RwChar *
PiTxDTranslateFilterMode( ToolAPI *api, RwTextureFilterMode mode, RwChar *inFilename )
{
    RwChar *name;

    name = NULL;

    if (mode == rwFILTERNEAREST)
    {
        name = g_FilterModeNearestStr;
    }
    else if (mode == rwFILTERLINEAR)
    {
        name = g_FilterModeLinearStr;
    }
    else if (mode == rwFILTERMIPNEAREST)
    {
        name = g_FilterModeNearestStr;
    }
    else if (mode == rwFILTERMIPLINEAR)
    {
        name = g_FilterModeLinearStr;
    }
    else if (mode == rwFILTERLINEARMIPNEAREST)
    {
        name = g_FilterModeLinearMipNearestStr;
    }
    else if (mode == rwFILTERLINEARMIPLINEAR)
    {
        name = g_FilterModeLinearMipLinearStr;
    }
    else
    {
        /* Unrecognised mode. Raise an error.  */
        sprintf(PiTxDMsg, "File <%s> : Filter mode <%d> not recognised.",
            inFilename, mode);
        ToolAPIReportError(api, PiTxDMsg);
    }

    return (name);
}

/*
 *
 */

static RwTextureFilterMode
PiTxDTranslateFilterModeStr( ToolAPI *api, RwChar *name, RwChar *inFilename )
{
    RwTextureFilterMode     mode;
    RwInt32                 l;
    RwChar                  *tmpBuf;

    if (name == NULL)
    {
        mode = DEFAULT_FILTER_MODE;

        /* Mode unset. Use default setting.  */
        l = strlen(inFilename) + strlen(DEFAULT_FILTER_MODE_STR) + DEFAULT_STR_LEN;
        tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

        if (tmpBuf != NULL)
        {
            PiTxDMsg = tmpBuf;

            sprintf(PiTxDMsg, "File <%s> : Filter mode not set. Using " DEFAULT_FILTER_MODE_STR ".",
                inFilename);
            ToolAPIReportWarning(api, PiTxDMsg);
        }
        else
        {
            /* Memory failure. */
            sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
            ToolAPIReportError(api, PiTxDMsg);
        }
    }
    else if (strcmp(name, g_FilterModeNearestStr) == 0)
    {
        mode = rwFILTERNEAREST;
    }
    else if (strcmp(name, g_FilterModeLinearStr) == 0)
    {
        mode = rwFILTERLINEAR;
    }
    else if (strcmp(name, g_FilterModeNearestStr) == 0)
    {
        mode = rwFILTERMIPNEAREST;
    }
    else if (strcmp(name, g_FilterModeLinearStr) == 0)
    {
        mode = rwFILTERMIPLINEAR;
    }
    else if (strcmp(name, g_FilterModeLinearMipNearestStr) == 0)
    {
        mode = rwFILTERLINEARMIPNEAREST;
    }
    else if (strcmp(name, g_FilterModeLinearMipLinearStr) == 0)
    {
        mode = rwFILTERLINEARMIPLINEAR;
    }
    else
    {
        mode = DEFAULT_FILTER_MODE;

        /* Unrecognised mode. Use default setting.  */
        l = strlen(inFilename) + strlen(name) + strlen(DEFAULT_FILTER_MODE_STR) + DEFAULT_STR_LEN;
        tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

        if (tmpBuf != NULL)
        {
            PiTxDMsg = tmpBuf;

            sprintf(PiTxDMsg, "File <%s> : Filter mode <%s> not recognised. Using " DEFAULT_FILTER_MODE_STR ".",
                inFilename, name);
            ToolAPIReportWarning(api, PiTxDMsg);
        }
        else
        {
            /* Memory failure. */
            sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
            ToolAPIReportError(api, PiTxDMsg);
        }
    }

    return (mode);
}


/*
 *
 */

static RwInt32
PiTxDReadPITexDictionary( ToolAPI *api, RwTexDictionary **txDict,
                          RwStream *inStream, RwStream *outStream, RwChar *inFilename )
{
    RwInt32             err;
    RwChunkHeaderInfo   chunkInfo;
    RwTexDictionary     *tmpTxDict;

    *txDict = NULL;
    tmpTxDict = NULL;

    err = 0;

    while ((tmpTxDict == NULL) && RwStreamReadChunkHeaderInfo(inStream, &chunkInfo) && (!err))
    {
        if (chunkInfo.type == rwID_PITEXDICTIONARY)
        {
            /* Load the tex dictionary. */
            tmpTxDict = RtPITexDictionaryStreamRead(inStream);

            if (tmpTxDict == NULL)
            {
                /* Error during reading the tex dict. Raise an error. */
                sprintf(PiTxDMsg, "File <%s> : Error during reading PI texture dictionary.",
                    inFilename);
                ToolAPIReportError(api, PiTxDMsg);

                err = 1;
            }
            else
            {
                /* Found a PI tex dictionary, so stop searching. */
                *txDict = tmpTxDict;

                sprintf(PiTxDMsg, "File <%s> : PI texture dictionary read.\n",
                    inFilename);
                ToolAPIReport(api, PiTxDMsg);
            }
        }
        else
        {
            /* Check if we need to output the stream. */
            if (outStream)
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
                        sprintf(PiTxDMsg, "File <%s> : Failed to create buffer.", inFilename);
                        ToolAPIReportError(api, PiTxDMsg);

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
                        sprintf(PiTxDMsg, "File <%s> : Error during reading file.", inFilename);
                        ToolAPIReportError(api, PiTxDMsg);

                        err = 1;
                    }
                }
            }
        }
    }

    /* PI text dictionary not found. Raise an error and stop. */
    if (tmpTxDict == NULL)
    {
        sprintf(PiTxDMsg, "File <%s> : PI texture dictionary not found.", inFilename);
        ToolAPIReportError(api, PiTxDMsg);

        err = 1;
    }

    return (err);
}

/*
 *
 */
static RwTexture *
PiTxDWriteTxCB( RwTexture *tex, void * data )
{
    RwTexture       *result;
    RwInt32         w, h, d, format, l;
    RwImage         *img;
    RwRaster        *raster;
    RwChar          *name, *inFilename, *tmpBuf;

    result = tex;
    inFilename = (RwChar *) data;

    /* Texture found. Write out the texture. */
    name = RwTextureGetName(tex);
    raster = RwTextureGetRaster(tex);

    l = strlen(inFilename) + strlen(name) + DEFAULT_STR_LEN;
    tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

    if (tmpBuf != NULL)
    {
        PiTxDMsg = tmpBuf;
    }
    else
    {
        /* Memory failure. */
        sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
        ToolAPIReportError(g_ToolAPI, PiTxDMsg);

        result = NULL;
        raster = NULL;
    }

    if (raster)
    {
        /* Query the raster's info. */
        format = (RwRasterFormat) RwRasterGetFormat(raster);
        if (format & rwRASTERFORMATPAL4)
        {
            d = 4;
        }
        else if (format & rwRASTERFORMATPAL8)
        {
            d = 8;
        }
        else
        {
            d = 32;
        }

        w = RwRasterGetWidth(raster);
        h = RwRasterGetHeight(raster);

        if ((w > 0) && (h > 0))
        {
            /* Create an image for output. */
            img = RwImageCreate(w, h, d);

            if (img)
            {
                if (RwImageAllocatePixels(img))
                {
                    /* Copy the raster into the image for output. */
                    RwImageSetFromRaster(img, raster);

                    /* Generate a filename and write out in png format. */
                    sprintf(PiTxDMsg, "%s.png", name);

                    RwImageWrite(img, PiTxDMsg);

                    /* Destroy the image. */
                    RwImageDestroy(img);

                    sprintf(PiTxDMsg, "File <%s> : Texture <%s> extracted.\n",
                        inFilename, name);
                    ToolAPIReport(g_ToolAPI, PiTxDMsg);
                }
                else
                {
                    /* Failed to create the image. Report error and stop. */
                    sprintf(PiTxDMsg, "File <%s> : Texture <%s> output fail. Failed to create image",
                        inFilename, name);
                    ToolAPIReportError(g_ToolAPI, PiTxDMsg);
                    result = NULL;
                }
            }
            else
            {
                /* Failed to create the image. Report error and stop. */
                sprintf(PiTxDMsg, "File <%s> : Texture <%s> output fail. Failed to create image",
                    inFilename, name);
                ToolAPIReportError(g_ToolAPI, PiTxDMsg);
                result = NULL;
            }
        }
    }

    return (result);
}

/*
 *
 */

static RwInt32
PiTxDExtractTextureAll( ToolAPI *api __RWUNUSED__, RwTexDictionary *txDict, RwChar *inFilename )
{
    RwInt32         err;

    err = 0;

    if (RwTexDictionaryForAllTextures(txDict, PiTxDWriteTxCB, (void *) inFilename) != txDict)
    {
        /* Something when wrong during texture saving. Raise error and stop. */
        err = 1;
    }

    return (err);
}

/*
 *
 */

static RwInt32
PiTxDExtractTextureList( ToolAPI *api, RwTexDictionary *txDict, RwChar *inFilename )
{
    RwInt32         err, i, num, l;
    RwChar          *filename, *tmpBuf;
    RwTexture       *tex;

    /* Query the global file list for number of files to append. */
    num = _rwSListGetNumEntries(g_ExtractList);

    i = 0;
    err = 0;

    while ((i < num) && (!err))
    {
        /* Grab the filename and append it to the output stream. */
        filename = *(RwChar **) _rwSListGetEntry(g_ExtractList, i);

        /* Query the dictionary for the image. */
        tex = RwTexDictionaryFindNamedTexture(txDict, filename);

        if (tex)
        {
            if (PiTxDWriteTxCB(tex, (void *) inFilename) != tex)
            {
                /* Something when wrong during texture saving. Raise error and stop. */
                err = 1;
            }
        }
        else
        {
            /* Texture not found. Report a warning but continue. */
            l = strlen(inFilename) + strlen(filename) + DEFAULT_STR_LEN;
            tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

            if (tmpBuf != NULL)
            {
                PiTxDMsg = tmpBuf;

                sprintf(PiTxDMsg, "File <%s> : Texture <%s> not found in dictionary.",
                    inFilename, filename);
                ToolAPIReportWarning(api, PiTxDMsg);
            }
            else
            {
                /* Memory failure. */
                sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
                ToolAPIReportError(api, PiTxDMsg);
                err = 1;
            }
        }

        /* Next texture. */
        i++;
    }

    return (err);
}

/*
 *
 */

static RwInt32
PiTxDRemoveTexture( ToolAPI *api, RwTexDictionary *txDict, RwChar *inFilename )
{
    RwInt32         err, i, num, l;
    RwChar          *name, *tmpBuf;
    RwTexture       *tex;

    /* Query the global file list for number of files to append. */
    num = _rwSListGetNumEntries(g_RemoveList);

    i = 0;
    err = 0;

    while ((i < num) && (!err))
    {
        /* Grab the filename and append it to the output stream. */
        name = *(RwChar **) _rwSListGetEntry(g_RemoveList, i);

        l = strlen(inFilename) + strlen(name) + DEFAULT_STR_LEN;
        tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

        if (tmpBuf != NULL)
        {
            PiTxDMsg = tmpBuf;
        }
        else
        {
            /* Memory failure. */
            sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
            ToolAPIReportError(api, PiTxDMsg);
            err = 1;
        }

        if (!err)
        {
            tex = RwTexDictionaryFindNamedTexture(txDict, name);

            if (tex)
            {
                /* Texture found. Remove and destroy. */
                RwTexDictionaryRemoveTexture(tex);

                RwTextureDestroy(tex);

                sprintf(PiTxDMsg, "File <%s> : Texture <%s> removed from dictionary.\n",
                    inFilename, name);
                ToolAPIReport(api, PiTxDMsg);
            }
            else
            {
                /* Texture not found. Report a warning but continue. */
                sprintf(PiTxDMsg, "File <%s> : Texture <%s> not found in dictionary.",
                    inFilename, name);
                ToolAPIReportWarning(api, PiTxDMsg);
            }
        }

        i++;
    }

    return (err);
}

/*
 *
 */

static RwInt32
PiTxDReplaceTexture( ToolAPI *api, RwTexDictionary *txDict, RwChar *inFilename )
{
    RwInt32         err, w, h, d, format, rasterFlags, l;
    RwTexture       *tex;
    RwImage         *img;
    RwRaster        *oldRaster, *newRaster;
    RwChar          *tmpBuf;

    err = 0;

    img = NULL;

    l = strlen(inFilename) + strlen(g_ReplaceTex[0]) + strlen(g_ReplaceTex[1]) + DEFAULT_STR_LEN;
    tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

    if (tmpBuf != NULL)
    {
        PiTxDMsg = tmpBuf;
    }
    else
    {
        /* Memory failure. */
        sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
        ToolAPIReportError(api, PiTxDMsg);
        err = 1;
    }

    if (!err)
    {
        tex = RwTexDictionaryFindNamedTexture(txDict, g_ReplaceTex[0]);
        img = RwImageRead(g_ReplaceTex[1]);

        if ((tex) && (img))
        {
            /* Query the image's info. */
            format = 0;
            rasterFlags = (rwRASTERTYPETEXTURE | rwRASTERFORMATMIPMAP);

            RwImageFindRasterFormat(img, rasterFlags, &w, &h, &d, &format);


            /* Create a new raster for the replacement image. */
            newRaster = RwRasterCreate(w, h, d, (rasterFlags | format));

            if (newRaster)
            {
                RwRasterLock(newRaster, 0, rwRASTERLOCKWRITE);
                RwRasterSetFromImage(newRaster, img);
                RwRasterUnlock(newRaster);

                /* Destroy the old raster. */
                oldRaster = RwTextureGetRaster(tex);
                RwRasterDestroy(oldRaster);

                RwTextureSetRaster(tex, newRaster);

                sprintf(PiTxDMsg, "File <%s> : Texture <%s> replace by texture <%s>.\n",
                    inFilename, g_ReplaceTex[0], g_ReplaceTex[1]);
                ToolAPIReport(api, PiTxDMsg);
            }
            else
            {
                /* Failed to create the raster. Raise error and stop. */
                sprintf(PiTxDMsg, "File <%s> : Texture raster <%s> creation failed.",
                    inFilename, g_ReplaceTex[0]);
                ToolAPIReportError(api, PiTxDMsg);
                err = 1;
            }
        }
        else
        {
            /* Texture not found. Report a warning but continue. */
            if (tex == NULL)
            {
                sprintf(PiTxDMsg, "File <%s> : Texture <%s> not found in dictionary.",
                    inFilename, g_ReplaceTex[0]);
                ToolAPIReportWarning(api, PiTxDMsg);
            }
            if (img == NULL)
            {
                sprintf(PiTxDMsg, "File <%s> : Texture <%s> not found.",
                    inFilename, g_ReplaceTex[1]);
                ToolAPIReportWarning(api, PiTxDMsg);
            }
        }
    }

    if (img)
    {
        RwImageDestroy(img);
    }

    return (err);
}

/*
 *
 */

static RwInt32
PiTxDAddTexture( ToolAPI *api, RwTexDictionary *txDict, RwChar *inFilename )
{
    RwInt32         err, i, num, l;
    RwChar          *name, *txName, *tmpBuf;
    RwTexture       *tex;

    /* Query the global file list for number of files to append. */
    num = _rwSListGetNumEntries(g_AddList);

    i = 0;
    err = 0;

    while ((i < num) && (!err))
    {
        /* Grab the filename and append it to the output stream. */
        txName = *(RwChar **) _rwSListGetEntry(g_AddList, i);

        l = strlen(inFilename) + strlen(txName) + DEFAULT_STR_LEN;
        tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

        if (tmpBuf != NULL)
        {
            PiTxDMsg = tmpBuf;
        }
        else
        {
            /* Memory failure. */
            sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
            ToolAPIReportError(api, PiTxDMsg);
            err = 1;
        }

        if (!err)
        {
            tex = RwTexDictionaryFindNamedTexture(txDict, txName);

            /* Temporary mask out the extension. */
            name = strrchr(txName, '.');
            if (name)
                *name = 0;

            tex = RwTexDictionaryFindNamedTexture(txDict, txName);

            /* Texture found. Report a warning but continue. */
            if (tex)
            {
                sprintf(PiTxDMsg, "File <%s> : Texture <%s> already exist. Cannot be added.",
                    inFilename, txName);
                ToolAPIReportWarning(api, PiTxDMsg);
            }
            else
            {
                /* Restore the extension. */
                if (name)
                    *name = '.';

                /* Texture not found. Load in the texture and add it to the dictionary */
                tex = RwTextureRead(txName, NULL);

                if (tex)
                {
                    /* Add the texture to the dictionary. */
                    RwTexDictionaryAddTexture(txDict, tex);

                    RwTextureSetAddressingU(tex, g_TexUAdr);
                    RwTextureSetAddressingV(tex, g_TexVAdr);
                    RwTextureSetFilterMode(tex, g_TexFilter);

                    sprintf(PiTxDMsg, "File <%s> : Texture <%s> added to dictionary.\n",
                        inFilename, txName);
                    ToolAPIReport(api, PiTxDMsg);
                }
                else
                {
                    /* Failed to load the texture. Raise a warning and continue. */
                    sprintf(PiTxDMsg, "File <%s> : Texture <%s> cannot be opened.",
                        inFilename, txName);
                    ToolAPIReportWarning(api, PiTxDMsg);
                }
            }
        }

        i++;
    }

    return (err);
}

/*
 *
 */

static RwInt32
PiTxDRenameTexture( ToolAPI *api, RwTexDictionary *txDict, RwChar *inFilename )
{
    RwInt32         err, l;
    RwTexture       *tex;
    RwChar          *txName, *tmpBuf;

    err = 0;

    l = strlen(inFilename) + strlen(g_RenameTex[0]) + strlen(g_RenameTex[1]) + DEFAULT_STR_LEN;
    tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

    if (tmpBuf != NULL)
    {
        PiTxDMsg = tmpBuf;
    }
    else
    {
        /* Memory failure. */
        sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
        ToolAPIReportError(api, PiTxDMsg);
        err = 1;
    }

    /* Grab the texture. */
    if (!err)
    {
        tex = RwTexDictionaryFindNamedTexture(txDict, g_RenameTex[0]);

        if (tex)
        {
            /* Texture found. Change the name */
            txName = RwTextureGetName(tex);
            sprintf(txName, "%s", g_RenameTex[1]);

            sprintf(PiTxDMsg, "File <%s> : Texture <%s> renamed to <%s>.\n",
                inFilename, g_RenameTex[0], g_RenameTex[1]);
            ToolAPIReport(api, PiTxDMsg);
        }
        else
        {
            /* Texture not found. Report a warning but continue. */
            sprintf(PiTxDMsg, "File <%s> : Texture <%s> not found in dictionary.",
                inFilename, g_RenameTex[0]);
            ToolAPIReportWarning(api, PiTxDMsg);
        }
    }

    return (err);
}

/*
 *
 */

static RwTexture *
PiTxDSwapTxCB(RwTexture *tex, void *pData)
{
    RwTexDictionary        *txDict;
    RwTexture              *findTex;
    RwChar                 *txName, *tmpBuf;
    RwInt32                 l, err;

    err = 0;

    txDict = (RwTexDictionary *) pData;

    /* Check if the texture of the same name already exist. */
    txName = RwTextureGetName(tex);

    l = strlen(g_InFilename) + strlen(txName) + strlen(g_AddTxDict) + DEFAULT_STR_LEN;
    tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

    if (tmpBuf != NULL)
    {
        PiTxDMsg = tmpBuf;
    }
    else
    {
        /* Memory failure. */
        sprintf(PiTxDMsg, "File <%s> : Memory failure.", g_InFilename);
        ToolAPIReportError(g_ToolAPI, PiTxDMsg);
        err = 1;
    }

    if (!err)
    {
        findTex = RwTexDictionaryFindNamedTexture(txDict, txName);

        if (!findTex)
        {
            /* Texture not found.*/

            /* Remove from the current active dict. */
            RwTexDictionaryRemoveTexture(tex);

            /* Add to the new dictionary. */
            RwTexDictionaryAddTexture(txDict, tex);

            sprintf(PiTxDMsg, "File <%s> : Texture  <%s> added to dictionary.\n",
                g_InFilename, txName);
            ToolAPIReport(g_ToolAPI, PiTxDMsg);
        }
        else
        {
            /* Texture found. Report a warning but continue. */
            sprintf(PiTxDMsg, "File <%s> : Texture <%s> already exist in dictionary <%s>.",
                g_InFilename, txName, g_AddTxDict);
            ToolAPIReportWarning(g_ToolAPI, PiTxDMsg);
        }
    }

    return(tex);
}

/*
 *
 */

static RwInt32
PiTxDAddDictionary( ToolAPI *api, RwTexDictionary *txDict, RwChar *inFilename )
{
    RwInt32         err, l;
    RwTexDictionary *tmpTxDict, *addTxDict;
    RwStream        *stream;
    RwChar          *tmpBuf;

    err = 0;

    l = strlen(inFilename) + strlen(g_AddTxDict) + DEFAULT_STR_LEN;
    tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

    if (tmpBuf != NULL)
    {
        PiTxDMsg = tmpBuf;
    }
    else
    {
        /* Memory failure. */
        sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
        ToolAPIReportError(api, PiTxDMsg);
        err = 1;
    }

    if (!err)
    {
        /* Replace the current dictionary */
        tmpTxDict = RwTexDictionaryGetCurrent();
        RwTexDictionarySetCurrent(txDict);

        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, g_AddTxDict);

        if (stream)
        {
            err = PiTxDReadPITexDictionary( api, &addTxDict, stream, NULL, inFilename );

            if ((addTxDict) && (!err))
            {
                /* Remove textures from txd1 and add it to txd2 */
                RwTexDictionaryForAllTextures(addTxDict, PiTxDSwapTxCB, (void *) txDict);

                /* Destroy the dictionary. */
                RwTexDictionaryDestroy(addTxDict);

                sprintf(PiTxDMsg, "File <%s> : PI texture dictionary <%s> added to dictionary.\n",
                    inFilename, g_AddTxDict);
                ToolAPIReport(api, PiTxDMsg);
            }
            else
            {
                /* Error during reading. Stop and exit. */
                sprintf(PiTxDMsg, "File <%s> : Error during reading PI texture dictionary <%s>.",
                    inFilename, g_AddTxDict);
                ToolAPIReportError(api, PiTxDMsg);

                err = 1;
            }

            RwStreamClose(stream, NULL);
        }
        else
        {
            /* Dictionary not found. Stop and exit. */
            sprintf(PiTxDMsg, "File <%s> : PI texture dictionary <%s> not found.",
                inFilename, g_AddTxDict);
            ToolAPIReportError(api, PiTxDMsg);

            err = 1;
        }

        /* Restore the current dictionary */
        RwTexDictionarySetCurrent(tmpTxDict);
    }

    return (err);
}

/*
 *
 */

static RwTexture *
PiTxDListTxCB(RwTexture *tex, void *pData)
{
    RwChar                  *inFilename, *name, *uAdr, *vAdr, *filter, *tmpBuf;
    RwInt32                 err, l;

    err = 0;

    inFilename = (RwChar *) pData;

    name = RwTextureGetName(tex);
    uAdr = PiTxDTranslateAdrMode(g_ToolAPI, RwTextureGetAddressingU(tex), inFilename);
    vAdr = PiTxDTranslateAdrMode(g_ToolAPI, RwTextureGetAddressingV(tex), inFilename);
    filter = PiTxDTranslateFilterMode(g_ToolAPI, RwTextureGetFilterMode(tex), inFilename);

    l = strlen(inFilename) + strlen(name) + strlen(uAdr) + strlen(vAdr) + strlen(filter) + DEFAULT_STR_LEN;
    tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

    if (tmpBuf != NULL)
    {
        PiTxDMsg = tmpBuf;
    }
    else
    {
        /* Memory failure. */
        sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
        ToolAPIReportError(g_ToolAPI, PiTxDMsg);
        err = 1;
    }

    if (!err)
    {
        sprintf(PiTxDMsg, "File <%s> : Texture  <%s> found. U Adr mode <%s> V Adr mode <%s> Filter mode <%s>.\n",
            inFilename, name, uAdr, vAdr, filter);
        ToolAPIReport(g_ToolAPI, PiTxDMsg);
    }

    return(tex);
}

/*
 *
 */

static RwInt32
PiTxDListTexture( ToolAPI *api __RWUNUSED__, RwTexDictionary *txDict, RwChar *inFilename )
{
    RwInt32         err;

    err = 0;

    /* Remove textures from txd1 and add it to txd2 */
    RwTexDictionaryForAllTextures(txDict, PiTxDListTxCB, (void *) inFilename);

    return (err);
}

/*
 *
 */

static RwInt32
PiTxDStartup(ToolAPI *api __RWUNUSED__, RwChar *currentPath __RWUNUSED__)
{
    g_Buffer = NULL;
    g_BufferSize = 0;

    if( !RwImageRegisterImageFormat(RWSTRING("bmp"), RtBMPImageRead, RtBMPImageWrite) )
    {
        return (1);
    }

    if( !RwImageRegisterImageFormat(RWSTRING("png"), RtPNGImageRead, RtPNGImageWrite) )
    {
        return (1);
    }

    g_ExtractList = rwSListCreate(sizeof(char *), 0);
    if (g_ExtractList == NULL)
        return (1);

    g_AddList = rwSListCreate(sizeof(char *), 0);
    if (g_AddList == NULL)
        return (1);

    g_RemoveList = rwSListCreate(sizeof(char *), 0);
    if (g_RemoveList == NULL)
        return (1);

    return 0;
}

/*
 *
 */

static RwInt32
PiTxDShutdown(ToolAPI *api __RWUNUSED__)
{
    if (g_Buffer)
        RwFree(g_Buffer);

    if (g_ExtractList)
        rwSListDestroy(g_ExtractList);
    g_ExtractList = NULL;

    if (g_AddList)
        rwSListDestroy(g_AddList);
    g_AddList = NULL;

    if (g_RemoveList)
        rwSListDestroy(g_RemoveList);
    g_RemoveList = NULL;

    g_Buffer = NULL;
    g_BufferSize = 0;

    return 0;
}


/*
 *
 */

static RwInt32
PiTxDProcessFile(ToolAPI *api, RwChar *inFilename)
{
    RwChar          *tmpBuf;
    RwInt32         err, output, num, l;
    RwTexDictionary *txDict, *tmpTxDict;
    RwStream        *inStream, *outStream, *memStream;
    RwMemory        memBuffer;

    err = 0;
    output = 0;
    PiTxDMsg = NULL;

    l = (2 * strlen(inFilename)) + DEFAULT_STR_LEN;
    tmpBuf = PiTxDResizeBuffer(PiTxDMsg, &l, &PiTxDMsgSize);

    if (tmpBuf != NULL)
    {
        PiTxDMsg = tmpBuf;
    }
    else
    {
        /* Memory failure. */
        sprintf(PiTxDMsg, "File <%s> : Memory failure.", inFilename);
        ToolAPIReportError(api, PiTxDMsg);
        err = 1;
    }

    if (!err)
    {
        /* Replace the current dictionary */
        tmpTxDict = RwTexDictionaryGetCurrent();

        inStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, inFilename);
        memStream = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMWRITE, NULL);
        outStream = NULL;

        memBuffer.start = NULL;
        memBuffer.length = 0;

        if (inStream && memStream)
        {
            err = PiTxDReadPITexDictionary(api, &txDict, inStream, memStream, inFilename);

            g_InFilename = inFilename;

            if ((txDict) && (!err))
            {
                /* Get the texture addressing mode. */
                g_TexUAdr = PiTxDTranslateAdrModeStr(api, g_TexUAdrName, inFilename);
                g_TexUAdr = PiTxDTranslateAdrModeStr(api, g_TexVAdrName, inFilename);
                g_TexFilter = PiTxDTranslateFilterModeStr(api, g_TexFilterName, inFilename);

                /* List textures in dictionary requested. */
                if ((!err) && (g_ListTex))
                    err = PiTxDListTexture(api, txDict, inFilename);

                /* Extract any texture dictionary requested. */
                if ((!err) && (g_ExtractAll))
                    err = PiTxDExtractTextureAll(api, txDict, inFilename);
                else
                    err = PiTxDExtractTextureList(api, txDict, inFilename);

                /* Remove any textures requested. */
                num = _rwSListGetNumEntries(g_RemoveList);
                if ((!err) && (num > 0))
                {
                    err = PiTxDRemoveTexture(api, txDict, inFilename);
                    output = 1;
                }

                /* Rename any textures requested. */
                if ((!err) && (g_RenameTex[0]))
                {
                    err = PiTxDRenameTexture(api, txDict, inFilename);
                    output = 1;
                }

                /* Add any textures requested. */
                num = _rwSListGetNumEntries(g_AddList);
                if ((!err) && (num > 0))
                {
                    err = PiTxDAddTexture(api, txDict, inFilename);
                    output = 1;
                }

                /* Replace any textures requested. */
                if ((!err) && (g_ReplaceTex[0]))
                {
                    err = PiTxDReplaceTexture(api, txDict, inFilename);
                    output = 1;
                }

                /* Merge any texture dictionary requested. */
                if ((!err) && (g_AddTxDict))
                {
                    err = PiTxDAddDictionary(api, txDict, inFilename);
                    output = 1;
                }

                /* No errors. Write out the dictionary. */
                if ((!err) && (output))
                {
                    /* Now write out the dictionary into memory stream. */
                    RtPITexDictionaryStreamWrite(txDict, memStream);

                    /* Transfer the remainder of the input stream into memory. */
                    err = PiTxDStreamTransfer(api, inStream, memStream, inFilename);

                    /* Now write out the memory stream to file. */
                    if (!err)
                    {
                        /* Close the input streams. */
                        RwStreamClose(inStream, NULL);
                        RwStreamClose(memStream, &memBuffer);

                        /* Open the streams for output. */
                        memStream = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &memBuffer);
                        outStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, inFilename);

                        if (memStream && outStream)
                        {
                            err = PiTxDStreamTransfer(api, memStream, outStream, inFilename);
                        }
                        else
                        {
                            /* Error during stream opening. Raise error and stop. */
                            if (outStream == NULL)
                            {
                                /* Error during reading. Stop and exit. */
                                sprintf(PiTxDMsg, "File <%s> : PI texture dictionary <%s> cannot be opened for writing.",
                                    inFilename, inFilename);
                                ToolAPIReportError(api, PiTxDMsg);
                            }

                            if (memStream == NULL)
                            {
                                /* Error during reading. Stop and exit. */
                                sprintf(PiTxDMsg, "File <%s> : Failed to create memory stream.",
                                    inFilename);
                                ToolAPIReportError(api, PiTxDMsg);
                            }

                            err = 1;
                        }

                        /* Set the input stream to NULL to prevent re-closing. */
                        inStream = NULL;
                    }
                }

                /* Clean up */
                RwTexDictionaryDestroy(txDict);
            }
            else
            {
                /* Failed to load the tex dict. Raise an error and stop. */
                /* Error during reading. Stop and exit. */
                sprintf(PiTxDMsg, "File <%s> : Error during reading PI texture dictionary <%s>.",
                    inFilename, g_AddTxDict);
                ToolAPIReportError(api, PiTxDMsg);

                err = 1;
            }
        }
        else
        {
            if (inStream == NULL)
            {
                /* Error during reading. Stop and exit. */
                sprintf(PiTxDMsg, "File <%s> : PI texture dictionary <%s> not found.",
                    inFilename, inFilename);
                ToolAPIReportError(api, PiTxDMsg);
            }

            if (memStream == NULL)
            {
                /* Error during reading. Stop and exit. */
                sprintf(PiTxDMsg, "File <%s> : Failed to create memory stream.",
                    inFilename);
                ToolAPIReportError(api, PiTxDMsg);
            }
        }
    }

    /* Close the streams. */
    if (inStream)
        RwStreamClose(inStream, NULL);

    if (memStream)
    {
        RwStreamClose(memStream, &memBuffer);

        if (memBuffer.start)
            RwFree(memBuffer.start);
    }

    if (outStream)
        RwStreamClose(outStream, NULL);

    /* Restore the dictionary. */
    RwTexDictionarySetCurrent(tmpTxDict);

    if (PiTxDMsg)
        RwFree(PiTxDMsg);

    return (err);
}

/*
 *
 */

RwInt32
AttachPlugins(ToolAPI *api __RWUNUSED__)
{
    const RwUInt32 succeeded = 0;
    const RwUInt32 failed = 1;

    if( !RpWorldPluginAttach() )
    {
        return failed;
    }

    if( !RpPVSPluginAttach() )
    {
        return failed;
    }

    if( !RpCollisionPluginAttach() )
    {
        return failed;
    }

    if (!RpSkinPluginAttach())
    {
        return failed;
    }

    if( !RpHAnimPluginAttach() )
    {
        return failed;
    }

    if (!RpMorphPluginAttach())
    {
        return failed;
    }

    if (!RpDMorphPluginAttach())
    {
        return failed;
    }

    if (!RpPatchPluginAttach())
    {
        return failed;
    }

    if( !RpMatFXPluginAttach() )
    {
        return failed;
    }

    if (!RpLtMapPluginAttach())
    {
        return failed;
    }

    if( !RpAnisotPluginAttach() )
    {
        return failed;
    }

    if( !RpSplinePluginAttach() )
    {
        return failed;
    }

    if( !RpUserDataPluginAttach() )
    {
        return failed;
    }

    if ( !RtAnimInitialize() )
    {
        return failed;
    }

    if( !RpPTankPluginAttach() )
    {
        return failed;
    }

    if ( !RpPrtStdPluginAttach() )
    {
        return failed;
    }

    if ( !RpPrtAdvPluginAttach() )
    {
        return failed;
    }

    if ( !RpToonPluginAttach() )
    {
        return failed;
    }
#ifndef SKY2_DRVMODEL_H
    if ( !RpMipmapKLPluginAttach() )
    {
        return failed;
    }
#endif

    if ( !RpUVAnimPluginAttach() )
    {
        return failed;
    }

    if ( !RpNormMapPluginAttach() )
    {
        return failed;
    }

    if ( !RpADCPluginAttach() )
    {
        return failed;
    }

    return succeeded;
}

/*
 *
 */

int main(int argc, char* argv[])
{
    ToolAPI api;
    RwInt32 err;

    ToolAPIInitAPI(&api, "PiTxD", "Edit a PI texture dictionary",
                   ToolOptions, NumToolOptions);

    api.userStartupFn = PiTxDStartup;
    api.userProcessFileFn = PiTxDProcessFile;
    api.userShutdownFn = PiTxDShutdown;
    api.userPluginAttachFn = AttachPlugins;

    g_ToolAPI = &api;

    err = ToolAPIExecute(&api, argc, argv);

    ToolAPIShutdownAPI(&api);

    return(err);
}

