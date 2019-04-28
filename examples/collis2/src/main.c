
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
 * Purpose: To illustrate the use of the RpCollision plugin to collide 
 *          an atomic with the static geometry in a world.
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpcollis.h"
#include "rprandom.h"

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

#include "collis2.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.7f)

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static RtCharset *Charset = NULL;
static RpWorld *World = NULL;
static RwCamera *Camera = NULL;

static RpLight *MainLight = NULL;
static RpLight *AmbientLight = NULL;

static RwReal TotalTilt = 0.0f;

RwBool GravityOn = TRUE;
RwBool DampingOn = FALSE;



/*
 *****************************************************************************
 */
static RpWorld *
CreateWorld(void)
{
    RwStream *stream;
    RwChar *path;
    RpWorld *world;

    path = RsPathnameCreate(RWSTRING ("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING ("models/world.bsp"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    world = NULL;

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL) )
        {
            world = RpWorldStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }
    
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
        RwRGBAReal color = {0.6f, 0.6f, 0.6f, 1.0f};

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

    light = RpLightCreate(rpLIGHTPOINT);
    if( light )
    {
        RwFrame *frame;

        frame = RwFrameCreate();
        if( frame )
        {
            RwV3d pos = {0.0f, 130.0f, 0.0};

            RpLightSetRadius(light, 1000.0f);

            RwFrameTranslate(frame, &pos, rwCOMBINEREPLACE);

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
static void 
ResetCamera(RwCamera *camera)
{
    RwFrame *cameraFrame;
    RwV3d *at, pos;

    cameraFrame = RwCameraGetFrame(camera);
    RwFrameSetIdentity(cameraFrame);

    at = RwMatrixGetAt(RwFrameGetMatrix(cameraFrame));
    RwV3dScale(&pos, at, -320.0f);
    RwFrameTranslate(cameraFrame, &pos, rwCOMBINEREPLACE);

    TotalTilt = 0.0f;

    return;
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
        ResetCamera(camera);

        RwCameraSetFarClipPlane(camera, 700.0f);
        RwCameraSetNearClipPlane(camera, 1.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
void 
CameraPoint(RwReal turn, RwReal tilt)
{
    RwFrame *cameraFrame;
    RwV3d delta, pos, *right;
    RwV3d yAxis = {0.0f, 1.0f, 0.0f};

    /*
     * Limit the camera's tilt so that it never quite reaches
     * exactly +90 or -90 degrees...
     */
    if( TotalTilt + tilt > 89.0f )
    {
        tilt = 89.0f - TotalTilt;

    }
    else if( TotalTilt + tilt < -89.0f )
    {
        tilt = -89.0f - TotalTilt;
    }

    TotalTilt += tilt;

    cameraFrame = RwCameraGetFrame(Camera);

    /*
     * Remember where the camera is...
     */
    pos = *RwMatrixGetPos(RwFrameGetMatrix(cameraFrame));

    /*
     * Remove the translation component...
     */
    RwV3dScale(&delta, &pos, -1.0f);
    RwFrameTranslate(cameraFrame, &delta, rwCOMBINEPOSTCONCAT);

    /*
     * Rotate to the new direction...
     */
    right = RwMatrixGetRight(RwFrameGetMatrix(cameraFrame));
    RwFrameRotate(cameraFrame, right, tilt, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(cameraFrame, &yAxis, -turn, rwCOMBINEPOSTCONCAT);

    /*
     * Put the camera back to where it was...
     */
    RwFrameTranslate(cameraFrame, &pos, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
void
TranslateCameraZ(RwReal dist)
{
	RwFrame *frame = NULL;
    RwV3d at;

    frame = RwCameraGetFrame(Camera);

    at = *RwMatrixGetAt(RwFrameGetMatrix(frame));

    RwV3dScale(&at, &at, dist);

    RwFrameTranslate(frame, &at, rwCOMBINEPOSTCONCAT);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Collision2 Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
ResetBallCallBack(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    ResetBall();

    return TRUE;
}


static RwBool
GravityCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    ResetGravity();

    return TRUE;
}


static RwBool
ResetCameraCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    ResetCamera(Camera);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{    
    static RwChar resetBallLabel[] = RWSTRING("Reset ball_R");
    static RwChar gravityLabel[]   = RWSTRING("Gravity_G");
    static RwChar dampingLabel[]   = RWSTRING("Damping_D");

    static RwChar resetCameraLabel[] = RWSTRING("Reset camera_C");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryTrigger(resetBallLabel, ResetBallCallBack);
        MenuAddEntryBool(gravityLabel, &GravityOn, GravityCallback);
        MenuAddEntryBool(dampingLabel, &DampingOn, NULL);
        MenuAddSeparator();

        MenuAddEntryTrigger(resetCameraLabel, ResetCameraCallback);
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

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

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

    if( !CreateBallAtomic(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create ball atomic."));

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

    DestroyBallAtomic();

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

#ifdef RWLOGO
    /* 
     * Attach logo plug-in...
     */
    if( !RpLogoPluginAttach() )
    {
        return FALSE;
    }
#endif

    /*
     * Attach collision plug-in...
     */
    if( !RpCollisionPluginAttach() )
    {
        return FALSE;
    }

    if( !RpRandomPluginAttach() )
    {
        return FALSE;
    }

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
    static RwBool firstCall = TRUE;

    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( firstCall )
        {
            RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);

            firstCall = FALSE;
        }

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

    Run(deltaTime);

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
