
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
 * Purpose: RenderWare Graphics camera texture example.
 ****************************************************************************/

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

#include "camtex.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

#define TEXSIZE (256)

static RtCharset *Charset = NULL;
static RpWorld *World = NULL;
static RpLight *AmbientLight = NULL;
static RpLight *MainLight = NULL;
static RwCamera *Camera2 = NULL;
static RwTexture *Texture = NULL;
static RwTexture *CameraTexture = NULL;

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
static RwV3d Yaxis = {0.0f, 1.0f, 0.0f};

RwCamera *Camera = NULL;
RpClump *Clump = NULL;
RwBool SpinOn = TRUE;



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
        RwRGBAReal color = {0.5f, 0.5f, 0.5f, 1.0f};

        RpLightSetColor(light, &color);

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
            RwRGBAReal color = {0.5f, 0.5f, 0.5f, 1.0f};

            RpLightSetColor(light, &color);

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
        RwCameraSetFarClipPlane(camera, 250.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static RwCamera *
CreateTextureCamera(RpWorld *world)
{
    RwRaster *raster;

    raster = RwRasterCreate(TEXSIZE, TEXSIZE, 0, rwRASTERTYPECAMERATEXTURE);

    if( raster )
    {
        RwRaster *zRaster;

        zRaster = RwRasterCreate(TEXSIZE, TEXSIZE, 0, rwRASTERTYPEZBUFFER);

        if( zRaster )
        {
            RwFrame *frame;

            frame = RwFrameCreate();

            if( frame )
            {
                RwCamera *camera;

                camera = RwCameraCreate();

                if( camera)
                {
                    RwV2d vw;

                    RwCameraSetRaster(camera, raster);
                    RwCameraSetZRaster(camera, zRaster);

                    RwCameraSetFrame(camera, frame);

                    RwCameraSetNearClipPlane(camera, 0.1f);
                    RwCameraSetFarClipPlane(camera, 250.0f);

                    vw.x = vw.y = 0.4f;
                    RwCameraSetViewWindow(camera, &vw);

                    RpWorldAddCamera(world, camera);

                    /*
                     * Create a texture from this camera's raster so we
                     * can use it on the clump...
                     */
                    CameraTexture = RwTextureCreate(raster);
                    RwTextureSetFilterMode(CameraTexture, rwFILTERLINEAR);

#ifdef RWLOGO
                    /*
                     * Do not render the logo for texture rendering...
                     */
                    RpLogoSetState(camera, FALSE);
#endif

                    return camera;
                }

                RwFrameDestroy(frame);
            }

            RwRasterDestroy(zRaster);
        }

        RwRasterDestroy(raster);
    }

    return NULL;
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

    path = RsPathnameCreate(RWSTRING("models/cube.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    clump = NULL;

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            clump = RpClumpStreamRead(stream);

            if( clump )
            {
                RwV3d pos = {0.0f, 0.0f, 30.0f};

                RwFrameTranslate(RpClumpGetFrame(clump), &pos, rwCOMBINEREPLACE);

                RpWorldAddClump(world, clump);
            }
        }

        RwStreamClose(stream, NULL);
    }

    return clump;
}


/*
 *****************************************************************************
 */
static RwTexture *
CreateTexture(void)
{
    RwChar *path;
    RwTexture *texture;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    texture = RwTextureRead(RWSTRING("whiteash"), NULL);
    if( texture )
    {
        RwTextureSetFilterMode(texture, rwFILTERLINEAR);

        return texture;
    }

    return NULL;
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Camera Texture Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{
    static RwChar spinLabel[] = RWSTRING("Spin_S");
    static RwChar fpsLabel[]  = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryBool(spinLabel, &SpinOn, NULL);
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

    Camera2 = CreateTextureCamera(World);
    if( Camera2 == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create texture camera."));

        return FALSE;
    }

    Clump = CreateClump(World);
    if( Clump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create clump."));

        return FALSE;
    }

    Texture = CreateTexture();
    if( Texture == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create texture."));

        return FALSE;
    }

    if( !InitializeMenu() )
    {
        RsErrorMessage(RWSTRING("Error initializing menu."));

        return FALSE;
    }

#ifdef RWMETRICS
    RsMetricsOpen(Camera);
#endif /* RWMETRICS */

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
#endif /* RWMETRICS */

    MenuClose();

    if( Texture )
    {
        RwTextureDestroy(Texture);
    }

    if( Clump )
    {
        RpWorldRemoveClump(World, Clump);

        RpClumpDestroy(Clump);
    }

    if( Camera2 )
    {
        RwFrame *frame;
        RwRaster *raster;

        RpWorldRemoveCamera(World, Camera2);

        frame = RwCameraGetFrame(Camera2);
        RwCameraSetFrame(Camera2, NULL);
        RwFrameDestroy(frame);

        raster = RwCameraGetRaster(Camera2);
        RwCameraSetRaster(Camera2, NULL);
        RwRasterDestroy(raster);

        raster = RwCameraGetZRaster(Camera2);
        RwCameraSetZRaster(Camera2, NULL);
        RwRasterDestroy(raster);

        RwCameraDestroy(Camera2);

        RwTextureSetRaster(CameraTexture, NULL);
        RwTextureDestroy(CameraTexture);
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
static RpMaterial *
MaterialApplyTexture(RpMaterial *material, void *data)
{
    RpMaterialSetTexture(material, (RwTexture *)data);

    return material;
}


static RpAtomic *
AtomicApplyTexture(RpAtomic *atomic, void *data)
{
    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
        MaterialApplyTexture, data);

    return atomic;
}


static void 
ClumpApplyTexture(RpClump *clump, RwTexture *texture)
{
    RpClumpForAllAtomics(clump, AtomicApplyTexture, (void *)texture);

    return;
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
    /*
     * First apply the texture to the clump...
     */
    ClumpApplyTexture(Clump, Texture);

#ifdef SKY
    /*
     * Lock the camera texture raster in memory while we 
     * render into it...
     */
    skyTexCacheRasterLock(RwCameraGetRaster(Camera2), TRUE);
#endif

    /*
     * Render scene into the camera texture raster...
     */
    RwCameraClear(Camera2, &ForegroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera2) )
    {
        RpWorldRender(World);

        RwCameraEndUpdate(Camera2);
    }

#ifdef DOLPHIN
    RwGameCubeCameraTextureFlush(RwCameraGetRaster(Camera2), FALSE);
#endif 

    /*
     * Apply the camera texture to the clump...
     */
    ClumpApplyTexture(Clump, CameraTexture);

    /*
     * Render scene as normal...
     */
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

#ifdef SKY
    skyTexCacheRasterLock(RwCameraGetRaster(Camera2), FALSE);
#endif

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
UpdateClump(RwReal timeStep)
{
    RwFrame *frame;
    RwV3d *up, *at;
    RwReal angle;

    frame = RpClumpGetFrame(Clump);

    up = RwMatrixGetUp(RwFrameGetMatrix(frame));
    at = RwMatrixGetAt(RwFrameGetMatrix(frame));

    angle = timeStep * 50.0f;

    RwFrameRotate(frame, up, angle, rwCOMBINEPRECONCAT);
    RwFrameRotate(frame, at, angle, rwCOMBINEPRECONCAT);

    return;
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
     * Do the rotations...
     */
    RwFrameRotate(clumpFrame, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(clumpFrame, &right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * And translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

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
     * Animation update time in seconds...
     */
    deltaTime = (thisTime - lastAnimTime) * 0.001f;

    if( SpinOn )
    {
        UpdateClump(deltaTime);
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
