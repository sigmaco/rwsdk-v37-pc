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
 * Purpose: Illustrates simple collision detection between an RpWorld and
 *          a line or sphere.
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpcollis.h"

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

#include "collis1.h"

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

static RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
static RwV3d Yaxis = {0.0f, 1.0f, 0.0f};

static RtCharset *Charset = NULL;
static RpWorld *World = NULL;
static RpLight *AmbientLight = NULL;
static RpLight *MainLight = NULL;

RwCamera *Camera = NULL;


/*
 ****************************************************************************
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
 ****************************************************************************
 */
static RpLight *
CreateAmbientLight(RpWorld *world)
{
    RpLight *light = NULL;

    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RwRGBAReal color = {0.4f, 0.4f, 0.4f, 1.0f};

        RpLightSetColor(light, &color);

        RpWorldAddLight(world, light);
    }

    return light;
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
        RwFrame *cameraFrame;
        RwBBox bbox;
        RwV3d pos;

        /*
         * Place the camera in the center, at the top of the world..
         */
        bbox = *RpWorldGetBBox(world);

        RwV3dAdd(&pos, &bbox.sup, &bbox.inf);
        RwV3dScale(&pos, &pos, 0.5f);
        pos.y = bbox.sup.y;

        cameraFrame = RwCameraGetFrame(camera);
        RwFrameRotate(cameraFrame, &Yaxis, 45.0f, rwCOMBINEREPLACE);
        RwFrameTranslate(cameraFrame, &pos, rwCOMBINEPOSTCONCAT);

        RwCameraSetFarClipPlane(camera, 500.0f);
        RwCameraSetNearClipPlane(camera, 0.5f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
void 
CameraPoint(RwCamera *camera, RwReal turn, RwReal tilt)
{
    RwFrame *cameraFrame;
    RwV3d delta, pos, *right;

    static RwReal totalTilt = 0.0f;

    /*
     * Limit the camera's tilt so that it never quite reaches
     * exactly +90 or -90 degrees...
     */
    if( totalTilt + tilt > 89.0f )
    {
        tilt = 89.0f - totalTilt;

    }
    else if( totalTilt + tilt < -89.0f )
    {
        tilt = -89.0f - totalTilt;
    }

    totalTilt += tilt;

    cameraFrame = RwCameraGetFrame(camera);

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
    RwFrameRotate(cameraFrame, &Yaxis, turn, rwCOMBINEPOSTCONCAT);

    /*
     * Put the camera back to where it was...
     */
    RwFrameTranslate(cameraFrame, &pos, rwCOMBINEPOSTCONCAT);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Collision1 Example");

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
    static RwChar collisionTypeLabel[] = RWSTRING("Collision mode");
    static RwChar floatLabel[]         = RWSTRING("Camera radius");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    static const RwChar *collisionStrings[] = 
    {
        RWSTRING("Line"),
        RWSTRING("Sphere")
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryInt(collisionTypeLabel, (RwInt32 *)&CurrentCollisionMode, 
            NULL, 0, 1, 1, collisionStrings);  

        MenuAddEntryReal(floatLabel, &CameraRadius, NULL, 1.0f, 25.0f, 0.1f);

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

        RpLightDestroy(MainLight);

        if( frame )
        {
            RwFrameDestroy(frame);
        }
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

#ifdef RWLOGO
    /* 
     * Attach logo plug-in...
     */
    if( !RpLogoPluginAttach() )
    {
        return FALSE;
    }
#endif

#if (defined(SKY))
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
     * Animation update time in seconds...
     */
    deltaTime = (thisTime - lastAnimTime) * 0.001f;

    CameraUpdate(Camera, World, deltaTime);

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
