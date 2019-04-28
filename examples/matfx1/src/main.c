
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
 * main.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the material effects plugin.
 *
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#if (defined(SKY))
#include "rppds.h"
#endif /* (defined(SKY)) */

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rpmatfx.h"

#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "main.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

#define ANGSPEED (90.0f)

#define NUM_ENV_MAPS (3)
#define NUM_BUMP_MAPS (2)
#define NUM_ENV_BUMP_MAPS (2)

typedef enum
{
    ENV_MAPPING = 0,
    BUMP_MAPPING,
    ENV_BUMP_MAPPING,
    DUAL_PASS_MAPPING,
    UV_TRANSFORM,
    DUAL_UV_TRANSFORM,
    NUM_MAT_EFFECTS,

    MATERIALEFFECTFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
}
MaterialEffect;

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static RwBool MaterialEffectsOn = TRUE;
static RwInt32 MaterialEffectType = ENV_MAPPING;

/* 
 * Environment-mapping resources...
 */
static RpClump *EnvClump = NULL;
static RwTexture *EnvMap[NUM_ENV_MAPS];
static RwInt32 CurrentEnvMap = 0;
static RwInt32 EnvMapActiveMaterials = 2;
static const RwChar *EnvMapName[NUM_ENV_MAPS] =
{
    RWSTRING("gloss"),
    RWSTRING("office"),
    RWSTRING("specular")
};
static RwReal EnvMapShininess = 0.5f;

/*
 * Bump-mapping resources...
 */
static RpClump *BumpClump = NULL;
static RwTexture *BumpMap[NUM_BUMP_MAPS];
static RwInt32 CurrentBumpMap = 0;
static RwInt32 BumpMapActiveMaterials = 2;
static const RwChar *BumpMapName[NUM_BUMP_MAPS] =
{
    RWSTRING("wood"), 
    RWSTRING("text")
};
static RwReal BumpMapBumpiness = 2.0f;

/*
 * Env-bump-mapping resources...
 */
static RpClump *EnvBumpClump = NULL;
static RwTexture *EnvBumpMap[NUM_ENV_BUMP_MAPS];
static const RwChar *EnvBumpMapName[NUM_ENV_BUMP_MAPS] =
{
    RWSTRING("highlite"),
    RWSTRING("ripples")
};

/*
 * Dual-pass mapping resources...
 */
static RpClump *DualPassClump = NULL;
static RwTexture *DualPassMap = NULL;
static const RwChar *DualPassMapName = RWSTRING("rw");

static RwBlendFunction DualPassSrcBlendMode = rwBLENDSRCALPHA;
static RwBlendFunction DualPassDestBlendMode = rwBLENDINVSRCALPHA;

static const RwChar *DualPassBlendModeNames[] =
{
    RWSTRING("rwBLENDZERO"),
    RWSTRING("rwBLENDONE"),
    RWSTRING("rwBLENDSRCCOLOR"),
    RWSTRING("rwBLENDINVSRCCOLOR"),
    RWSTRING("rwBLENDSRCALPHA"),
    RWSTRING("rwBLENDINVSRCALPHA"),
    RWSTRING("rwBLENDDESTALPHA"),
    RWSTRING("rwBLENDINVDESTALPHA"),
    RWSTRING("rwBLENDDESTCOLOR"),
    RWSTRING("rwBLENDINVDESTCOLOR"),
    RWSTRING("rwBLENDSRCALPHASAT")
};

/*
 * UV tranforms animation
 */
static RwMatrix *UVAnimBaseTransform = NULL;
static RwMatrix *UVAnimDualTransform = NULL;
#define UVANIMPERIOD (3.0f)


static RpWorld *World = NULL;
static RwCamera *Camera = NULL;
static RtCharset *Charset = NULL;
static RpLight *MainLight = NULL;
static RpClump *CurrentClump = NULL;

static RwBool LightSpinOn = FALSE;



/*
 *****************************************************************************
 */
static RpWorld *
CreateWorld(void)
{
    RpWorld *world;
    RwBBox bb;

    bb.inf.x = bb.inf.y = bb.inf.z = -100.0f;
    bb.sup.x = bb.sup.y = bb.sup.z = 100.0f;

    world = RpWorldCreate(&bb);

    return world;
}


/*
 *****************************************************************************
 */
static RwCamera *
CreateCamera(RpWorld *world)
{
    RwCamera *camera;

    camera = CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);

    if( camera )
    {
        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 40.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static void
LightResetFrame(RpLight *light)
{
    RwFrame *frame;
    RwV3d xAxis = {1.0f, 0.0f, 0.0f};
    RwV3d yAxis = {0.0f, 1.0f, 0.0f};

    frame = RpLightGetFrame(light);

    RwFrameRotate(frame, &xAxis, 45.0f, rwCOMBINEREPLACE);
    RwFrameRotate(frame, &yAxis, 45.0f, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
static RwBool
CreateLights(RpWorld *world)
{
    RpLight *light = NULL;

    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RpWorldAddLight(world, light);
    }
    else
    {
        return FALSE;
    }

    light = NULL;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    if( light )
    {
        RwFrame *frame = NULL;
        
        frame = RwFrameCreate();
        if( frame )
        {
            RpLightSetFrame(light, frame);

            LightResetFrame(light);

            RpWorldAddLight(world, light);

            MainLight = light;

            return TRUE;
        }

        RpLightDestroy(light);
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static void
LightRotate(RwReal deltaTime)
{
    RwMatrix *cameraMatrix;
    RwV3d *at;
    RwFrame *frame;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    at = RwMatrixGetAt(cameraMatrix);

    frame = RpLightGetFrame(MainLight);

    RwFrameRotate(frame, at, deltaTime  * ANGSPEED, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
void
ClumpRotate(RwReal angleX, RwReal angleY)
{
    RwMatrix *cameraMatrix;
    RwV3d right, up, pos;
    RwFrame *frame;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    frame = RpClumpGetFrame(CurrentClump);
    pos = *RwMatrixGetPos(RwFrameGetMatrix(frame));

    /*
     * First translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * ...do the rotations...
     */
    RwFrameRotate(frame, &up, angleX, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(frame, &right, angleY, rwCOMBINEPOSTCONCAT);

    /*
     * ...and translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
void 
ClumpTranslateZ(RwReal deltaZ)
{
    RwMatrix *cameraMatrix;
    RwV3d at;
    RwFrame *frame;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    at = *RwMatrixGetAt(cameraMatrix);

    frame = RpClumpGetFrame(CurrentClump);

    RwV3dScale(&at, &at, deltaZ);

    RwFrameTranslate(frame, &at, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
static RpClump *
LoadClump(const RwChar *filename)
{
    RwStream *stream = NULL;
    RpClump *clump = NULL;
    RwChar *path;

    path = RsPathnameCreate(filename);    
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            clump = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    return clump;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialSetShininess(RpMaterial *material,
                     void *data __RWUNUSED__)
{
    RpMatFXMaterialFlags flags;

    flags = RpMatFXMaterialGetEffects(material);

    if( flags == rpMATFXEFFECTENVMAP || flags == rpMATFXEFFECTBUMPENVMAP )
    {
        RpMatFXMaterialSetEnvMapCoefficient(material, EnvMapShininess);
    }

    return material;
}


static RpAtomic *
AtomicSetShininess(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RpGeometryForAllMaterials(geometry, MaterialSetShininess, data);
    }

    return atomic;
}


static void 
ClumpSetEnvMapShininess(RpClump *clump)
{
    RpClumpForAllAtomics(clump, AtomicSetShininess, NULL);

    return;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialSetEnvMap(RpMaterial *material, void *data)
{
    RwTexture *matTexture;

    matTexture = RpMaterialGetTexture(material);
    if( matTexture )
    {
        RwChar *textureName;
        RwBool useEnvMap = FALSE;

        if( EnvMapActiveMaterials == 2 )
        {
            useEnvMap = TRUE;
        }
        else
        {
            textureName = RwTextureGetName(matTexture);

            if( (EnvMapActiveMaterials == 0) && 
                !rwstrcmp(textureName, RWSTRING("whiteash")) )
            {
                useEnvMap = TRUE;
            }
            else if( (EnvMapActiveMaterials == 1) && 
                     !rwstrcmp(textureName, RWSTRING("world2")) )
            {
                useEnvMap = TRUE;
            }
        }

        if( useEnvMap )
        {
            RwFrame *frame;
            RwTexture *texture = (RwTexture *)data;

            textureName = RwTextureGetName(texture);
            if( rwstrcmp(textureName, RWSTRING("specular")) )
            {
                frame = RwCameraGetFrame(Camera);
            }
            else
            {
                frame = RpLightGetFrame(MainLight);
            }

            RpMatFXMaterialSetEffects(material, rpMATFXEFFECTENVMAP);

            RpMatFXMaterialSetupEnvMap(material, 
                                       texture, frame, FALSE, EnvMapShininess);
        }
        else
        {
            RpMatFXMaterialSetEffects(material, rpMATFXEFFECTNULL);
        }
    }

    return material;
}


static RpAtomic *
AtomicSetEnvMap(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;

    RpMatFXAtomicEnableEffects(atomic);

    geometry = RpAtomicGetGeometry(atomic);
    if( geometry )
    {
        RpGeometryForAllMaterials(geometry, MaterialSetEnvMap, data); 
    }

    return atomic;
}


static void
ClumpSetEnvMap(RpClump *clump, RwTexture *texture)
{
    RpClumpForAllAtomics(clump, AtomicSetEnvMap, (void *)texture);
    
    return;
}


/*
 *****************************************************************************
 */
static RpClump *
CreateEnvClump(void)
{
    RpClump *clump = NULL;
    RwChar *path;
    RwInt32 i;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    /*
     * Create the environment-maps...
     */
    for (i=0; i<NUM_ENV_MAPS; i++)
    {
        RwTexture * checkTexture;
        
        EnvMap[i] = RwTextureRead(EnvMapName[i], NULL);

        if( !EnvMap[i] )
        {
            return NULL;
        }

        checkTexture =
            RwTextureSetFilterMode(EnvMap[i], rwFILTERLINEAR);
    }

    clump = LoadClump(RWSTRING("models/torus.dff"));

    if( clump )
    {
        RwV3d xAxis = {1.0f, 0.0f, 0.0f};
        RwV3d yAxis = {0.0f, 1.0f, 0.0f};
        RwV3d pos   = {0.0f, 0.0f, 5.0f};
        RwFrame *clumpFrame = NULL;

        clumpFrame = RpClumpGetFrame(clump);

        RwFrameRotate(clumpFrame, &xAxis, 45.0f, rwCOMBINEREPLACE);
        RwFrameRotate(clumpFrame, &yAxis, 45.0f, rwCOMBINEPOSTCONCAT);

        RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

        /*
         * Setup the environment-map...
         */
        ClumpSetEnvMap(clump, EnvMap[CurrentEnvMap]);
    }

    return clump;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialSetBumpiness(RpMaterial *material,
                     void *data __RWUNUSED__)
{
    RpMatFXMaterialFlags flags;

    flags = RpMatFXMaterialGetEffects(material);

    if( flags == rpMATFXEFFECTBUMPMAP || flags == rpMATFXEFFECTBUMPENVMAP )
    {
        RpMatFXMaterialSetBumpMapCoefficient(material, BumpMapBumpiness);
    }

    return material;
}


static RpAtomic *
AtomicSetBumpiness(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RpGeometryForAllMaterials(geometry, MaterialSetBumpiness, data);
    }

    return atomic;
}


static void 
ClumpSetBumpMapBumpiness(RpClump *clump)
{
    RpClumpForAllAtomics(clump, AtomicSetBumpiness, NULL);

    return;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialSetBumpMap(RpMaterial *material, void *data)
{
    RwTexture *matTexture;

    matTexture = RpMaterialGetTexture(material);
    if( matTexture )
    {
        RwChar *textureName;
        RwBool useBumpMap = FALSE;

        if( BumpMapActiveMaterials == 2 )
        {
            useBumpMap = TRUE;
        }
        else
        {
            textureName = RwTextureGetName(matTexture);

            if( (BumpMapActiveMaterials == 0) && 
                !rwstrcmp(textureName, RWSTRING("whiteash")) )
            {
                useBumpMap = TRUE;
            }
            else if( (BumpMapActiveMaterials == 1) && 
                     !rwstrcmp(textureName, RWSTRING("dai")) )
            {
                useBumpMap = TRUE;
            }
        }

        if( useBumpMap )
        {
            RwFrame *frame;
            RwTexture *texture = (RwTexture *)data;

            frame = RpLightGetFrame(MainLight);

            RpMatFXMaterialSetEffects(material, rpMATFXEFFECTBUMPMAP);

            RpMatFXMaterialSetupBumpMap(material, texture, frame, BumpMapBumpiness);
        }
        else
        {
            RpMatFXMaterialSetEffects(material, rpMATFXEFFECTNULL);
        }
    }

    return material;
}


static RpAtomic *
AtomicSetBumpMap(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;

    RpMatFXAtomicEnableEffects(atomic);

    geometry = RpAtomicGetGeometry(atomic);
    if( geometry )
    {
        RpGeometryForAllMaterials(geometry, MaterialSetBumpMap, data);
    }

    return atomic;
}


static void
ClumpSetBumpMap(RpClump *clump, RwTexture *texture)
{
    RpClumpForAllAtomics(clump, AtomicSetBumpMap, (void *)texture);

    return;
}


/*
 *****************************************************************************
 */
static RpClump *
CreateBumpClump(void)
{
    RpClump *clump = NULL;
    RwChar *path;
    RwInt32 i;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    /*
     * Create the bump-maps...
     */
    for(i=0; i<NUM_BUMP_MAPS; i++)
    {
        RwTexture * checkTexture;

        BumpMap[i] = RwTextureRead(BumpMapName[i], NULL);

        if( !BumpMap[i] )
        {
            return NULL;
        }

        checkTexture = 
            RwTextureSetFilterMode(BumpMap[i], rwFILTERLINEAR);
    }

    clump = LoadClump(RWSTRING("models/bucky.dff"));

    if( clump )
    {
        RwV3d pos = {0.0f, 0.0f, 7.0f};
        RwFrame *clumpFrame = NULL;

        clumpFrame = RpClumpGetFrame(clump);

        RwFrameTranslate(clumpFrame, &pos, rwCOMBINEREPLACE);

        /*
         * Setup the bump-map...
         */
        ClumpSetBumpMap(clump, BumpMap[CurrentBumpMap]);
    }

    return clump;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialSetEnvBumpMap(RpMaterial *material, void *data)
{
    RwFrame *frame;
    RwRGBA white = {255, 255, 255, 255};

    RpMaterialSetColor(material, &white);
    RpMaterialSetTexture(material, (RwTexture *)data);

    RpMatFXMaterialSetEffects(material, rpMATFXEFFECTBUMPENVMAP);

    frame = RpLightGetFrame(MainLight);

    RpMatFXMaterialSetupEnvMap(material, 
                               EnvBumpMap[0], frame, TRUE, EnvMapShininess);

    RpMatFXMaterialSetupBumpMap(material, EnvBumpMap[1], frame, BumpMapBumpiness);

    return material;
}


static RpAtomic *
AtomicSetEnvBumpMap(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;

    RpMatFXAtomicEnableEffects(atomic);

    geometry = RpAtomicGetGeometry(atomic);
    if( geometry )
    {
        RpGeometryForAllMaterials(geometry, MaterialSetEnvBumpMap, data);
    }

    return atomic;
}


static void
ClumpSetEnvBumpMap(RpClump *clump)
{
    RwTexture *texture;

    texture = RwTextureRead(RWSTRING("monet"), NULL);

    RwTextureSetFilterMode(texture, rwFILTERLINEAR);

    RpClumpForAllAtomics(clump, AtomicSetEnvBumpMap, (void *)texture);

    return;
}


/*
 *****************************************************************************
 */
static RpClump *
CreateEnvBumpClump(void)
{
    RpClump *clump = NULL;
    RwChar *path;
    RwInt32 i;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    /*
     * Create the env-bump-maps...
     */
    for(i=0; i<NUM_ENV_BUMP_MAPS; i++)
    {
        RwTexture * checkTexture;

        EnvBumpMap[i] = RwTextureRead(EnvBumpMapName[i], NULL);

        if( !EnvBumpMap[i] )
        {
            return NULL;
        }

        checkTexture = 
            RwTextureSetFilterMode(EnvBumpMap[i], rwFILTERLINEAR);
    }

    clump = LoadClump(RWSTRING("models/goblet.dff"));

    if( clump )
    {
        RwV3d xAxis = {1.0f, 0.0f, 0.0f};
        RwV3d yAxis = {0.0f, 1.0f, 0.0f};
        RwV3d pos   = {0.0f, 0.0f, 3.0f};
        RwFrame *clumpFrame = NULL;

        clumpFrame = RpClumpGetFrame(clump);

        RwFrameRotate(clumpFrame, &xAxis, -150.0f, rwCOMBINEREPLACE);
        RwFrameRotate(clumpFrame, &yAxis, -45.0f, rwCOMBINEPOSTCONCAT);

        RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

        /*
         * Setup the env-bump-maps...
         */
        ClumpSetEnvBumpMap(clump);
    }

    return clump;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialSetEffect(RpMaterial *material, void *data)
{

    switch ((MaterialEffect)data)
    {
    case DUAL_PASS_MAPPING:
        {
            RpMatFXMaterialSetEffects(material, rpMATFXEFFECTDUAL);

            RpMatFXMaterialSetupDualTexture(material, DualPassMap, 
                DualPassSrcBlendMode, DualPassDestBlendMode);

            break;
        }
    case UV_TRANSFORM:
        {
            RpMatFXMaterialSetEffects(material, rpMATFXEFFECTUVTRANSFORM);
            RpMatFXMaterialSetUVTransformMatrices(material, 
                UVAnimBaseTransform, NULL);
            break;
        }

    case DUAL_UV_TRANSFORM:
    default:
        {
            RpMatFXMaterialSetEffects(material, rpMATFXEFFECTDUALUVTRANSFORM);

            RpMatFXMaterialSetupDualTexture(material, 
                DualPassMap, DualPassSrcBlendMode, DualPassDestBlendMode);

            RpMatFXMaterialSetUVTransformMatrices(material, 
                UVAnimBaseTransform, UVAnimDualTransform);
            break;
        }
    }

    return material;
}


static RpAtomic *
AtomicSetEffect(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;

    RpMatFXAtomicEnableEffects(atomic);

    geometry = RpAtomicGetGeometry(atomic);
    if( geometry )
    {
        RpGeometryForAllMaterials(geometry, MaterialSetEffect, data);
    }

    return atomic;
}


static void
ClumpSetEffect(RpClump *clump, MaterialEffect effect)
{
    RpClumpForAllAtomics(clump, AtomicSetEffect, (void *)effect);

    return;
}


/*
 *****************************************************************************
 */
static RpClump *
CreateDualPassClump(void)
{
    RpClump *clump = NULL;
    RwChar *path;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    /*
     * Create the dual-pass map...
     */
    DualPassMap = RwTextureRead(DualPassMapName, NULL);

    if( !DualPassMap )
    {
        return NULL;
    }

    RwTextureSetFilterMode(DualPassMap, rwFILTERLINEAR);
    RwTextureSetAddressing(DualPassMap, rwTEXTUREADDRESSWRAP);

    clump = LoadClump(RWSTRING("models/sphere.dff"));

    if( clump )
    {
        RwV3d xAxis = {1.0f, 0.0f, 0.0f};
        RwV3d yAxis = {0.0f, 1.0f, 0.0f};
        RwV3d pos = {0.0f, 0.0f, 2.5f};
        RwFrame *clumpFrame = NULL;

        clumpFrame = RpClumpGetFrame(clump);

        RwFrameRotate(clumpFrame, &xAxis, 90.0f, rwCOMBINEREPLACE);
        RwFrameRotate(clumpFrame, &yAxis, -90.0f, rwCOMBINEPOSTCONCAT);

        RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

        /*
         * Setup the dual-pass-map...
         */
        ClumpSetEffect(clump, DUAL_PASS_MAPPING);
    }

    return clump;
}


/*
 *****************************************************************************
 */
static RwBool
CreateUVAnimTransforms()
{
    UVAnimBaseTransform = RwMatrixCreate();
    if( !UVAnimBaseTransform )
    {
        return FALSE;
    }

    UVAnimDualTransform = RwMatrixCreate();
    if( !UVAnimDualTransform )
    {
        return FALSE;
    }

    return TRUE;     
}

static void
DestroyUVAnimTransforms()
{
    if (UVAnimBaseTransform)
    {
        RwMatrixDestroy(UVAnimBaseTransform);
    }

    if (UVAnimDualTransform)
    {
        RwMatrixDestroy(UVAnimDualTransform);
    }
}

static void
UpdateUVAnimTransforms(RwReal deltaTime)
{
    RwV3d          *right, *up, *pos, scl;
    RwReal          osc;
    static RwReal   cyclePos = 0.0f;
#if (defined(__MWERKS__) && defined(WIN32))
    volatile RwInt32			temp;
#endif /* (defined(__MWERKS__) && defined(WIN32)) */

    osc = (RwReal) RwSin(2*rwPI*cyclePos);

    /*
     * Here we set up some matrices to transform texture coordinates.
     * These are 2d transformations but are represented by a full
     * 3x4 RwMatrix, where only certain elements are used.
     */

    /*
     *  Shear the base texture. 
     */
    right = RwMatrixGetRight(UVAnimBaseTransform);
    up  = RwMatrixGetUp(UVAnimBaseTransform);
    pos = RwMatrixGetPos(UVAnimBaseTransform);

    right->x =  1.0f;
    up->x    =  0.5f * osc;     /* Shear factor */
    pos->x   = -0.5f * up->x;   /* Shear relative to middle of texture */

    right->y =  0.0f;
    up->y    =  1.0f;
    pos->y   =  0.0f;

    RwMatrixUpdate(UVAnimBaseTransform);

    /*
     *  Scale the dual pass texture
     */
    scl.x = scl.y = scl.z = 1.0f + 0.3f*osc;
    RwMatrixScale(UVAnimDualTransform, &scl, rwCOMBINEREPLACE);

    /*
     *  Update position in cycle.
     */
    cyclePos += deltaTime/UVANIMPERIOD;
    
    /* CodeWarrior PC 8.3 compiler bug temporary fixup!! */
#if (defined(__MWERKS__) && defined(WIN32))
    temp = (RwInt32)cyclePos;
    cyclePos -= temp;
#else /* (defined(__MWERKS__) && defined(WIN32)) */
    cyclePos -= (RwInt32) cyclePos;
#endif /* (defined(__MWERKS__) && defined(WIN32)) */
}

/*
 *****************************************************************************
 */
static RpMaterial *
MaterialResetEffects(RpMaterial *material,
                     void *data __RWUNUSED__)
{
    RpMatFXMaterialSetEffects(material, rpMATFXEFFECTNULL);

    return material;
}


static RpAtomic *
AtomicResetMaterialEffects(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RpGeometryForAllMaterials(geometry, MaterialResetEffects, data);
    }

    return atomic;
}


static void
ResetMaterialEffects(RpClump *clump)
{
    RpClumpForAllAtomics(clump, AtomicResetMaterialEffects, NULL);

    return;
}


/*
 *****************************************************************************
 */
static RwBool
ToggleMaterialEffectsCallback(RwBool testEnable)
{
    if( testEnable ) 
    {
        return TRUE;
    }

    if( MaterialEffectsOn )
    {
        switch( MaterialEffectType )
        {
            case ENV_MAPPING:
                {
                    ClumpSetEnvMap(EnvClump, EnvMap[CurrentEnvMap]);

                    break;
                }

            case BUMP_MAPPING:
                {
                    ClumpSetBumpMap(BumpClump, BumpMap[CurrentBumpMap]);

                    break;
                }

            case ENV_BUMP_MAPPING:
                {
                    ClumpSetEnvBumpMap(EnvBumpClump);

                    break;
                }

            case DUAL_PASS_MAPPING:
            case UV_TRANSFORM:
            case DUAL_UV_TRANSFORM:
                {
                    ClumpSetEffect(DualPassClump, MaterialEffectType);

                    break;
                }
        }
    }
    else
    {
        ResetMaterialEffects(CurrentClump);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
SelectMaterialEffectCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return MaterialEffectsOn;
    }

    if( CurrentClump && RpClumpGetWorld(CurrentClump) )
    {
        RpWorldRemoveClump(World, CurrentClump);
    }

    switch( MaterialEffectType )
    {
        case ENV_MAPPING:
            {
                /*
                 * Display environment-mapped clump...
                 */
                RpWorldAddClump(World, EnvClump);

                CurrentClump = EnvClump;

                break;
            }

        case BUMP_MAPPING:
            {
                /* 
                 * Display bump-mapped clump...
                 */
                RpWorldAddClump(World, BumpClump);

                CurrentClump = BumpClump;

                break;
            }

        case ENV_BUMP_MAPPING:
            {
                /* 
                 * Display environment and bump-mapped clump...
                 */
                RpWorldAddClump(World, EnvBumpClump);

                CurrentClump = EnvBumpClump;

                break;
            }

        case DUAL_PASS_MAPPING:
        case UV_TRANSFORM:
        case DUAL_UV_TRANSFORM:
            {
                /* 
                 * Display dual-pass-mapped/uv-transform clump...
                 */
                ClumpSetEffect(DualPassClump, MaterialEffectType);

                RpWorldAddClump(World, DualPassClump);

                CurrentClump = DualPassClump;

                break;
            }
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
EnvMapSetTextureCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return MaterialEffectsOn && (MaterialEffectType == ENV_MAPPING);
    }
    
    ClumpSetEnvMap(EnvClump, EnvMap[CurrentEnvMap]);
    
    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
EnvMapSetShininessCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return MaterialEffectsOn && ((MaterialEffectType == ENV_MAPPING) || 
                                     MaterialEffectType == ENV_BUMP_MAPPING);
    }

    /*
     * Update both environment-mapped clumps to keep them in step
     * with the global shininess value...
     */
    ClumpSetEnvMapShininess(EnvClump);

    ClumpSetEnvMapShininess(EnvBumpClump);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
BumpMapSetTextureCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return MaterialEffectsOn && (MaterialEffectType == BUMP_MAPPING);
    }

    ClumpSetBumpMap(BumpClump, BumpMap[CurrentBumpMap]);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
BumpMapSetBumpinessCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return MaterialEffectsOn && ((MaterialEffectType == BUMP_MAPPING) || 
                                     MaterialEffectType == ENV_BUMP_MAPPING);
    }

    /*
     * Update both bump-mapped clumps to keep them in step
     * with the global bumpiness value...
     */
    ClumpSetBumpMapBumpiness(BumpClump);

    ClumpSetBumpMapBumpiness(EnvBumpClump);

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
DualPassSetBlendModeCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return MaterialEffectsOn && 
            (MaterialEffectType == DUAL_PASS_MAPPING
             || MaterialEffectType == DUAL_UV_TRANSFORM);
    }

    if (DualPassClump != NULL)
    {
        ClumpSetEffect(DualPassClump, MaterialEffectType);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool 
ToggleLightSpinCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    if( !LightSpinOn )
    {
        LightResetFrame(MainLight);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{
    static RwChar materialEffectsLabel[] = RWSTRING("Material effects_M");
    static RwChar effectTypeLabel[]      = RWSTRING("Mapping type_T");

    static RwChar envMapNameLabel[] = RWSTRING("Environment-map_E");
    static RwChar envMapMaterialsLabel[]  = RWSTRING("Active material_A");
    static RwChar shininesslabel[]  = RWSTRING("Shininess_S");
    
    static RwChar bumpClumpLabel[] = RWSTRING("Bump-map_B");
    static RwChar bumpMaterialsLabel[]  = RWSTRING("Active material_A");
    static RwChar bumpinessLabel[] = RWSTRING("Bumpiness_U");

    static RwChar dualPassSetSrcBlendModeLabel[]  = RWSTRING("Src Blend Mode");
    static RwChar dualPassSetDestBlendModeLabel[] = RWSTRING("Dest Blend Mode");

    static RwChar spinLightLabel[] = RWSTRING("Spin light_L");
    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    static const RwChar *materialNames[] = 
    {
        RWSTRING("first"),
        RWSTRING("second"),
        RWSTRING("both")
    };

    static const RwChar *materialEffectName[NUM_MAT_EFFECTS] = 
    {
        RWSTRING("environment"), 
        RWSTRING("bump"),
        RWSTRING("environment & bump"),
        RWSTRING("dual-pass"),
        RWSTRING("uv-transform"),
        RWSTRING("dual uv-transform")
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        /*
         * Toggles all material effects...
         */
        MenuAddEntryBool(materialEffectsLabel, &MaterialEffectsOn, 
                         ToggleMaterialEffectsCallback);

        /*
         * Selects the current material effect...
         */
        MenuAddEntryInt(effectTypeLabel, &MaterialEffectType,
                        SelectMaterialEffectCallback, 0, NUM_MAT_EFFECTS-1, 1, 
                        materialEffectName);

        MenuAddSeparator();

        /*
         * Environment-map effect...
         */
        MenuAddEntryInt(envMapNameLabel, &CurrentEnvMap,
                        EnvMapSetTextureCallback, 0, NUM_ENV_MAPS-1, 1, EnvMapName);

        MenuAddEntryInt(envMapMaterialsLabel, &EnvMapActiveMaterials,
                        EnvMapSetTextureCallback, 0, 2, 1, materialNames);

        MenuAddEntryReal(shininesslabel, &EnvMapShininess, 
                         EnvMapSetShininessCallback, 0.0f, 1.0f, 0.01f);

        MenuAddSeparator();

        /* 
         * Bump-map effect...
         */
        MenuAddEntryInt(bumpClumpLabel, &CurrentBumpMap,
                        BumpMapSetTextureCallback, 0, NUM_BUMP_MAPS-1, 1, BumpMapName);

        MenuAddEntryInt(bumpMaterialsLabel, &BumpMapActiveMaterials,
                        BumpMapSetTextureCallback, 0, 2, 1, materialNames);

        MenuAddEntryReal(bumpinessLabel, &BumpMapBumpiness, 
                         BumpMapSetBumpinessCallback, -5.0f, 5.0f, 0.01f);

        MenuAddSeparator();

        /* 
         * Dual Pass effect...
         */
        MenuAddEntryInt(dualPassSetSrcBlendModeLabel,
                        (RwInt32 *)&DualPassSrcBlendMode,
                        DualPassSetBlendModeCallback,
                        rwBLENDZERO, rwBLENDSRCALPHASAT,
                        1, DualPassBlendModeNames);

        MenuAddEntryInt(dualPassSetDestBlendModeLabel,
                        (RwInt32 *)&DualPassDestBlendMode, 
                        DualPassSetBlendModeCallback,
                        rwBLENDZERO, rwBLENDSRCALPHASAT,
                        1, DualPassBlendModeNames);

        MenuAddSeparator();

        /*
         * Spin light and frame counter...
         */
        MenuAddEntryBool(spinLightLabel, &LightSpinOn, ToggleLightSpinCallback);

        MenuAddEntryBool(fpsLabel, &FPSOn, NULL);

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool 
Initialize(void)
{
    if( RsInitialize() )
    {
        if( !RsGlobal.maximumWidth )
        {
            RsGlobal.maximumWidth = DEFAULT_SCREEN_WIDTH;
        }

        if( !RsGlobal.maximumHeight )
        {
            RsGlobal.maximumHeight = DEFAULT_SCREEN_HEIGHT;
        }

        RsGlobal.appName = RWSTRING("RenderWare Graphics Material Effects Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool 
Initialize3D(void *param)
{
    if( !RsRwInitialize(param) )
    {
        RsErrorMessage(RWSTRING("Error initializing RenderWare."));

        return FALSE;
    }

    Charset = RtCharsetCreate(&ForegroundColor, &BackgroundColor);
    if( Charset == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create raster charset."));
    
        return FALSE;
    }

    World = CreateWorld();
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    if( !CreateLights(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create light sources"));
    }

    /* 
     * Create environment-mapped clump...
     */
    EnvClump = CreateEnvClump();
    if( !EnvClump )
    {
        RsErrorMessage(RWSTRING("Cannot load environment-map clump."));
        
        return FALSE;
    }

    /* 
     * Create bump-mapped clump...
     */
    BumpClump = CreateBumpClump();
    if( !BumpClump )
    {
        RsErrorMessage(RWSTRING("Cannot load bump-map clump."));
        
        return FALSE;
    }

    /*
     * Create environment- and bump-mapped clump...
     */
    EnvBumpClump = CreateEnvBumpClump();
    if( !EnvBumpClump )
    {
        RsErrorMessage(RWSTRING("Cannot load env-bump-map clump."));
        
        return FALSE;
    }

    /*
     * Create dual-pass clump...
     */
    DualPassClump = CreateDualPassClump();
    if( !DualPassClump )
    {
        RsErrorMessage(RWSTRING("Cannot load dual-pass clump."));
        
        return FALSE;
    }

    if( !CreateUVAnimTransforms() )
    {
        RsErrorMessage(RWSTRING("Cannot create UV anim transforms."));
        
        return FALSE;
    }

    if( !InitializeMenu() )
    {
        RsErrorMessage(RWSTRING("Error initializing menu."));

        return FALSE;
    }

#ifdef RWMETRICS
    RsMetricsOpen(Camera);
#endif

    /*
     * Add one clump to the world...
     */
    SelectMaterialEffectCallback(FALSE);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RpLight*
DestroyLight(RpLight *light,
             void *data __RWUNUSED__)
{
    RwFrame *frame = NULL;

    RpWorldRemoveLight(World, light);

    frame = RpLightGetFrame(light);
    if( frame )
    {
        RpLightSetFrame(light, NULL);

        RwFrameDestroy(frame);
    }

    RpLightDestroy(light);

    return light;
}


/*
 *****************************************************************************
 */
static void 
Terminate3D(void)
{
    RwInt32 i;

#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    if( DualPassClump )
    {
        if( RpClumpGetWorld(DualPassClump) )
        {
            RpWorldRemoveClump(World, DualPassClump);
        }

        RpClumpDestroy(DualPassClump);
    }

    DestroyUVAnimTransforms();

    if( DualPassMap )
    {
        RwTextureDestroy(DualPassMap);
    }

    if( EnvBumpClump )
    {
        if( RpClumpGetWorld(EnvBumpClump) )
        {
            RpWorldRemoveClump(World, EnvBumpClump);
        }

        RpClumpDestroy(EnvBumpClump);
    }

    for(i=0; i<NUM_ENV_BUMP_MAPS; i++)
    {
        if( EnvBumpMap[i] )
        {
            RwTextureDestroy(EnvBumpMap[i]);
        }
    }

    if( BumpClump )
    {
        if( RpClumpGetWorld(BumpClump) )
        {
            RpWorldRemoveClump(World, BumpClump);
        }

        RpClumpDestroy(BumpClump);
    }

    for(i=0; i<NUM_BUMP_MAPS; i++)
    {
        if( BumpMap[i] )
        {
            RwTextureDestroy(BumpMap[i]);
        }
    }

    if( EnvClump )
    {
        if( RpClumpGetWorld(EnvClump) )
        {
            RpWorldRemoveClump(World, EnvClump);
        }

        RpClumpDestroy(EnvClump);
    }

    for(i=0; i<NUM_ENV_MAPS; i++)
    {
        if( EnvMap[i] )
        {
            RwTextureDestroy(EnvMap[i]);
        }
    }

    RpWorldForAllLights(World, DestroyLight, NULL);

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        CameraDestroy(Camera);
    }

    if( World )
    {
        RpWorldDestroy(World);
    }

    if( Charset )
    {
        RwRasterDestroy(Charset);
    }

    RsRwTerminate();

    return;
}


/*
 *****************************************************************************
 */
static RwBool 
AttachPlugins(void)
{
    /* 
     * Attach world plug-in...
     */
    if( !RpWorldPluginAttach() )
    {
        return FALSE;
    }

    /* 
     * Attach material effects plug-in...
     */
    if( !RpMatFXPluginAttach() )
    {
        return FALSE;
    }

    /* For Sky builds register the matfx PDS pipes */
#if (defined(SKY))
    RpMatfxPipesAttach();
#endif /* (defined(SKY)) */

#ifdef RWLOGO
    /* 
     * Attach logo plug-in...
     */
    if( !RpLogoPluginAttach() )
    {
        return FALSE;
    }
#endif

    return TRUE;
}


/*
 *****************************************************************************
 */
static void 
DisplayOnScreenInfo(void)
{
    RwChar caption[256];

    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }

    return;
}


/*
 *****************************************************************************
 */
static void 
Render(void)
{
    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            RpWorldRender(World);

            DisplayOnScreenInfo();
        }

        MenuRender(Camera, NULL);

#ifdef RWMETRICS
        RsMetricsRender();
#endif

        RwCameraEndUpdate(Camera);
    }

    /* 
     * Display camera's raster...
     */
    RsCameraShowRaster(Camera);

    FrameCounter++;

    return;
}


/*
 *****************************************************************************
 */
static void 
Idle(void)
{
    RwUInt32 thisTime;
    RwReal deltaTime;

    static RwBool firstCall = TRUE;
    static RwUInt32 lastFrameTime, lastAnimTime;

    if( firstCall )
    {
        lastFrameTime = lastAnimTime = RsTimer();

        firstCall = FALSE;
    }

    thisTime = RsTimer();

    /* 
     * Has a second elapsed since we last updated the FPS...
     */
    if( thisTime > (lastFrameTime + 1000) )
    {
        /* 
         * Capture the frame counter...
         */
        FramesPerSecond = FrameCounter;
        
        /*
         * ...and reset...
         */
        FrameCounter = 0;
        
        lastFrameTime = thisTime;
    }

    /*
     * Time in second since the last update...
     */
    deltaTime = (thisTime - lastAnimTime) * 0.001f;

    /*
     * Animate the second-pass texture mapping...
     */
    if (    MaterialEffectType == UV_TRANSFORM 
        ||  MaterialEffectType == DUAL_UV_TRANSFORM)
    {
        UpdateUVAnimTransforms(deltaTime);
    }

    /*
     * Vary the directional of the light...
     */
    if( LightSpinOn )
    {
        LightRotate(deltaTime);
    }

    lastAnimTime = thisTime;

    Render();

    return;
}


/*
 *****************************************************************************
 */
RsEventStatus
AppEventHandler(RsEvent event, void *param)
{
    switch( event )
    {
        case rsINITIALIZE:
            {
                return Initialize() ? rsEVENTPROCESSED : rsEVENTERROR;
            }

        case rsCAMERASIZE:
            {
                CameraSize(Camera, (RwRect *)param, 
                           DEFAULT_VIEWWINDOW, DEFAULT_ASPECTRATIO);

                return rsEVENTPROCESSED;
            }

        case rsRWINITIALIZE:
            {
                return Initialize3D(param) ? rsEVENTPROCESSED : rsEVENTERROR;
            }

        case rsRWTERMINATE:
            {
                Terminate3D();

                return rsEVENTPROCESSED;
            }

        case rsPLUGINATTACH:
            {
                return AttachPlugins() ? rsEVENTPROCESSED : rsEVENTERROR;
            }

        case rsINPUTDEVICEATTACH:
            {
                AttachInputDevices();

                return rsEVENTPROCESSED;
            }

        case rsIDLE:
            {
                Idle();

                return rsEVENTPROCESSED;
            }

        default:
            {
                return rsEVENTNOTPROCESSED;
            }
    }
}

/*
*****************************************************************************
*/

