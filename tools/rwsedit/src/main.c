
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <rwcore.h>
#include <rttoc.h>

#include "shared.h"


/*
 * Defines.
 */
#define rtNAMEGROUPPOSSTART                 1

#define rtNAMEGROUPPATHSEPARATOR            '/'

#define rtNAMEGROUPPATHROOT                 "/"

#define rtNAMEGROUPVIEWLABELOPENTOKEN       "[-]--/"
#define rtNAMEGROUPVIEWLABELCLOSETOKEN      "[+]--/"
#define rtNAMEGROUPVIEWCHUNKTOKEN           " |----"
#define rtNAMEGROUPVIEWBLANKTOKEN           "    "
#define rtNAMEGROUPVIEWBRANCHTOKEN          " |  "


/*
 * Private data types for chunk dictionary.
 */
typedef struct RtNameGroupChunkDictEntry RtNameGroupChunkDictEntry;

struct RtNameGroupChunkDictEntry
{
    RwUInt32                type;
    const RwChar            *name;

};

/*
 * Some plugin IDs that we can't get from public headers
 */
#define rwID_BINMESHPLUGIN      MAKECHUNKID(rwVENDORID_CRITERIONWORLD, 0x0E)
#define rwID_NATIVEDATAPLUGIN   MAKECHUNKID(rwVENDORID_CRITERIONWORLD, 0x10)

RtNameGroupChunkDictEntry rtNGroupChunkDict[] =
{
    {rwID_2DANIM, RWSTRING("rwID_2DANIM")},
    {rwID_2DANIMPLUGIN, RWSTRING("rwID_2DANIMPLUGIN")},
    {rwID_2DBRUSH, RWSTRING("rwID_2DBRUSH")},
    {rwID_2DFONT, RWSTRING("rwID_2DFONT")},
    {rwID_2DFONT, RWSTRING("rwID_2DFONT")},
    {rwID_2DKEYFRAME, RWSTRING("rwID_2DKEYFRAME")},
    {rwID_2DMAESTRO, RWSTRING("rwID_2DMAESTRO")},
    {rwID_2DOBJECT, RWSTRING("rwID_2DOBJECT")},
    {rwID_2DOBJECTSTRING, RWSTRING("rwID_2DOBJECTSTRING")},
    {rwID_2DPATH, RWSTRING("rwID_2DPATH")},
    {rwID_2DPICKREGION, RWSTRING("rwID_2DPICKREGION")},
    {rwID_2DPLUGIN, RWSTRING("rwID_2DPLUGIN")},
    {rwID_2DSCENE, RWSTRING("rwID_2DSCENE")},
    {rwID_2DSHAPE, RWSTRING("rwID_2DSHAPE")},
    {rwID_ALTPIPE, RWSTRING("rwID_ALTPIPE")},
    {rwID_ALTPIPETOOLKIT, RWSTRING("rwID_ALTPIPETOOLKIT")},
    {rwID_ANIMDATABASE, RWSTRING("rwID_ANIMDATABASE")},
    {rwID_ANIMPLUGIN, RWSTRING("rwID_ANIMPLUGIN")},
    {rwID_ANIMTOOLKIT, RWSTRING("rwID_ANIMTOOLKIT")},
    {rwID_ANISOTPLUGIN, RWSTRING("rwID_ANISOTPLUGIN")},
    {rwID_ATOMIC, RWSTRING("rwID_ATOMIC")},
    {rwID_ATOMICSECT, RWSTRING("rwID_ATOMICSECT")},
    {rwID_BARYCENTRIC, RWSTRING("rwID_BARYCENTRIC")},
    {rwID_BINMESHPLUGIN, RWSTRING("rwID_BINMESHPLUGIN")},
    {rwID_BMPIMAGEPLUGIN, RWSTRING("rwID_BMPIMAGEPLUGIN")},
    {rwID_BONEPLUGIN, RWSTRING("rwID_BONEPLUGIN")},
    {rwID_CAMERA, RWSTRING("rwID_CAMERA")},
    {rwID_CHAINPLUGIN, RWSTRING("rwID_CHAINPLUGIN")},
    {rwID_CHARSEPLUGIN, RWSTRING("rwID_CHARSEPLUGIN")},
    {rwID_CLUMP, RWSTRING("rwID_CLUMP")},
    {rwID_COLLISPLUGIN, RWSTRING("rwID_COLLISPLUGIN")},
    {rwID_CROWD, RWSTRING("rwID_CROWD")},
    {rwID_CROWDPPPLUGIN, RWSTRING("rwID_CROWDPPPLUGIN")},
    {rwID_DATABASEPLUGIN, RWSTRING("rwID_DATABASEPLUGIN")},
    {rwID_DATABASEPLUGIN, RWSTRING("rwID_DATABASEPLUGIN")},
    {rwID_DMORPHANIMATION, RWSTRING("rwID_DMORPHANIMATION")},
    {rwID_DMORPHPLUGIN, RWSTRING("rwID_DMORPHPLUGIN")},
    {rwID_EXTENSION, RWSTRING("rwID_EXTENSION")},
    {rwID_FRAMELIST, RWSTRING("rwID_FRAMELIST")},
    {rwID_GCNMATPLUGIN, RWSTRING("rwID_GCNMATPLUGIN")},
    {rwID_GEOMETRY, RWSTRING("rwID_GEOMETRY")},
    {rwID_GEOMETRYLIST, RWSTRING("rwID_GEOMETRYLIST")},
    {rwID_GEOMTXPLUGIN, RWSTRING("rwID_GEOMTXPLUGIN")},
    {rwID_GIFIMAGEPLUGIN, RWSTRING("rwID_GIFIMAGEPLUGIN")},
    {rwID_GLOSSPLUGIN, RWSTRING("rwID_GLOSSPLUGIN")},
    {rwID_GPVSPLUGIN, RWSTRING("rwID_GPVSPLUGIN")},
    {rwID_HANIMANIMATION, RWSTRING("rwID_HANIMANIMATION")},
    {rwID_HANIMPLUGIN, RWSTRING("rwID_HANIMPLUGIN")},
    {rwID_IMAGE, RWSTRING("rwID_IMAGE")},
    {rwID_IMPUTILPLUGIN, RWSTRING("rwID_IMPUTILPLUGIN")},
    {rwID_INTSECPLUGIN, RWSTRING("rwID_INTSECPLUGIN")},
    {rwID_JPEGIMAGEPLUGIN, RWSTRING("rwID_JPEGIMAGEPLUGIN")},
    {rwID_LABELPLUGIN, RWSTRING("rwID_LABELPLUGIN")},
    {rwID_LIBRARYPLUGIN, RWSTRING("rwID_LIBRARYPLUGIN")},
    {rwID_LIGHT, RWSTRING("rwID_LIGHT")},
    {rwID_LODATMPLUGIN, RWSTRING("rwID_LODATMPLUGIN")},
    {rwID_LOGOPLUGIN, RWSTRING("rwID_LOGOPLUGIN")},
    {rwID_LTMAPPLUGIN, RWSTRING("rwID_LTMAPPLUGIN")},
    {rwID_MATERIAL, RWSTRING("rwID_MATERIAL")},
    {rwID_MATERIALEFFECTSPLUGIN, RWSTRING("rwID_MATERIALEFFECTSPLUGIN")},
    {rwID_MATLIST, RWSTRING("rwID_MATLIST")},
    {rwID_MATRIX, RWSTRING("rwID_MATRIX")},
    {rwID_MEMINFOPLUGIN, RWSTRING("rwID_MEMINFOPLUGIN")},
    {rwID_MEMLEAKPLUGIN, RWSTRING("rwID_MEMLEAKPLUGIN")},
    {rwID_MEPLUGIN, RWSTRING("rwID_MEPLUGIN")},
    {rwID_METRICSPLUGIN, RWSTRING("rwID_METRICSPLUGIN")},
    {rwID_MIPMAPKPLUGIN, RWSTRING("rwID_MIPMAPKPLUGIN")},
    {rwID_MIPMAPPLUGIN, RWSTRING("rwID_MIPMAPPLUGIN")},
    {rwID_MIPSPLITPLUGIN, RWSTRING("rwID_MIPSPLITPLUGIN")},
    {rwID_MORPHPLUGIN, RWSTRING("rwID_MORPHPLUGIN")},
    {rwID_MRMPLUGIN, RWSTRING("rwID_MRMPLUGIN")},
    {rwID_MTEFFECTDICT, RWSTRING("rwID_MTEFFECTDICT")},
    {rwID_MTEFFECTNATIVE, RWSTRING("rwID_MTEFFECTNATIVE")},
    {rwID_MULTITEXPLUGIN, RWSTRING("rwID_MULTITEXPLUGIN")},
    {rwID_NATIVEDATAPLUGIN, RWSTRING("rwID_NATIVEDATAPLUGIN")},
    {rwID_NOHSWORLDPLUGIN, RWSTRING("rwID_NOHSWORLDPLUGIN")},
    {rwID_OPTIMPLUGIN, RWSTRING("rwID_OPTIMPLUGIN")},
    {rwID_PARTICLESPLUGIN, RWSTRING("rwID_PARTICLESPLUGIN")},
    {rwID_PARTICLESYSTEMPLUGIN, RWSTRING("rwID_PARTICLESYSTEMPLUGIN")},
    {rwID_PARTPPPLUGIN, RWSTRING("rwID_PARTPPPLUGIN")},
    {rwID_PATCHPLUGIN, RWSTRING("rwID_PATCHPLUGIN")},
    {rwID_PDSPLUGIN, RWSTRING("rwID_PDSPLUGIN")},
    {rwID_PICKPLUGIN, RWSTRING("rwID_PICKPLUGIN")},
    {rwID_PIPEDS, RWSTRING("rwID_PIPEDS")},
    {rwID_PITEXDICTIONARY, RWSTRING("rwID_PITEXDICTIONARY")},
    {rwID_PITEXDICTIONARYTK, RWSTRING("rwID_PITEXDICTIONARYTK")},
    {rwID_PLANESECT, RWSTRING("rwID_PLANESECT")},
    {rwID_PNGIMAGEPLUGIN, RWSTRING("rwID_PNGIMAGEPLUGIN")},
    {rwID_PRTSTDGLOBALDATA, RWSTRING("rwID_PRTSTDGLOBALDATA")},
    {rwID_PRTSTDPLUGIN, RWSTRING("rwID_PRTSTDPLUGIN")},
    {rwID_PTANKPLUGIN, RWSTRING("rwID_PTANKPLUGIN")},
    {rwID_PVSPLUGIN, RWSTRING("rwID_PVSPLUGIN")},
    {rwID_QUATPLUGIN, RWSTRING("rwID_QUATPLUGIN")},
    {rwID_RANDOMPLUGIN, RWSTRING("rwID_RANDOMPLUGIN")},
    {rwID_RASIMAGEPLUGIN, RWSTRING("rwID_RASIMAGEPLUGIN")},
    {rwID_RAYPLUGIN, RWSTRING("rwID_RAYPLUGIN")},
    {rwID_RAYTRACEPLUGIN, RWSTRING("rwID_RAYTRACEPLUGIN")},
    {rwID_REFINEPLUGIN, RWSTRING("rwID_REFINEPLUGIN")},
    {rwID_RIGHTTORENDER, RWSTRING("rwID_RIGHTTORENDER")},
    {rwID_SKINANIMATION, RWSTRING("rwID_SKINANIMATION")},
    {rwID_SKINFXPLUGIN, RWSTRING("rwID_SKINFXPLUGIN")},
    {rwID_SKINPLUGIN, RWSTRING("rwID_SKINPLUGIN")},
    {rwID_SKYMIPMAPVAL, RWSTRING("rwID_SKYMIPMAPVAL")},
    {rwID_SLERPPLUGIN, RWSTRING("rwID_SLERPPLUGIN")},
    {rwID_SPLINE, RWSTRING("rwID_SPLINE")},
    {rwID_SPLINEPLUGIN, RWSTRING("rwID_SPLINEPLUGIN")},
    {rwID_SPLINEPVSPLUGIN, RWSTRING("rwID_SPLINEPVSPLUGIN")},
    {rwID_STEREOPLUGIN, RWSTRING("rwID_STEREOPLUGIN")},
    {rwID_STQPPPLUGIN, RWSTRING("rwID_STQPPPLUGIN")},
    {rwID_STRING, RWSTRING("rwID_STRING")},
    {rwID_STRUCT, RWSTRING("rwID_STRUCT")},
    {rwID_SYNTHCOREPLUGIN, RWSTRING("rwID_SYNTHCOREPLUGIN")},
    {rwID_TEAM, RWSTRING("rwID_TEAM")},
    {rwID_TEAMDICTIONARY, RWSTRING("rwID_TEAMDICTIONARY")},
    {rwID_TEAMPLUGIN, RWSTRING("rwID_TEAMPLUGIN")},
    {rwID_TEXDICTIONARY, RWSTRING("rwID_TEXDICTIONARY")},
    {rwID_TEXTURE, RWSTRING("rwID_TEXTURE")},
    {rwID_TEXTURENATIVE, RWSTRING("rwID_TEXTURENATIVE")},
    {rwID_TGAIMAGEPLUGIN, RWSTRING("rwID_TGAIMAGEPLUGIN")},
    {rwID_TIFFIMAGEPLUGIN, RWSTRING("rwID_TIFFIMAGEPLUGIN")},
    {rwID_TILERENDPLUGIN, RWSTRING("rwID_TILERENDPLUGIN")},
    {rwID_TLWORLDPLUGIN, RWSTRING("rwID_TLWORLDPLUGIN")},
    {rwID_TOC, RWSTRING("rwID_TOC")},
    {rwID_TOCTOOLKIT, RWSTRING("rwID_TOCTOOLKIT")},
    {rwID_TOONPLUGIN, RWSTRING("rwID_TOONPLUGIN")},
    {rwID_TPLTOOLKIT, RWSTRING("rwID_TPLTOOLKIT")},
    {rwID_UNICODESTRING, RWSTRING("rwID_UNICODESTRING")},
    {rwID_USERDATAPLUGIN, RWSTRING("rwID_USERDATAPLUGIN")},
    {rwID_VCATPLUGIN, RWSTRING("rwID_VCATPLUGIN")},
    {rwID_VRMLANIMPLUGIN, RWSTRING("rwID_VRMLANIMPLUGIN")},
    {rwID_VRMLPLUGIN, RWSTRING("rwID_VRMLPLUGIN")},
    {rwID_WORLD, RWSTRING("rwID_WORLD")},
    {rwID_XBOXMATPLUGIN, RWSTRING("rwID_XBOXMATPLUGIN")}
};

RwUInt32 NumChunkDictEntry = (sizeof(rtNGroupChunkDict) / sizeof(RtNameGroupChunkDictEntry));

/*
 * Tools skeleton.
 */
RwChar                      *g_Pathname = NULL;
RwChar                      *g_Create = NULL;
RwChar                      *g_Move[2] = {NULL, NULL};
RwChar                      *g_Delete = NULL;
RwChar                      *g_Rename[2] = {NULL, NULL};
RwChar                      *g_Copy[2] = {NULL, NULL};
RwInt32                      g_List = 0;

ToolOption ToolOptions[] = {
    { "create", "mk",  " <path>",              ToolParamTypeStringE,       &g_Create,   "Create a new name group", NULL },
    { "move",   "mv",  " <srcpath> <dstpath>", ToolParamTypeString2E,      &g_Move,     "Move a group or an object", NULL },
    { "copy",   "cp",  " <srcpath> <dstpath>", ToolParamTypeString2E,      &g_Copy,     "Copy a group or an object", NULL },
    { "remove", "rm",  " <path>",              ToolParamTypeStringE,       &g_Delete,   "Remove a group or an object", NULL },
    { "rename", "rn",  " <oldpath> <newpath>", ToolParamTypeString2E,      &g_Rename,   "Rename a group", NULL },
    { "list",   "l",   " <depth>",             ToolParamTypeIntE,          &g_List,     "List the label's content to a given depth", NULL },
    { "path",   "p",   " <path>",              ToolParamTypeStringE,       &g_Pathname, "Perform commands relative to a given path", NULL }
};

RwUInt32 NumToolOptions = sizeof(ToolOptions) / sizeof(ToolOption);


/*
 * Private data types for hierachical representation of the stream.
 */
typedef enum RtNameGroupNodeType RtNameGroupNodeType;

typedef struct RtNameGroupNode RtNameGroupNode;
typedef struct RtNameGroupData RtNameGroupData;
typedef struct RtNameGroupLabel RtNameGroupLabel;

typedef struct RtNameGroupGlobals RtNameGroupGlobals;

enum RtNameGroupNodeType
{
    rtNAMEGROUPTYPEUNKNOWN = 0,
    rtNAMEGROUPTYPELABEL,
    rtNAMEGROUPTYPEDATA,
    rtNAMEGROUPTYPEROOT,
    rtNAMEGROUPTYPESUB,

    rtNAMEGROUPTYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

struct RtNameGroupLabel
{
    RwChunkGroup    *group;

    RwInt32         size;
    RwChar          *data;
};

struct RtNameGroupData
{
    RwChunkHeaderInfo       chunkInfo;

    RwChar                  *data;
};

struct RtNameGroupNode
{
    struct RtNameGroupNode  *prev;
    struct RtNameGroupNode  *next;

    struct RtNameGroupNode  *parent;
    struct RtNameGroupNode  *child;

    RtNameGroupNodeType     type;

    RwInt32                 tocIndex;
    RtTOCEntry              tocEntry;

    void                    *data;
};

/*
 * Global container.
 */
struct RtNameGroupGlobals
{
    ToolAPI                 *toolAPI;
    RwChar                  *processFile;

    RwFreeList              *nGroupLabelFList,
                            *nGroupDataFList,
                            *nGroupNodeFList;

    RtNameGroupNode         *nodeRoot,
                            *nodeCurr;

    RwUInt32                nodeCount;

    RwChar                  msg[80];
    RwChar                  buffer[256];

    RtTOC                   *toc;
    RwInt32                 tocIndex;
    RwInt32                 tocOffset;
};

/*
 * Global vars.
 */

RtNameGroupGlobals          rtNGroupVars;


/*
 * Forward declaration of functions.
 */

/*
 * Chunk dictionary utility functions.
 */
static RtNameGroupChunkDictEntry *
RtNameGroupChunkDictGetEntry( RwUInt32 type );

/*
 * Toc utility functions.
 */
static RtNameGroupNode *
RtNameGroupGetTOCEntry( RtNameGroupNode *node, RwChunkHeaderInfo *chunkInfo );

static RtNameGroupNode *
RtNameGroupUpdateTOCOffset( RtNameGroupNode *node, RwInt32 *offset );

static RtNameGroupNode *
RtNameGroupUpdateTOC( RtNameGroupNode *nodeHead );

/*
 * Path utility functions.
 */
static RwBool
RtNameGroupNameIsValid( RwChar *path );

static RwChar *
RtNameGroupPathSplitHead( RwChar *path, RwChar **tail );

static RwChar *
RtNameGroupPathSplitTail( RwChar *path,  RwChar **tail );

static RwChar *
RtNameGroupPathJoin( RwChar *head, RwChar *tail );

static RwBool
RtNameGroupPathIsRelative( RwChar *path, RwChar **head );

static RwBool
RtNameGroupPathIsValid( RwChar *path );

static RwInt32
RtNameGroupPathIsPos( RwChar *path );

#if UNUSED

static RwBool
RtNameGroupPathIsSingle( RwChar *path );

#endif /* UNUSED */

static RwBool
RtNameGroupPathIsRoot( RwChar *path );

/*
 * NameSpace node utility functions.
 */
static RtNameGroupNode *
RtNameGroupList( RtNameGroupNode *nodeHead, RwChar *prefix, RwBool recurse );

static RtNameGroupNode *
RtNameGroupFindNode( RtNameGroupNode *nodeHead, RwChar *name, RwBool recurse );

static RtNameGroupNode *
RtNameGroupSetParent( RtNameGroupNode *nodeHead, RtNameGroupNode *parent);

static RwBool
RtNameGroupFindParent( RtNameGroupNode *node, RtNameGroupNode *parent );


/*
 * Create, Destroy and Copy functions.
 */
static RtNameGroupLabel *
RtNameGroupRenameLabel( RtNameGroupLabel *nGroupLabel, RwChar *name );

static RtNameGroupLabel *
RtNameGroupCreateLabel( void );

static RtNameGroupData *
RtNameGroupCreateData( void );

static RtNameGroupNode *
RtNameGroupCreateNode( void );

static RwBool
RtNameGroupDestroyLabel( RtNameGroupLabel *nGroupLabel );

static RwBool
RtNameGroupDestroyData( RtNameGroupData *nGroupData );

static RwBool
RtNameGroupDestroyNode( RtNameGroupNode *node );

static RtNameGroupLabel *
RtNameGroupCopyLabel( RtNameGroupLabel *nGroupLabel );

static RtNameGroupData *
RtNameGroupCopyData( RtNameGroupData *nGroupData );

static RwBool
RtNameGroupDestroy( RtNameGroupNode *nodeHead );

static RtNameGroupNode *
RtNameGroupCopy( RtNameGroupNode *nodeHead, RtNameGroupNode *parent, RwBool list );


/*
 * Path create destroy functions.
 */
static RtNameGroupNode *
RtNameGroupCreatePath( RtNameGroupNode *nodeHead, RwChar *pathname, RtNameGroupNode *parent );

static RtNameGroupNode *
RtNameGroupFindPath( RtNameGroupNode *nodeHead, RwChar *pathname );

/*
 * Add and remove functions.
 */
static RtNameGroupNode *
RtNameGroupAddNodeBefore( RtNameGroupNode *nodeHead, RtNameGroupNode *node, RtNameGroupNode *newNode );

static RtNameGroupNode *
RtNameGroupAddNodeAfter( RtNameGroupNode *nodeHead, RtNameGroupNode *node, RtNameGroupNode *newNode );

static RtNameGroupNode *
RtNameGroupAddNodePos( RtNameGroupNode *nodeHead, RwInt32 pos, RtNameGroupNode *newNode );

static RtNameGroupNode *
RtNameGroupRemoveNode( RtNameGroupNode *nodeHead, RtNameGroupNode *node );

static RtNameGroupNode *
RtNameGroupAddNodePath( RtNameGroupNode *nodeHead, RwChar *path, RtNameGroupNode *newNode, RtNameGroupNode *parent );

/*
 * Stream read and write functions.
 */
static RtNameGroupLabel *
RtNameGroupStreamReadStartLabel( RwStream *stream, RwChunkHeaderInfo *chunkInfo);

static RtNameGroupLabel *
RtNameGroupStreamReadEndLabel( RwStream *stream, RwChunkHeaderInfo *chunkInfo, RtNameGroupLabel *nGroupLabel );


static RtNameGroupData *
RtNameGroupStreamReadData( RwStream *stream, RwChunkHeaderInfo *chunkInfo);

static RtNameGroupLabel *
RtNameGroupStreamWriteStartLabel( RwStream *stream, RtNameGroupLabel *nGroupLabel );

static RtNameGroupLabel *
RtNameGroupStreamWriteEndLabel( RwStream *stream, RtNameGroupLabel *nGroupLabel );

static RtNameGroupData *
RtNameGroupStreamWriteData( RwStream *stream, RtNameGroupData *nGroupData );

static RtNameGroupNode *
RtNameGroupStreamRead( RwStream *stream, RtNameGroupNode *parent, RwInt32 *pos );

static RtNameGroupNode *
RtNameGroupStreamWrite( RwStream *stream, RtNameGroupNode *nodeHead );

/*
 * Top level tool functions.
 */
static RwInt32
StreamEditCreate( ToolAPI *api, RwChar *path );

static RwInt32
StreamEditDelete( ToolAPI *api, RwChar *path );

static RwInt32
StreamEditCopy( ToolAPI *api, RwChar *inPath, RwChar *outPath );

static RwInt32
StreamEditMove( ToolAPI *api, RwChar *inPath, RwChar *outPath );

static RwInt32
StreamEditRename( ToolAPI *api, RwChar *inPath, RwChar *outPath );

static RwInt32
StreamEditStartup( ToolAPI *api, RwChar *currentPath );

static RwInt32
StreamEditProcessFile( ToolAPI *api, RwChar *inFilename );

static RwInt32
StreamEditShutdown( ToolAPI *api );

/*****************************************************************************
 *
 *
 *
 *****************************************************************************/

/*
 * Chunk dictionary utility functions.
 */
static RtNameGroupChunkDictEntry *
RtNameGroupChunkDictGetEntry( RwUInt32 type )
{
    RtNameGroupChunkDictEntry       *entry;
    RwUInt32                        i;

    entry = rtNGroupChunkDict;

    for (i = 0; i < NumChunkDictEntry; i++, entry++)
    {
        if (entry->type == type)
        {
            return entry;
        }
    }

    return NULL;
}

/*
 * Query the toc for an entry for this chunk.
 */
static RtNameGroupNode *
RtNameGroupGetTOCEntry( RtNameGroupNode *node, RwChunkHeaderInfo *chunkInfo )
{
    if (rtNGroupVars.toc)
    {
        if (chunkInfo->type == (RwUInt32) rtNGroupVars.toc->entry[rtNGroupVars.tocIndex].id)
        {
            node->tocEntry = rtNGroupVars.toc->entry[rtNGroupVars.tocIndex];
            node->tocIndex = rtNGroupVars.tocIndex;

            rtNGroupVars.tocIndex++;
        }
        else
        {
            node->tocIndex = -1;
        }
    }

    return (node);
}

/*
 * Update all offsets for the Toc.
 */
static RtNameGroupNode *
RtNameGroupUpdateTOCOffset( RtNameGroupNode *nodeHead, RwInt32 *offset )
{
    RtNameGroupNode             *node;
    const RtNameGroupLabel      *nGroupLabel;
    const RtNameGroupData       *nGroupData;

    node = nodeHead;

    if (node)
    {
        /* Main process loop. */
        do
        {
            /* Update the offset in the toc. */
            if (node->tocIndex >= 0)
                node->tocEntry.offset = *offset;

            /* Check the type. */
            if (node->type == rtNAMEGROUPTYPELABEL)
            {
                nGroupLabel = (const RtNameGroupLabel *) node->data;
                *offset += RwChunkGroupStreamGetSize(nGroupLabel->group) + rwCHUNKHEADERSIZE;

                /* Update the offset for the children. */
                if (node->child)
                    RtNameGroupUpdateTOCOffset(node->child, offset);

                /* Label stop marker. */
                *offset += rwCHUNKHEADERSIZE;
            }
            else if (node->type == rtNAMEGROUPTYPEDATA)
            {
                /* Data chunk. */
                nGroupData = (const RtNameGroupData *) node->data;
                *offset += nGroupData->chunkInfo.length + rwCHUNKHEADERSIZE;
            }
            else
            {
                /* No updates. */
            }

            node = node->next;

        } while (node != nodeHead);
    }

    return (nodeHead);
}

/*
 * Recreate the toc. This just copies the chunk's old toc entry into the toc,
 * taking into account any chunk re-arrangement and offsets. No new TOC entry
 * are created.
 */
static RtNameGroupNode *
RtNameGroupUpdateTOC( RtNameGroupNode *nodeHead )
{
    RtNameGroupNode             *node;

    node = nodeHead;

    if (node)
    {
        /* Main process loop. */
        do
        {
            /* Update the entry in the toc. */
            if (node->tocIndex >= 0)
            {
                rtNGroupVars.toc->entry[rtNGroupVars.tocIndex] = node->tocEntry;

                rtNGroupVars.tocIndex++;
            }

            /* Check the type. */
            if (node->type == rtNAMEGROUPTYPELABEL)
            {
                /* Update the offset for the children. */
                if (node->child)
                    RtNameGroupUpdateTOC(node->child);
            }

            node = node->next;

        } while (node != nodeHead);
    }

    return (nodeHead);
}

/*
 *  Validates a name. It mus be alphanumeric.
 */
static RwBool
RtNameGroupNameIsValid( RwChar *path )
{
    RwBool      result, i, l, alphaFound, digitFound;
    RwChar      *marker;

    result = TRUE;

    alphaFound = FALSE;
    digitFound = FALSE;

    if (path)
    {
        marker = path;
        l = strlen(path);
        i = 0;

        while ((result) && (i < l))
        {
            /* Check if is an alphabet, numeric or others. */
            if (isalpha(*marker))
            {
                alphaFound = TRUE;
            }
            else if (isdigit(*marker))
            {
                digitFound = TRUE;
            }
            else
            {
                /* Non alpha numeric found. */
                result = FALSE;
            }

            marker++;
            i++;
        }

        if (alphaFound == FALSE)
            result = FALSE;
    }
    else
    {
        /* Empty path is not valid. */
        result = FALSE;
    }


    return (result);
}

/*
 * Split a path. The first element is seperated from the path.
 */
static RwChar *
RtNameGroupPathSplitHead( RwChar *path,  RwChar **tail )
{
    RwChar      *marker, *head;

    /* First check if it is relative. */
    RtNameGroupPathIsRelative(path, &head);

    /* Look for the first path seperator. */
    marker = strchr(head, rtNAMEGROUPPATHSEPARATOR);
    if (marker)
    {
        /* Split the path by inserting NULL in between. */
        *marker = 0;
        *tail = marker + 1;
    }
    else
    {
        /* No tail element found. */
        *tail = NULL;
    }

    return (path);
}

/*
 * Split a path. The last element is seperated from the path.
 */
static RwChar *
RtNameGroupPathSplitTail( RwChar *path, RwChar **tail )
{
    RwChar      *marker, *head;

    /* First check if it is relative. */
    RtNameGroupPathIsRelative(path, &head);

    /* Look for the last path separator. */
    marker = strrchr(head, rtNAMEGROUPPATHSEPARATOR);
    if (marker)
    {
        /* Split the path by inserting NULL in between. */
        *marker = 0;
        *tail = marker + 1;
    }
    else
    {
        /* No tail element found. */
        *tail = NULL;
    }

    return (path);
}

/*
 * Joins two path.
 */
static RwChar *
RtNameGroupPathJoin( RwChar *head, RwChar *tail )
{
    /* Paths are split by inserting a NULL in between. This restores the
     * split path by reinsert the path seperator. */

    if (tail)
        *(tail - 1) = rtNAMEGROUPPATHSEPARATOR;

    return (head);
}

/*
 * Check if a path is absolute or relative, returning a pointer to the first
 * pathname.
 */
static RwBool
RtNameGroupPathIsRelative( RwChar *path, RwChar **head )
{
    RwBool      result;

    /* First element is a path seperator, so treat as absolute. */
    if (path[0] == rtNAMEGROUPPATHSEPARATOR)
    {
        result = FALSE;
    }

    if (head)
    {
        if (result)
        {
            *head = path;
        }
        else
        {
            *head = path + 1;
        }
    }

    return (result);
}

/*
 *  Validates a path. The following are applied.
 *
 *      Double path separators are illegal.
 *      Names may only contain alpha numeric strings.
 *      Numeric only name is only allowed as the last element in the path.
 */
static RwBool
RtNameGroupPathIsValid( RwChar *path )
{
    RwBool      result, i, l, alphaFound, digitFound, minusFound;
    RwChar      *marker, *subPath;

    result = TRUE;

    alphaFound = FALSE;
    digitFound = FALSE;
    minusFound = FALSE;

    if (path)
    {
        marker = path;
        subPath = path;
        l = strlen(path);
        i = 0;

        while ((result) && (i < l))
        {
            if (*marker == rtNAMEGROUPPATHSEPARATOR)
            {
                if ((marker == subPath) && (i > 0))
                {
                    /* Double path seperator found. */
                    result = FALSE;
                }

                if ((alphaFound == FALSE) && (i > 0))
                {
                    /* Numeric only value found in path. */
                    result = FALSE;
                }

                alphaFound = FALSE;
                digitFound = FALSE;
                minusFound = FALSE;

                /* Move the start of the subpath to just after the path separator. */
                subPath = marker + 1;
            }
            else
            {
                /* Check if is an alphabet, numeric or others. */
                if (isalpha(*marker))
                {
                    alphaFound = TRUE;
                }
                else if (isdigit(*marker))
                {
                    digitFound = TRUE;
                }
                else
                {
                    if (*marker == '-')
                    {
                        /* Minus sign must be the first element to support -ve numbers. */
                        if ((alphaFound) || (digitFound) || (minusFound))
                        {
                            /* Minus sign is not the first element. Raise error and stop. */
                            result = FALSE;
                        }
                        else
                        {
                            minusFound = TRUE;
                        }
                    }
                    else
                    {
                        /* Non alpha numeric found. */
                        result = FALSE;
                    }
                }
            }

            marker++;
            i++;
        }
    }
    else
    {
        /* Empty path is not valid. */
        result = FALSE;
    }

    return (result);
}

/*
 * Checks if a name is a path or an item number, -1 if is a label.
 */
static RwInt32
RtNameGroupPathIsPos( RwChar *path )
{
    RwChar      *pathHead, *pathTail;
    RwInt32     i;

    /* Absolute or relative ? */
    RtNameGroupPathIsRelative(path, &pathHead);

    /* Split the tail. */
    RtNameGroupPathSplitTail(pathHead, &pathTail);

    if (pathTail)
    {
        if (sscanf(pathTail, "%d", &i) == 0)
            i = -1;

        RtNameGroupPathJoin(pathHead, pathTail);
    }
    else
    {
        if (sscanf(pathHead, "%d", &i) == 0)
            i = -1;
    }

    return (i);
}

/*
 * Checks if the path is a singular or a hierarchical.
 */

#ifdef UNUSED

static RwBool
RtNameGroupPathIsSingle( RwChar *path )
{
    RwBool      result;
    RwChar      *marker;

    result = FALSE;

    marker = strchr(path, rtNAMEGROUPPATHSEPARATOR);
    if (marker)
        result = TRUE;

    return (result);
}

#endif /* UNUSED */

/*
 * Checks if the path is the root.
 */
static RwBool
RtNameGroupPathIsRoot( RwChar *path )
{
    RwBool      result;

    result = FALSE;

    if (strcmp(path, rtNAMEGROUPPATHROOT) == 0)
        result = TRUE;

    return (result);
}

/*
 * Produce a tree view of the stream.
 */
static RtNameGroupNode *
RtNameGroupList( RtNameGroupNode *nodeHead, RwChar *prefix, RwInt32 level )
{
    RwUInt32                    l;
    RwChar                      *subprefix;
    RtNameGroupNode             *node;
    RtNameGroupLabel            *nGroupLabel;
    RtNameGroupData             *nGroupData;
    RtNameGroupChunkDictEntry   *nGroupDictEntry;

    /* Simple debugging to check the tree is correct by traversing and counting. */

    if (nodeHead)
    {
        node = nodeHead;

        /* Find the length of the prefix string. */
        l = rwstrlen(prefix);
        subprefix = &prefix[l];

        /* Main process loop. */
        do
        {
            /* Check if is a name space start node. */
            if (node->type == rtNAMEGROUPTYPELABEL)
            {
                nGroupLabel = (RtNameGroupLabel *) node->data;

                if (level < g_List)
                {
                    /* Display the label info. */
                    if ((nGroupLabel) && (nGroupLabel->group->name))
                    {
                        sprintf(subprefix, "%s %s\n",
                            rtNAMEGROUPVIEWLABELOPENTOKEN, nGroupLabel->group->name);
                    }
                    else
                    {
                        sprintf(subprefix, "%s\n", rtNAMEGROUPVIEWLABELOPENTOKEN);
                    }

                    ToolAPIReport(rtNGroupVars.toolAPI, prefix);

                    /* Display the children. */
                    /* Check if next item is last. */
                    if (node->next == nodeHead)
                    {
                        sprintf(subprefix, "%s", rtNAMEGROUPVIEWBLANKTOKEN);
                    }
                    else
                    {
                        sprintf(subprefix, "%s", rtNAMEGROUPVIEWBRANCHTOKEN);
                    }

                    RtNameGroupList(node->child, prefix, (level + 1));
                }
                else
                {
                    /* Display the label info. */
                    if ((nGroupLabel) && (nGroupLabel->group->name))
                    {
                        sprintf(subprefix, "%s %s\n", rtNAMEGROUPVIEWLABELCLOSETOKEN, nGroupLabel->group->name);
                    }
                    else
                    {
                        sprintf(subprefix, "%s\n", rtNAMEGROUPVIEWLABELCLOSETOKEN);
                    }

                    ToolAPIReport(rtNGroupVars.toolAPI, prefix);
                }
            }
            else if ((node->type == rtNAMEGROUPTYPEDATA) ||
                     (node->type == rtNAMEGROUPTYPESUB))
            {
                nGroupData = (RtNameGroupData *) node->data;

                nGroupDictEntry = RtNameGroupChunkDictGetEntry(nGroupData->chunkInfo.type);

                if (node->child)
                {
                    if (level < g_List)
                    {
                        /* Display the data info. */
                        if (nGroupDictEntry)
                        {
                            /* Display the chunk info. */
                            sprintf(subprefix, "%s Type : %s Ver : 0x%08x Size : 0x%08x\n",
                                rtNAMEGROUPVIEWLABELOPENTOKEN,
                                nGroupDictEntry->name, nGroupData->chunkInfo.version, nGroupData->chunkInfo.length);
                        }
                        else
                        {
                            /* Display the chunk info. */
                            sprintf(subprefix, "%s Type : 0x%08x Ver : 0x%08x Size : 0x%08x\n",
                                rtNAMEGROUPVIEWLABELOPENTOKEN,
                                nGroupData->chunkInfo.type, nGroupData->chunkInfo.version, nGroupData->chunkInfo.length);
                        }

                        ToolAPIReport(rtNGroupVars.toolAPI, prefix);

                        /* Display the children. */
                        /* Check if next item is last. */
                        if (node->next == nodeHead)
                        {
                            sprintf(subprefix, "%s", rtNAMEGROUPVIEWBLANKTOKEN);
                        }
                        else
                        {
                            sprintf(subprefix, "%s", rtNAMEGROUPVIEWBRANCHTOKEN);
                        }

                        RtNameGroupList(node->child, prefix, (level + 1));
                    }
                    else
                    {
                        /* Display the data info. */
                        if (nGroupDictEntry)
                        {
                            /* Display the chunk info. */
                            sprintf(subprefix, "%s Type : %s Ver : 0x%08x Size : 0x%08x\n",
                                rtNAMEGROUPVIEWLABELCLOSETOKEN,
                                nGroupDictEntry->name, nGroupData->chunkInfo.version, nGroupData->chunkInfo.length);
                        }
                        else
                        {
                            /* Display the chunk info. */
                            sprintf(subprefix, "%s Type : 0x%08x Ver : 0x%08x Size : 0x%08x\n",
                                rtNAMEGROUPVIEWLABELCLOSETOKEN,
                                nGroupData->chunkInfo.type, nGroupData->chunkInfo.version, nGroupData->chunkInfo.length);
                        }

                        ToolAPIReport(rtNGroupVars.toolAPI, prefix);
                    }
                }
                else
                {
                    if (nGroupDictEntry)
                    {
                        /* Display the chunk info. */
                        sprintf(subprefix, "%s Type : %s Ver : 0x%08x Size : 0x%08x\n",
                            rtNAMEGROUPVIEWCHUNKTOKEN,
                            nGroupDictEntry->name, nGroupData->chunkInfo.version, nGroupData->chunkInfo.length);
                    }
                    else
                    {
                        /* Display the chunk info. */
                        sprintf(subprefix, "%s Type : 0x%08x Ver : 0x%08x Size : 0x%08x\n",
                            rtNAMEGROUPVIEWCHUNKTOKEN,
                            nGroupData->chunkInfo.type, nGroupData->chunkInfo.version, nGroupData->chunkInfo.length);
                    }

                    ToolAPIReport(rtNGroupVars.toolAPI, prefix);
                }
            }
            else
            {
                /* Something is wrong. */
            }

            node = node->next;
        } while (node != nodeHead);

        /* Restore the prefix string. */
        prefix[l] = 0;
    }

    return (nodeHead);
}

/*
 * Search for a node in the hierachy. Children are searched before siblings.
 */
static RtNameGroupNode *
RtNameGroupFindNode( RtNameGroupNode *nodeHead, RwChar *name, RwBool recurse )
{
    RwInt32             pos, count;
    RtNameGroupNode     *node, *nodeFound;
    RtNameGroupLabel    *nGroupLabel;

    nodeFound = NULL;

    if ((nodeHead) && (name))
    {
        node = nodeHead;

        /* Search for a label or item ? */
        pos = RtNameGroupPathIsPos(name);

        if (pos < 0)
        {
            /* Searching for a label. */

            /* Main process loop. */
            do
            {
                /* Check if is a name space start node. */
                if (node->type == rtNAMEGROUPTYPELABEL)
                {
                    nGroupLabel = (RtNameGroupLabel *) node->data;

                    if ((nGroupLabel) && (nGroupLabel->group->name))
                    {
                        /* Check if the name matches. */
                        if (rwstrcmp(name, nGroupLabel->group->name) == 0)
                        {
                            /* Yes, so stop */
                            nodeFound = node;
                        }
                        else
                        {
                            /* No, so search the children ? */
                            if (recurse)
                                nodeFound = RtNameGroupFindNode(node->child, name, recurse);
                        }
                    }
                }

                node = node->next;
            } while ((node != nodeHead) && (nodeFound == NULL));
        }
        else
        {
            /* Searching for the n'th item. */

            /* Main process loop. */
            node = nodeHead;
            count = rtNAMEGROUPPOSSTART;
            do
            {
                if (count == pos)
                {
                    /* Found it. */
                    nodeFound = node;
                }

                count++;

                node = node->next;
            } while ((node != nodeHead) && (nodeFound == NULL));
        }
    }

    return (nodeFound);
}

/*
 * Set the parent pointer for all the nodes in the list.
 */
static RtNameGroupNode *
RtNameGroupSetParent( RtNameGroupNode *nodeHead, RtNameGroupNode *parent)
{
    RtNameGroupNode         *node;

    if (nodeHead)
    {
        node = nodeHead;

        /* Main process loop. */
        do
        {
            node->parent = parent;

            node = node->next;
        } while (node != nodeHead);
    }

    return (nodeHead);
}

/*
 * Check if a parent exist in the parent back pointer.
 */
static RwBool
RtNameGroupFindParent( RtNameGroupNode *node, RtNameGroupNode *parent)
{
    RwBool      result;

    result = FALSE;

    if ((node) && (parent))
    {
        if (node == parent)
        {
            result = TRUE;
        }
        else if (node->type == rtNAMEGROUPTYPEROOT)
        {
            /* Reach the root node. */
            result = FALSE;
        }
        else
        {
            if (node->parent == parent)
                result = TRUE;
            else
            {
                result = RtNameGroupFindParent(node->parent, parent);
            }
        }
    }

    return (result);
}

/*
 * Rename a label.
 */
static RtNameGroupLabel *
RtNameGroupRenameLabel( RtNameGroupLabel *nGroupLabel, RwChar *name )
{
    RtNameGroupLabel    *result;

    result = NULL;

    if ((nGroupLabel) && (name))
    {
        /* Okay to rename. */
        RwChunkGroupSetName(nGroupLabel->group, name);

        result = nGroupLabel;
    }

    return (result);
}

/*
 * Create an empty name space start label.
 */
static RtNameGroupLabel *
RtNameGroupCreateLabel( void )
{
    RtNameGroupLabel       *nGroupLabel;

    nGroupLabel = (RtNameGroupLabel *)
        RwFreeListAlloc(rtNGroupVars.nGroupLabelFList, 0);

    if (nGroupLabel)
    {
        nGroupLabel->group = RwChunkGroupCreate();
        nGroupLabel->size = 0;
        nGroupLabel->data = NULL;
    }

    return (nGroupLabel);
}

/*
 * Destroy a single name space start label data type.
 */
static RwBool
RtNameGroupDestroyLabel( RtNameGroupLabel *nGroupLabel )
{
    if (nGroupLabel)
    {
        if (nGroupLabel->group)
            RwChunkGroupDestroy(nGroupLabel->group);
        nGroupLabel->group = NULL;

        if (nGroupLabel->data)
            RwFree(nGroupLabel->data);
        nGroupLabel->data = NULL;

        nGroupLabel->size = 0;

        RwFreeListFree(rtNGroupVars.nGroupLabelFList, nGroupLabel);
    }

    return (TRUE);
}

/*
 * Copy a start label.
 */
static RtNameGroupLabel *
RtNameGroupCopyLabel( RtNameGroupLabel *nGroupLabel )
{
    RwBool                      result;
    RwUInt32                    size;
    RtNameGroupLabel            *newnGroupLabel;

    result = FALSE;

    newnGroupLabel = NULL;

    if (nGroupLabel)
    {
        newnGroupLabel = RtNameGroupCreateLabel();

        /* Create a new start label */
        if (newnGroupLabel)
        {
            /* Check if the name needs copying. */
            if (nGroupLabel->group)
            {
                RwChunkGroupSetName(newnGroupLabel->group, nGroupLabel->group->name);
            }

            /* Check if the data needs copying. */
            if (result && (nGroupLabel->data))
            {
                size = nGroupLabel->size;

                newnGroupLabel->data = RwMalloc(size, 0);

                if (newnGroupLabel->data)
                {
                    /* Copy the data. */
                    memcpy(newnGroupLabel->data, nGroupLabel->data, size);
                    newnGroupLabel->size = size;

                    result = TRUE;
                }
                else
                {
                    /* Memory failure. Raise error and stop. */
                    result = FALSE;
                }
            }
        }
        else
        {
            /* Memory failure. Raise error and stop. */
            result = FALSE;
        }
    }

    if (!result)
    {
        if (newnGroupLabel)
            RtNameGroupDestroyLabel(newnGroupLabel);

        newnGroupLabel = NULL;
    }

    return (newnGroupLabel);
}

/*
 * Create an empty name space data.
 */
static RtNameGroupData *
RtNameGroupCreateData( void )
{
    RtNameGroupData         *nGroupData;

    nGroupData = (RtNameGroupData *)
        RwFreeListAlloc(rtNGroupVars.nGroupDataFList, 0);

    if (nGroupData)
    {
        nGroupData->data = NULL;
    }

    return (nGroupData);
}

/*
 * Destroy a single name space data data type.
 */
static RwBool
RtNameGroupDestroyData( RtNameGroupData *nGroupData )
{
    if (nGroupData)
    {
        if (nGroupData->data)
            RwFree(nGroupData->data);

        nGroupData->data = NULL;

        RwFreeListFree(rtNGroupVars.nGroupDataFList, nGroupData);
    }

    return (TRUE);
}

/*
 * Copy a name space data type.
 */
static RtNameGroupData *
RtNameGroupCopyData( RtNameGroupData *nGroupData )
{
    RwBool                  result;
    RtNameGroupData         *newnGroupData;

    result = FALSE;

    newnGroupData = NULL;

    if (nGroupData)
    {
        newnGroupData = RtNameGroupCreateData();

        /* Create a new Data label */
        if (newnGroupData)
        {
            /* Check if data needs copying. */
            if (nGroupData->data)
            {
                newnGroupData->data = RwMalloc(nGroupData->chunkInfo.length, 0);

                if (newnGroupData->data)
                {
                    /* Copy the data. */
                    memcpy(newnGroupData->data, nGroupData->data, nGroupData->chunkInfo.length);

                    newnGroupData->chunkInfo = nGroupData->chunkInfo;

                    result = TRUE;
                }
                else
                {
                    /* Memory failure. Raise error and stop. */
                    result = FALSE;
                }
            }
        }
        else
        {
            /* Memory failure. Raise error and stop. */
            result = FALSE;
        }
    }

    if (!result)
    {
        if (newnGroupData)
            RtNameGroupDestroyData(newnGroupData);

        newnGroupData = NULL;
    }

    return (newnGroupData);
}

/*
 * Create an empty name space node.
 */
static RtNameGroupNode *
RtNameGroupCreateNode( void )
{
    RtNameGroupNode         *node;

    node = (RtNameGroupNode *)
        RwFreeListAlloc(rtNGroupVars.nGroupNodeFList, 0);

    if (node)
    {
        node->prev = node;
        node->next = node;

        node->child = NULL;
        node->parent = NULL;

        node->type = rtNAMEGROUPTYPEUNKNOWN;
        node->data = NULL;
        node->tocIndex = -1;
    }

    return (node);
}

/*
 * Destroy a single name space node data type.
 */
static RwBool
RtNameGroupDestroyNode( RtNameGroupNode *node )
{
    if (node)
    {
        /* Destroy the node. */
        node->prev = NULL;
        node->next = NULL;
        node->child = NULL;
        node->parent = NULL;
        node->type = rtNAMEGROUPTYPEUNKNOWN;
        node->data = NULL;

        RwFreeListFree(rtNGroupVars.nGroupNodeFList, node);
    }

    return (TRUE);
}

/*
 * Destroy all name space nodes in list, including any children.
 */
static RwBool
RtNameGroupDestroy( RtNameGroupNode *nodeHead )
{
    RtNameGroupNode     *node, *nodeNext;

    if (nodeHead)
    {
        node = nodeHead;

        /* Main process loop. */
        do
        {
            nodeNext = node->next;

            /* Check the type. */
            if (node->type == rtNAMEGROUPTYPELABEL)
            {
                /* Name space start */
                RtNameGroupDestroyLabel((RtNameGroupLabel *) node->data);
            }
            else if ((node->type == rtNAMEGROUPTYPEDATA) ||
                     (node->type == rtNAMEGROUPTYPESUB))
            {
                /* Data chunk. */
                RtNameGroupDestroyData((RtNameGroupData *) node->data);
            }
            else
            {
                /* Empty data. */
            }

            /* Destroy any children */
            if (node->child)
                RtNameGroupDestroy(node->child);

            /* Destroy the node. */
            RtNameGroupDestroyNode(node);

            node = nodeNext;
        } while (node != nodeHead);
    }

    return (TRUE);
}

/*
 * Copy all name space nodes in list, including any children.
 */
static RtNameGroupNode *
RtNameGroupCopy( RtNameGroupNode *nodeHead, RtNameGroupNode *newNodeParent, RwBool list )
{
    RwBool              result;
    RtNameGroupNode     *node, *newNode, *newNodeHead, *newNodePrev;

    newNodeHead = NULL;
    newNodePrev = NULL;

    result = TRUE;

    if (nodeHead)
    {
        node = nodeHead;

        /* Main process loop. */
        do
        {
            /* Create a new node. */
            newNode = RtNameGroupCreateNode();

            if (newNode)
            {
                /* Set up parent and add it to the new list. */
                newNode->parent = newNodeParent;
                newNodeHead = RtNameGroupAddNodeAfter(newNodeHead, newNodePrev, newNode);

                newNode->type = node->type;

                /* Check the type. */
                if (node->type == rtNAMEGROUPTYPELABEL)
                {
                    /* Name space start */
                    if (node->data)
                    {
                        newNode->data = RtNameGroupCopyLabel(node->data);
                    }
                }
                else if ((node->type == rtNAMEGROUPTYPEDATA) ||
                         (node->type == rtNAMEGROUPTYPESUB))
                {
                    /* Name space data */
                    if (node->data)
                    {
                        newNode->data = RtNameGroupCopyData(node->data);
                    }
                }
                else
                {
                    /* No data to copy. */
                }

                if ((newNode->data == NULL) && (node->data != NULL))
                {
                    /* Memory failure. Raise error and stop. */
                    result = FALSE;
                }

                /* Copy any children of this node. */
                if ((result) && (node->child))
                {
                    newNode->child = RtNameGroupCopy(node->child, newNode, TRUE);

                    if (node->child == NULL)
                    {
                        /* Memory failure. Raise error and stop. */
                        result = FALSE;
                    }
                }
            }
            else
            {
                /* Memory failure. Raise error and stop. */
                result = FALSE;
            }

            newNodePrev = newNode;

            node = node->next;

        } while ((node != nodeHead) && (result) && (list));
    }

    if (!result)
    {
        if (newNodeHead)
            RtNameGroupDestroy(newNodeHead);

        newNodeHead = NULL;
    }

    return (newNodeHead);
}

/*
 * Add a new node list to an existing list before the insertion point.
 */
static RtNameGroupNode *
RtNameGroupAddNodeBefore( RtNameGroupNode *nodeHead, RtNameGroupNode *node, RtNameGroupNode *newNode )
{

    RtNameGroupNode     *newNodeHead, *nodePrev;

    if (nodeHead == NULL)
    {
        /* The list is empty, so just return the newNode. */
        newNodeHead = newNode;
    }
    else
    {
        /* Update the parent pointer. */
        RtNameGroupSetParent(newNode, nodeHead->parent);

        /* Insert before the node. */
        nodePrev = node->prev;

        nodePrev->next = newNode;
        node->prev = newNode->prev;

        newNode->prev->next = node;
        newNode->prev = nodePrev;

        if (node == nodeHead)
            newNodeHead = newNode;
        else
            newNodeHead = nodeHead;
    }

    return (newNodeHead);
}

/*
 * Add a new node list to an existing list after the insertion point.
 */
static RtNameGroupNode *
RtNameGroupAddNodeAfter( RtNameGroupNode *nodeHead, RtNameGroupNode *node, RtNameGroupNode *newNode )
{
    RtNameGroupNode     *newNodeHead, *nodeNext;

    if (nodeHead == NULL)
    {
        /* The list is empty, so just return the newNode. */
        newNodeHead = newNode;
    }
    else
    {
        /* Update the parent pointer. */
        RtNameGroupSetParent(newNode, nodeHead->parent);

        /* Insert after the node. */
        nodeNext = node->next;

        node->next = newNode;
        nodeNext->prev = newNode->prev;

        newNode->prev->next = nodeNext;
        newNode->prev = node;

        newNodeHead = nodeHead;
    }

    return (newNodeHead);
}

/*
 * Add a new node list to an existing list at the given position.
 */
static RtNameGroupNode *
RtNameGroupAddNodePos( RtNameGroupNode *nodeHead, RwInt32 pos, RtNameGroupNode *newNode )
{
    RtNameGroupNode     *newNodeHead, *nodeFound, *node;
    RwInt32             count;

    if (nodeHead == NULL)
    {
        /* The list is empty, so just return the newNode. */
        newNodeHead = newNode;
    }
    else
    {
        /* Update the parent pointer. */
        RtNameGroupSetParent(newNode, nodeHead->parent);

        /* Find the insertion point. -1 denotes insert at the end. */
        nodeFound = NULL;
        node = nodeHead;
        if (pos >= 0)
        {
            count = rtNAMEGROUPPOSSTART;
            do
            {
                if (pos == count)
                {
                    nodeFound = node;
                }

                count++;
                node = node->next;
            } while ((node != nodeHead) && (nodeFound == NULL));
        }

        if (nodeFound)
        {
            newNodeHead = RtNameGroupAddNodeBefore(nodeHead, nodeFound, newNode);
        }
        else
        {
            newNodeHead = RtNameGroupAddNodeAfter(nodeHead, node->prev, newNode);
        }
    }

    return (newNodeHead);
}

/*
 * Remove a single node from an exising list.
 */
static RtNameGroupNode *
RtNameGroupRemoveNode( RtNameGroupNode *nodeHead, RtNameGroupNode *node )
{
    RtNameGroupNode     *newNodeHead, *nodePrev, *nodeNext;

    if (node->next == node)
    {
        /* Node is the only element in the list. */
        newNodeHead = NULL;
    }
    else
    {
        /* Remove the node from the list. */
        nodePrev = node->prev;
        nodeNext = node->next;

        node->prev = node;
        node->next = node;

        nodePrev->next = nodeNext;
        nodeNext->prev = nodePrev;

        if (node == nodeHead)
            newNodeHead = nodeNext;
        else
            newNodeHead = nodeHead;

        /* Update the parent pointer. */
        RtNameGroupSetParent(node, NULL);
    }

    return (newNodeHead);
}

/*
 * Add a new node list to an existing list at the given position.
 */
static RtNameGroupNode *
RtNameGroupAddNodePath( RtNameGroupNode *nodeHead, RwChar *path, RtNameGroupNode *newNode, RtNameGroupNode *parent )
{
    RwChar              *pathTail;
    RwInt32             pos;
    RtNameGroupNode     *newNodeHead, *nodeFound;
    RtNameGroupLabel    *nGroupLabel;

    if (nodeHead == NULL)
    {
        /* The list is empty, so just return the newNode. */
        RtNameGroupSetParent(newNode, parent);
        newNodeHead = newNode;
    }
    else if (path == NULL)
    {
        if (newNode->type == rtNAMEGROUPTYPELABEL)
        {
            /* Node is a label, check the label does not exist. */
            nGroupLabel = (RtNameGroupLabel *) newNode->data;
            nodeFound = RtNameGroupFindNode(nodeHead, nGroupLabel->group->name, FALSE);

            if (nodeFound == NULL)
            {
                /* Node is a data chunk, so just add it. */
                RtNameGroupSetParent(newNode, parent);
                newNodeHead = RtNameGroupAddNodePos(nodeHead, -1, newNode);
            }
            else
            {
                /* Node already exist. Raise error and stop. */
                newNodeHead = NULL;
            }
        }
        else
        {
            /* Node is a data chunk, so just add it. */
            RtNameGroupSetParent(newNode, parent);
            newNodeHead = RtNameGroupAddNodePos(nodeHead, -1, newNode);
        }
    }
    else
    {
        RtNameGroupPathSplitHead(path, &pathTail);

        pos = RtNameGroupPathIsPos(path);

        if (pos < 0)
        {
            nodeFound = RtNameGroupFindNode(nodeHead, path, FALSE);

            if (nodeFound)
            {
                newNodeHead = RtNameGroupAddNodePath(nodeFound->child, pathTail, newNode, nodeFound);

                if (newNodeHead)
                {
                    nodeFound->child = newNodeHead;
                    newNodeHead = nodeHead;
                }
            }
            else
            {
                /* Node not found. Raise error and stop. */
                newNodeHead = NULL;
            }
        }
        else
        {
            if (newNode->type == rtNAMEGROUPTYPELABEL)
            {
                /* Node is a label, check the label does not exist. */
                nGroupLabel = (RtNameGroupLabel *) newNode->data;
                nodeFound = RtNameGroupFindNode(nodeHead, nGroupLabel->group->name, FALSE);

                if (nodeFound == NULL)
                {
                    /* Node is a data chunk, so just add it. */
                    RtNameGroupSetParent(newNode, parent);
                    newNodeHead = RtNameGroupAddNodePos(nodeHead, pos, newNode);
                }
                else
                {
                    /* Node already exist. Raise error and stop. */
                    newNodeHead = NULL;
                }
            }
            else
            {
                /* Node is a data chunk, so just add it. */
                RtNameGroupSetParent(newNode, parent);
                newNodeHead = RtNameGroupAddNodePos(nodeHead, pos, newNode);
            }
        }

        if (pathTail)
            RtNameGroupPathJoin(path, pathTail);
    }

    return (newNodeHead);
}

/*
 * Read name space label from the input stream.
 */
static RtNameGroupLabel *
RtNameGroupStreamReadStartLabel(RwStream *stream, RwChunkHeaderInfo *chunkInfo __RWUNUSED__)
{
    RwBool              result;

    RtNameGroupLabel    *nGroupLabel;

    result = FALSE;

    nGroupLabel = RtNameGroupCreateLabel();

    if ((stream) && (nGroupLabel))
    {
        nGroupLabel->group = RwChunkGroupBeginStreamRead(stream);

        if (nGroupLabel->group != NULL)
            result = TRUE;
    }

    /* Clean if there was an error. */
    if (!result)
    {
        if (nGroupLabel)
            RtNameGroupDestroyLabel(nGroupLabel);

        nGroupLabel = NULL;
    }

    return (nGroupLabel);
}

/*
 * Write a name space start label to the output stream.
 */
static RtNameGroupLabel *
RtNameGroupStreamWriteStartLabel(RwStream *stream, RtNameGroupLabel *nGroupLabel )
{
    /* Check if there are data to write. */
    if (nGroupLabel)
    {
        RwChunkGroupBeginStreamWrite(nGroupLabel->group, stream);
    }

    return (nGroupLabel);
}

/*
 * Read a name space end label from the input stream.
 */
static RtNameGroupLabel *
RtNameGroupStreamReadEndLabel(RwStream *stream, RwChunkHeaderInfo *chunkInfo __RWUNUSED__, RtNameGroupLabel *nGroupLabel)
{
    if ((stream) && (nGroupLabel))
    {
        /* Read the group end marker. */
        RwChunkGroupEndStreamRead(nGroupLabel->group, stream);
    }

    return (nGroupLabel);
}

/*
 * Write a name space end label to the output stream.
 */
static RtNameGroupLabel *
RtNameGroupStreamWriteEndLabel(RwStream *stream, RtNameGroupLabel *nGroupLabel )
{
    /* Check if there are data to write. */
    if ((stream) && (nGroupLabel))
    {
        /* Write out the chunk header. */
        RwChunkGroupEndStreamWrite(nGroupLabel->group, stream);
    }

    return (nGroupLabel);
}

/*
 * Read a name space data chunk from the input stream.
 */
static RtNameGroupData *
RtNameGroupStreamReadData(RwStream *stream, RwChunkHeaderInfo *chunkInfo)
{
    RwBool              result;
    RtNameGroupData     *nGroupData;

    result = FALSE;

    /* Create the name space structure */
    nGroupData = RtNameGroupCreateData();

    if (nGroupData)
    {
        /* Allocate for data buffer. */
        if (chunkInfo->isComplex || (chunkInfo->type == rwID_EXTENSION))
        {
            /* Blank data. */
            result = TRUE;
            nGroupData->data = NULL;
            nGroupData->chunkInfo = *chunkInfo;
        }
        else
        {
            if (chunkInfo->length > 0)
            {
                nGroupData->data = (RwChar *) RwMalloc(chunkInfo->length, 0);

                /* Check if data can be read in. */
                if (nGroupData->data)
                {
                    if (RwStreamRead(stream, nGroupData->data, chunkInfo->length) == chunkInfo->length)
                    {
                        nGroupData->chunkInfo = *chunkInfo;

                        result = TRUE;
                    }
                    else
                    {
                        /* Error during streaming. */
                        sprintf(rtNGroupVars.msg, "File <%s> : Error during reading chunk data.",
                            rtNGroupVars.processFile);
                        ToolAPIReportError(rtNGroupVars.toolAPI, rtNGroupVars.msg);
                    }
                }
                else
                {
                    /* Memory allocation failure. Raise error and stop. */
                    sprintf(rtNGroupVars.msg, "File <%s> : Memory allocation failure.",
                        rtNGroupVars.processFile);
                    ToolAPIReportError(rtNGroupVars.toolAPI, rtNGroupVars.msg);
                }
            }
            else
            {
                /* Blank data. */
                result = TRUE;
                nGroupData->data = NULL;
                nGroupData->chunkInfo = *chunkInfo;
            }
        }
    }

    /* Clean if there was an error. */
    if (!result)
    {
        if (nGroupData)
            RtNameGroupDestroyData(nGroupData);

        nGroupData = NULL;
    }

    return (nGroupData);
}

/*
 * Write a name space data chunk to the output stream.
 */
static RtNameGroupData *
RtNameGroupStreamWriteData(RwStream *stream, RtNameGroupData *nGroupData )
{
    /* Check if there are data to write. */
    if (nGroupData)
    {
        /* Write out the header and data. */
        _rwStreamWriteVersionedChunkHeader(stream,
            nGroupData->chunkInfo.type, nGroupData->chunkInfo.length,
            nGroupData->chunkInfo.version, nGroupData->chunkInfo.buildNum);

        if (nGroupData->data)
            RwStreamWrite(stream, nGroupData->data, nGroupData->chunkInfo.length);
    }
    else
    {
        /* Something is wrong. */
        nGroupData = NULL;
    }

    return (nGroupData);
}

/*
 * Read a name space from the input stream. This will continue reading until the
 * end of stream is reached.
 */
static RtNameGroupNode *
RtNameGroupStreamRead( RwStream *stream, RtNameGroupNode *parent, RwInt32 *pos )
{
    RwBool                  result, end;
    RtNameGroupNode         *node, *nodeHead, *nodePrev;
    RtNameGroupData         *nGroupData;
    RtNameGroupNodeType     nodeType;
    RwChunkHeaderInfo       chunkInfo;
    RwInt32                 overrun, endPos;

    result = TRUE;
    end = FALSE;

    nodeHead = NULL;
    nodePrev = NULL;

    if ((parent->type == rtNAMEGROUPTYPEDATA) || (parent->type == rtNAMEGROUPTYPESUB))
    {
        nGroupData = (RtNameGroupData *) parent->data;
        endPos = *pos + nGroupData->chunkInfo.length;
        nodeType = rtNAMEGROUPTYPESUB;
    }
    else
    {
        endPos = 0x7FFFFFFF;
        nodeType = rtNAMEGROUPTYPEDATA;
    }

    while ((result) && (!end) && (RwStreamReadChunkHeaderInfo(stream, &chunkInfo)))
    {
        *pos += rwCHUNKHEADERSIZE;

        /* Check the type. */
        if (chunkInfo.type == rwID_CHUNKGROUPSTART)
        {
            *pos += chunkInfo.length;

            /* Label start. */
            node = RtNameGroupCreateNode();

            if (node)
            {
                rtNGroupVars.nodeCount++;

                nodeHead = RtNameGroupAddNodeAfter(nodeHead, nodePrev, node);

                /* Name space start */
                node->type = rtNAMEGROUPTYPELABEL;
                node->data = (void *) RtNameGroupStreamReadStartLabel(stream, &chunkInfo);
                node->parent = parent;

                /* Get the toc index. */
                RtNameGroupGetTOCEntry(node, &chunkInfo);

                /* Read the child of this name space. */
                node->child = RtNameGroupStreamRead(stream, node, pos);
            }
            else
            {
                /* Memory failure. Raise error and stop. */
                result = FALSE;
            }
        }
        else if (chunkInfo.type == rwID_CHUNKGROUPEND)
        {
            /* Label end. */

            /* Read in any data into parent and exit. */
            if (RtNameGroupStreamReadEndLabel(stream, &chunkInfo, parent->data) != parent->data)
            {
                /* Something went wrong. Raise error and stop. */
                result = FALSE;
            }



            end = TRUE;
        }
        else if (chunkInfo.type == rwID_TOC)
        {
            /* Toc. */
            rtNGroupVars.toc = RtTOCStreamRead(stream);

            if (rtNGroupVars.toc != NULL)
            {
                rtNGroupVars.tocOffset = chunkInfo.length + rwCHUNKHEADERSIZE;
                rtNGroupVars.tocIndex = 0;

                /* reset the chunkInfo length to avoid error check. */
                chunkInfo.length = 0;
            }
            else
            {
                /* Something went wrong. Raise error and stop. */
                result = FALSE;
            }
        }
        else
        {
            /* Data chunk. */
            node = RtNameGroupCreateNode();

            if (node)
            {
                rtNGroupVars.nodeCount++;

                nodeHead = RtNameGroupAddNodeAfter(nodeHead, nodePrev, node);

                node->type = nodeType;
                node->data = (void *) RtNameGroupStreamReadData(stream, &chunkInfo);
                node->parent = parent;

                if (chunkInfo.isComplex || (chunkInfo.type == rwID_EXTENSION))
                {
                    if (chunkInfo.length > 0)
                        node->child = RtNameGroupStreamRead(stream, node, pos);
                    else
                        node->child = NULL;
                }
                else
                {
                    *pos += chunkInfo.length;
                    node->child = NULL;
                }

                /* Get the toc index. */
                RtNameGroupGetTOCEntry(node, &chunkInfo);

                overrun = *pos - endPos;
                if (overrun > 0)
                {
                    /* File overrun error. Raise error and stop. */
                    result = FALSE;
                }

                if (*pos >= endPos)
                    end = TRUE;
            }
            else
            {
                /* Memory failure. Raise error and stop. */
                result = FALSE;
            }
        }

        /* Check for error during reading. */
        if ((chunkInfo.length > 0) && (node->data == NULL))
        {
            /* Something is wrong. Raise error and stop. */
            result = FALSE;
        }

        nodePrev = node;
    }

    if (!result)
    {
        /* Something is wrong. Clean up and exit. */

        nodeHead = NULL;
    }

    return (nodeHead);
}

/*
 * Write a name space to the output stream. All nodes in the list and children are
 * also written out.
 */
static RtNameGroupNode *
RtNameGroupStreamWrite( RwStream *stream, RtNameGroupNode *nodeHead )
{
    RtNameGroupNode     *node;

    node = nodeHead;

    if (node)
    {
        /* Main process loop. */
        do
        {
            /* Check the type. */
            if (node->type == rtNAMEGROUPTYPELABEL)
            {
                /* Label start marker. */
                RtNameGroupStreamWriteStartLabel(stream, node->data);

                /* Write out the children. */
                RtNameGroupStreamWrite(stream, node->child);

                /* Label stop marker. */
                RtNameGroupStreamWriteEndLabel(stream, node->data);
            }
            else if ((node->type == rtNAMEGROUPTYPEDATA) ||
                     (node->type == rtNAMEGROUPTYPESUB))
            {
                /* Data chunk. */
                RtNameGroupStreamWriteData(stream, node->data);

                /* Write out the children. */
                if (node->child)
                    RtNameGroupStreamWrite(stream, node->child);
            }
            else
            {
                /* No data write. */
            }

            node = node->next;

        } while (node != nodeHead);
    }

    return (nodeHead);
}

/*
 * Create a set of name space nodes that matches the pathname. Sub paths
 * are automatically created.
 */
static RtNameGroupNode *
RtNameGroupCreatePath( RtNameGroupNode *node, RwChar *pathname, RtNameGroupNode *parent )
{
    RwBool              result;
    RwChar              *subPath;
    RwUInt32            size;

    RtNameGroupNode         *nodeFound;
    RtNameGroupLabel        *nGroupLabel;

    result = FALSE;

    if (pathname)
    {
        /* Pathname contains subpath. Split into current path and sub path. */
        RtNameGroupPathSplitHead(pathname, &subPath);

        /* Search the current level for the path */
        if (node)
            nodeFound = RtNameGroupFindNode(node, pathname, FALSE);
        else
            nodeFound = NULL;

        if (nodeFound == NULL)
        {
            /* This path does not exist. Add one to the current list. */
            size = rwstrlen(pathname) + 1;

            nodeFound = RtNameGroupCreateNode();
            nGroupLabel = RtNameGroupCreateLabel();

            /* Set up the new node and add it to the current list. */
            if ((nodeFound) && (nGroupLabel))
            {
                RwChunkGroupSetName(nGroupLabel->group, pathname);

                nodeFound->type = rtNAMEGROUPTYPELABEL;
                nodeFound->data = nGroupLabel;
                nodeFound->parent = parent;
                nodeFound->child = NULL;

                /* Add the node to the end of the current list. */
                node = RtNameGroupAddNodePos(node, -1, nodeFound);

                parent->child = node;
            }
            else
            {
                /* Memory failure. Raise an error and stop. */
                sprintf(rtNGroupVars.msg, "File <%s> : Create Path. Memory allocation failure.",
                    rtNGroupVars.processFile);
                ToolAPIReportError(rtNGroupVars.toolAPI, rtNGroupVars.msg);

                if (nodeFound)
                    RtNameGroupDestroyNode(nodeFound);
                nodeFound = NULL;

                if (nGroupLabel)
                    RtNameGroupDestroyLabel(nGroupLabel);
                nGroupLabel = NULL;

                result = FALSE;
            }
        }

        if (nodeFound)
        {
            /* Ceate any sub paths present. */
            if (subPath)
            {
                node = RtNameGroupCreatePath(nodeFound->child, subPath, nodeFound);

                if (node)
                {
                    /* Everything is okay. */
                    result = TRUE;
                }
                else
                {
                    /* Something is wrong. Raise error and stop. */
                    result = FALSE;
                }
            }
            else
            {
                node = nodeFound;
                result = TRUE;
            }

            /* Restore the path name */
            if (subPath)
                RtNameGroupPathJoin(pathname, subPath);
        }
    }
    else
    {
        /* Empty pathname. Raise a warning. */
        node = parent;
    }

    if (!result)
    {
        node = NULL;
    }

    return (node);
}

/*
 * Traverse the hierachy specified by the path.
 */
static RtNameGroupNode *
RtNameGroupFindPath( RtNameGroupNode *node, RwChar *pathname )
{
    RwChar                  *subPath;

    RtNameGroupNode         *nodeFound;
    RwInt32                 pos;

    nodeFound = NULL;

    if ((pathname) && (node))
    {
        /* Strip the first element. */
        RtNameGroupPathSplitHead(pathname, &subPath);

        /* Check if it is a path or item. */
        pos = RtNameGroupPathIsPos(pathname);

        nodeFound = RtNameGroupFindNode(node, pathname, FALSE);

        if (pos < 0)
        {
            /* Node was a label, continue to search down the path. */

            /* Traverse any sub paths present. */
            if ((subPath) && (nodeFound))
            {
                nodeFound = RtNameGroupFindPath(nodeFound->child, subPath);
            }
        }

        /* Restore the path name */
        if (subPath)
            RtNameGroupPathJoin(pathname, subPath);
    }

    return (nodeFound);
}

/*
 * Create a path specified by the pathname.
 */
static RwInt32
StreamEditCreate(ToolAPI *api , RwChar *pathname)
{
    RwBool                  err;
    RwChar                  *subPath;
    RtNameGroupNode         *node;

    err = 0;

    /* Check if the pathnames are valid. */
    if (RtNameGroupPathIsValid(pathname) != TRUE)
    {
        sprintf(rtNGroupVars.msg, "File <%s> : Creation failed. Path <%s> not valid .",
            rtNGroupVars.processFile, pathname);
        ToolAPIReportError(api, rtNGroupVars.msg);

        return (1);
    }

    /* Check if the path is absolute or relative. */
    if (RtNameGroupPathIsRelative(pathname, &subPath))
    {
        /* Path is relative. Create path from curr path. */
        node = rtNGroupVars.nodeCurr;
    }
    else
    {
        /* Path is absolute. Create path from the root. */
        node = rtNGroupVars.nodeRoot;
    }

    node = RtNameGroupCreatePath(node->child, subPath, node);

    if (node == NULL)
    {
        /* Something is wrong. Raise an error and stop. */
        sprintf(rtNGroupVars.msg, "File <%s> : Create failed. Memory allocation failure.",
            rtNGroupVars.processFile);
        ToolAPIReportError(api, rtNGroupVars.msg);

        err = 1;
    }

    return (err);
}

/*
 * Move an object from one path to another.
 */
static RwInt32
StreamEditMove(ToolAPI *api , RwChar *inPathname, RwChar *outPathname )
{
    RwBool                  err;
    RwChar                  *inPathHead, *outPathHead;
    RtNameGroupNode         *node, *inNode, *outNode, *inParent;

    err = 0;

    /* Check if the pathnames are valid. */
    if (RtNameGroupPathIsValid(inPathname) != TRUE)
    {
        sprintf(rtNGroupVars.msg, "File <%s> : Move failed. Path <%s> not valid .",
            rtNGroupVars.processFile, inPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);

        return (1);
    }

    if (RtNameGroupPathIsValid(outPathname) != TRUE)
    {
        sprintf(rtNGroupVars.msg, "File <%s> : Move failed. Path <%s> not valid.",
            rtNGroupVars.processFile, outPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);

        return(1);
    }

    /* Check if the path is the root. */
    if (RtNameGroupPathIsRoot(inPathname))
    {
        sprintf(rtNGroupVars.msg, "File <%s> : Move failed. Cannot move <%s> root.",
            rtNGroupVars.processFile, outPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);
        return(1);
    }

    /* Check if the path is absolute or relative. */
    if (RtNameGroupPathIsRelative(inPathname, &inPathHead))
    {
        /* Path is relative. Find path from curr path. */
        inNode = rtNGroupVars.nodeCurr;
    }
    else
    {
        /* Path is absolute. Find path from the root. */
        inNode = rtNGroupVars.nodeRoot;
    }

    /* Grab the input path. */
    inNode = RtNameGroupFindPath(inNode->child, inPathHead);

    /* Is the in node valid ? */
    if (inNode)
    {

        /* Check if the path is absolute or relative. */
        if (RtNameGroupPathIsRelative(outPathname, &outPathHead))
        {
            /* Path is relative. Find path from curr path. */
            outNode = rtNGroupVars.nodeCurr;
        }
        else
        {
            /* Path is absolute. Find path from the root. */
            outNode = rtNGroupVars.nodeRoot;
        }

        inParent = inNode->parent;
        inParent->child = RtNameGroupRemoveNode(inParent->child, inNode);

        node = RtNameGroupAddNodePath(outNode->child, outPathname, inNode, outNode);

        if (node)
        {
            outNode->child = node;
        }
        else
        {
            /* Clean up. */
            RtNameGroupDestroy(inNode);

            /* Destination does not exist. Raise error and stop. */
            sprintf(rtNGroupVars.msg, "File <%s> : Move failed. Error during move to <%s>.",
                rtNGroupVars.processFile, outPathname);
            ToolAPIReportError(api, rtNGroupVars.msg);
            err = 1;
        }

    }
    else
    {
        /* Source not found. Raise error and stop. */
        sprintf(rtNGroupVars.msg, "File <%s> : Move failed. Source <%s> not found.",
            rtNGroupVars.processFile, inPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);
        err = 1;
    }

    return (err);
}

/*
 * Move an object from one path to another.
 */
static RwInt32
StreamEditDelete(ToolAPI *api, RwChar *inPathname)
{
    RwBool                  err;
    RwChar                  *inPathHead;
    RtNameGroupNode         *inNode, *inParent;

    err = 0;

    inPathHead = NULL;

    /* Check if the pathnames are valid. */
    if (RtNameGroupPathIsValid(inPathname) != TRUE)
    {
        sprintf(rtNGroupVars.msg, "File <%s> : Delete failed. Path <%s> not valid .",
            rtNGroupVars.processFile, inPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);

        return(1);
    }

    /* Check if the path is absolute or relative. */
    if (RtNameGroupPathIsRelative(inPathname, &inPathHead))
    {
        /* Path is relative. Find path from curr path. */
        inNode = rtNGroupVars.nodeCurr;
    }
    else
    {
        /* Path is absolute. Find path from the root. */
        inNode = rtNGroupVars.nodeRoot;
    }

    inNode = RtNameGroupFindNode(inNode->child, inPathHead, FALSE);

    if (inNode)
    {
        /* Node found. Delete the path or item. */
        inParent = inNode->parent;
        inParent->child = RtNameGroupRemoveNode(inParent->child, inNode);

        RtNameGroupDestroy(inNode);
    }
    else
    {
        /* Source not found. Raise error and stop. */
        sprintf(rtNGroupVars.msg, "File <%s> : Delete failed. Source <%s> not found.",
            rtNGroupVars.processFile, inPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);

        err = 1;
    }

    return (err);
}

/*
 * Copy an object from one path to another.
 */
static RwInt32
StreamEditCopy(ToolAPI *api, RwChar *inPathname, RwChar *outPathname)
{
    RwBool                  err;
    RwChar                  *inPathHead, *outPathHead, *outPathTail;
    RtNameGroupNode         *node, *inNode, *outNode, *inParent, *newNode;

    err = 0;

    inPathHead = NULL;

    outPathHead = NULL;
    outPathTail = NULL;

    /* Check if the pathnames are valid. */
    if (RtNameGroupPathIsValid(inPathname) != TRUE)
    {
        sprintf(rtNGroupVars.msg, "File <%s> : Copy failed. Source path <%s> not valid .",
            rtNGroupVars.processFile, inPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);

        return (1);
    }

    if (RtNameGroupPathIsValid(outPathname) != TRUE)
    {
        sprintf(rtNGroupVars.msg, "File <%s> : Copy failed. Destination path <%s> not valid.",
            rtNGroupVars.processFile, outPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);

        return(1);
    }

    /* Check if the path is absolute or relative. */
    if (RtNameGroupPathIsRelative(inPathname, &inPathHead))
    {
        /* Path is relative. Find path from curr path. */
        inNode = rtNGroupVars.nodeCurr;
    }
    else
    {
        /* Path is absolute. Find path from the root. */
        inNode = rtNGroupVars.nodeRoot;
    }

    /* Grab the input path. */
    inNode = RtNameGroupFindPath(inNode->child, inPathHead);

    /* Is the in node valid ? */
    if (inNode)
    {

        /* Check if the path is absolute or relative. */
        if (RtNameGroupPathIsRelative(outPathname, &outPathHead))
        {
            /* Path is relative. Find path from curr path. */
            outNode = rtNGroupVars.nodeCurr;
        }
        else
        {
            /* Path is absolute. Find path from the root. */
            outNode = rtNGroupVars.nodeRoot;
        }

        inParent = inNode->parent;
        newNode = RtNameGroupCopy(inNode, inParent, FALSE);

        if (newNode)
        {
            node = RtNameGroupAddNodePath(outNode->child, outPathname, newNode, outNode);

            if (node)
            {
                outNode->child = node;
            }
            else
            {
                /* Clean up. */
                RtNameGroupDestroy(newNode);

                /* Memory allocation failure. */
                sprintf(rtNGroupVars.msg, "File <%s> : Copy failed. Error during copy to <%s>.",
                    rtNGroupVars.processFile, outPathname);
                ToolAPIReportError(api, rtNGroupVars.msg);
                err = 1;
            }
        }
        else
        {
            /* Memory allocation failure. */
            sprintf(rtNGroupVars.msg, "File <%s> : Copy failed. Memory allocation failure.",
                rtNGroupVars.processFile);
            ToolAPIReportError(api, rtNGroupVars.msg);
            err = 1;
        }
    }
    else
    {
        /* Source not found. Raise error and stop. */
        sprintf(rtNGroupVars.msg, "File <%s> : Copy failed. Source <%s> not found.",
            rtNGroupVars.processFile, inPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);
        err = 1;
    }

    return (err);
}

/*
 * Rename an object from one path to another.
 */
static RwInt32
StreamEditRename(ToolAPI *api, RwChar *inPathname, RwChar *outPathname)
{
    RwBool                  err;
    RwChar                  *inPathHead;
    RtNameGroupNode         *inNode, *nodeFound;
    RtNameGroupLabel        *nGroupLabel;

    err = 0;

    inPathHead = NULL;

    /* Check if the pathnames are valid. */
    if (RtNameGroupPathIsValid(inPathname) != TRUE)
    {
        sprintf(rtNGroupVars.msg, "File <%s> : Rename failed. Path <%s> not valid .",
            rtNGroupVars.processFile, inPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);

        return(1);
    }

    if (RtNameGroupNameIsValid(outPathname) != TRUE)
    {
        sprintf(rtNGroupVars.msg, "File <%s> : Rename failed. Name <%s> not valid .",
            rtNGroupVars.processFile, outPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);

        return(1);
    }

    /* Check if the path is absolute or relative. */
    if (RtNameGroupPathIsRelative(inPathname, &inPathHead))
    {
        /* Path is relative. Find path from curr path. */
        inNode = rtNGroupVars.nodeCurr;
    }
    else
    {
        /* Path is absolute. Find path from the root. */
        inNode = rtNGroupVars.nodeRoot;
    }

    inNode = RtNameGroupFindPath(inNode->child, inPathHead);

    if (inNode)
    {
        /* Node found. */

        /* Can this node be renamed ? */
        if (inNode->type == rtNAMEGROUPTYPELABEL)
        {
            /* Check if the name already exist. */
            nGroupLabel = (RtNameGroupLabel *) inNode->data;

            if (nGroupLabel->group->name)
                nodeFound = RtNameGroupFindNode(inNode, outPathname, FALSE);

            if (nodeFound == NULL)
            {
                if (RtNameGroupRenameLabel(inNode->data, outPathname) != inNode->data)
                {
                    /* Memory allocation failure. Raise error and stop. */
                    sprintf(rtNGroupVars.msg, "File <%s> : Rename failed. Memory allocation failure.",
                        rtNGroupVars.processFile);
                    ToolAPIReportError(api, rtNGroupVars.msg);
                    err = 1;
                }
            }
            else
            {
                /* Error. Name already exist. Cannot rename. */
                sprintf(rtNGroupVars.msg, "File <%s> : Rename failed. Destination <%s> already exist.",
                    rtNGroupVars.processFile, inPathname);
                ToolAPIReportError(api, rtNGroupVars.msg);
                err = 1;
            }
        }
        else
        {
            /* Not a label. Raise error and stop. */

            /* Source not found. Raise error and stop. */
            sprintf(rtNGroupVars.msg, "File <%s> : Rename failed. Source <%s> is not a label.",
                rtNGroupVars.processFile, inPathname);
            ToolAPIReportError(api, rtNGroupVars.msg);
            err = 1;
        }
    }
    else
    {
        /* Source not found. Raise error and stop. */
        sprintf(rtNGroupVars.msg, "File <%s> : Rename failed. Source <%s> not found.",
            rtNGroupVars.processFile, inPathname);
        ToolAPIReportError(api, rtNGroupVars.msg);
        err = 1;
    }

    return (err);
}

/*
 *
 */
static RwInt32
StreamEditStartup(ToolAPI *api __RWUNUSED__, RwChar *currentPath __RWUNUSED__)
{
    RwBool      err;

    err = 0;

    rtNGroupVars.toc = NULL;

    rtNGroupVars.nGroupLabelFList =
        RwFreeListCreate(sizeof(RtNameGroupLabel), 20, 0, 0);
    if (rtNGroupVars.nGroupLabelFList == NULL)
        err = 1;

    rtNGroupVars.nGroupDataFList = NULL;
    if (!err)
    {
        rtNGroupVars.nGroupDataFList =
            RwFreeListCreate(sizeof(RtNameGroupData), 20, 0, 0);
        if (rtNGroupVars.nGroupDataFList == NULL)
            err = 1;
    }

    rtNGroupVars.nGroupNodeFList = NULL;
    if (!err)
    {
        rtNGroupVars.nGroupNodeFList =
            RwFreeListCreate(sizeof(RtNameGroupNode), 20, 0, 0);
        if (rtNGroupVars.nGroupNodeFList == NULL)
            err = 1;
    }


    return err;
}

/*
 *
 */
static RwInt32
StreamEditShutdown(ToolAPI *api __RWUNUSED__)
{
    if (rtNGroupVars.nGroupLabelFList)
        RwFreeListDestroy(rtNGroupVars.nGroupLabelFList);
    rtNGroupVars.nGroupLabelFList = NULL;

    if (rtNGroupVars.nGroupDataFList)
        RwFreeListDestroy(rtNGroupVars.nGroupDataFList);
    rtNGroupVars.nGroupDataFList = NULL;

    if (rtNGroupVars.nGroupNodeFList)
        RwFreeListDestroy(rtNGroupVars.nGroupNodeFList);
    rtNGroupVars.nGroupNodeFList = NULL;

    if (rtNGroupVars.toc)
        RtTOCDestroy(rtNGroupVars.toc);
    rtNGroupVars.toc = NULL;

    return 0;
}

/*
 *
 */
static RwInt32
StreamEditProcessFile( ToolAPI *api, RwChar *inFilename )
{
    RwStream                *stream;
    RwInt32                 err, offset, pos;
    RtNameGroupNode         *nodeRoot;
    RwChar                  *pathname, *prefix;

    err = 0;
    nodeRoot = NULL;

    rtNGroupVars.toolAPI = api;
    rtNGroupVars.processFile = inFilename;


    /* Attempt to open the output stream. */
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, inFilename);

    if (stream)
    {
        /* Read in the stream into an internal hierachy */
        rtNGroupVars.nodeCount = 0;

        rtNGroupVars.toc = NULL;
        rtNGroupVars.tocIndex = -1;

        nodeRoot = RtNameGroupCreateNode();

        if (nodeRoot)
        {
            pos = 0;

            nodeRoot->type = rtNAMEGROUPTYPEROOT;

            nodeRoot->child = RtNameGroupStreamRead(stream, nodeRoot, &pos);

            rtNGroupVars.nodeRoot = nodeRoot;
        }
        else
        {
            /* Memory failure. */
            sprintf(rtNGroupVars.msg, "File <%s> : Failed to create default root node.",
                rtNGroupVars.processFile);
            ToolAPIReportError(api, rtNGroupVars.msg);

            err = 1;
        }

        RwStreamClose(stream, NULL);

        if ((!err) && (nodeRoot) && (nodeRoot->child))
        {
            /* Check if an optional path is set, if so move to its position. */
            if (g_Pathname)
            {
                /* Strip possible leading slash. */
                RtNameGroupPathIsRelative(g_Pathname, &pathname);

                /* Move down the hierachy specified by the path. */
                rtNGroupVars.nodeCurr = RtNameGroupFindPath(nodeRoot->child, pathname);

                if (rtNGroupVars.nodeCurr == NULL)
                {
                    /* Path doesn't exist. Raise error and stop. */
                    err = 1;
                }
            }
            else
            {
                rtNGroupVars.nodeCurr = nodeRoot;
            }

            /* Process any options. */
            if (!err)
            {
                /* Create any path. */
                if (g_Create)
                {
                    err = StreamEditCreate(api, g_Create);
                }

                if ((!err) && (g_Move[0]) && (g_Move[1]))
                {
                    err = StreamEditMove(api, g_Move[0], g_Move[1]);
                }

                if ((!err) && (g_Rename[0]) && (g_Rename[1]))
                {
                    err = StreamEditRename(api, g_Rename[0], g_Rename[1]);
                }

                if ((!err) && (g_Delete))
                {
                    err = StreamEditDelete(api, g_Delete);
                }

                if ((!err) && (g_Copy[0]) && (g_Copy[1]))
                {
                    err = StreamEditCopy(api, g_Copy[0], g_Copy[1]);
                }

                /* Display the hierarchy. */
                if (g_List >= 0)
                {
                    {
                        sprintf(rtNGroupVars.msg, "%s /\n", rtNAMEGROUPVIEWLABELOPENTOKEN);
                        ToolAPIReport(api, rtNGroupVars.msg);
                    }

                    prefix = rtNGroupVars.buffer;
                    sprintf(prefix, "%s", rtNAMEGROUPVIEWBLANKTOKEN);

                    RtNameGroupList(nodeRoot->child, prefix, 0);
                }
            }

            /* Write out the node. */
            if (!err)
            {
                stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, inFilename);

                if (stream)
                {
                    /* Check if the toc needs to be updated. */
                    if (rtNGroupVars.toc)
                    {
                        offset =  rtNGroupVars.tocOffset;
                        rtNGroupVars.tocIndex = 0;

                        RtNameGroupUpdateTOCOffset(rtNGroupVars.nodeRoot->child, &offset);
                        RtNameGroupUpdateTOC(rtNGroupVars.nodeRoot->child);
                        if (RtTOCStreamWrite(rtNGroupVars.toc, stream) != rtNGroupVars.toc)
                        {
                            /* Error during output. Raise error and stop. */
                            err = 1;
                        }
                    }

                    if (err == 0)
                    {
                        if (RtNameGroupStreamWrite(stream, rtNGroupVars.nodeRoot->child) == NULL)
                        {
                            /* Error during output. Raise error and stop. */
                            err = 1;
                        }
                    }

                    RwStreamClose(stream, NULL);
                }
                else
                {
                    /* Failed to create the output stream. Raise error an stop. */
                    err = 1;
                }
            }
        }
        else
        {
            /* Failed to read the stream. Raise error and stop. */
            err = 1;
        }
    }
    else
    {
        /* Fail to open the file. Raise error and stop. */
        sprintf(rtNGroupVars.msg, "File <%s> : Failed to open file.",
            rtNGroupVars.processFile);
        ToolAPIReportError(api, rtNGroupVars.msg);

        err = 1;
    }

    /* Destroy the node. */
    if (nodeRoot)
    {
        RtNameGroupDestroy(nodeRoot);
    }

    return (err);
}

/*
 *
 */

int main(int argc, char* argv[])
{
    ToolAPI api;
    RwInt32 err;

    ToolAPIInitAPI(&api, "rwsed", "Stream editing tool",
                   ToolOptions, NumToolOptions);

    api.userStartupFn = StreamEditStartup;
    api.userProcessFileFn = StreamEditProcessFile;
    api.userShutdownFn = StreamEditShutdown;

    err = ToolAPIExecute(&api, argc, argv);

    ToolAPIShutdownAPI(&api);

    return(err);
}

