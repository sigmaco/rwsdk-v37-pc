
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
 * Purpose: Example of parallel-projection shadow rendering using
 *          3D immediate mode.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#if (defined(SKY))
#include "rppds.h"
#endif /* (defined(SKY)) */
#include "rpskin.h"
#include "rphanim.h"
#include "rpcollis.h"

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

#include "scene.h"
#include "shadow.h"

/******************************************************************************
 * Defines */

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

/* Initial camera settings */
static const RwV3d  InitCameraPos = { 250.0f, 170.0f, -250.0f };

#define CAM_NEAR 1.0f
#define CAM_FAR  700.0f
#define CAM_ELEVATION 30.0f
#define CAM_AZIMUTH 54.0f

/******************************************************************************
 * Globals */

static RwBool       FPSOn = FALSE;

static RwInt32      FrameCounter = 0;
static RwInt32      FramesPerSecond = 0;

static RwRGBA       ForegroundColor = { 200, 200, 200, 255 };
static RwRGBA       BackgroundColor = { 64, 64, 64, 0 };

RwCamera           *Camera = NULL;
RtCharset          *Charset = NULL;

/* Navigation */
RwReal              CameraPitchRate = 0.0f;
RwReal              CameraTurnRate = 0.0f;
RwReal              CameraSpeed = 0.0f;
RwReal              CameraStrafeSpeed = 0.0f;

static RwV3d        Xaxis = { 1.0f, 0.0f, 0.0f };
static RwV3d        Yaxis = { 0.0f, 1.0f, 0.0f };

/*
 *****************************************************************************
 */
static RwCamera *
CreateCamera(RpWorld * world)
{
    RwCamera           *camera;

    camera =
        CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight,
                     TRUE);

    if (camera)
    {
        RwFrame            *frame;

        /* Set clip planes */
        RwCameraSetNearClipPlane(camera, CAM_NEAR);
        RwCameraSetFarClipPlane(camera, CAM_FAR);

        /* Position camera */
        frame = RwCameraGetFrame(camera);
        RwFrameRotate(frame, &Xaxis, CAM_ELEVATION, rwCOMBINEREPLACE);
        RwFrameRotate(frame, &Yaxis, -CAM_AZIMUTH, rwCOMBINEPOSTCONCAT);
        RwFrameTranslate(frame, &InitCameraPos, rwCOMBINEPOSTCONCAT);

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
    if (RsInitialize())
    {
        if (!RsGlobal.maximumWidth)
        {
            RsGlobal.maximumWidth = DEFAULT_SCREEN_WIDTH;
        }

        if (!RsGlobal.maximumHeight)
        {
            RsGlobal.maximumHeight = DEFAULT_SCREEN_HEIGHT;
        }

        RsGlobal.appName = RWSTRING("RenderWare Graphics Imshadow Example");

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
    static RwChar       fpsLabel[] = RWSTRING("FPS_F");

    if (MenuOpen(TRUE, &ForegroundColor, &BackgroundColor))
    {
        MenuAddEntryBool(fpsLabel, &FPSOn, NULL);

        if (SceneMenuInit())
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*
 *****************************************************************************
 */
static RwBool
Initialize3D(void *param)
{
    RpWorld            *world;

    if (!RsRwInitialize(param))
    {
        RsErrorMessage(RWSTRING("Error initializing RenderWare."));

        return FALSE;
    }

    Charset = RtCharsetCreate(&ForegroundColor, &BackgroundColor);
    if (Charset == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create raster charset."));

        return FALSE;
    }

    /*
     *  Create the world.
     */
    world = SceneOpen();
    if (world == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

    /*
     *  Create a camera using the democom way...
     */
    Camera = CreateCamera(world);
    if (Camera == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    if (!InitializeMenu())
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

    if (Camera)
    {
        RpWorldRemoveCamera(RwCameraGetWorld(Camera), Camera);
        CameraDestroy(Camera);
    }

    SceneClose();

    if (Charset)
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
    if (!RpWorldPluginAttach())
    {
        return FALSE;
    }

    /* 
     * Attach anim toolkit...
     */
    if (!RtAnimInitialize())
    {
        return FALSE;
    }
    
    /* 
     * Attach hanim plug-in...
     */
    if (!RpHAnimPluginAttach())
    {
        return FALSE;
    }

    /* 
     * Attach skin plug-in...
     */
    if (!RpSkinPluginAttach())
    {
        return FALSE;
    }
    /* For Sky builds register the skin PDS pipes */
#if (defined(SKY))
    RpSkinPipesAttach();
#endif /* (defined(SKY)) */

    /* 
     * Attach collision plug-in...
     */
    if (!RpCollisionPluginAttach())
    {
        return FALSE;
    }

#ifdef RWLOGO
    /* 
     * Attach logo plug-in...
     */
    if (!RpLogoPluginAttach())
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
    RwChar              caption[256];

    if (FPSOn)
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 5, rsPRINTPOSTOPRIGHT);
    }

    RsSprintf(caption, RWSTRING("Im3D Triangles: %05d"),
              ShadowNumTriangles);

    RsCharsetPrint(Charset, caption, 0, 6, rsPRINTPOSTOPRIGHT);

    return;
}

/*
 *****************************************************************************
 */
static void
Render(void)
{
    RwCameraClear(Camera, &BackgroundColor,
                  rwCAMERACLEARZ | rwCAMERACLEARIMAGE);

    if (RwCameraBeginUpdate(Camera))
    {
        if (MenuGetStatus() != HELPMODE)
        {
            /*
             * Scene rendering here...
             */
            SceneRender();

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
 ******************************************************************************
 */
static RwBool
CameraUpdate(RwReal deltaTime)
{
    if (CameraSpeed != 0.0f || CameraPitchRate != 0.0f
        || CameraTurnRate != 0.0f || CameraStrafeSpeed != 0.0f)
    {
        RwFrame            *frame = RwCameraGetFrame(Camera);
        RwMatrix           *m = RwFrameGetMatrix(frame);
        RwV3d              *right = RwMatrixGetRight(m);
        RwV3d               pos, invPos;

        pos = *RwMatrixGetPos(m);

        /*
         * Move camera to origin for rotation.
         */
        RwV3dNegate(&invPos, &pos);
        RwFrameTranslate(frame, &invPos, rwCOMBINEPOSTCONCAT);

        /*
         * Rotate camera
         */
        RwFrameRotate(frame, right, CameraPitchRate * deltaTime,
                      rwCOMBINEPOSTCONCAT);
        RwFrameRotate(frame, &Yaxis, CameraTurnRate * deltaTime,
                      rwCOMBINEPOSTCONCAT);
        /*
         * Move to new position
         */
        RwV3dIncrementScaled(&pos, RwMatrixGetAt(m),
                             CameraSpeed * deltaTime);
        RwV3dIncrementScaled(&pos, RwMatrixGetRight(m),
                             CameraStrafeSpeed * deltaTime);

        RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static void
Idle(void)
{
    RwUInt32            thisTime;
    RwReal              deltaTime;

    static RwBool       firstCall = TRUE;
    static RwUInt32     lastFrameTime, lastAnimTime;

    if (firstCall)
    {
        lastFrameTime = lastAnimTime = RsTimer();

        firstCall = FALSE;
    }

    thisTime = RsTimer();

    /* 
     * Has a second elapsed since we last updated the FPS...
     */
    if (thisTime > (lastFrameTime + 1000))
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

    /*
     * Update camera
     */
    CameraUpdate(deltaTime);

    /*
     * Update shadows
     */
    SceneUpdate(deltaTime);

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
    switch (event)
    {
        case rsINITIALIZE:
            {
                return Initialize()? rsEVENTPROCESSED : rsEVENTERROR;
            }

        case rsCAMERASIZE:
            {
                CameraSize(Camera, (RwRect *) param,
                           DEFAULT_VIEWWINDOW, DEFAULT_ASPECTRATIO);

                return rsEVENTPROCESSED;
            }

        case rsRWINITIALIZE:
            {
                return Initialize3D(param) ? rsEVENTPROCESSED :
                    rsEVENTERROR;
            }

        case rsRWTERMINATE:
            {
                Terminate3D();

                return rsEVENTPROCESSED;
            }

        case rsPLUGINATTACH:
            {
                return AttachPlugins()? rsEVENTPROCESSED : rsEVENTERROR;
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
