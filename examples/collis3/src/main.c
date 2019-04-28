
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
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrates the detection of collisions with atomics and
 *          the creation of geometry collision data to speed up the
 *          intersection tests.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpcollis.h"
#include "rprandom.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rtcharse.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#include "collis3.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

#define WORLD_SCALE (12.0f)

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;
static RwBool FPSOn = TRUE;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static const RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
static const RwV3d Yaxis = {0.0f, 1.0f, 0.0f};

static RtCharset *Charset = NULL;
static RpWorld *World = NULL;

static RpLight *AmbientLight = NULL;
static RpLight *MainLight = NULL;
static RwV3d LightDir = {-1.0f, -2.0f, 0.0f};

static RwCamera *Camera = NULL;
static RwV3d CameraPos = {-10.4f, 8.0f, -6.0f};
static RwReal CameraAzimuth = 60.0f;
static RwReal CameraPitch = 55.0f;



/*
 ****************************************************************************
 */
static RpWorld * 
CreateWorld(void)
{
    RpWorld *world;
    RwBBox bbox;

    /*
     * Create an empty world...
     */
    bbox.inf.x = bbox.inf.y = bbox.inf.z = -WORLD_SCALE;
    bbox.sup.x = bbox.sup.y = bbox.sup.z =  WORLD_SCALE;

    world = RpWorldCreate(&bbox);

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
        RwRGBAReal color = {0.4f, 0.4f, 0.4f, 1.0f};

        RpLightSetColor(light, &color);

        RpWorldAddLight(world, light);

        return light;
    }

    return NULL;
}


/*
 ****************************************************************************
 */
static RpLight *
CreateMainLight(RpWorld *world, RwV3d *direction)
{
    RpLight *light = NULL;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);

    if( light )
    {
        RwFrame *frame;
        RwMatrix *matrix;
        RwV3d *right, *up, *at;
        RwRGBAReal color = {0.8f, 0.8f, 0.8f, 1.0f};

        /*
         * Point light's at vector in the given direction and 
         * get orthogonal vectors for right and up.
         */
        frame = RwFrameCreate();
        matrix = RwFrameGetMatrix(frame);

        right = RwMatrixGetRight(matrix);
        up = RwMatrixGetUp(matrix);
        at = RwMatrixGetAt(matrix);

        RwV3dNormalize(at, direction);
        RwV3dCrossProduct(right, &Yaxis, at);
        RwV3dNormalize(right, right);
        RwV3dCrossProduct(up, at, right);
        RwMatrixUpdate(matrix);

        RpLightSetColor(light, &color);

        RpLightSetFrame(light, frame);
        
        RpWorldAddLight(world, light);
    }

    return light;
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
        RwFrame *cameraFrame;

        cameraFrame = RwCameraGetFrame(camera);

        /*
         * Set default position and orientation...
         */
        RwFrameRotate(cameraFrame, &Xaxis, CameraPitch, rwCOMBINEREPLACE);
        RwFrameRotate(cameraFrame, &Yaxis, CameraAzimuth, rwCOMBINEPOSTCONCAT);
        RwFrameTranslate(cameraFrame, &CameraPos, rwCOMBINEPOSTCONCAT);

        RwCameraSetFarClipPlane(camera,  4.00f * WORLD_SCALE);
        RwCameraSetNearClipPlane(camera, 0.04f * WORLD_SCALE);

        RpWorldAddCamera(world, camera);

        return camera;
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Collision3 Example");

        return TRUE;
    }

    return FALSE;
}

/*
 *****************************************************************************
 */
static RwBool
ResetCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    /*
     * Restart the example by destroying and recreating objects...
     */
    CollisionObjectsDestroy(World);
    CollisionObjectsCreate(World);

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{
    static RwChar restartLabel[] = RWSTRING("Reset_R");

    static RwChar buildCollisLabel[] = RWSTRING("Build collision data_B");
    static RwChar saveCollisLabel[]  = RWSTRING("Save collision data_S");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryTrigger(restartLabel, ResetCallback);
        MenuAddSeparator();

        MenuAddEntryTrigger(buildCollisLabel, CollisionDataBuildCallback);
        MenuAddEntryTrigger(saveCollisLabel, CollisionDataSaveCallback);
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

    MainLight = CreateMainLight(World, &LightDir);
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

    if( !CollisionObjectsCreate(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create collision objects."));

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

    CollisionObjectsDestroy(World);

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

    /* 
     * Attach collision plug-in...
     */
    if( !RpCollisionPluginAttach() )
    {
        return FALSE;
    }

    /* 
     * Attach random plug-in...
     */
    if( !RpRandomPluginAttach() )
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

#ifdef SKY
    RpSkySelectTrueTSClipper(TRUE);
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
        /*
         * Display the frame counter...
         */
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }

    if( !CollisionDataGenerated )
    {
        RsSprintf(caption, 
            RWSTRING("Select \"Build collision data\" to optimize collisions"));

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSBOTTOMRIGHT);
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
     * Animation update time in seconds...
     */
    deltaTime = (thisTime - lastAnimTime) * 0.001f;

    CollisionObjectsUpdate(World, deltaTime);

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
