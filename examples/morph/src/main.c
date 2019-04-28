
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
 * Purpose: Example to demonstrate the morph plugin.
 *
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpmorph.h"

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

#include "morph.h"

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
static RtCharset *Charset = NULL;

static RwV3d Xaxis = {1.0f, 0.0f, 0.0f};


RpClump *Clump = NULL;
RwCamera *Camera = NULL;


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
static RwBool
CreateLights(RpWorld *world)
{
    RpLight *light = NULL;

    /* 
     * Add ambient light source...
     */
    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RpWorldAddLight(world, light);
    }
    else
    {  
        return FALSE;
    }
    
    /* 
     * Add directional light source...
     */
    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    if( light )
    {
        RwFrame *frame;

        frame = RwFrameCreate();
        if( frame )
        {
            RwFrameRotate(frame, &Xaxis, 45.0f, rwCOMBINEREPLACE);

            RpLightSetFrame(light, frame);

            RpWorldAddLight(world, light);
        }
        else
        {
            RpLightDestroy(light);

            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
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
        RwCameraSetNearClipPlane(camera,  0.1f);
        RwCameraSetFarClipPlane(camera, 25.0f);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Morph Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool 
MorphSpeedCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return MorphOn;    
    }

    return TRUE;
}


static RwBool 
InitializeMenu(void)
{     
    static RwChar morphOnLabel[]    = RWSTRING("Run morph_M");
    static RwChar morphSpeedLabel[] = RWSTRING("Morph speed");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {                
        MenuAddEntryBool(morphOnLabel, &MorphOn, NULL);
        MenuAddEntryReal(morphSpeedLabel, &MorphSpeed, 
            MorphSpeedCallback, 0.0f, 50.0f, 0.05f); 
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
    if(!World)
    {
        RsErrorMessage(RWSTRING("Cannot create world"));

        return FALSE;
    }
    
    if( !CreateLights(World))
    {
        RsErrorMessage(RWSTRING("Cannot create lights."));
        
        return FALSE;
    }

    Camera = CreateCamera(World);
    if( !Camera)
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    Clump = CreateClump(World);
    if( Clump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create clump"));

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
static RpLight *
LightDestroy(RpLight *light, void *data)
{
    RwFrame *frame;

    RpWorldRemoveLight((RpWorld *)data, light);

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

    RpWorldForAllLights(World, LightDestroy, World);

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
     * Attach morph plug-in...
     */
    if( !RpMorphPluginAttach() )
    {
        return FALSE;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicGetMorphInfo(RpAtomic *atomic, void *data)
{
    MorphParams *params;
    RpInterpolator *interp;

    params = (MorphParams *)data;
    interp = RpAtomicGetInterpolator(atomic);

    params->numTargets = RpGeometryGetNumMorphTargets(RpAtomicGetGeometry(atomic));
    params->numInterpolators = NumInterpolators;

    params->interpolator = RpMorphAtomicGetCurrentInterpolator(atomic);

    params->startTarget = RpInterpolatorGetStartMorphTarget(interp);
    params->endTarget = RpInterpolatorGetEndMorphTarget(interp);

    params->value = RpInterpolatorGetValue(interp);
    params->scale = RpInterpolatorGetScale(interp);

    return NULL;
}


static void 
DisplayOnScreenInfo(void)
{
    RwChar caption[256];
    MorphParams params;
    RwInt32 line;

    /* Display the FPS counter... */
    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }
   
    /*
     * Display morph info... 
     */
    RpClumpForAllAtomics(Clump, AtomicGetMorphInfo, &params);

    line = 2;
    RsSprintf(caption, RWSTRING("Interpolators ...... %d"), params.numInterpolators);
    RsCharsetPrint(Charset, caption, 0, line, rsPRINTPOSTOPRIGHT);

    line++;
    RsSprintf(caption, RWSTRING("Morph targets ...... %d"), params.numTargets);
    RsCharsetPrint(Charset, caption, 0, line, rsPRINTPOSTOPRIGHT);


    line = -4;
    RsSprintf(caption, RWSTRING("Interpolator ....... %d"), params.interpolator);
    RsCharsetPrint(Charset, caption, 0, line, rsPRINTPOSBOTTOMRIGHT);

    line++;
    RsSprintf(caption, RWSTRING("Start target ....... %d"), params.startTarget);
    RsCharsetPrint(Charset, caption, 0, line, rsPRINTPOSBOTTOMRIGHT);

    line++;
    RsSprintf(caption, RWSTRING("End target ......... %d"), params.endTarget);
    RsCharsetPrint(Charset, caption, 0, line, rsPRINTPOSBOTTOMRIGHT);

    line++;
    RsSprintf(caption, RWSTRING("Value ........... %1.2f"), params.value);
    RsCharsetPrint(Charset, caption, 0, line, rsPRINTPOSBOTTOMRIGHT);

    line++;
    RsSprintf(caption, RWSTRING("Scale ............ %1.1f"), params.scale);
    RsCharsetPrint(Charset, caption, 0, line, rsPRINTPOSBOTTOMRIGHT);

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

    if( MorphOn )
    {
        ClumpAdvanceMorph(Clump, deltaTime);
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
