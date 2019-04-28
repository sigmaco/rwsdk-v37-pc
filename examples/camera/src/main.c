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
 * Purpose: RenderWare Graphics camera example.
 *          can be calculated.
 ****************************************************************************/


#include "rwcore.h"
#include "rpworld.h"

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

#include "main.h"
#include "viewer.h"
#include "camexamp.h"

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

static RwRGBA ForegroundColor =    {200, 200, 200, 255};
static RwRGBA BackgroundColor =    { 64,  64,  64,   0};
static RwRGBA BackgroundColorSub = { 74,  74,  74,   0};

static RtCharset *Charset = NULL;
static RpWorld *World = NULL;

RwV3d XAxis = {1.0f, 0.0f, 0.0f};
RwV3d YAxis = {0.0f, 1.0f, 0.0f};
RwV3d ZAxis = {0.0f, 0.0f, 1.0f};

RpClump *Clump = NULL;

RwBool Rendering = FALSE;



/*
 *****************************************************************************
 */
static RpWorld *
WorldCreate(void)
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
LightsCreate(RpWorld *world)
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
            RwFrameRotate(frame, &XAxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &YAxis, 30.0f, rwCOMBINEPOSTCONCAT);

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
static RpClump *
ClumpCreate(RpWorld *world)
{
    RwChar *path;
    RpClump *clump = NULL;
    RwStream *stream = NULL;
    
    path = RsPathnameCreate(RWSTRING("models/clump/"));
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
        RwV3d pos = {0.0f, 0.0f, 8.0f};

        RwFrameTranslate(RpClumpGetFrame(clump), &pos, rwCOMBINEREPLACE);

        RpWorldAddClump(world, clump);
    }

    return clump;
}


/*
 *****************************************************************************
 */
RpClump *
ClumpRotate(RpClump *clump, RwCamera *camera, RwReal xAngle, RwReal yAngle)
{
    RwFrame *clumpFrame = NULL;
    RwFrame *cameraFrame = NULL;
    RwMatrix *cameraMatrix = NULL;
    RwMatrix *clumpMatrix = NULL;
    
    RwV3d right, up, pos;

    /*
     * Rotate clump about it's origin...
     */           
    clumpFrame = RpClumpGetFrame(clump);
    cameraFrame = RwCameraGetFrame(camera); 
     
    clumpMatrix = RwFrameGetMatrix(clumpFrame);
    cameraMatrix = RwFrameGetMatrix(cameraFrame);

    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    pos = *RwMatrixGetPos(clumpMatrix);

    /*
     * Translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * Do the rotation...
     */
    RwFrameRotate(clumpFrame, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(clumpFrame, &right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * And translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

    return clump;
}


/*
 ***************************************************************************
 */
RpClump *
ClumpTranslate(RpClump *clump, RwCamera *camera, RwReal xDelta, RwReal zDelta)
{
    RwFrame *clumpFrame, *cameraFrame;
    RwV3d delta, deltaX, deltaZ;

    clumpFrame = RpClumpGetFrame(clump);
    cameraFrame = RwCameraGetFrame(camera);

    RwV3dScale(&deltaX, RwMatrixGetRight(RwFrameGetMatrix(cameraFrame)), xDelta);
    RwV3dScale(&deltaZ, RwMatrixGetAt(RwFrameGetMatrix(cameraFrame)), zDelta);

    RwV3dAdd(&delta, &deltaX, &deltaZ);

    RwFrameTranslate(clumpFrame, &delta, rwCOMBINEPOSTCONCAT);

    return clump;
}


/*
 *****************************************************************************
 */
RpClump *
ClumpSetPosition(RpClump *clump, RwV3d *position)
{
    RwFrameTranslate(RpClumpGetFrame(clump), position, rwCOMBINEREPLACE);
 
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Camera Example");

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
    static RwChar cameraLabel[]     = RWSTRING("Camera_C");
    static RwChar projectionLabel[] = RWSTRING("Projection_P");

    static RwChar nearClipPlaneLabel[] = RWSTRING("Near clip-plane");
    static RwChar farClipPlaneLabel[]  = RWSTRING("Far clip-plane");

    static RwChar resetLabel[]      = RWSTRING("Reset_R");
    static RwChar miniCameraLabel[] = RWSTRING("Mini-view_M");
    static RwChar fpsLabel[]        = RWSTRING("FPS_F");

    static const RwChar *cameraNames[] =
    {
        RWSTRING("Main camera"),
        RWSTRING("Sub camera")
    };

    static const RwChar *cameraProjectionNames[] =
    {
        RWSTRING("rwPERSPECTIVE"),
        RWSTRING("rwPARALLEL")
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryInt(cameraLabel, (RwInt32 *)&CameraSelected,
                        CameraSelectCallback, 0, 1, 1, cameraNames);

        MenuAddEntryInt(projectionLabel, (RwInt32 *)&ProjectionIndex,
                        ProjectionCallback, 0, 1, 1, cameraProjectionNames);

        MenuAddSeparator();

        MenuAddEntryReal(nearClipPlaneLabel, &SubCameraData.nearClipPlane,
                         NearClipPlaneCallback, VIEWERNEARCLIPPLANEMIN,
                         SubCameraData.farClipPlane-VIEWERNEARCLIPPLANESTEP,
                         VIEWERNEARCLIPPLANESTEP);

        MenuAddEntryReal(farClipPlaneLabel, &SubCameraData.farClipPlane,
                         FarClipPlaneCallback,
                         SubCameraData.nearClipPlane+VIEWERFARCLIPPLANESTEP,
                         VIEWERFARCLIPPLANEMAX, VIEWERFARCLIPPLANESTEP);

        MenuAddSeparator();

        MenuAddEntryTrigger(resetLabel, ResetCameraAndClumpCallback);
        MenuAddEntryBool(miniCameraLabel, &SubCameraMiniView, NULL);
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

    World = WorldCreate();
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

    if( !CamerasCreate(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create cameras."));

        return FALSE;
    }

    if( !LightsCreate(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create lights."));
        
        return FALSE;
    }

    Clump = ClumpCreate(World);
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
    RsMetricsOpen(GetMainCamera());
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

    RpWorldForAllLights(World, DestroyLight, World);

    CamerasDestroy(World);

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

    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);
        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSBOTTOMRIGHT);
    }

    RsSprintf(caption, RWSTRING("View window (%.2f, %.2f)"),
        SubCameraData.viewWindow.x, SubCameraData.viewWindow.y);
    RsCharsetPrint(Charset, caption, 0, -1, rsPRINTPOSBOTTOMRIGHT);

    RsSprintf(caption, RWSTRING("View offset (%.2f, %.2f)"),
        SubCameraData.offset.x, SubCameraData.offset.y);
    RsCharsetPrint(Charset, caption, 0, -2, rsPRINTPOSBOTTOMRIGHT);

    return;
}


/*
 *****************************************************************************
 */
static void
MainCameraRender(RwCamera *camera)
{
    /*
     * Lock the camera texture raster in memory while we 
     * render into it...
     */
    LockRaster(TRUE);

    /* Lets render the camera texture of the sub view */            
    RenderTextureCamera(&BackgroundColorSub, 
            rwCAMERACLEARZ | rwCAMERACLEARIMAGE, World); 

    /* Lets render the main view */
    RwCameraClear(camera, &BackgroundColor, 
            rwCAMERACLEARZ | rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            RpWorldRender(World);

            DrawCameraViewplaneTexture(&SubCameraData);

            DrawCameraFrustum(&SubCameraData);

            DisplayOnScreenInfo();
        }

        MenuRender(camera, NULL);

#ifdef RWMETRICS
        RsMetricsRender();
#endif
        RwCameraEndUpdate(camera);
    }

    LockRaster(FALSE);

    if( SubCameraMiniView && (MenuGetStatus() != HELPMODE) )
    {
        RenderSubCamera(&BackgroundColorSub,
            rwCAMERACLEARZ | rwCAMERACLEARIMAGE, World);
    }

    return;
}


/*
 *****************************************************************************
 */
static void
SubCameraRender(RwCamera *camera)
{
    /* Lets render the sub view */
    RwCameraClear(camera, &BackgroundColorSub, 
                rwCAMERACLEARZ | rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(camera) )
    {
        RpWorldRender(World);

        DisplayOnScreenInfo();

        MenuRender(camera, NULL);

#ifdef RWLOGO
        RpLogoSetState(camera, FALSE);
#endif

#ifdef RWMETRICS
        RsMetricsRender();
#endif
        RwCameraEndUpdate(camera);
    }

    return;
}


/*
 *****************************************************************************
 */
static void
Render(void)
{
    RwCamera *camera;

    /*
     * Set this to prevent the menu switching between main camera
     * and sub-camera views while rendering is taking place...
     */
    Rendering = TRUE;

    switch( CameraSelected )
    {
        case 0:
        {
            camera = GetMainCamera();

            MainCameraRender(camera);

            break;
        }


        case 1:
        {
            camera = GetSubCamera();

            SubCameraRender(camera);

            break;
        }

        default:
        {
            camera = GetMainCamera();

            break;
        }
    }

    /*
     * Display camera's raster...
     */
    RsCameraShowRaster(camera);

    FrameCounter++;

    /*
     * Rendering done, give control back to the menu...
     */
    Rendering = FALSE;

    return;
}


/*
 *****************************************************************************
 */
static void
Idle(void)
{
    RwUInt32 thisTime;

    static RwBool firstCall = TRUE;
    static RwUInt32 lastFrameTime;

    if( firstCall )
    {
        lastFrameTime = RsTimer();

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
            CameraSizeUpdate((RwRect *)param, 
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
