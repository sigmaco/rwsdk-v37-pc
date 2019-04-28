
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
 ****************************************************************************/

/****************************************************************************
 *
 * main.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how RenderWare clumps can be generated dynamically.
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#if (defined(SKY))
#include "rppds.h"
#endif /* (defined(SKY)) */
#include "rppatch.h"

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

#include "patch.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

static RwBool       FPSOn = FALSE;

static RwInt32      FrameCounter = 0;
static RwInt32      FramesPerSecond = 0;

static RwRGBA       ForegroundColor = { 200, 200, 200, 255 };
static RwRGBA       BackgroundColor = { 64, 64, 64, 0 };

static RpWorld     *World = NULL;
static RpLight     *MainLight = NULL;
static RwCamera    *Camera = NULL;
static RtCharset   *Charset = NULL;

static PatchState   patchState =
    { (RpClump *) NULL, (RpPatchMesh *) NULL
};

/*
 *****************************************************************************
 */
static RpWorld     *
CreateWorld(void)
{
    RpWorld            *world;
    RwBBox              bb;

    bb.inf.x = bb.inf.y = bb.inf.z = -100.0f;
    bb.sup.x = bb.sup.y = bb.sup.z = 100.0f;

    world = RpWorldCreate(&bb);

    return world;
}

/*
 *****************************************************************************
 */
static RpLight     *
CreateMainLight(RpWorld * world)
{
    RpLight            *light;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    if (light)
    {
        RwFrame            *frame;

        frame = RwFrameCreate();
        if (frame)
        {
            static RwV3d        Xaxis = { 1.0f, 0.0f, 0.0f };

            RwFrameRotate(frame, &Xaxis, 60.0f, rwCOMBINEREPLACE);

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
static RwCamera    *
CreateCamera(RpWorld * world)
{
    RwCamera           *camera;

    camera =
        CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight,
                     TRUE);
    if (camera)
    {
        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 30.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}

/*
 *****************************************************************************
 */
static              RwBool
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Patch Example");

        return TRUE;
    }

    return FALSE;
}

/*
 *****************************************************************************
 */
static              RwBool
InitializeMenu(void)
{
    static RwChar       fpsLabel[] = RWSTRING("FPS_F");

    if (MenuOpen(TRUE, &ForegroundColor, &BackgroundColor))
    {
        MenuAddEntryBool(fpsLabel, &FPSOn, NULL);

        return TRUE;
    }

    return FALSE;
}


#if (defined(AMBIENTLIGHT))
static RpLight     *AmbientLight = NULL;

/*
 *****************************************************************************
 */
static RpLight     *
CreateAmbientLight(RpWorld * world)
{
    RpLight            *light;

    light = RpLightCreate(rpLIGHTAMBIENT);
    if (light)
    {
        RpWorldAddLight(world, light);

        return light;
    }

    return NULL;
}

#endif /* (defined(AMBIENTLIGHT)) */

/*
 *****************************************************************************
 */
static              RwBool
Initialize3D(void *param)
{
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

    World = CreateWorld();
    if (World == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

#if (defined(AMBIENTLIGHT))
    AmbientLight = CreateAmbientLight(World);
    if (AmbientLight == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create ambient light."));

        return FALSE;
    }
#endif /* (defined(AMBIENTLIGHT)) */

    MainLight = CreateMainLight(World);
    if (MainLight == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create main light."));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if (Camera == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    /*
     * Create a patch of unit size centered about the origin...
     */
    CreatePatchMesh(&patchState);

    if (patchState.Clump == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create clump."));

        return FALSE;
    }
    else
    {
        /*
         * Push it in front of the camera so we can see it...
         */
        RwV3d               pos = { 0.0f, 0.0f, 4.0f };

        RwFrameTranslate(RpClumpGetFrame(patchState.Clump), &pos,
                         rwCOMBINEREPLACE);

        RpWorldAddClump(World, patchState.Clump);
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
    RWCRTCHECKMEMORY();

    /*
     * Create a patch of unit size centered about the origin...
     */

    if (patchState.Clump)
        RpWorldRemoveClump(World, patchState.Clump);

    DestroyPatchMesh(&patchState);

    if (Camera)
    {
        RpWorldRemoveCamera(World, Camera);
        

        CameraDestroy(Camera);
        
    }

    if (MainLight)
    {
        RwFrame            *frame;

        

        RpWorldRemoveLight(World, MainLight);
        

        frame = RpLightGetFrame(MainLight);
        
        RpLightSetFrame(MainLight, NULL);
        
        RwFrameDestroy(frame);
        

        RpLightDestroy(MainLight);
        
    }

#if (defined(AMBIENTLIGHT))
    if (AmbientLight)
    {
        RpWorldRemoveLight(World, AmbientLight);
        

        RpLightDestroy(AmbientLight);
        
    }
#endif /* (defined(AMBIENTLIGHT)) */

    if (World)
    {
        RpWorldDestroy(World);
        
    }

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
static              RwBool
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
     * Attach patch plug-in...
     */
    if (!RpPatchPluginAttach())
    {
        return FALSE;
    }
    /* For Sky builds register the patch PDS pipes */
#if (defined(SKY))
    RpPatchPipesAttach();
#endif /* (defined(SKY)) */

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
DisplayOnScreenInfo(RwCamera * camera __RWUNUSED__)
{
    RwChar              caption[256];

    if (FPSOn)
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
    RwCameraClear(Camera, &BackgroundColor,
                  rwCAMERACLEARZ | rwCAMERACLEARIMAGE);

    if (RwCameraBeginUpdate(Camera))
    {
        if (MenuGetStatus() != HELPMODE)
        {
            RpWorldRender(World);

            DisplayOnScreenInfo(Camera);
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
UpdateClump(RwReal timeStep)
{
    RwFrame            *frame;
    RwV3d              *up, *at;
    RwReal              angle;

    frame = RpClumpGetFrame(patchState.Clump);

    up = RwMatrixGetUp(RwFrameGetMatrix(frame));
    at = RwMatrixGetAt(RwFrameGetMatrix(frame));

    angle = timeStep * 30.0f;

    RwFrameRotate(frame, up, angle, rwCOMBINEPRECONCAT);
    RwFrameRotate(frame, at, angle, rwCOMBINEPRECONCAT);

    return;
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

    UpdateClump(deltaTime);

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
