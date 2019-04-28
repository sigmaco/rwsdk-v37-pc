
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
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 *
 * Purpose: Main 2d Viewer file.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include <ctype.h> /* isprint */


#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#include "rt2d.h"
#include "rt2danim.h"
#if (defined(SKY))
#include "rppds.h"
#endif /* (defined(SKY)) */

#include "view.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#ifdef RWMOUSE
#include "mouse.h"
#endif

#include "button.h"


#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (1.0f)

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

RpWorld *World = NULL;
RwCamera *Camera = NULL;
RtCharset *Charset = NULL;
RpLight *Light = NULL;

RwBool ResetDeltaTime = FALSE;

const RwChar *ExampleFileName = RWSTRING("models/Combination.anm");

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
        RpWorldAddCamera(world, camera);

        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 30.0f);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Maestro1 Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
ResetCB(RwBool justCheck)
{
    if(justCheck)
    {
        return TRUE;
    }

    Rt2dCTMSetIdentity();
    ViewChanged = TRUE;

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
InitializeMenu(void)
{
    static RwChar fpsLabel[] = RWSTRING("FPS_F");
    static RwChar viewResetLabel[] = RWSTRING("Reset View_R");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryBool(fpsLabel, &FPSOn, NULL);

        MenuAddEntryTrigger(viewResetLabel, ResetCB);

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

    /*
     * Create an empty world if not loading a BSP...
     */
    World = CreateWorld();
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }


    /*
     * Needed to make material colors act correctly
     */
    Light = RpLightCreate(rpLIGHTAMBIENT);

    if( Light == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create light."));

        return FALSE;
    }

    if( Light )
    {
        RwRGBAReal color = {0.0f, 0.0f, 0.0f, 1.0f};
        RpLightSetColor(Light, &color);
    }

    RpWorldAddLight(World, Light);

    /*
     * Create a camera using the democom way...
     */
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

    if( CreateViewer(Camera) == FALSE)
    {
        RsErrorMessage(RWSTRING("Error initializing viewer."));

        return FALSE;
    }

#ifdef RWMETRICS
    RsMetricsOpen(Camera);
#endif

#if ((defined(SKY2_DRVMODEL_H)) || (defined(GCN_DRVMODEL_H)) || (defined(XBOX_DRVMODEL_H)))

#ifdef RWMOUSE
    InitializeMouseCursor();
#endif /* RWMOUSE */

#endif /* ((defined(SKY2_DRVMODEL_H)) || (defined(GCN_DRVMODEL_H)) || (defined(XBOX_DRVMODEL_H)))*/


    /*
     * Load maestro animation
     */
    PreLoadObject(ExampleFileName);

    /*
     * Don't show the menu thanks
     */
    MenuSetStatus(MENUOFF);

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
Terminate3D(void)
{
    /*
     * Close or destroy RenderWare components in the reverse order they
     * were opened or created...
     */

#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        /*
         * This assumes the camera was created the democom way...
         */
        CameraDestroy(Camera);
    }

    if (Light)
    {
        RpWorldRemoveLight(World, Light);

        RpLightDestroy(Light);
    }

    if( World )
    {
        RpWorldDestroy(World);
    }

    if( Charset )
    {
        RwRasterDestroy(Charset);
    }

    DestroyViewer();

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

/* For Sky builds register the 2d PDS pipes */
#if (defined(SKY))
    Rt2DPipesAttach();
#endif /* (defined(SKY)) */

    return TRUE;
}


extern RwV2d CursorPos;

/*
 *****************************************************************************
 */
static void
DisplayOnScreenInfo(RwCamera *camera)
{
    RwChar caption[256];
    RtCharsetDesc charsetDesc;
    RwInt32 crw, crh;

    crw = RwRasterGetWidth(RwCameraGetRaster(camera));
    crh = RwRasterGetHeight(RwCameraGetRaster(camera));


    RtCharsetGetDesc(Charset, &charsetDesc);

    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption,0,0,rsPRINTPOSTOPRIGHT);
    }

    return;
}


/*
 *****************************************************************************
 */
static void
Render(RwReal deltaTime)
{
    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            /*
             * Scene rendering here...
             */

            RenderViewer(Camera, deltaTime);

            DisplayOnScreenInfo(Camera);
        }

        MenuRender(Camera, NULL);

#ifdef RWMETRICS
        RsMetricsRender();
#endif

        RwCameraEndUpdate(Camera);
    }

#if ((defined(SKY2_DRVMODEL_H)) || (defined(GCN_DRVMODEL_H)) || (defined(XBOX_DRVMODEL_H)))

#ifdef RWMOUSE
    rsMouseRender(Camera);
#endif /* RWMOUSE */

#endif /* ((defined(SKY2_DRVMODEL_H)) || (defined(GCN_DRVMODEL_H)) || (defined(XBOX_DRVMODEL_H)))*/

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

    if( firstCall || justLoaded)
    {
        lastFrameTime = lastAnimTime = RsTimer();

        firstCall = FALSE;
        justLoaded = FALSE;
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

    /*
     * Update any animations here...
     */

    lastAnimTime = thisTime;



    /*
     * Take user input into account
     */
    InputUpdateViewer();

    /* If an event reset the animation, this may have taken some time. */
    /* Prevent skipping (deltaTime) of the new animation               */

    /*
     * Update 2d viewer animation...
     */
    UpdateViewer(deltaTime);

    Render(deltaTime);

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
            if( Camera )
            {
                CameraSize(Camera, (RwRect *)param,
                    DEFAULT_VIEWWINDOW, DEFAULT_ASPECTRATIO);

                WinWidth = RwRasterGetWidth(RwCameraGetRaster(Camera));
                WinHeight = RwRasterGetHeight(RwCameraGetRaster(Camera));

                Rt2dDeviceSetCamera(Camera);
            }

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
