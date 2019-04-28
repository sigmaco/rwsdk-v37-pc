

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

/*
 *
 */

#ifdef UNUSED

static RpMesh *
LtMapMeshPreLightClearCB(RpMesh * mesh, RpMeshHeader * meshHeader __RWUNUSED__, void *pData)
{
    RwRGBA      black = {0, 0, 0, 255}, *preLit;
    RwUInt32    i;

    preLit = (RwRGBA *) pData;

    if (!(RtLtMapMaterialGetFlags(mesh->material) & rtLTMAPOBJECTVERTEXLIGHT))
    {
        /* Material is not vertex lit, so clear to black. */
        for (i = 0; i < mesh->numIndices; i++)
        {
            preLit[mesh->indices[i]] = black;
        }
    }

    return(mesh);
}

#endif /* UNUSED */

/*
 *
 */
#ifdef UNUSED

static RpWorldSector *
LtMapSectorPreLightClearCB(RpWorldSector *sector, void *data)
{
    RwBool clearVertexLitObjects = *(RwBool *)data;
    RwRGBA *preLit;

    /* Initialize prelights */
    if (NULL != sector->preLitLum)
    {
        /* We only clear vertex-lit objects if we're explicitly told to */
        if (TRUE == clearVertexLitObjects)
        {
            RwRGBA      clearCol = {128, 128, 128, 255}, black = {0, 0, 0, 255};
            RwUInt32    i;

            /* We clear lightmapped objects to black, non-lightmapped ones to white
             * (the checkerboared makes cleared, lightmapped objects visible) */
            if (NULL != RpLtMapWorldSectorGetLightMap(sector))
            {
                clearCol = black;
            }

            for (i = 0;i < (RwUInt32)sector->numVertices;i++)
            {
                sector->preLitLum[i] = clearCol;
            }
        }
        else if (!(RtLtMapWorldSectorGetFlags(sector) & rtLTMAPOBJECTVERTEXLIGHT))
        {
            /* Sector is not vertex lit, clear the prelit based on materials. */
            preLit = sector->preLitLum;

            RpWorldSectorForAllMeshes(sector, LtMapMeshPreLightClearCB, (void *) preLit);
        }
    }

    return(sector);
}

#endif /* UNUSED */

/*
 *
 */

#ifdef UNUSED

static RpAtomic *
LtMapAtomicPreLightClearCB(RpAtomic *atomic, void *data)
{
    RwBool clearVertexLitObjects = *(RwBool *)data;
    RpGeometry *geom = RpAtomicGetGeometry(atomic);
    RwRGBA *preLit;

    /* Initialize prelights */
    if (NULL != RpGeometryGetPreLightColors(geom))
    {
        /* We only clear vertex-lit objects if we're explicitly told to */
        if (TRUE == clearVertexLitObjects)
        {
            RwRGBA      clearCol = {128, 128, 128, 255}, black = {0, 0, 0, 255};
            RwUInt32    i;

            /* We clear lightmapped objects to black, non-lightmapped ones to white
             * (the checkerboared makes cleared, lightmapped objects visible) */
            if (NULL != RpLtMapAtomicGetLightMap(atomic))
            {
                clearCol = black;
            }

            RpGeometryLock(geom, rpGEOMETRYLOCKPRELIGHT);
            preLit = RpGeometryGetPreLightColors(geom);

            for (i = 0;i < (RwUInt32)RpGeometryGetNumVertices(geom);i++)
            {
               preLit[i] = clearCol;
            }

            RpGeometryUnlock(geom);
        }
        else if (!(RtLtMapAtomicGetFlags(atomic) & rtLTMAPOBJECTVERTEXLIGHT))
        {
            /* Atomic is not vertex lit, clear the prelit base on materials. */
            RpGeometryLock(geom, rpGEOMETRYLOCKPRELIGHT);
            preLit = RpGeometryGetPreLightColors(geom);

            RpGeometryForAllMeshes(geom, LtMapMeshPreLightClearCB, (void *) preLit);

            RpGeometryUnlock(geom);
        }
    }

    return(atomic);
}

#endif /* UNUSED */

/*
 *
 */

#ifdef UNUSED

static RpClump *
LtMapClumpPreLightClearCB(RpClump *clump, void *data)
{
    RpClumpForAllAtomics(clump, LtMapAtomicPreLightClearCB, data);

    return(clump);
}

#endif /* UNUSED */

/*
 *
 */
static RwInt32
LtMapSetupAreaLights( ToolAPI *api, RtLtMapLightingSession *session )
{
    RwInt32         err;

    err = 0;

    if (TRUE == g_LtMapVars.areaLightOn)
    {
        RtLtMapSetAreaLightDensityModifier(g_LtMapVars.areaLightDensityModifier);

        RtLtMapSetAreaLightRadiusModifier(g_LtMapVars.areaLightRadiusModifier);

        RtLtMapSetAreaLightErrorCutoff(g_LtMapVars.areaLightErrorCutoff);

        g_LtMapVars.areaLightGrp =
            RtLtMapAreaLightGroupCreate(session, g_LtMapVars.areaLightDensity);

        if (g_LtMapVars.areaLightGrp == NULL)
        {
            /* Error during area light creation. */
            sprintf(g_LtMapVars.msg, "File <%s> : Failed to create area light group.", g_LtMapVars.mainPath);

            ToolAPIReportError(api, g_LtMapVars.msg);

            err = 1;
        }
    }
    else
    {
        g_LtMapVars.areaLightGrp = NULL;
    }

    return (err);
}


/*
 *
 */
#ifdef UNUSED

static RwInt32
LtMapSetupWorld( ToolAPI *api __RWUNUSED__, RtLtMapLightingSession *session __RWUNUSED__ )
{
    RwInt32         err;
    RwBool          clear;

    err = 0;
    clear = FALSE;

    RpWorldForAllWorldSectors(
        g_LtMapVars.world, LtMapSectorPreLightClearCB, &clear);
    RpWorldForAllClumps(
        g_LtMapVars.world, LtMapClumpPreLightClearCB,  &clear);

    return (err);
}

#endif /* UNUSED */

/*
 *
 */
RwInt32
LtMapCreateLtMap( ToolAPI *api, RtLtMapLightingSession *session )
{
    RwInt32         err;

    err = 0;

    /* Progress. */
    sprintf(g_LtMapVars.msg, "File <%s> : Lightmap creation start.", g_LtMapVars.mainPath);
    ToolAPIReportLog(api, g_LtMapVars.msg);

    /* Do lightmaps exist for the world ? */
    if (RpLtMapWorldLightMapsQuery(g_LtMapVars.world))
    {
        /* Destroy the lightmaps in the scene */
        RtLtMapLightMapsDestroy(session);
    }

    /* Setup the raster format. */
    RpLtMapSetRasterFormat(g_LtMapVars.ltMapRasterFormat);

    RtLtMapLightMapSetDefaultSize(g_LtMapVars.ltMapSize);

    if ((RtLtMapLightMapsCreate(session, g_LtMapVars.ltMapDensity, (RwRGBA *)NULL) != session))
    {
        /* Error during light map creation. */
        sprintf(g_LtMapVars.msg, "File <%s> : LightMap creation error.", g_LtMapVars.mainPath);

        ToolAPIReportError(api, g_LtMapVars.msg);
        err = 1;
    }

    /* Progress. */
    sprintf(g_LtMapVars.msg, "File <%s> : Lightmap creation end.", g_LtMapVars.mainPath);
    ToolAPIReportLog(api, g_LtMapVars.msg);

    return (err);
}


/*
 *
 */
RwInt32
LtMapIlluminate( ToolAPI *api, RtLtMapLightingSession *session )
{
    RwInt32         err;
    RwInt32         numObj;

    err = 0;

    /* Progress. */
    sprintf(g_LtMapVars.msg, "File <%s> : Lightmap illumination start.", g_LtMapVars.mainPath);
    ToolAPIReportLog(api, g_LtMapVars.msg);

    /* Set up any area lights. */
    err = LtMapSetupAreaLights(api, session);

    if (0 == err)
    {
        /* Set up session to light all objects. */
        session->startObj = 0;
        session->numObj = 0;
        session->camera = NULL;

        numObj = RtLtMapIlluminate(session, g_LtMapVars.areaLightGrp, (RwUInt32) g_LtMapVars.ltMapSuperSample);

        if (numObj < 0)
        {
            /* Error during lighting. */
            sprintf(g_LtMapVars.msg, "File <%s> : Error during RtLtMapIlluminate.", g_LtMapVars.mainPath);

            ToolAPIReportError(api, g_LtMapVars.msg);
            err = 1;
        }
    }

    if (g_LtMapVars.areaLightGrp)
    {
        /* Destroy the area lights. */
        RtLtMapAreaLightGroupDestroy(g_LtMapVars.areaLightGrp);
    }

    /* Progress. */
    sprintf(g_LtMapVars.msg, "File <%s> : Lightmap illumination end.", g_LtMapVars.mainPath);
    ToolAPIReportLog(api, g_LtMapVars.msg);

    return (err);
}

/*
 *
 */
RwInt32
LtMapPostProcess( ToolAPI *api __RWUNUSED__, RtLtMapLightingSession *session )
{
    RwInt32         err;

    err = 0;

    /* Progress. */
    sprintf(g_LtMapVars.msg, "File <%s> : Lightmap post process start.", g_LtMapVars.mainPath);
    ToolAPIReportLog(api, g_LtMapVars.msg);

    /*
     * Invert the lightmaps into darkmaps before texture processing.
     */
    if (g_LtMapVars.postDarkMap)
    {
        RtLtMapSetLightMapProcessCallBack(RtLtMapSkyLightMapMakeDarkMap);
        RtLtMapLightingSessionLightMapProcess(session);
    }

    /* Post process the base texture. */
    if (g_LtMapVars.postCalcLumIndex == 1)
    {
        RtLtMapSkySetLumCalcCallBack(RtLtMapSkyLumCalcMaxCallBack);
        RtLtMapSkyLightingSessionBaseTexturesProcess(session);
    }
    else if (g_LtMapVars.postCalcLumIndex == 2)
    {
        RtLtMapSkySetLumCalcCallBack(RtLtMapSkyLumCalcSigmaCallBack);
        RtLtMapSkyLightingSessionBaseTexturesProcess(session);
    }

    /* Progress. */
    sprintf(g_LtMapVars.msg, "File <%s> : Lightmap post process end.", g_LtMapVars.mainPath);
    ToolAPIReportLog(api, g_LtMapVars.msg);

    return (err);
}

