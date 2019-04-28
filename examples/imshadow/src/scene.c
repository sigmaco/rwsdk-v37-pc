/****************************************************************************
 *
 * This file is a product of Criterion Software Ltd.
 *
 * This file is provided as is with no warranties of any kind and is
 * provided without any obligation on Criterion Software Ltd.
 * or Canon Inc. to assist in its use or modification.
 *
 * Criterion Software Ltd. and Canon Inc. will not, under any
 * circumstances, be liable for any lost revenue or other damages
 * arising from the use of this file.
 *
 * Copyright (c) 2001 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * scene.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example of parallel-projection shadow rendering using
 *          3D immediate mode.
 *
*****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpskin.h"
#include "rphanim.h"
#include "rpcollis.h"

#include "skeleton.h"
#include "menu.h"

#include "shadow.h"
#include "scene.h"

#include "pipeline.h"

extern RwCamera *Camera;

/*
 *  World objects
 */
static RpWorld     *World = NULL;
static RwChar       WorldTexturePath[] = RWSTRING("models/textures/");
static RwChar       WorldPath[] = RWSTRING("models/world.bsp");

static RpClump     *Clump = NULL;
static RwChar       ClumpPath[] = RWSTRING("models/clump.dff");
static const RwV3d  ClumpStartPos = { 30.0f, 46.0f, 0.0f };
static RwSphere     ClumpBoundingSphere;
static RwSphere     ClumpWorldBoundingSphere;

static RtAnimAnimation *Anim = NULL;
static RwChar       AnimPath[] = RWSTRING("models/clump.anm");
static RpHAnimHierarchy *Hierarchy = NULL;
static RwBool       AnimEnabled = TRUE;
static RwReal       AnimSpeedFactor = 0.3f; /* 1.0f is normal speed */

#define ANIM_CLUMP_SPEED  100.0f

/*
 *  Lights
 */
static RpLight     *DirectionalLight = NULL;
static RpLight     *AmbientLight = NULL;

#define ALIGHT_VALUE        0.2f /* Ambient light level */
#define DLIGHT_VALUE        0.8f /* Directional light intensity */
#define DLIGHT_ELEVATION    45.0f
#define DLIGHT_AZIMUTH      60.0f

/*
 *  Shadow
 */
static RwCamera    *ShadowCamera = NULL;
static RwRaster    *ShadowCameraRaster = NULL;

static RwCamera    *ShadowIPCamera = NULL;
static RwRaster    *ShadowRasterAA = NULL;
static RwRaster    *ShadowRasterTemp = NULL;

static RwRaster    *ShadowRenderRaster = NULL;

static RwV2d        ShadowPreviewVerts[2] =
    { {0.83f, 0.02f}, {0.98f, 0.17f} };

static RwBool       ShadowEnabled = TRUE;
static RwInt32      ShadowResolutionIndex = 7;
static RwInt32      ShadowBlur = 1;
static RwBool       ShadowAA = TRUE;
static RwReal       ShadowZoneRadius = 130.0f;
static RwReal       ShadowStrength = 0.8f;
static RwBool       ShadowFade = TRUE;

static RpIntersection ShadowZone;

/*
 * Fast shadow pipeline
 */
static RpClump      *ShadowClump = NULL;
static RwBool       ShadowOptimized = FALSE;
static RxPipeline   *ShadowPipeline = NULL;

/*
 *  Misc
 */
static const RwV3d  Xaxis = { 1.0f, 0.0f, 0.0f };
static const RwV3d  Yaxis = { 0.0f, 1.0f, 0.0f };
static const RwV3d  Zaxis = { 0.0f, 0.0f, 1.0f };

/*
 * Shadow MultiTexture
 */
static RwTexture    *ShadowRenderTexture = NULL;

static RwBool ShadowProjectionOptimized = FALSE;

static RxPipeline   *ShadowProjectionPipeline = NULL;

/*
 *****************************************************************************
 */
static RpWorld *
WorldLoad(RwChar * filename)
{
    RpWorld            *world = NULL;
    RwChar             *path;

    /*
     *  Get path.
     */
    path = RsPathnameCreate(filename);
    if (path)
    {
        RwStream           *stream;

        /*
         *  Open stream to file.
         */
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
        RsPathnameDestroy(path);
        if (stream)
        {
            /*
             *  Find world data chunk in the stream.
             */
            if (RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL))
            {
                world = RpWorldStreamRead(stream);
            }

            RwStreamClose(stream, NULL);
        }
    }

    return (world);
}

/*
 *****************************************************************************
 */
static RpClump *
ClumpLoad(RwChar * filename)
{
    RwChar             *path;
    RpClump            *clump = NULL;

    /*
     *  Get path.
     */
    path = RsPathnameCreate(filename);
    if (path)
    {
        RwStream           *stream;

        /* 
         *  Open stream to file.
         */
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
        RsPathnameDestroy(path);
        if (stream)
        {
            /*
             *  Find a clump chunk in the stream.
             */
            if (RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL))
            {
                clump = RpClumpStreamRead(stream);
            }

            RwStreamClose(stream, NULL);
        }
    }

    return clump;
}

/*
 ******************************************************************************
 */
static RwFrame *
ClumpGetHierarchyRecurse(RwFrame * frame, void *ptr)
{
    RpHAnimHierarchy  **hierarchy = (RpHAnimHierarchy **) ptr;

    *hierarchy = RpHAnimFrameGetHierarchy(frame);
    if (!*hierarchy)
    {
        /*
         *  Look in child frames.
         */
        RwFrameForAllChildren(frame, ClumpGetHierarchyRecurse,
                              (void *) hierarchy);

        if (!*hierarchy)
        {
            return frame;      /* Still haven't found it */
        }
    }

    return NULL;
}

static RpHAnimHierarchy *
ClumpGetHierarchy(RpClump * clump)
{
    RwFrame            *frame = RpClumpGetFrame(clump);
    RpHAnimHierarchy   *hierarchy;

    /*
     *  Find the point in a clump frame hierarchy connected to RpHAnimHierarchy
     */

    hierarchy = RpHAnimFrameGetHierarchy(frame);
    if (!hierarchy)
    {
        RwFrameForAllChildren(frame, ClumpGetHierarchyRecurse,
                              (void *) &hierarchy);
    }

    return hierarchy;
}

/*
 ******************************************************************************
 */
static RpAtomic *
GetFirstAtomic(RpAtomic * atomic, void *data)
{
    RpAtomic          **firstAtomic = (RpAtomic **) data;

    *firstAtomic = atomic;
    return NULL;
}

/*
 ******************************************************************************
 */
static RwBool
SetupClump(void)
{
    RpAtomic           *atomic;
    RwChar             *path;

    /*
     *  Load clump, and get the atomic (we assume there's only one)
     */
    Clump = ClumpLoad(ClumpPath);
    if (!Clump)
    {
        return FALSE;
    }

    RpClumpForAllAtomics(Clump, GetFirstAtomic, (void *) &atomic);

    /*
     *  Position the clump and initialize bounding sphere data
     */
    RwFrameTranslate(RpClumpGetFrame(Clump), &ClumpStartPos,
                     rwCOMBINEREPLACE);

    ClumpBoundingSphere = *RpAtomicGetBoundingSphere(atomic);

    ClumpWorldBoundingSphere.radius = ClumpBoundingSphere.radius;

    RwV3dTransformPoint(&ClumpWorldBoundingSphere.center,
                        &ClumpBoundingSphere.center,
                        RwFrameGetMatrix(RpClumpGetFrame(Clump)));

    /*
     *  Setup hierarchy animation.
     */
    path = RsPathnameCreate(AnimPath);
    Anim = RtAnimAnimationRead(path);
    RsPathnameDestroy(path);
    if (!Anim)
    {
        return FALSE;
    }

    Hierarchy = ClumpGetHierarchy(Clump);
    RpHAnimHierarchySetCurrentAnim(Hierarchy, Anim);
    RpHAnimHierarchyUpdateMatrices(Hierarchy);

    /* 
     *  Set hierarchy for the atomic's skin.
     */
    RpSkinAtomicSetHAnimHierarchy(atomic, Hierarchy);

    /* If a fast shadow pipeline exists then load a copy of the clump and
       attach the optimized pipeline to it. */
    if (NULL != ShadowPipeline)
    {
        ShadowClump = ClumpLoad(ClumpPath);
        if (NULL == ShadowClump) return FALSE;

        RpClumpSetFrame(ShadowClump, RpClumpGetFrame(Clump));
        RpClumpForAllAtomics(ShadowClump, GetFirstAtomic, (void *)&atomic);
        RpSkinAtomicSetHAnimHierarchy(atomic, Hierarchy);
        RpAtomicSetPipeline(atomic, ShadowPipeline);
    }

    return TRUE;
}

/*
 ******************************************************************************
 */
static void
DestroyClump(void)
{
    if (Anim)
    {
        RtAnimAnimationDestroy(Anim);
        Anim = NULL;
    }

    if (NULL != ShadowClump)
    {
        RpClumpSetFrame(ShadowClump, NULL);
        RpClumpDestroy(ShadowClump);
        ShadowClump = NULL;
    }

    if (Clump)
    {
        RpClumpDestroy(Clump);
        Clump = NULL;
    }

    return;
}

/*
 ******************************************************************************
 */
static void
UpdateClump(RwReal deltaTime)
{
    if (AnimEnabled)
    {
        RwFrame            *frame = RpClumpGetFrame(Clump);
        RwMatrix           *matrix = RwFrameGetMatrix(frame);
        RwV3d              *pos = RwMatrixGetPos(matrix);
        const RwBBox       *bbox = RpWorldGetBBox(World);
        RwV3d               tr;

        /*
         *  Adjust the time step.
         */
        deltaTime *= AnimSpeedFactor;

        /* 
         *  Update animation on hierarchy.
         */
        if (deltaTime < 0.0f)
        {
            RpHAnimHierarchySubAnimTime(Hierarchy, -deltaTime);
        }
        else
        {
            /* check if we've been playing backwards */
            if (!Hierarchy->currentAnim->pNextFrame)
            {
                /* we have so reset the animation for forward playback */
                RwReal              targetTime;
                RtAnimAnimation     *currentAnim;
                currentAnim = RpHAnimHierarchyGetCurrentAnim(Hierarchy);

                targetTime = Hierarchy->currentAnim->currentTime + deltaTime;
                RpHAnimHierarchySetCurrentAnim(Hierarchy,currentAnim);
                RpHAnimHierarchySetCurrentAnimTime(Hierarchy,
                                                   targetTime);
            }
            else
            {
                RpHAnimHierarchyAddAnimTime(Hierarchy, deltaTime);
            }
        }
        RpHAnimHierarchyUpdateMatrices(Hierarchy);

        /*
         *  Translate the clump along z axis, but wrap around when we
         *  reach the edge of the world.
         */
        tr.x = tr.y = 0.0f;

        if (pos->z > bbox->sup.z)
        {
            tr.z = bbox->inf.z - bbox->sup.z;
        }
        else if (pos->z < bbox->inf.z)
        {
            tr.z = bbox->sup.z - bbox->inf.z;
        }
        else
        {
            tr.z = deltaTime * ANIM_CLUMP_SPEED;
        }

        RwFrameTranslate(frame, &tr, rwCOMBINEPOSTCONCAT);

        /*
         *  Update the clump world bounding sphere
         */
        RwV3dTransformPoint(&ClumpWorldBoundingSphere.center,
                            &ClumpBoundingSphere.center, matrix);
    }

    return;
}

/*
 *****************************************************************************
 */
static RwBool
SetupLights(void)
{
    RwFrame            *frame;
    RwRGBAReal          col;

    /* 
     *  Create ambient light
     */
    AmbientLight = RpLightCreate(rpLIGHTAMBIENT);
    if (!AmbientLight)
    {
        return FALSE;
    }

    col.red = col.green = col.blue = ALIGHT_VALUE;
    col.alpha = 0.0f;
    RpLightSetColor(AmbientLight, &col);

    RpWorldAddLight(World, AmbientLight);

    /*
     *  Create directional light
     */
    DirectionalLight = RpLightCreate(rpLIGHTDIRECTIONAL);
    if (!DirectionalLight)
    {
        return FALSE;
    }

    col.red = col.green = col.blue = DLIGHT_VALUE;
    col.alpha = 0.0f;
    RpLightSetColor(DirectionalLight, &col);

    frame = RwFrameCreate();
    RwFrameRotate(frame, &Xaxis, DLIGHT_ELEVATION, rwCOMBINEREPLACE);
    RwFrameRotate(frame, &Yaxis, -DLIGHT_AZIMUTH, rwCOMBINEPOSTCONCAT);
    RpLightSetFrame(DirectionalLight, frame);

    RpWorldAddLight(World, DirectionalLight);

    return TRUE;
}

/*
 *****************************************************************************
 */
static void
DestroyLights(void)
{
    RwFrame            *frame;

    /*
     *  Destroy ambient light.
     */
    RpWorldRemoveLight(World, AmbientLight);
    RpLightDestroy(AmbientLight);
    AmbientLight = NULL;

    /*
     *  Destroy directional light.
     */
    RpWorldRemoveLight(World, DirectionalLight);
    frame = RpLightGetFrame(DirectionalLight);
    RpLightSetFrame(DirectionalLight, NULL);
    RwFrameDestroy(frame);
    RpLightDestroy(DirectionalLight);
    DirectionalLight = NULL;

    return;
}

/*
 ******************************************************************************
 */
static RwBool
SetupShadow(void)
{
    RwUInt32            res, res2;

    /*
     *  Get resolution for shadow camera raster, and the other image
     *  processing rasters. The latter are half the size if we use
     *  an anti-alias step.
     */
    res = 1 << ShadowResolutionIndex;

    if (ShadowAA)
    {
        res2 = res >> 1;
    }
    else
    {
        res2 = res;
    }

    /*
     *  Create a shadow camera, and set the lighting direction, frustum
     *  size, and frustum center.
     */
    ShadowCamera = ShadowCameraCreate(res);
    if (!ShadowCamera)
    {
        return FALSE;
    }

    ShadowCameraSetLight(ShadowCamera, DirectionalLight);
    ShadowCameraSetFrustum(ShadowCamera,
                           1.1f * ClumpBoundingSphere.radius);
    ShadowCameraSetCenter(ShadowCamera,
                          &ClumpWorldBoundingSphere.center);

    RpWorldAddCamera(World, ShadowCamera);

    /*
     *  Create a second camera for image processing.
     */
    ShadowIPCamera = ShadowCameraCreate(res2);
    if (!ShadowIPCamera)
    {
        return FALSE;
    }

    /*
     *  Create the main raster for rendering shadows into.
     */
    ShadowCameraRaster = ShadowRasterCreate(res);
    if (!ShadowCameraRaster)
    {
        return FALSE;
    }

    RwCameraSetRaster(ShadowCamera, ShadowCameraRaster);

    /*
     *  Create rasters for processed images (anti-alias and blur).
     */
    if (ShadowAA)
    {
        ShadowRasterAA = ShadowRasterCreate(res2);
        if (!ShadowRasterAA)
        {
            return FALSE;
        }
    }

    ShadowRasterTemp = ShadowRasterCreate(res2);
    if (!ShadowRasterTemp)
    {
        return FALSE;
    }

    return TRUE;
}

/*
 ******************************************************************************
 */
static void
DestroyShadow(void)
{
    if (ShadowCamera)
    {
        RpWorldRemoveCamera(World, ShadowCamera);
        ShadowCameraDestroy(ShadowCamera);
        ShadowCamera = NULL;
    }

    if (ShadowIPCamera)
    {
        ShadowCameraDestroy(ShadowIPCamera);
        ShadowIPCamera = NULL;
    }

    if (ShadowCameraRaster)
    {
        ShadowRasterDestroy(ShadowCameraRaster);
        ShadowCameraRaster = NULL;
    }

    if (ShadowRasterAA)
    {
        ShadowRasterDestroy(ShadowRasterAA);
        ShadowRasterAA = NULL;
    }

    if (ShadowRasterTemp)
    {
        ShadowRasterDestroy(ShadowRasterTemp);
        ShadowRasterTemp = NULL;
    }

    return;
}

static RpAtomic *
RemoveAtomicFlags(RpAtomic *atomic, void *data __RWUNUSED__)
{
    RpGeometry *geometry;
    RwUInt32 flags;

    geometry = RpAtomicGetGeometry(atomic);

    flags = RpGeometryGetFlags(geometry);

#if defined(XBOX_DRVMODEL_H)
    flags &= ~(/*rxGEOMETRY_LIGHT | */rxGEOMETRY_MODULATE);
#else
    flags &= ~(rxGEOMETRY_LIGHT | rxGEOMETRY_MODULATE);
#endif

    RpGeometrySetFlags(geometry, flags);

    return atomic;
}

static RpAtomic *
RestoreAtomicFlags(RpAtomic *atomic, void *data __RWUNUSED__)
{
    RpGeometry *geometry;
    RwUInt32 flags;

    geometry = RpAtomicGetGeometry(atomic);

    flags = RpGeometryGetFlags(geometry);

#if defined(XBOX_DRVMODEL_H)
    flags |= (/*rxGEOMETRY_LIGHT | */rxGEOMETRY_MODULATE);
#else
    flags |= (rxGEOMETRY_LIGHT | rxGEOMETRY_MODULATE);
#endif

    RpGeometrySetFlags(geometry, flags);

    return atomic;
}

/*
 ******************************************************************************
 */
static void
UpdateShadow(RwReal deltaTime __RWUNUSED__)
{
    RwUInt32 dFlags;
    RwUInt32 aFlags;

    /*
     *  Track shadow camera along with the clump
     */
    ShadowCameraSetCenter(ShadowCamera,
                          &ClumpWorldBoundingSphere.center);

    dFlags = RpLightGetFlags(DirectionalLight);
    aFlags = RpLightGetFlags(AmbientLight);
    if (!ShadowOptimized)
    {
        /* 
         *  Remove geometry flags to not light the model
         */
        RpClumpForAllAtomics(Clump, RemoveAtomicFlags, NULL);

        /* 
         *  Disable lights 
         */
        RpLightSetFlags(DirectionalLight, 0);
        RpLightSetFlags(AmbientLight, 0);
    }

    /* 
     *  Render the clump into the shadow camera. This is a generic method
     *  that will work for most things. Ideally, for best performance,
     *  a special pipeline would be used to just render black polygons.
     */
#if defined(SKY2_DRVMODEL_H) || defined(SOFTRAS_DRVMODEL_H)
    ShadowCameraUpdate(ShadowCamera, 
        ShadowOptimized ? ShadowClump : Clump, FALSE);
#else
    ShadowCameraUpdate(ShadowCamera, Clump, (ShadowProjectionOptimized == FALSE));
#endif /* defined(SKY2_DRVMODEL_H) || defined(SOFTRAS_DRVMODEL_H) */

    if (!ShadowOptimized)
    {
        /*
         *  Enable lights again
         */
        RpLightSetFlags(DirectionalLight, dFlags);
        RpLightSetFlags(AmbientLight, aFlags);

        /*
         *  Restore flags
         */
        RpClumpForAllAtomics(Clump, RestoreAtomicFlags, NULL);
    }

    /*
     *  Anti-alias and set the raster to be used for shadow rendering. 
     */
    if (ShadowAA)
    {
        ShadowRasterResample(ShadowRasterAA, ShadowCameraRaster,
                             ShadowIPCamera);

        ShadowRenderRaster = ShadowRasterAA;
    }
    else
    {
        ShadowRenderRaster = ShadowCameraRaster;
    }

    /*
     *  Blur the shadow. Both anti-alias and blur may be omitted to
     *  minimize the impact on performance, and the texture cache usage on PS2.
     */
    if (ShadowBlur)
    {
        ShadowRasterBlur(ShadowRenderRaster, ShadowRasterTemp,
                         ShadowIPCamera, ShadowBlur);
    }

    /*
     * If the optimized projection pipeline is using multi-texturing, 
     * then we need to fake the shadow strength and fade by blending a
     * quad on top of the shadow raster.
     */
#if !defined(SKY)
    if ( ShadowProjectionOptimized &&
         ((RwReal)RwFabs(ShadowStrength) < 1.0f || ShadowFade) )
    {
        ShadowRasterFade(ShadowIPCamera, ShadowRenderRaster,
                         ShadowStrength,
                         ShadowZoneRadius,
                         ClumpBoundingSphere.radius,
                         ShadowFade);
    }
#endif /* !defined(SKY) */

    return;
}

/*
 ******************************************************************************
 */
static void
CalculateShadowZone(RpIntersection *shadowZone)
{
    RwV3d               boxDiag;
    RwV3d               zoneVector;

    /*
     *  The ShadowRender() function requires an intersection
     *  primitive for use in a collision test on the world to find shadow
     *  triangles.
     *
     *  We start with a bounding box around the clump...
     */
    shadowZone->type = rpINTERSECTBOX;

    boxDiag.x = boxDiag.y = boxDiag.z = ClumpBoundingSphere.radius;

    if (ShadowProjectionOptimized)
    {
        shadowZone->t.box.inf = ClumpWorldBoundingSphere.center;
        shadowZone->t.box.sup = ClumpWorldBoundingSphere.center;
    }
    else
    {
        RwV3dAdd(&shadowZone->t.box.sup,
                &ClumpWorldBoundingSphere.center, &boxDiag);
        RwV3dSub(&shadowZone->t.box.inf,
                &ClumpWorldBoundingSphere.center, &boxDiag);
    }

    /*
     *  ... and now extend it to cover the space out to the 
     *  ShadowZoneRadius along the camera/light direction.
     */
    RwV3dScale(&zoneVector,
               RwMatrixGetAt(RwFrameGetMatrix
                             (RwCameraGetFrame(ShadowCamera))),
               ShadowZoneRadius);

    if (zoneVector.x > 0.0f)
    {
        shadowZone->t.box.sup.x += zoneVector.x;
    }
    else
    {
        shadowZone->t.box.inf.x += zoneVector.x;
    }

    if (zoneVector.y > 0.0f)
    {
        shadowZone->t.box.sup.y += zoneVector.y;
    }
    else
    {
        shadowZone->t.box.inf.y += zoneVector.y;
    }

    if (zoneVector.z > 0.0f)
    {
        shadowZone->t.box.sup.z += zoneVector.z;
    }
    else
    {
        shadowZone->t.box.inf.z += zoneVector.z;
    }
}

/*
 ******************************************************************************
 */
static RwBool
ShadowRenderCB(RwBool testEnable __RWUNUSED__)
{
    return NULL != ShadowPipeline;
}

/*
 ******************************************************************************
 */
static RwBool
ShadowProjection(RwBool justCheck)
{
    if (justCheck)
    {
        /*
         *  Make sure there is an optimized pipeline.
         */
        if (ShadowProjectionPipeline != NULL)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*
 ******************************************************************************
 */
static RwBool
ResetShadow(RwBool justCheck)
{
    if (!justCheck)
    {
        /*
         *  Destroy and recreate everything for new texture resolution
         */
        DestroyShadow();
        SetupShadow();
    }

    return TRUE;
}

/*
 ******************************************************************************
 */
RwBool
SceneMenuInit(void)
{
    static RwChar       animLabel[] = RWSTRING("Animation");
    static RwChar       animSpeedLabel[] = RWSTRING("Animation speed");
    static RwChar       shadowLabel[] = RWSTRING("Shadow_S");
    static RwChar       shadowRenderLabel[] = RWSTRING("Shadow rendering_R");
    static RwChar       shadowProjectionLabel[] = RWSTRING("Shadow projection_P");
    static const RwChar *shadowProjections[] =
    {
        RWSTRING("Generic"),
        RWSTRING("Optimized"),
    };
    static RwChar       resolutionLabel[] = RWSTRING("Shadow resolution");
    static RwChar       aaLabel[] = RWSTRING("Shadow anti-alias_A");
    static RwChar       blurLabel[] = RWSTRING("Shadow blur");
    static RwChar       radiusLabel[] = RWSTRING("Shadow zone radius");
    static RwChar       strengthLabel[] = RWSTRING("Shadow strength");
    static RwChar       fadeLabel[] = RWSTRING("Shadow fade");

    MenuAddEntryBool(animLabel, &AnimEnabled, NULL);

    MenuAddEntryReal(animSpeedLabel, &AnimSpeedFactor, NULL, -2.0f,
                     5.0f, 0.1f);

    MenuAddSeparator();

    MenuAddEntryBool(shadowLabel, &ShadowEnabled, NULL);

    MenuAddEntryInt(shadowRenderLabel, &ShadowOptimized, ShadowRenderCB,
                    0, 1, 1, shadowProjections);

    MenuAddEntryInt(shadowProjectionLabel, &ShadowProjectionOptimized,
                    ShadowProjection, 0, 1, 1, shadowProjections);

    MenuAddEntryInt(resolutionLabel, &ShadowResolutionIndex,
                    ResetShadow, 5, 8, 1, NULL);

    MenuAddEntryBool(aaLabel, &ShadowAA, ResetShadow );

    MenuAddEntryInt(blurLabel, &ShadowBlur, NULL, 0, 10, 1, NULL);

    MenuAddEntryReal(radiusLabel, &ShadowZoneRadius, NULL, 0.0f, 500.0f,
                     10.0f);

    MenuAddEntryReal(strengthLabel, &ShadowStrength, NULL, -1.0f, 1.0f,
                     0.05f);

    MenuAddEntryBool(fadeLabel, &ShadowFade, NULL);

    return TRUE;
}

/****************************************************************************
 *  ShadowProjectionCreate
 */
static RwBool
ShadowProjectionCreate(void)
{
    /* Shadow projection pipeline can fail to say we have no optimized pipeline */
    ShadowProjectionPipeline = ProjectionPipelineCreate();
    if (NULL == ShadowProjectionPipeline)
    {
        return TRUE;
    }

    ShadowRenderTexture = RwTextureCreate(NULL);
    if (ShadowRenderTexture != NULL)
    {
        RwTextureSetFilterMode(ShadowRenderTexture, rwFILTERLINEAR);
        RwTextureSetAddressing(ShadowRenderTexture, rwTEXTUREADDRESSCLAMP);

        return TRUE;
    }

    return FALSE;
}

/*
 ******************************************************************************
 */
RpWorld            *
SceneOpen(void)
{
    RwChar *path;

    ShadowPipeline = ShadowPipelineCreate();

    /*
     *  Setup the world, clump, lights and shadow.
     */
    path = RsPathnameCreate(WorldTexturePath);
    RwImageSetPath(path);
    RsPathnameDestroy(path);
    World = WorldLoad(WorldPath);

    if (World &&
        SetupClump() &&
        SetupLights() &&
        SetupShadow() &&
        ShadowProjectionCreate())
    {
        return World;
    }

    return NULL;
}

/*
 ******************************************************************************
 */
void
SceneClose(void)
{
    /*
     *  Destroy shadow, lights, clump, and world.
     */
    if (ShadowRenderTexture != NULL)
    {
        RwTextureSetRaster(ShadowRenderTexture, NULL);
        RwTextureDestroy(ShadowRenderTexture);
    }

#if !defined(SKY2_DRVMODEL_H)
    if (NULL != ShadowProjectionPipeline)
    {
        RxPipelineDestroy(ShadowProjectionPipeline);
    }
#endif
    ShadowProjectionPipeline = NULL;

#if !defined(SKY2_DRVMODEL_H)
    if (NULL != ShadowPipeline)
    {
        RxPipelineDestroy(ShadowPipeline);
    }
#endif
    ShadowPipeline = NULL;

    DestroyShadow();
    DestroyLights();
    DestroyClump();

    if (World)
    {
        RpWorldDestroy(World);
        World = NULL;
    }

    return;
}

/*
 ******************************************************************************
 */
RwBool
SceneUpdate(RwReal deltaTime)
{
    /*
     *  Update clump animation.
     */
    UpdateClump(deltaTime);

    /*
     *  Update shadow.
     */
    if (ShadowEnabled)
    {
        UpdateShadow(deltaTime);
    }

    return TRUE;
}

/****************************************************************************
 *  ShadowMultitextureFindSector
 */
static RpWorldSector *
ShadowMultitextureFindSector(RpWorldSector *sector,
                             void *pData)
{
    RpWorldSector **sectorFind = ((RpWorldSector **)pData);

    if (sector == *sectorFind)
    {
        *sectorFind = NULL;

        return NULL;
    }

    return sector;
}

/****************************************************************************
 *  ShadowProjectionPipeSectorRender
 */
static RpWorldSector *
ShadowProjectionPipeSectorRender(RpWorldSector *sector,
                                   void *pData __RWUNUSED__)
{
    RpWorldSector *sectorFind;
    const RwBBox *sectorBox;

    sectorFind = sector;

    sectorBox = RpWorldSectorGetTightBBox(sector);

    /* check if this sector is inside the shadow camera frustrum */
    RwCameraForAllSectorsInFrustum(ShadowCamera, ShadowMultitextureFindSector, &sectorFind);

    /* Check if this sector intersects the shadow zone */
    if ( sectorFind == NULL &&
         (sectorBox->inf.x < ShadowZone.t.box.sup.x && sectorBox->sup.x > ShadowZone.t.box.inf.x) &&
         (sectorBox->inf.y < ShadowZone.t.box.sup.y && sectorBox->sup.y > ShadowZone.t.box.inf.y) &&
         (sectorBox->inf.z < ShadowZone.t.box.sup.z && sectorBox->sup.z > ShadowZone.t.box.inf.z) )
    {
        RxPipeline *oldpipe;

        RpWorldSectorGetPipeline(sector, &oldpipe);

#if defined(SKY2_DRVMODEL_H)
        /* Render with standard pipeline */
        RpWorldSectorRender(sector);

        /* Set renderstates for shadow */
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 
            RwTextureGetRaster(ShadowRenderTexture));
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, 
            (void *)RwTextureGetFilterMode(ShadowRenderTexture));
        RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS,
            (void *)RwTextureGetAddressing(ShadowRenderTexture));
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDZERO);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, ShadowStrength < 0.0f
            ? (void *)rwBLENDSRCALPHA : (void *)rwBLENDINVSRCALPHA);
#endif

        /* Set projected shadow pipeline */
        RpWorldSectorSetPipeline(sector, ShadowProjectionPipeline);

        /* Render with shadow pipeline */
        RpWorldSectorRender(sector);

#if defined(SKY2_DRVMODEL_H)
        /* Restore renderstates */
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
#endif

        RpWorldSectorSetPipeline(sector, oldpipe);
    }
    else
    {
        /* Use standard pipeline */
        RpWorldSectorRender(sector);
    }

    return (sector);
}

/*
 ******************************************************************************
 */
RwBool
SceneRender(void)
{
    CalculateShadowZone(&ShadowZone);

    /*
     *  Render the world sectors
     */
    if (ShadowEnabled &&
        ShadowStrength != 0.0f &&
        ShadowProjectionOptimized)
    {
        /* Attach the right raster to the shadow texture */
        RwTextureSetRaster(ShadowRenderTexture, ShadowRenderRaster);

        /* Prepare platform specific data */
        ProjectionPipelineBegin(ShadowRenderTexture,
                                Camera,
                                ShadowCamera,
                                ShadowStrength,
                                ShadowZoneRadius);

        /* Render sectors */
        RwCameraForAllSectorsInFrustum(Camera, ShadowProjectionPipeSectorRender, World);

        ProjectionPipelineEnd();
    }
    else
    {
        RpWorldRender(World);
    }

    /*
    *  Render the clump
    */
    RpClumpRender(Clump);

    /*
    *  Render shadow
    */
    if (ShadowEnabled)
    {
        /*
        *  Render shadow raster preview.
        */
        ShadowRasterRender(ShadowRenderRaster, ShadowPreviewVerts);

        /*
        *  Render shadow.
        */
        if (ShadowProjectionOptimized)
        {
            ShadowNumTriangles = 0;
        }
        else if (ShadowStrength != 0.0f)
        {
            ShadowRender(ShadowCamera, ShadowRenderRaster,
                        World, &ShadowZone, ShadowStrength,
                        ShadowZoneRadius,
                        ShadowFade);
        }
    }

    return TRUE;
}
