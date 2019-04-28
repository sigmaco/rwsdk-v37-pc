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
 * Copyright (c) 2000 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * main.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics mipmaps example.
 *
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "mipmap.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static RpWorld *World = NULL;
static RpLight *MainLight = NULL;
static RpLight *AmbientLight = NULL;
static RtCharset *Charset = NULL;

static RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
static RwV3d Yaxis = {0.0f, 1.0f, 0.0f};

static RwChar MipBlending[64] = RWSTRING("Linear");
static RwChar TexelBlending[64] = RWSTRING("Linear");

static RwInt32 TextureFilterModeIndex = 5;
static RwInt32 NumMipLevels;

#ifdef SKY
static RwReal MipmapKValue = -1.0f;
#endif

RwCamera *Camera = NULL;
RpClump *Clump = NULL;



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
static RpLight *
CreateAmbientLight(RpWorld *world)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RpWorldAddLight(world, light);

        return light;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static RpLight *
CreateMainLight(RpWorld *world)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    if( light )
    {
        RwFrame *frame;

        frame = RwFrameCreate();
        if( frame )
        {
            RwFrameRotate(frame, &Xaxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &Yaxis, 30.0f, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, frame);

            RpWorldAddLight(world, light);

            return light;
        }

        RpLightDestroy(light);
    }

    return NULL;
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
        RwCameraSetFarClipPlane(camera, 100.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialGetNumMipLevels(RpMaterial *material, void *data)
{
    RwTexture *texture;

    texture = RpMaterialGetTexture(material);

    *(RwInt32 *)data = RwRasterGetNumLevels(RwTextureGetRaster(texture));

    return NULL;
}


static RpAtomic *
AtomicGetNumMipLevels(RpAtomic *atomic, void *data)
{
    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
        MaterialGetNumMipLevels, data);

    return NULL;
}


static RwInt32 
TextureGetNumMipLevels(RpClump *clump)
{
    RwInt32 numLevels = 0;

    RpClumpForAllAtomics(clump, AtomicGetNumMipLevels, &numLevels);

    return numLevels;
}


/*
 *****************************************************************************
 */
static RpClump *
CreateClump(RpWorld *world)
{
    RpClump *clump;
    RwChar *path;
    RwStream *stream;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING("models/bucky.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    clump = NULL;

#ifdef SKY
    RpSkyTextureSetDefaultMipmapK(MipmapKValue);
#endif

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            clump = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    if( clump )
    {
        RwV3d pos = {0.0f, 0.0f, 4.0f};

        RwFrameTranslate(RpClumpGetFrame(clump), &pos, rwCOMBINEREPLACE);

        RpWorldAddClump(world, clump);

        NumMipLevels = TextureGetNumMipLevels(clump);
    }

    return clump;
}


/*
 *****************************************************************************
 */
void
ClumpRotate(RpClump *clump, RwCamera *camera, RwReal xAngle, RwReal yAngle)
{
    RwMatrix *cameraMatrix;
    RwV3d right, up, pos;
    RwFrame *clumpFrame;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    clumpFrame = RpClumpGetFrame(clump);
    pos = *RwMatrixGetPos(RwFrameGetMatrix(clumpFrame));

    /*
     * First translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * ...do the rotations...
     */
    RwFrameRotate(clumpFrame, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(clumpFrame, &right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * ...and translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 ***************************************************************************
 */
void
ClumpTranslateZ(RpClump *clump, RwCamera *camera, RwReal zDelta)
{
    RwFrame *clumpFrame, *cameraFrame;
    RwV3d delta;

    clumpFrame = RpClumpGetFrame(clump);
    cameraFrame = RwCameraGetFrame(camera);

    RwV3dScale(&delta, RwMatrixGetAt(RwFrameGetMatrix(cameraFrame)), zDelta); 

    RwFrameTranslate(clumpFrame, &delta, rwCOMBINEPOSTCONCAT);

    return;
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Mipmap Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialSetTextureFilterMode(RpMaterial *material, void *data)
{
    RwTexture * const texture = RpMaterialGetTexture(material);
    const RwTextureFilterMode filterMode = *(RwTextureFilterMode *)data;
    RwTexture *checkTexture;

    checkTexture = RwTextureSetFilterMode(texture, filterMode);
    
    return material;
}


static RpAtomic *
AtomicSetTextureFilterMode(RpAtomic *atomic, void *data)
{
    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
        MaterialSetTextureFilterMode, data);

    return atomic;
}


static void 
ClumpSetTextureFilterMode(RpClump *clump, RwInt32 modeIndex)
{
    RwTextureFilterMode filterFlag;

    switch( modeIndex )
    {
        case 0:
        {
            filterFlag = rwFILTERNEAREST;

            rwstrcpy(MipBlending, RWSTRING("None"));
            rwstrcpy(TexelBlending, RWSTRING("Nearest"));

            break;
        }

        case 1:
        {
            filterFlag = rwFILTERLINEAR;

            rwstrcpy(MipBlending, RWSTRING("None"));
            rwstrcpy(TexelBlending, RWSTRING("Linear"));

            break;
        }

        case 2:
        {
            filterFlag = rwFILTERMIPNEAREST;

            rwstrcpy(MipBlending, RWSTRING("Nearest"));
            rwstrcpy(TexelBlending, RWSTRING("Nearest"));

            break;
        }

        case 3:
        {
            filterFlag = rwFILTERMIPLINEAR;

            rwstrcpy(MipBlending, RWSTRING("Nearest"));
            rwstrcpy(TexelBlending, RWSTRING("Linear"));

            break;
        }

        case 4:
        {
            filterFlag = rwFILTERLINEARMIPNEAREST;

            rwstrcpy(MipBlending, RWSTRING("Linear"));
            rwstrcpy(TexelBlending, RWSTRING("Nearest"));

            break;
        }

        case 5:
        {
            filterFlag = rwFILTERLINEARMIPLINEAR;

            rwstrcpy(MipBlending, RWSTRING("Linear"));
            rwstrcpy(TexelBlending, RWSTRING("Linear"));

            break;
        }

        default:
        {
            break;
        }
    }

    RpClumpForAllAtomics(clump, 
        AtomicSetTextureFilterMode, (void *)&filterFlag);

    return;
}


/*
 *****************************************************************************
 */
static RwBool 
TextureFilterCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    ClumpSetTextureFilterMode(Clump, TextureFilterModeIndex);

    return TRUE;
}


/*
 *****************************************************************************
 */
#ifdef SKY

static RpMaterial *
MaterialSetMipmapK(RpMaterial *material, void *data)
{
    RpSkyTextureSetMipmapK(
        RpMaterialGetTexture(material), *(RwReal *)data);

    return material;
}


static RpAtomic *
AtomicSetMipmapK(RpAtomic *atomic, void *data)
{
    RpGeometryForAllMaterials(
        RpAtomicGetGeometry(atomic), MaterialSetMipmapK, data);

    return atomic;
}


static RwBool 
MipmapKValueCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    RpClumpForAllAtomics(Clump, AtomicSetMipmapK, (void *)&MipmapKValue);

    return TRUE;
}

#endif /* SKY */


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{
    static RwChar textureFilterLabel[] = RWSTRING("Filter mode");
#ifdef SKY
    static RwChar mipmapKValueLabel[]  = RWSTRING("Mipmap K-value");
#endif
    static RwChar fpsLabel[]           = RWSTRING("FPS_F");

    static const RwChar *textureFilterStrings[] = 
    {
        RWSTRING("rwFILTERNEAREST"),
        RWSTRING("rwFILTERLINEAR"),
        RWSTRING("rwFILTERMIPNEAREST"),
        RWSTRING("rwFILTERMIPLINEAR"),
        RWSTRING("rwFILTERLINEARMIPNEAREST"),
        RWSTRING("rwFILTERLINEARMIPLINEAR")
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryInt(textureFilterLabel, &TextureFilterModeIndex,
            TextureFilterCallback, 0, 5, 1, textureFilterStrings);

#ifdef SKY
        MenuAddEntryReal(mipmapKValueLabel, &MipmapKValue,
            MipmapKValueCallback, -10.0f, 10.0f, 0.05f);
#endif

        MenuAddSeparator();

        MenuAddEntryBool(fpsLabel, &FPSOn, NULL);

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

    AmbientLight = CreateAmbientLight(World);
    if( AmbientLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create ambient light."));

        return FALSE;
    }

    MainLight = CreateMainLight(World);
    if( MainLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create main light."));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    Clump = CreateClump(World);
    if( Clump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create clump."));

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

    return TRUE;
}


/*
 *****************************************************************************
 */
static void 
Terminate3D(void)
{
#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    if( Clump )
    {
        RpWorldRemoveClump(World, Clump);

        RpClumpDestroy(Clump);
    }

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        CameraDestroy(Camera);
    }

    if( MainLight )
    {
        RwFrame *frame;

        RpWorldRemoveLight(World, MainLight);

        frame = RpLightGetFrame(MainLight);
        RpLightSetFrame(MainLight, NULL);
        RwFrameDestroy(frame);

        RpLightDestroy(MainLight);
    }

    if( AmbientLight )
    {
        RpWorldRemoveLight(World, AmbientLight);

        RpLightDestroy(AmbientLight);
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

    RsSprintf(caption, RWSTRING("Number mip levels: %d"), NumMipLevels);
    RsCharsetPrint(Charset, caption, 0, -2, rsPRINTPOSBOTTOMRIGHT);

    RsSprintf(caption, RWSTRING("Texel blending: %7s"), TexelBlending);
    RsCharsetPrint(Charset, caption, 0, -1, rsPRINTPOSBOTTOMRIGHT);

    RsSprintf(caption, RWSTRING("Mip level blending: %7s"), MipBlending);
    RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSBOTTOMRIGHT);

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

    static RwBool firstCall = TRUE;
    static RwUInt32 lastFrameTime;

    if( firstCall )
    {
        lastFrameTime = RsTimer();

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
