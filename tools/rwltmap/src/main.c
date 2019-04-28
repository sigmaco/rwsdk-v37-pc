
#define NUMOBJLIST  3

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

#include "ltmap.h"

RwBool      g_DarkMap           = FALSE;
RwUInt32    g_CalcLumIndex      = 0;
RwSList     *g_ObjFilesList     = NULL;
RwSList     *g_OccFilesList     = NULL;
RwSList     *g_VltFilesList     = NULL;
RwBool      g_ALOn              = FALSE;
RwReal      g_ALDensity         = (RwReal)LTMAPAREALIGHTDENSITY;
RwReal      g_ALDensityMod      = (RwReal)LTMAPAREALIGHTDENSITYMOD;
RwReal      g_ALRadiusMod       = (RwReal)LTMAPAREALIGHTRADIUSMOD;
RwReal      g_ALErrorCutoff     = (RwReal)LTMAPAREALIGHTERROR;
RwReal      g_VtxWeld           = (RwReal)LTMAPVERTEXWELD;
RwReal      g_Sliver            = (RwReal)LTMAPSLIVER;
RwReal      g_LtMapDensity      = (RwReal)LTMAPDENSITY;
RwUInt32    g_LtMapSize         = LTMAPSIZE;
RwUInt32    g_LtMapSuperSample  = LTMAPSUPERSAMPLE;
RwUInt32    g_LtMapRasterFormat = (rwRASTERFORMAT888|rwRASTERFORMATMIPMAP|rwRASTERFORMATAUTOMIPMAP);

ToolOption ToolOptions[] = {
    { "darkmap",        "dm",   "",                     ToolParamTypeImplicitBoolE,     &g_DarkMap,          "Generate Darlmap",                 NULL },
    { "lumin",          "lum",  "<index>",              ToolParamTypeIntE,              &g_CalcLumIndex,     "Calc Lum for base texture",        NULL },
    { "object",         "obj",  "<file>",               ToolParamTypeSListE,            &g_ObjFilesList,     "Additional lightmap obj files",    NULL },
    { "occluder",       "occ",  "<file>",               ToolParamTypeSListE,            &g_OccFilesList,     "Additional occluder obj files",    NULL },
    { "vtxlit",         "vlt",  "<file>",               ToolParamTypeSListE,            &g_VltFilesList,     "Additional vertex lit obj files",  NULL },
    { "arealight",      "al",   "",                     ToolParamTypeImplicitBoolE,     &g_ALOn,             "Area light on",                    NULL },
    { "aldensity",      "ald",  "<density>",            ToolParamTypeRealE,             &g_ALDensity,        "Area light density",               NULL },
    { "aldensitymod",   "alm",  "<density modifier>",   ToolParamTypeRealE,             &g_ALDensityMod,     "Area light density modifier",      NULL },
    { "alradiusmod",    "alr",  "<radius modifier>",    ToolParamTypeRealE,             &g_ALRadiusMod,      "Area light radius modifier",       NULL },
    { "alerror",        "alc",  "<error cutoff>",       ToolParamTypeRealE,             &g_ALErrorCutoff,    "Area light error cutoff",          NULL },
    { "vtxweld",        "vtw",  "<weld threshold>",     ToolParamTypeRealE,             &g_VtxWeld,          "Vertex weld threshold",            NULL },
    { "sliver",         "slv",  "<sliver>",             ToolParamTypeRealE,             &g_Sliver,           "Sliver threshold",                 NULL },
    { "density",        "d",    "<density>",            ToolParamTypeRealE,             &g_LtMapDensity,     "Lightmap density",                 NULL },
    { "size",           "s",    "<size>",               ToolParamTypeIntE,              &g_LtMapSize,        "Lightmap size",                    NULL },
    { "supersample",    "ss",   "<supersample>",        ToolParamTypeIntE,              &g_LtMapSuperSample, "Lightmap super sample",            NULL },
    { "format",         "f",    "<format>",             ToolParamTypeIntE,              &g_LtMapRasterFormat,"Lightmap raster format",           NULL }
};

RwUInt32 NumToolOptions = sizeof(ToolOptions) / sizeof(ToolOption);

LtMapGlobals    g_LtMapVars;

ToolAPI         *g_ToolAPI;

/*
 *
 */
static RwBool
LtMapProgressCallBack( RwInt32 messageID, RwReal progress )
{
    if ((((RwInt32) progress) - g_LtMapVars.progress) >= 1)
    {
        g_LtMapVars.progress = (RwInt32) progress;

        sprintf(g_LtMapVars.msg, "File <%s> : Lightmap illumination progress %03.3d%%.",
                g_LtMapVars.mainPath, g_LtMapVars.progress);
        ToolAPIReportLog(g_ToolAPI, g_LtMapVars.msg);
    }

    return (TRUE);
}

/*
 *
 */
static RpAtomic *
LtMapAtomicAddBSphereCentreCB(RpAtomic *atomic, void *data)
{
    RpGeometry  *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RwV3d           center;
        RwMatrix        *ltm;
        RpMorphTarget   *morphTarget;
        RwSphere        *clumpSphere;

        clumpSphere = (RwSphere *)data;

        /*
         * Establish the average center of this atomic over all morph targets
         */
        morphTarget = RpGeometryGetMorphTarget(geometry, 0);
        center = RpMorphTargetGetBoundingSphere(morphTarget)->center;

        /*
         * Tranform the average center of the atomic to world space
         */
        ltm = RwFrameGetLTM(RpAtomicGetFrame(atomic));
        RwV3dTransformPoints(&center, &center, 1, ltm);

        /*
         * Add the average center of the atomic up in order to calculate the center of the clump
         */
        RwV3dAdd(&clumpSphere->center, &clumpSphere->center, &center);
    }

    return (atomic);
}

/*
 *
 */
static RpAtomic *
LtMapAtomicCompareBSphereCB(RpAtomic *atomic, void *data)
{
    RpGeometry      *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RwSphere        *clumpSphere, morphTargetSphere;
        RwV3d           tmpVec;
        RpMorphTarget   *morphTarget;
        RwReal          dist;
        RwMatrix        *ltm;

        clumpSphere = (RwSphere *)data;

        ltm = RwFrameGetLTM(RpAtomicGetFrame(atomic));

        morphTarget = RpGeometryGetMorphTarget(geometry, 0);
        morphTargetSphere = *RpMorphTargetGetBoundingSphere(morphTarget);

        RwV3dTransformPoints(&morphTargetSphere.center,
            &morphTargetSphere.center, 1, ltm);

        RwV3dSub(&tmpVec, &morphTargetSphere.center, &clumpSphere->center);

        dist = RwV3dLength(&tmpVec) + morphTargetSphere.radius;

        if (clumpSphere->radius < dist)
            clumpSphere->radius = dist;
    }

    return (atomic);
}

/*
 *
 */
static RpClump *
LtMapClumpGetBSphere(RpClump *clump, RwSphere *clumpSphere)
{
    /*
     * First find the mean of all the atomics' bounding sphere centers.
     * All morph targets of all atomics and all frame animations are taken into account.
     * The result is the clump's bounding sphere center...
     */
    clumpSphere->center.x = clumpSphere->center.y = clumpSphere->center.z = (RwReal) 0.0;

    /*
     * average over morph targets and atomics
     */
    if (RpClumpForAllAtomics(clump, LtMapAtomicAddBSphereCentreCB, &clumpSphere) != clump)
    {
        /* Something is wrong. Raise error and stop. */
        /*
        sprintf(g_LtMapVars.msg, "File %s : Error during clump setup.",
                g_LtMapVars.mainPath);
        ToolAPIReportError(api, g_LtMapVars.msg);
         */

        return (NULL);
    }

    RwV3dScale(&clumpSphere->center,
        &clumpSphere->center, 1.0f / RpClumpGetNumAtomics (clump));

    /*
     * Now, given the clump's bounding sphere center, determine the radius
     * by finding the greatest distance from the center that encloses all
     * the atomics' bounding spheres.  All morph targets, atomics and animations
     * are taken into account
     */
    clumpSphere->radius = -RwRealMAXVAL;

    if (RpClumpForAllAtomics(clump, LtMapAtomicCompareBSphereCB, clumpSphere) != clump)
    {
        /*
        sprintf(g_LtMapVars.msg, "File %s : Error during clump setup.",
                g_LtMapVars.mainPath);
        ToolAPIReportError(api, g_LtMapVars.msg);
         */

        return (NULL);
    }

    return (clump);
}

/*
 *
 */
static RwInt32
LtMapInitialiseParams( ToolAPI *api __RWUNUSED__ )
{
    RwChar      **p;

    g_LtMapVars.postDarkMap = g_DarkMap;
    g_LtMapVars.postCalcLumIndex = g_CalcLumIndex;
    g_LtMapVars.areaLightOn = g_ALOn;
    g_LtMapVars.areaLightDensity = g_ALDensity;
    g_LtMapVars.areaLightDensityModifier = g_ALDensityMod;
    g_LtMapVars.areaLightRadiusModifier = g_ALRadiusMod;
    g_LtMapVars.areaLightErrorCutoff = g_ALErrorCutoff;
    g_LtMapVars.ltMapVertexWield = g_VtxWeld;
    g_LtMapVars.ltMapSliver = g_Sliver;
    g_LtMapVars.ltMapDensity = g_LtMapDensity;
    g_LtMapVars.ltMapSize = g_LtMapSize;
    g_LtMapVars.ltMapSuperSample = g_LtMapSuperSample;
    g_LtMapVars.ltMapRasterFormat = g_LtMapRasterFormat | rwRASTERTYPETEXTURE;

    p = (RwChar **) _rwSListGetNewEntry(g_LtMapVars.fileList[LTMAPOBJLISTMAIN], 0);
    *p = g_LtMapVars.mainPath;

    return (1);
}

/*
 *
 */
static RpMaterial *
LtMapSetupMaterialsCB(RpMaterial *mat, void *data __RWUNUSED__)
{
    /* Most textures need to be lightmapped */
    RtLtMapMaterialSetFlags(mat,
        RtLtMapMaterialGetFlags(mat) | rtLTMAPMATERIALLIGHTMAP);

    return (mat);
}


/*
 *
 */
static RpWorldSector *
LtMapSectorSetupCB(RpWorldSector *sector, void * data __RWUNUSED__)
{
    /* All objects get lightmapped by default */
    RtLtMapWorldSectorSetFlags(sector,
        RtLtMapWorldSectorGetFlags(sector) | rtLTMAPOBJECTLIGHTMAP);

    return (sector);
}

/*
 *
 */
static RpAtomic *
LtMapAtomicSetupCB(RpAtomic *atomic, void *data)
{
    RpGeometry  *geometry;
    RwInt32     type;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        type = (RwInt32) data;

        if ((type == LTMAPOBJLISTLIGHTMAP) || (type == LTMAPOBJLISTMAIN))
        {
            /* Can only lightmap objects with a second set of UVs. */
            if (RpGeometryGetNumTexCoordSets(geometry) >= 2)
            {
                /* Set up other material properties */
                if (RpGeometryForAllMaterials(geometry, LtMapSetupMaterialsCB, NULL) != geometry)
                {
                    /*
                    sprintf(g_LtMapVars.msg, "File %s : Error during material setup.",
                            g_LtMapVars.mainPath);
                    ToolAPIReportError(api, g_LtMapVars.msg);
                    */
                    return (NULL);
                }

                RtLtMapAtomicSetFlags(atomic,
                    (RtLtMapAtomicGetFlags(atomic) | rtLTMAPOBJECTLIGHTMAP));
            }
            else
            {
                RtLtMapAtomicSetFlags(atomic,
                    (RtLtMapAtomicGetFlags(atomic) & ~rtLTMAPOBJECTLIGHTMAP));
            }
        }
        else if (type == LTMAPOBJLISTOCCLUDER)
        {
            RtLtMapAtomicSetFlags(atomic,
                (RtLtMapAtomicGetFlags(atomic) & ~rtLTMAPOBJECTLIGHTMAP));
        }
        else if (type == LTMAPOBJLISTVERTEXLIT)
        {
            RtLtMapAtomicSetFlags(atomic,
                (RtLtMapAtomicGetFlags(atomic) | rtLTMAPOBJECTVERTEXLIGHT) & ~rtLTMAPOBJECTLIGHTMAP);
        }
        else
        {

        }
    }

    return (atomic);
}

/*
 *
 */
static LtMapChunkList *
LtMapChunkListAdd( LtMapChunkList *chkListHead, LtMapChunkList *chkList )
{
    if (chkListHead)
    {
        chkList->next = chkListHead->next;

        chkListHead->next = chkList;
    }
    else
    {
        chkList->next = chkList;
    }

    return (chkList);
}

/*
 *
 */
static LtMapChunkList *
LtMapWorldStreamRead( ToolAPI *api, RwStream *stream, RwChunkHeaderInfo *chkHdr, RwChar *filename)
{
    RwInt32             err;
    LtMapChunkList      *chkList;
    RpWorld             *world;

    err = 0;

    chkList = RwFreeListAlloc(g_LtMapVars.chkListFList, 0);

    if (chkList)
    {
        world = RpWorldStreamRead(stream);

        if (world)
        {
            /* Detect bad data. There must be two sets of texture co-ordinates. */
            if (!(RpWorldGetFlags(world) & rpWORLDTEXTURED2))
            {
                sprintf(g_LtMapVars.msg, "File %s : World must contain at least two texture coordinate sets.",
                        g_LtMapVars.mainPath);
                ToolAPIReportError(api, g_LtMapVars.msg);
                err = 1;
            }
            else
            {
                chkList->chkHdr = *chkHdr;
                chkList->data = (void *) world;
                chkList->filename = filename;
            }
        }
        else
        {
            /* Read error. Raise error and stop. */
            sprintf(g_LtMapVars.msg, "File %s : File <%s> read error.",
                    g_LtMapVars.mainPath, filename);
            ToolAPIReportError(api, g_LtMapVars.msg);
            err = 1;
        }
    }
    else
    {
        /* Memory failure. Raise error and stop. */
        sprintf(g_LtMapVars.msg, "File %s : Memory failure.",
                g_LtMapVars.mainPath);
        ToolAPIReportError(api, g_LtMapVars.msg);
        err = 1;
    }

    if (err)
    {
        if (chkList)
            RwFreeListFree(g_LtMapVars.chkListFList, chkList);
        chkList = NULL;

        if (world)
            RpWorldDestroy(world);
    }

    return (chkList);
}

/*
 *
 */
static LtMapChunkList *
LtMapTexDictStreamRead( ToolAPI *api, RwStream *stream, RwChunkHeaderInfo *chkHdr, RwChar *filename)
{
    RwInt32             err;
    LtMapChunkList      *chkList;
    RwTexDictionary     *txDict;

    err = 0;

    chkList = RwFreeListAlloc(g_LtMapVars.chkListFList, 0);

    if (chkList)
    {
        if (chkHdr->type == rwID_TEXDICTIONARY)
        {
            txDict = RwTexDictionaryStreamRead(stream);
        }
        else
        {
            txDict = RtPITexDictionaryStreamRead(stream);
        }

        if (txDict)
        {
            chkList->chkHdr = *chkHdr;
            chkList->data = (void *) txDict;
            chkList->filename = filename;
        }
        else
        {
            /* Read error. Raise error and stop. */
            sprintf(g_LtMapVars.msg, "File %s : File <%s> read error.",
                    g_LtMapVars.mainPath, filename);
            ToolAPIReportError(api, g_LtMapVars.msg);
            err = 1;
        }
    }
    else
    {
        /* Memory failure. Raise error and stop. */
        sprintf(g_LtMapVars.msg, "File %s : Memory failure.",
                g_LtMapVars.mainPath);
        ToolAPIReportError(api, g_LtMapVars.msg);
        err = 1;
    }

    if (err)
    {
        if (chkList)
            RwFreeListFree(g_LtMapVars.chkListFList, chkList);
        chkList = NULL;

        if (txDict)
            RwTexDictionaryDestroy(txDict);
    }

    return (chkList);
}

/*
 *
 */
static LtMapChunkList *
LtMapClumpStreamRead( ToolAPI *api, RwStream *stream, RwChunkHeaderInfo *chkHdr, RwChar *filename)
{
    RwInt32             err = 1;
    LtMapChunkList      *chkList;
    RpClump             *clump;

    err = 0;

    chkList = RwFreeListAlloc(g_LtMapVars.chkListFList, 0);

    if (chkList)
    {
        clump = RpClumpStreamRead(stream);

        if (clump)
        {
            chkList->chkHdr = *chkHdr;
            chkList->data = (void *) clump;
            chkList->filename = filename;
        }
        else
        {
            /* Read error. Raise error and stop. */
            sprintf(g_LtMapVars.msg, "File %s : File <%s> read error.",
                    g_LtMapVars.mainPath, filename);
            ToolAPIReportError(api, g_LtMapVars.msg);
            err = 1;
        }
    }
    else
    {
        /* Memory failure. Raise error and stop. */
        sprintf(g_LtMapVars.msg, "File %s : Memory failure.",
                g_LtMapVars.mainPath);
        ToolAPIReportError(api, g_LtMapVars.msg);
        err = 1;
    }

    if (err)
    {
        if (chkList)
            RwFreeListFree(g_LtMapVars.chkListFList, chkList);
        chkList = NULL;

        if (clump)
            RpClumpDestroy(clump);
    }

    return (chkList);
}

/*
 *
 */
static LtMapChunkList *
LtMapDataStreamRead( ToolAPI *api, RwStream *stream, RwChunkHeaderInfo *chkHdr, RwChar *filename)
{
    RwInt32             err;
    LtMapChunkList      *chkList;
    void                *data;

    err = 0;

    data = NULL;
    chkList = RwFreeListAlloc(g_LtMapVars.chkListFList, 0);

    if (chkList)
    {
        if (chkHdr->length > 0)
        {
            data = RwMalloc(chkHdr->length, 0);

            if (data)
            {
                if (RwStreamRead(stream, data, chkHdr->length) == chkHdr->length)
                {
                    chkList->chkHdr = *chkHdr;
                    chkList->data = (void *) data;
                    chkList->filename = filename;
                }
                else
                {
                    /* Read error. Raise error and stop. */
                    sprintf(g_LtMapVars.msg, "File %s : File <%s> read error.",
                            g_LtMapVars.mainPath, filename);
                    ToolAPIReportError(api, g_LtMapVars.msg);
                    err = 1;
                }
            }
            else
            {
                sprintf(g_LtMapVars.msg, "File %s : Memory failure.",
                        g_LtMapVars.mainPath);
                ToolAPIReportError(api, g_LtMapVars.msg);
                err = 1;
            }
        }
        else
        {
            /* Empty chunk. */
            chkList->chkHdr = *chkHdr;
            chkList->data = (void *) NULL;
            chkList->filename = filename;
        }
    }
    else
    {
        /* Memory failure. Raise error and stop. */
        sprintf(g_LtMapVars.msg, "File %s : Memory failure.",
                g_LtMapVars.mainPath);
        ToolAPIReportError(api, g_LtMapVars.msg);
        err = 1;
    }

    if (err)
    {
        if (chkList)
            RwFreeListFree(g_LtMapVars.chkListFList, chkList);
        chkList = NULL;

        if (data)
            RwFree(data);
    }

    return (chkList);
}

static RpWorld *
LtMapEmptyWorldCreate( ToolAPI *api )
{
    RwInt32             i, err;
    RwReal              dist, curDist, f;
    RpWorld             *world;
    RwBBox              bbox;
    RwSphere            clumpSphere;
    LtMapChunkList      *chkListHead, *chkList;

    err = 0;

    /* Calculate the bounding sphere of all the clumps. */
    curDist = -RwRealMAXVAL;

    for (i = 0; i < LTMAPNUMOBJLIST; i++)
    {
        chkListHead = g_LtMapVars.chkList[i];
        chkList = chkListHead;

        if (chkListHead)
        {
            do
            {
                if (chkList->chkHdr.type == rwID_CLUMP)
                {
                    if (LtMapClumpGetBSphere((RpClump *) chkList->data, &clumpSphere)
                        != (RpClump *) chkList->data)
                    {
                        sprintf(g_LtMapVars.msg, "File %s : Error during clump setup.",
                                g_LtMapVars.mainPath);
                        ToolAPIReportError(api, g_LtMapVars.msg);

                        err = 1;
                    }
                    else
                    {
                        f = RwV3dDotProduct(&clumpSphere.center, &clumpSphere.center);

                        if (f > (RwReal) 0.0)
                            rwSqrt(&dist, f);
                        else
                            dist = f;

                        dist += clumpSphere.radius;

                        if (curDist < dist)
                            curDist = dist;
                    }
                }

                chkList = chkList->next;
            }
            while ((err == 0) && (chkListHead != chkList));
        }
    }

    if (err == 0)
    {
        bbox.inf.x = bbox.inf.y = bbox.inf.z = -curDist;
        bbox.sup.x = bbox.sup.y = bbox.sup.z =  curDist;

        world = RpWorldCreate(&bbox);
    }
    else
    {
        return ((RpWorld *) NULL);
    }

    return (world);
}

/*
 *
 */
static RwBool
LtMapPostLoadData( ToolAPI *api )
{
    RwInt32             i, err;
    LtMapChunkList      *chkList, *chkListHead;

    err = 0;

    /* Check if a world was streamed in. If not, then create a dummy world. */
    if (g_LtMapVars.world == NULL)
    {
        g_LtMapVars.world = LtMapEmptyWorldCreate(api);

        if (g_LtMapVars.world == NULL)
        {
            /* Empty world creation failure. Raise error and stop. */
            err = 1;
        }
    }
    else
    {
        /* Set up sector properties for the BSP */
        RpWorldForAllWorldSectors(g_LtMapVars.world, LtMapSectorSetupCB, NULL);

        /* Set up the world's materials. */
        RpWorldForAllMaterials(g_LtMapVars.world, LtMapSetupMaterialsCB, NULL);
    }

    /* Transfer all the clumps to the world. */
    if (err == 0)
    {
        for (i = 0; i < LTMAPNUMOBJLIST; i++)
        {
            chkListHead = g_LtMapVars.chkList[i];
            chkList = chkListHead;

            if (chkListHead)
            {
                do
                {
                    if (chkList->chkHdr.type == rwID_CLUMP)
                    {
                        RpWorldAddClump(g_LtMapVars.world, (RpClump *) chkList->data);

                        /* Set up the atomics. */
                        if (RpClumpForAllAtomics((RpClump *) chkList->data, LtMapAtomicSetupCB, (void *) i) != (RpClump *) chkList->data)
                        {
                            sprintf(g_LtMapVars.msg, "File %s : Error during clump setup.",
                                    g_LtMapVars.mainPath);
                            ToolAPIReportError(api, g_LtMapVars.msg);

                            err = 1;
                        }
                    }

                    chkList = chkList->next;
                }
                while ((err == 0) && (chkListHead != chkList));
            }
        }
    }

    return (err);
}

/*
 *
 */
static RwBool
LtMapPreSaveData( ToolAPI *api __RWUNUSED__ )
{
    RwInt32             i, err;
    LtMapChunkList      *chkList, *chkListHead;

    err = 0;

    /* Transfer all the clumps out of the world. */
    if (err == 0)
    {
        for (i = 0; i < LTMAPNUMOBJLIST; i++)
        {
            chkListHead = g_LtMapVars.chkList[i];
            chkList = chkListHead;

            if (chkListHead)
            {
                do
                {
                    if (chkList->chkHdr.type == rwID_CLUMP)
                    {
                        RpWorldRemoveClump(g_LtMapVars.world, (RpClump *) chkList->data);
                    }

                    chkList = chkList->next;
                }
                while ((err == 0) && (chkListHead != chkList));
            }
        }
    }

    return (err);
}


/*
 *
 */
static RwInt32
LtMapLoadData( ToolAPI *api )
{
    RwInt32             err, txDictCount, worldCount, i, j, num;
    RwChar              *filename;
    RwChunkHeaderInfo   chkHdr;
    RwStream            *stream;
    RwTexDictionary     *txDict;
    LtMapChunkList      *chkList, *chkListHead;

    err = 0;

    /* Create a dictionary. */
    worldCount = 0;
    txDictCount = 0;
    g_LtMapVars.txDict = RwTexDictionaryCreate();

    if (NULL == g_LtMapVars.txDict)
    {
        sprintf(g_LtMapVars.msg, "File %s : Failed to create texture dictionary.",
                g_LtMapVars.mainPath);
        ToolAPIReportError(api, g_LtMapVars.msg);
        err = 1;
    }
    else
    {
        txDictCount++;
        RwTexDictionarySetCurrent(g_LtMapVars.txDict);
    }

    g_LtMapVars.world = NULL;

    /* Check and load any lightmap obj data, texture dictionarys or clumps. */
    for (i = 0; i < LTMAPNUMOBJLIST; i++)
    {
        chkListHead = g_LtMapVars.chkList[i];

        num = _rwSListGetNumEntries(g_LtMapVars.fileList[i]);
        j = 0;

        while ((j < num) && (!err))
        {
            /* Grab the filename and append it to the output stream. */
            filename = *(RwChar **) _rwSListGetEntry(g_LtMapVars.fileList[i], j);
            j++;

            stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);

            if (NULL != stream)
            {
                while ((!err) && (RwStreamReadChunkHeaderInfo(stream, &chkHdr) == stream))
                {
                    if (chkHdr.type == rwID_WORLD)
                    {
                        if (worldCount > 1)
                        {
                            /* Multiple dictionary found. Raise error and stop. */
                            sprintf(g_LtMapVars.msg, "File %s : Multiple world found.",
                                    g_LtMapVars.mainPath);
                            ToolAPIReportError(api, g_LtMapVars.msg);
                            err = 1;
                        }
                        else
                        {
                            chkList = LtMapWorldStreamRead(api, stream, &chkHdr, filename);
                            if (chkList)
                            {
                                chkListHead = LtMapChunkListAdd(chkListHead, chkList);

                                worldCount++;

                                g_LtMapVars.world = (RpWorld *) chkList->data;
                            }
                            else
                            {
                                err = 1;
                            }
                        }
                    }
                    else if (chkHdr.type == rwID_CLUMP)
                    {
                        chkList = LtMapClumpStreamRead(api, stream, &chkHdr, filename);
                        if (g_LtMapVars.txDict)
                        {
                            chkListHead = LtMapChunkListAdd(chkListHead, chkList);
                        }
                        else
                        {
                            err = 1;
                        }
                    }
                    else if (chkHdr.type == rwID_PITEXDICTIONARY)
                    {
                        if (txDictCount > 1)
                        {
                            /* Multiple dictionary found. Raise error and stop. */
                            sprintf(g_LtMapVars.msg, "File %s : Multiple texture dictionary found.",
                                    g_LtMapVars.mainPath);
                            ToolAPIReportError(api, g_LtMapVars.msg);
                            err = 1;
                        }
                        else
                        {
                            /* It is platform independant. */
                            g_LtMapVars.piTxDict = TRUE;
                            chkList = LtMapTexDictStreamRead(api, stream, &chkHdr, filename);
                            if (chkList)
                            {
                                chkListHead = LtMapChunkListAdd(chkListHead, chkList);
                                txDictCount++;

                                /* Destroy the temp one. */
                                g_LtMapVars.txDict = (RwTexDictionary *) chkList->data;
                                txDict = RwTexDictionaryGetCurrent();
                                RwTexDictionarySetCurrent(g_LtMapVars.txDict);
                                RwTexDictionaryDestroy(txDict);
                            }
                            else
                            {
                                err = 1;
                            }
                        }
                    }
                    else if (chkHdr.type == rwID_TEXDICTIONARY)
                    {
                        if (txDictCount > 1)
                        {
                            /* Multiple dictionary found. Raise error and stop. */
                            sprintf(g_LtMapVars.msg, "File %s : Multiple texture dictionary found.",
                                    g_LtMapVars.mainPath);
                            ToolAPIReportError(api, g_LtMapVars.msg);
                            err = 1;
                        }
                        else
                        {
                            /* It is platform dependant. */
                            g_LtMapVars.piTxDict = FALSE;
                            chkList = LtMapTexDictStreamRead(api, stream, &chkHdr, filename);
                            if (chkList)
                            {
                                chkListHead = LtMapChunkListAdd(chkListHead, chkList);
                                txDictCount++;

                                /* Destroy the temp one. */
                                g_LtMapVars.txDict = (RwTexDictionary *) chkList->data;
                                txDict = RwTexDictionaryGetCurrent();
                                RwTexDictionarySetCurrent(g_LtMapVars.txDict);
                                RwTexDictionaryDestroy(txDict);
                            }
                            else
                            {
                                err = 1;
                            }
                        }
                    }
                    else
                    {
                        /* Unsupported lightmap chunks. */

                        /* Add it to the main file only. */
                        if (i == LTMAPOBJLISTMAIN)
                        {
                            chkList = LtMapDataStreamRead(api, stream, &chkHdr, filename);
                            if (chkList)
                            {
                                chkListHead = LtMapChunkListAdd(chkListHead, chkList);
                            }
                            else
                            {
                                err = 1;
                            }
                        }
                        else
                        {
                            RwStreamSkip(stream, chkHdr.length);
                        }

                    }
                }

                RwStreamClose(stream, NULL);
            }
            else
            {
                sprintf(g_LtMapVars.msg, "File %s : File <%s> not found",
                        g_LtMapVars.mainPath, filename);
                ToolAPIReportError(api, g_LtMapVars.msg);
                err = 1;
            }
        }

        /* Save the new list head. */
        g_LtMapVars.chkList[i] = chkListHead;
    }

    return (err);
}

/*
 *
 */
static RwInt32
LtMapSaveData( ToolAPI *api )
{
    RwInt32             err;
    RwStream            *stream;
    RwChar              *filename, *c;
    LtMapChunkList      *chkList, *chkListHead;
    RwBool              txDictOut;
    RwInt32             i;

    err = 0;

    /* Generate the filename. */
    txDictOut = 0;
    filename = g_LtMapVars.buffer;

    for (i = 0; i < NUMOBJLIST; i++)
    {
        /* Only save out vertex lit or light mapped objects. */
        if ((i == LTMAPOBJLISTLIGHTMAP) || (i == LTMAPOBJLISTVERTEXLIT))
        {
            if (g_LtMapVars.chkList[i])
            {
                chkListHead = g_LtMapVars.chkList[i]->next;
                chkList = chkListHead;

                do
                {
                    if (1)
                    {
                        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, chkList->filename);

                        if (stream)
                        {
                            if (chkList->chkHdr.type == rwID_TEXDICTIONARY)
                            {
                                if (RwTexDictionaryStreamWrite(g_LtMapVars.txDict, stream) != g_LtMapVars.txDict)
                                {
                                    /* Output error. Raise error and stop. */
                                    err = 1;
                                }
                                else
                                {
                                    txDictOut = TRUE;
                                }
                            }
                            else if (chkList->chkHdr.type == rwID_PITEXDICTIONARY)
                            {
                                if (RtPITexDictionaryStreamWrite(g_LtMapVars.txDict, stream) != g_LtMapVars.txDict)
                                {
                                    /* Output error. Raise error and stop. */
                                    err = 1;
                                }
                                else
                                {
                                    txDictOut = TRUE;
                                }
                            }
                            else if (chkList->chkHdr.type == rwID_CLUMP)
                            {
                                if (RpClumpStreamWrite((RpClump *) chkList->data, stream) != (RpClump *) chkList->data)
                                {
                                    /* Output error. Raise error and stop. */
                                    err = 1;
                                }
                            }
                            else
                            {
                                /* Unrecognised chunk. Raise warning and cont. */
                                sprintf(g_LtMapVars.msg, "File %s : Unrecognised chunk found during output.",
                                        g_LtMapVars.mainPath);
                                ToolAPIReportWarning(api, g_LtMapVars.msg);
                            }

                            if (err)
                            {
                                /* Display error. */
                                sprintf(g_LtMapVars.msg, "File %s : File <%s> write error.",
                                        g_LtMapVars.mainPath, filename);
                                ToolAPIReportWarning(api, g_LtMapVars.msg);
                            }

                            RwStreamClose(stream, NULL);
                        }
                        else
                        {
                            /* File open failure. Raise error and stop. */
                            sprintf(g_LtMapVars.msg, "File %s : Unable to open File <%s> for output.",
                                    g_LtMapVars.mainPath, filename);
                            ToolAPIReportError(api, g_LtMapVars.msg);
                            err = 1;
                        }
                    }

                    chkList = chkList->next;
                }
                while ((!err) && (chkList != chkListHead));
            }
        }
    }


    /* Save out the main file. */
    if (0 == err)
    {
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, g_LtMapVars.mainPath);

        if (stream)
        {
            chkListHead = g_LtMapVars.chkList[LTMAPOBJLISTMAIN]->next;
            chkList = chkListHead;

            do
            {
                if (chkList->chkHdr.type == rwID_TEXDICTIONARY)
                {
                    if (RwTexDictionaryStreamWrite(g_LtMapVars.txDict, stream) != g_LtMapVars.txDict)
                    {
                        err = 1;
                    }
                    else
                    {
                        txDictOut = TRUE;
                    }
                }
                else if (chkList->chkHdr.type == rwID_PITEXDICTIONARY)
                {
                    if (RtPITexDictionaryStreamWrite(g_LtMapVars.txDict, stream) != g_LtMapVars.txDict)
                    {
                        err = 1;
                    }
                    else
                    {
                        txDictOut = TRUE;
                    }
                }
                else if (chkList->chkHdr.type == rwID_CLUMP)
                {
                    if (RpClumpStreamWrite((RpClump *) chkList->data, stream) != (RpClump *) chkList->data)
                        err = 1;
                }
                else if (chkList->chkHdr.type == rwID_WORLD)
                {
                    if (RpWorldStreamWrite(g_LtMapVars.world, stream) != g_LtMapVars.world)
                        err = 1;
                }
                else
                {
                    /* Write out the header and data. */
                    _rwStreamWriteVersionedChunkHeader(stream,
                        chkList->chkHdr.type, chkList->chkHdr.length, chkList->chkHdr.version, chkList->chkHdr.buildNum);

                    if (chkList->data)
                        RwStreamWrite(stream, chkList->data, chkList->chkHdr.length);
                }

                if (err)
                {
                    /* Display error. */
                    sprintf(g_LtMapVars.msg, "File %s : File <%s> write error.",
                            g_LtMapVars.mainPath, filename);
                    ToolAPIReportWarning(api, g_LtMapVars.msg);
                }

                chkList = chkList->next;
            }
            while ((!err) && (chkList != chkListHead));

            RwStreamClose(stream, NULL);
        }
        else
        {
            /* File open failure. Raise error and stop. */
            sprintf(g_LtMapVars.msg, "File %s : Unable to open file <%s> for output.",
                    g_LtMapVars.mainPath, filename);
            ToolAPIReportError(api, g_LtMapVars.msg);
            err = 1;
        }
    }

    /* Check if a texture dictionary was written out. Write one out if required. */
    if (txDictOut == FALSE)
    {
        c = strrchr(g_LtMapVars.mainPath, '.');
        if (c)
            *c = '\0';

        rwsprintf(filename, "%s_pilm.txd", g_LtMapVars.mainPath);

        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, filename);

        if (stream)
        {
            if (RtPITexDictionaryStreamWrite(g_LtMapVars.txDict, stream) != g_LtMapVars.txDict)
            {
                /* Output error. Raise error and stop. */
                sprintf(g_LtMapVars.msg, "File %s : File <%s> write error.",
                        g_LtMapVars.mainPath, filename);
                ToolAPIReportWarning(api, g_LtMapVars.msg);
                err = 1;
            }

            RwStreamClose(stream, NULL);
        }
        else
        {
            /* File open failure. Raise error and stop. */
            sprintf(g_LtMapVars.msg, "File %s : Unable to open file <%s> for output.",
                    g_LtMapVars.mainPath, filename);
            ToolAPIReportError(api, g_LtMapVars.msg);
            err = 1;
        }

        if (c)
            *c = '.';
    }

    return (err);
}


/*
 *
 */
static RwInt32
LtMapCleanup( ToolAPI *api __RWUNUSED__ , RtLtMapLightingSession *session )
{
    RwInt32             err, i;
    LtMapChunkList      *chkList, *chkListHead, *chkListNext;

    err = 0;

    /* Calculate the bounding sphere of all the clumps. */
    for (i = 0; i < LTMAPNUMOBJLIST; i++)
    {
        chkListHead = g_LtMapVars.chkList[i];

        if (chkListHead)
        {
            chkListHead = chkListHead->next;
            chkList = chkListHead;

            do
            {
                if (chkList->chkHdr.type == rwID_TEXDICTIONARY)
                {
                    /* Do nothing. */
                }
                else if (chkList->chkHdr.type == rwID_PITEXDICTIONARY)
                {
                    /* Do nothing. */
                }
                else if (chkList->chkHdr.type == rwID_CLUMP)
                {
                    RpWorldRemoveClump(g_LtMapVars.world, (RpClump *)chkList->data);
                    RpClumpDestroy((RpClump *) chkList->data);
                }
                else if (chkList->chkHdr.type == rwID_WORLD)
                {
                    /* Do nothing. */
                }
                else
                {
                    if (chkList->data)
                        RwFree(chkList->data);
                }

                chkListNext = chkList->next;

                RwFreeListFree(g_LtMapVars.chkListFList, chkList);

                chkList = chkListNext;
            }
            while (chkListHead != chkList);
        }
    }

    /* Do lightmaps exist for the world ? */
    if (0 && g_LtMapVars.world)
    {
        if (RpLtMapWorldLightMapsQuery(g_LtMapVars.world) > 0)
        {
            /* Destroy the lightmaps in the scene */
            RtLtMapLightMapsDestroy(session);
        }
    }

    /* Destroy the world. */
    if (g_LtMapVars.world)
        RpWorldDestroy(g_LtMapVars.world);

    g_LtMapVars.world = NULL;

    /* Destroy the texture dictionary. */
    if (g_LtMapVars.txDict)
        RwTexDictionaryDestroy(g_LtMapVars.txDict);

    g_LtMapVars.txDict = NULL;
    RwTexDictionarySetCurrent(g_LtMapVars.defaultTxDict);

    return (err);
}

/*
 *
 */
static RwInt32
LtMapPluginAttach(ToolAPI *api __RWUNUSED__ )
{
    /* It is important that we attach this, otherwise a world
     * saved on a PC will lose KL values required when the world
     * is loaded on PS2 - resulting in blurry textures.
     * Note: you shouldn't add this plugin for SKY2 since the driver
     *       adds it automatically and adding it twice causes a crash. */
    if(!RpMipmapKLPluginAttach())
    {
        return(1);
    }

    /* Attach world plug-in... */
    if(!RpWorldPluginAttach())
    {
        return(1);
    }

    /* Attach collision plug-in */
    if(!RpCollisionPluginAttach())
    {
        return(1);
    }

    /* Attach PVS plugin */
    if (!RpPVSPluginAttach())
    {
        return(1);
    }

    /* Attach random plug-in... */
    if(!RpRandomPluginAttach())
    {
        return(1);
    }

    /* Attach lightmap plug-in... */
    if(!RpLtMapPluginAttach())
    {
        return(1);
    }

    return (0);
}

static RwInt32
LtMapStartup(ToolAPI *api __RWUNUSED__, RwChar *currentPath __RWUNUSED__ )
{
    RwInt32             i;

    memset(&g_LtMapVars, 0, sizeof(LtMapGlobals));

    /* Register image format. */
    if( !RwImageRegisterImageFormat(RWSTRING("bmp"), RtBMPImageRead, RtBMPImageWrite) )
    {
        return (1);
    }

    if( !RwImageRegisterImageFormat(RWSTRING("png"), RtPNGImageRead, RtPNGImageWrite) )
    {
        return (1);
    }

    for (i = 0; i < LTMAPNUMOBJLIST; i++)
    {
        g_LtMapVars.fileList[i] = rwSListCreate(sizeof(char *), 0);
        if (g_LtMapVars.fileList[i] == NULL)
            return (1);
    }

    g_ObjFilesList = g_LtMapVars.fileList[LTMAPOBJLISTLIGHTMAP];
    g_OccFilesList = g_LtMapVars.fileList[LTMAPOBJLISTOCCLUDER];
    g_VltFilesList = g_LtMapVars.fileList[LTMAPOBJLISTVERTEXLIT];

    g_LtMapVars.chkListFList =
        RwFreeListCreate(sizeof(LtMapChunkList), 20, 0, 0);
    if (g_LtMapVars.chkListFList == NULL)
    {
        return (1);
    }

    return(0);
}


/*
 *
 */
static RwInt32
LtMapShutdown(ToolAPI *api __RWUNUSED__ )
{
    RwInt32             i;

    if (g_LtMapVars.world)
        RpWorldDestroy(g_LtMapVars.world);

    if (g_LtMapVars.txDict)
        RwTexDictionaryDestroy(g_LtMapVars.txDict);

    for (i = 0; i < LTMAPNUMOBJLIST; i++)
    {
        if (g_LtMapVars.fileList[i])
            rwSListDestroy(g_LtMapVars.fileList[i]);

        g_LtMapVars.fileList[i] = NULL;
    }

    if (g_LtMapVars.chkListFList)
        RwFreeListDestroy(g_LtMapVars.chkListFList);

    g_LtMapVars.chkListFList = NULL;

    g_ObjFilesList = NULL;
    g_OccFilesList = NULL;

    g_LtMapVars.world = NULL;
    g_LtMapVars.txDict = NULL;

    return 0;
}

/*
 *
 */
static RwInt32
LtMapProcessFile( ToolAPI *api, RwChar *mainPath )
{
    RwBool          err;

    err = 0;

    g_LtMapVars.mainPath = mainPath;
    g_ToolAPI = api;

    /* Initialise the globals. */
    LtMapInitialiseParams(api);

    g_LtMapVars.defaultTxDict = RwTexDictionaryGetCurrent();

    /* Load the world and/or texture dictionary. */
    err = LtMapLoadData(api);

    if (0 == err)
        err = LtMapPostLoadData(api);

    /* Initialise the session. */
    if (0 == err)
        RtLtMapLightingSessionInitialize(&g_LtMapVars.session, g_LtMapVars.world);

    /* First create the lightmaps. */
    if (0 == err)
        err = LtMapCreateLtMap(api, &g_LtMapVars.session);

    /* Enable the progress callback. */
    g_LtMapVars.progress = -1;
    g_LtMapVars.session.progressCallBack = LtMapProgressCallBack;

    /* Set up the session for lighting. */
    if (0 == err)
        err = LtMapIlluminate(api, &g_LtMapVars.session);

    /* Post process the lightmaps. */
    if (0 == err)
        err = LtMapPostProcess(api, &g_LtMapVars.session);

    RtLtMapLightingSessionDeInitialize(&g_LtMapVars.session);

    /* Pre process the data before saving. */
    if (0 == err)
        err = LtMapPreSaveData(api);

    /* Save the world and texdictionary. */
    if (0 == err)
        err = LtMapSaveData(api);

    /* Clean up */
    LtMapCleanup(api, &g_LtMapVars.session);

    return (err);
}

/*
 *
 */
int main(int argc, char* argv[])
{
    ToolAPI api;
    RwInt32 err;

    ToolAPIInitAPI(&api, RWSTRING("rwltmap"), RWSTRING("Lightmap generator toolkit"),
                   ToolOptions, NumToolOptions);

    api.userStartupFn = LtMapStartup;
    api.userProcessFileFn = LtMapProcessFile;
    api.userShutdownFn = LtMapShutdown;
    api.userPluginAttachFn = LtMapPluginAttach;

    err = ToolAPIExecute(&api, argc, argv);

    ToolAPIShutdownAPI(&api);

    return(err);
}

