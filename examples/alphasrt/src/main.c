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
 * Purpose: An example to illustrate how to sort a collection of atomics 
 *          with alpha components, such that they are rendered in the 
 *          correct back-to-front order.
 *                         
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

#include "alpha.h"
#include "main.h"

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

static RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
static RwV3d Yaxis = {0.0f, 1.0f, 0.0f};

static RtCharset *Charset = NULL;
static RpWorld *World = NULL;
static RwCamera *Camera = NULL;
static RpClump *Clump = NULL;

static RwBool Spin = FALSE;
static RwReal RotateX = 0.0f;
static RwReal RotateY = 0.0f;

static RwBool AlphaSort = TRUE;



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

    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RwRGBAReal color = {0.1f, 0.1f, 0.1f, 1.0f};

        RpLightSetColor(light, &color);

        RpWorldAddLight(world, light);
    }
    else
    {
        return FALSE;
    }

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
        RwFrame *cameraFrame;
        RwV3d pos = {0.0f, 0.0f, -40.0f};

        cameraFrame = RwCameraGetFrame(camera);
        RwFrameTranslate(cameraFrame, &pos, rwCOMBINEREPLACE);

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
static RpClump *
CreateClump(RpWorld *world)
{
    RwChar *path;
    RwStream *stream = NULL;
    RpClump *clump = NULL;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING("models/clump.dff"));
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
        /*
         * Generate a list of all atomics in the clump that
         * have alpha components...
         */
        ClumpGetAlphaAtomics(clump);

        /*
         * Set the atomic render callback function according to
         * the initial sort parameter...
         */
        ClumpSetAtomicRenderCallback(clump, AlphaSort);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Alpha Sorting Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool 
AlphaSortMenuCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    ClumpSetAtomicRenderCallback(Clump, AlphaSort);

    return TRUE;
}


static RwBool
ResetRotationMenuCallback(RwBool testEnable)
{
    if( testEnable ) 
    {
        return TRUE;
    }

    if( Spin )
    {
        RotateX = RotateY = 0.0f;
    }

    return TRUE;
}


static RwBool 
InitializeMenu(void)
{    
    static RwChar alphaSortLabel[] = RWSTRING("Alpha sorting_A");

    static RwChar spinLabel[] = RWSTRING("Spin_S");
    static RwChar fpsLabel[]  = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {   
        MenuAddEntryBool(alphaSortLabel, &AlphaSort, AlphaSortMenuCallback);
        MenuAddSeparator();

        MenuAddEntryBool(spinLabel, &Spin, ResetRotationMenuCallback);
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

    if( !CreateLights(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create lights."));

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
static RpLight *
DestroyLight(RpLight *light, void *data)
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
    DestroyAlphaAtomicsList();

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

    RpWorldForAllLights(World, DestroyLight, (void *)World);

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

    /* Display the order the transparent atomics are rendered... */
    RsSprintf(caption, RWSTRING("Render order: -%s"), RenderOrderString);
    RsCharsetPrint(Charset, caption, 0, 1, rsPRINTPOSTOPRIGHT);

    /*
     * Reset for the next time round...
     */
    RenderOrderString[0] = '\0';

    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }

    /*
     * Display user actions...
     */
    if( Rotate && Translate )
    {
        RsSprintf(caption, RWSTRING("Clump rotate and camera translate"));
    }
    else if( Rotate )
    {
        RsSprintf(caption, RWSTRING("Clump rotate"));
    }
    else if( Translate )
    {
        RsSprintf(caption, RWSTRING("Camera translate"));
    }
    else
    {
        RsSprintf(caption, RWSTRING(""));
    }

    RsCharsetPrint(Charset, caption, 0, -1, rsPRINTPOSBOTTOMRIGHT);

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
            /*
             * Render the clump. 
             * If alpha-sorting is on, this call will render all non-alpha
             * atomics; otherwise, all atomics are rendered...
             */
            RpClumpRender(Clump);

            if( AlphaSort )
            {
                /* 
                 * Now render the deferred list of transparent atomics...
                 */
                RenderAlphaSortedAtomics();
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
void
CameraTranslateZ(RwReal zDelta)
{
    RwFrame *frame;
    RwV3d at;

    frame = RwCameraGetFrame(Camera);
    at = *RwMatrixGetAt(RwFrameGetMatrix(frame));

    RwV3dScale(&at, &at, zDelta);
    RwFrameTranslate(frame, &at, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
static RpClump *
RotateClump(RpClump *clump, RwReal deltaTime)
{
    RwFrame *clumpFrame;

    clumpFrame = RpClumpGetFrame(clump);

    /*
     * Do the rotation.
     * Use a time step to give a constant rate of movement 
     * independent of FPS...
     */  
    RwFrameRotate(clumpFrame, &Yaxis, 
        RotateY * deltaTime , rwCOMBINEPOSTCONCAT);

    RwFrameRotate(clumpFrame, &Xaxis, 
        RotateX * deltaTime, rwCOMBINEPOSTCONCAT);

    return clump;
}


/*
 *****************************************************************************
 */
void
SetRotation(RwReal deltaX, RwReal deltaY)
{
    RwReal maxRot = 360.0f;

    /* 
     * Accumulate rotation up to some maximum, for spinning...
     */
    RotateX = (RotateX > maxRot) ? maxRot : 
        (RotateX < -maxRot) ? -maxRot : RotateX - deltaY;
    
    RotateY = (RotateY > maxRot) ? maxRot : 
        (RotateY < -maxRot) ? -maxRot : RotateY + deltaX;

    if( !Spin )
    {
        RwReal timeStep = 0.5f;

        /* 
         * Rotate at a fixed rate...
         */
        RotateClump(Clump, timeStep);

        /* 
         * Reset rotation values...
         */
        RotateX = 0.0f;
        RotateY = 0.0f;
    }
    
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
   
    deltaTime = (thisTime - lastAnimTime) * 0.001f;

    /* 
     * Rotate Clumps...
     */
    if( Spin )
    {
        RotateClump(Clump, deltaTime);
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
