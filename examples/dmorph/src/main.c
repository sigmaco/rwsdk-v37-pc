
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
 * Purpose: To demonstrate the DMorph plugin
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rphanim.h"
#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "rpdmorph.h"
#include "dmorph.h"
#include "geom.h"

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
static RpLight *SecondLight = NULL;
static RpLight *ThirdLight = NULL;

RpClump *SurfaceBaseClump = NULL; 
RpClump *FaceBaseClump = NULL;
RpGeometry *DeltaGeom[2] = {NULL,NULL};/* the two geometry deltas to augment the surface geometry */
RpGeometry *SurfaceBaseGeom = NULL;

RwV3d zAxis = {0.0f, 0.0f, 1.0f};
RwV3d yAxis = {0.0f, 1.0f, 0.0f};
RwV3d xAxis = {1.0f, 0.0f, 0.0f};

RwReal duration[2] = {1.0f, 1.0f};     /* face geometry durations */
RwReal contribution[2]={0.0f, 0.0f};   /* surface geometry contributions */ 

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
        RwRGBAReal color = {0.3f, 0.3f, 0.3f, 1.0f};

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
            RwRGBAReal color = {0.0f, 1.0f, 0.0f, 1.0f};

            RpLightSetColor(light, &color);

            RwFrameRotate(frame, &yAxis, 0.0f, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, frame);

            RpWorldAddLight(world, light); 

            return light;
        }

        RpLightDestroy(light);
    }

    return NULL;
}

static RpLight *
CreateSecondLight(RpWorld *world)
{
    RpLight *light = NULL;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    if( light )
    {
        RwFrame *frame = RwFrameCreate();

        if( frame )
        {
            RwRGBAReal color = {1.0f, 0.0f, 0.0f, 1.0f};

            RpLightSetColor(light, &color);

            RwFrameRotate(frame, &yAxis, 15.0f, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, frame);

            RpWorldAddLight(world, light); 

            return light;
        }

        RpLightDestroy(light);
    }

    return NULL;
}

static RpLight *
CreateThirdLight(RpWorld *world)
{
    RpLight *light = NULL;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    if( light )
    {
        RwFrame *frame = RwFrameCreate();

        if( frame )
        {
            RwRGBAReal color = {0.0f, 0.0f, 1.0f, 1.0f};

            RpLightSetColor(light, &color);

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
        RwV3d pos = {0.0f, 10.0f, -100.0f};

        RwCameraSetNearClipPlane(camera, 5.0f);
        RwCameraSetFarClipPlane(camera, 5000.0f);

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
CreateFaceClump(RpWorld *world, const RwChar *file)
{
    RpClump *clump = NULL;
    RwStream *stream = NULL;
    RwChar *path = NULL;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

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
        /* Move the face into view... */
        RwV3d by = {-120.0f, 0.0f, 400.0f};
        
        RwFrameRotate(RpClumpGetFrame(clump), &xAxis, 90.0f, rwCOMBINEREPLACE);
        RwFrameRotate(RpClumpGetFrame(clump), &zAxis, 180.0f, rwCOMBINEPOSTCONCAT);
        RwFrameRotate(RpClumpGetFrame(clump), &yAxis, -15.0f, rwCOMBINEPOSTCONCAT);
        RwFrameTranslate(RpClumpGetFrame(clump), &by, rwCOMBINEPOSTCONCAT);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics DMorph Basic Example");

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
    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
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

    /* Create three lights.  Red green and blue - just for fun... */
    
    MainLight = CreateMainLight(World);
    if( MainLight == NULL )
    {
        RsErrorMessage(RWSTRING("Error cannot create main light."));

        return FALSE;
    }

    SecondLight = CreateSecondLight(World);
    if( SecondLight == NULL )
    {
        RsErrorMessage(RWSTRING("Error cannot create secondary light."));

        return FALSE;
    }

    ThirdLight = CreateThirdLight(World);
    if( ThirdLight == NULL )
    {
        RsErrorMessage(RWSTRING("Error cannot create third light."));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    /* Create the face geometry.... */
    FaceBaseClump = CreateFaceClump(World, RWSTRING("models/face.dff"));

    /* Create the surface geometry.... */
    SurfaceBaseGeom = CreateSurface(0);
    SurfaceBaseClump = PutInClump(SurfaceBaseGeom);
    RpWorldAddClump(World, SurfaceBaseClump);

    DeltaGeom[0] = CreateSurface(1);
    DeltaGeom[1] = CreateSurface(2);

    if( FaceBaseClump ==NULL || SurfaceBaseClump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create all clumps."));

        return FALSE;
    }
    else
    {
        /* Move the surface into place */
        RwV3d by = {20.0f, 0.0f, 0.0f};
        RwFrameTranslate(RpClumpGetFrame(SurfaceBaseClump), &by, rwCOMBINEREPLACE);
    }



    if( !CreateMorph() )
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

    DestroyDMorph();

    RpWorldForAllClumps(World, DestroyClumps, (void *)World);


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

    if( SecondLight )
    {
        RwFrame *frame = NULL;

        RpWorldRemoveLight(World, SecondLight);

        frame = RpLightGetFrame(SecondLight);
        if( frame )
        {
            RpLightSetFrame(SecondLight, NULL);
            
            RwFrameDestroy(frame);
        }

        RpLightDestroy(SecondLight);
    }

    if( ThirdLight )
    {
        RwFrame *frame = NULL;

        RpWorldRemoveLight(World, ThirdLight);

        frame = RpLightGetFrame(ThirdLight);
        if( frame )
        {
            RpLightSetFrame(ThirdLight, NULL);
            
            RwFrameDestroy(frame);
        }

        RpLightDestroy(ThirdLight);
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
     * Attach Delta Morphing plug-in...
     */
    if( !RpDMorphPluginAttach() )
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
    RtCharsetDesc charsetDesc;


    RtCharsetGetDesc(Charset, &charsetDesc);
   
    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOP);
    }


    /* Face info */
    RsSprintf(caption, RWSTRING("Durations %3.1f, %3.1f"), duration[0], duration[1]);
    RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);

    /* Surface info */
    RsSprintf(caption, RWSTRING("Contributions: %3.1f, %3.1f"), contribution[0], contribution[1]);
    RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPLEFT);

    return;
}


/*
 *****************************************************************************
 */
static void 
Render(void)
{
    RwCullMode FaceCullMode = rwCULLMODECULLNONE;
    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        
        if( MenuGetStatus() != HELPMODE )
        {
            FaceCullMode = rwCULLMODECULLNONE;
            RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)FaceCullMode);
            RpClumpRender(SurfaceBaseClump);

            FaceCullMode = rwCULLMODECULLBACK;
            RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)FaceCullMode);
            RpClumpRender(FaceBaseClump);

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

static void
RotateSurfaceClump(RwReal timeStep)
{
    RwFrame *frame;
    RwV3d *up, *at, *right;
    RwReal angle;

    frame = RpClumpGetFrame(SurfaceBaseClump);

    up = RwMatrixGetUp(RwFrameGetMatrix(frame));
    at = RwMatrixGetAt(RwFrameGetMatrix(frame));
    right = RwMatrixGetRight(RwFrameGetMatrix(frame));

    angle = timeStep * 12.0f;

    RwFrameRotate(frame, up, angle * 1.0f, rwCOMBINEPRECONCAT);

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

    AdvanceAnimation(deltaTime); /* Change facial expressions */
    RotateSurfaceClump(deltaTime); /* Rotate surface */

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
