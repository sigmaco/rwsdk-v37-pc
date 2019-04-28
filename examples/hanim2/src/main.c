
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
 * Purpose: To illustrate how two H-anim sequences can be run together,
 *          with the second animation being a delta of the first.
 * *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#if (defined(SKY))
#include "rppds.h"
#endif /* (defined(SKY)) */

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rpskin.h"
#include "rphanim.h"
#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "hanim.h"

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
static RwCamera *Camera = NULL;
static RtCharset *Charset = NULL;
static RpLight *AmbientLight = NULL;
static RpLight *MainLight = NULL;

static RpClump *BaseClump = NULL;
static RpClump *DeltaClump = NULL;
static RpClump *OutClump = NULL;



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
    RpLight *light = NULL;

    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RwRGBAReal color = {0.2f, 0.2f, 0.2f, 1.0f};

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
    RpLight *light = NULL;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    if( light )
    {
        RwFrame *frame = RwFrameCreate();

        if( frame )
        {
            RwV3d xAxis = {1.0f, 0.0f, 0.0f};
            RwV3d yAxis = {0.0f, 1.0f, 0.0f};
            RwRGBAReal color = {0.8f, 0.8f, 0.8f, 1.0f};

            RpLightSetColor(light, &color);

            RwFrameRotate(frame, &xAxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &yAxis, 30.0f, rwCOMBINEPOSTCONCAT);

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
        RwFrame *frame;
        RwV3d pos = {0.0f, -15.0f, -100.0f};

        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 120.0f);

        frame = RwCameraGetFrame(camera);

        RwFrameTranslate(frame, &pos, rwCOMBINEREPLACE);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static RpClump *
CreateClump(RpWorld *world, const RwChar *file)
{
    RpClump *clump = NULL;
    RwStream *stream = NULL;
    RwChar *path = NULL;

    path = RsPathnameCreate(file);
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

    if( clump )
    {
        RwV3d yAxis = {0.0f, 1.0f, 0.0f};

        RwFrameRotate(RpClumpGetFrame(clump), &yAxis, 180.0f, rwCOMBINEREPLACE);

        RpWorldAddClump(world, clump);
    }

    return clump;
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics H-Anim Delta Example");

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
    static RwChar animationLabel[] = RWSTRING("Animation_A"); 

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    static const RwChar *animationStrings[3] = 
    {
        RWSTRING("base"),
        RWSTRING("delta"),
        RWSTRING("base & delta")
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryInt(animationLabel, &CurrentAnimation,
            ChangeAnimationCallBack, 0, 2, 1, animationStrings);

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
        RsErrorMessage(RWSTRING("Error cannot create ambient light."));

        return FALSE;
    }

    MainLight = CreateMainLight(World);
    if( MainLight == NULL )
    {
        RsErrorMessage(RWSTRING("Error cannot create main light."));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    BaseClump = CreateClump(World, RWSTRING("models/base.dff"));
    if( BaseClump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create base clump."));

        return FALSE;
    }

    DeltaClump = CreateClump(World, RWSTRING("models/delta.dff"));
    if( DeltaClump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create delta clump."));

        return FALSE;
    }

    /*
     * Because the app can switch rendering to the base animation,
     * delta animation, or the result of adding the base and delta
     * hierarchies, a separate set of skinned atomics is require, otherwise
     * the app could run the result animation on either the BaseClump or
     * DeltaClump skinned atomics.
     * It does not matter if the app uses the base.dff or delta.dff as
     * it is the structure of the hierarchy that is important and is the same
     * in both models...
     */
    OutClump = CreateClump(World, RWSTRING("models/base.dff"));
    if( OutClump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create out clump."));

        return FALSE;
    }

    CreateHierarchies(BaseClump, DeltaClump, OutClump);

    if( !CreateAnims() )
    {
        RsErrorMessage(RWSTRING("Cannot create animations."));

        return TRUE;
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
static RpClump *
DestroyClumps(RpClump *clump, void *data)
{
    RpWorldRemoveClump((RpWorld *)data, clump);

    RpClumpDestroy(clump);

    return clump;
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

    RpWorldForAllClumps(World, DestroyClumps, (void *)World);

    DestroyAnims();

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        CameraDestroy(Camera);
    }

    if( MainLight )
    {
        RwFrame *frame = NULL;

        RpWorldRemoveLight(World, MainLight);

        frame = RpLightGetFrame(MainLight);
        if( frame )
        {
            RpLightSetFrame(MainLight, NULL);
            
            RwFrameDestroy(frame);
        }

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

    /*
     * Attach skin plug-in...
     */
    if( !RpSkinPluginAttach() )
    {
        return FALSE;
    }
    /* For Sky builds register the skin PDS pipes */
#if (defined(SKY))
    RpSkinPipesAttach();
#endif /* (defined(SKY)) */

    /*
     * Attach animation toolkit...
     */
    if( !RtAnimInitialize() )
    {
        return FALSE;
    }

    /*
     * Attach hierarchical animation plug-in...
     */
    if( !RpHAnimPluginAttach() )
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
    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            switch( CurrentAnimation )
            {
                case BASE_ANIM:
                {
                    RpClumpRender(BaseClump);

                    break;
                }

                case DELTA_ANIM:
                {
                    RpClumpRender(DeltaClump);

                    break;
                }

                case BASE_AND_DELTA_ANIM:
                {
                    RpClumpRender(OutClump);

                    break;
                }
            }

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

    UpdateAnimation(deltaTime);

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
