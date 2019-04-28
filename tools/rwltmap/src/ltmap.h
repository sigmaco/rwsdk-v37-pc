
#ifndef LTMAP_H
#define LTMAP_H

#include <stdio.h>
#include <string.h>

#include <rwcore.h>
#include <rpworld.h>
#include <rpcollis.h>
#include <rppvs.h>
#include <rprandom.h>
#include <rpmipkl.h>
#include <rpltmap.h>
#include <rtltmap.h>
#include <rtpitexd.h>
#include <rtpng.h>
#include <rtbmp.h>

#include "shared.h"

#define LTMAPOBJLISTMAIN                0
#define LTMAPOBJLISTLIGHTMAP            1
#define LTMAPOBJLISTOCCLUDER            2
#define LTMAPOBJLISTVERTEXLIT           3

#define LTMAPNUMOBJLIST                 4

#define LTMAPFLAGMODIFIED               0x0001

#define LTMAPAREALIGHTDENSITY           1.0
#define LTMAPAREALIGHTDENSITYMOD        1.0
#define LTMAPAREALIGHTRADIUSMOD         1.0
#define LTMAPAREALIGHTERROR             rpLTMAPDEFAULTAREALIGHTROICUTOFF
#define LTMAPVERTEXWELD                 rpLTMAPDEFAULTVERTEXWELDTHRESHOLD
#define LTMAPSLIVER                     rpLTMAPDEFAULTSLIVERAREATHRESHOLD
#define LTMAPDENSITY                    1.0
#define LTMAPSIZE                       256
#define LTMAPSUPERSAMPLE                2

typedef struct LtMapChunkList LtMapChunkList;

struct LtMapChunkList
{
    LtMapChunkList              *next;
    RwUInt32                    flag;

    RwChar                      *filename;
    RwChunkHeaderInfo           chkHdr;
    void                        *data;
};

typedef struct LtMapGlobals LtMapGlobals;

struct LtMapGlobals
{
    RpWorld                     *world;
    RwTexDictionary             *txDict, *defaultTxDict;
    RwBool                      piTxDict;

    RtLtMapAreaLightGroup       *areaLightGrp;
    RwBool                      areaLightOn;
    RwReal                      areaLightDensity,
                                areaLightDensityModifier,
                                areaLightRadiusModifier,
                                areaLightErrorCutoff;

    RwReal                      ltMapVertexWield,
                                ltMapSliver,
                                ltMapDensity;

    RwUInt32                    ltMapSize,
                                ltMapSuperSample;

    RwUInt32                    postDarkMap,
                                postCalcLumIndex;

    RwUInt32                    ltMapRasterFormat;

    RtLtMapLightingSession      session;

    RwChar                      *mainPath;
    RwSList                     *fileList[LTMAPNUMOBJLIST];
    LtMapChunkList              *chkList[LTMAPNUMOBJLIST];

    RwChar                      msg[80];
    RwChar                      buffer[256];

    RwInt32                     progress;

    RwFreeList                  *chkListFList;
};

extern LtMapGlobals    g_LtMapVars;

extern ToolAPI         *g_ToolAPI;

#ifdef    __cplusplus
extern              "C"
{
#endif                          /* __cplusplus */

/*
 *
 */
extern RwInt32
LtMapCreateLtMap( ToolAPI *api, RtLtMapLightingSession *session );

extern RwInt32
LtMapIlluminate( ToolAPI *api, RtLtMapLightingSession *session );

extern RwInt32
LtMapPostProcess( ToolAPI *api, RtLtMapLightingSession *session );

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* LTMAP_H */
