
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
 * Purpose: To illustrate the different lights that are available for use in
 *          RenderWare.
 *
*****************************************************************************/

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

#include "lights.h"
#include "main.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.7f)

static RwRaster *Charset = NULL;
static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

RwCamera *Camera = NULL;
RpWorld *World = NULL;

RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
RwV3d Yaxis = {0.0f, 1.0f, 0.0f};
RwV3d Zaxis = {0.0f, 0.0f, 1.0f};


/*
 *****************************************************************************
 */
static RpWorld *
CreateWorld(void)
{
    RpWorld *world;
    RwBBox bb;

    bb.inf.x = bb.inf.y = bb.inf.z = -100.0f;
    bb.sup.x = bb.sup.y = bb.sup.z = 100.f;

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
        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 300.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static RwBool
CreateTestScene(RpWorld *world)
{
    RwStream *stream;
    RwChar *path;
    RpClump *clump;

    path = RsPathnameCreate(RWSTRING("models/checker.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    clump = NULL;

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            clump = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);

        if( clump )
        {
            RpClump *clone;
            RwFrame *clumpFrame;
            RwV3d pos;
            RwReal zOffset = 75.0f;

            /* 
             * Bottom Panel...
             */
            clumpFrame = RpClumpGetFrame(clump);

            RwFrameRotate(clumpFrame, &Xaxis, 90.0f, rwCOMBINEREPLACE);

            pos.x = 0.0f;
            pos.y = -25.0f;
            pos.z = zOffset;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

            RpWorldAddClump(world, clump);

            /* 
             * Top Panel...
             */
            clone = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clone);

            RwFrameRotate(clumpFrame, &Xaxis, -90.0f, rwCOMBINEREPLACE);

            pos.x = 0.0f;
            pos.y = 25.0f;
            pos.z = zOffset;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);
            
            /*
             * Left Panel...
             */
            clone = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clone);

            RwFrameRotate(clumpFrame, &Xaxis, 0.0f, rwCOMBINEREPLACE);
            RwFrameRotate(clumpFrame, &Yaxis, 90.0f, rwCOMBINEPOSTCONCAT);

            pos.x = 25.0f;
            pos.y = 0.0f;
            pos.z = zOffset;
            
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);
            
            /* 
             * Right Panel...
             */
            clone = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clone);

            RwFrameRotate(clumpFrame, &Xaxis, 0.0f, rwCOMBINEREPLACE);
            RwFrameRotate(clumpFrame, &Yaxis, -90.0f, rwCOMBINEPOSTCONCAT);

            pos.x = -25.0f;
            pos.y = 0.0f;
            pos.z = zOffset;

            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);
            
            /* 
             * Back Panel...
             */
            clone = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clone);

            RwFrameRotate(clumpFrame, &Xaxis, 0.0f,  rwCOMBINEREPLACE);

            pos.x = pos.y = 0.0f;
            pos.z = zOffset + 25.0f;

            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);
            
            /*
             * Create bounding box for room...
             */
            pos.x = 25.0f;
            pos.y = 25.0f;
            pos.z = zOffset - 25.0f;
            RwBBoxInitialize(&RoomBBox, &pos);

            pos.x = -25.0f;
            pos.y = -25.0f;
            pos.z = zOffset + 25.0f;
            RwBBoxAddPoint(&RoomBBox, &pos);

            return TRUE;
        }
    }

    return FALSE;
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Lights Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
LightDrawCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return LightOn && (CurrentLight != AmbientLight);
    }

    return TRUE;
}


static RwBool 
BaseAmbientLightCallback(RwBool testEnable)
{
    if( testEnable ) 
    {
        return TRUE;
    }

    if( BaseAmbientLightOn )
    {
        RpWorldAddLight(World, BaseAmbientLight);
    }
    else
    {
        RpWorldRemoveLight(World, BaseAmbientLight);
    }

    return TRUE;
}


static RwBool
LightTypeCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return LightOn;
    }

    return TRUE;
}


static RwBool
LightRadiusCallback(RwBool testEnable)
{
    if( testEnable )
    { 
        return LightOn && 
            (CurrentLight != AmbientLight) && (CurrentLight != DirectLight);
    }

    return TRUE;
}


static RwBool
LightConeAngleCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return LightOn &&
            ((CurrentLight == SpotLight) || (CurrentLight == SpotSoftLight));
    }

    return TRUE;
}


static RwBool
LightRedCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return LightOn;
    }

    return TRUE;
}


static RwBool
LightGreenCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return LightOn;
    }

    return TRUE;
}


static RwBool
LightBlueCallback(RwBool testEnable)
{
    if( testEnable ) 
    {
        return LightOn;
    }

    return TRUE;
}


static RwBool
InitializeMenu(void)
{
    static RwChar lightLabel[]            = RWSTRING("Light_L");
    static RwChar lightDrawLabel[]        = RWSTRING("Draw light_D");
    static RwChar lightTypeLabel[]        = RWSTRING("Light type_T");
    static RwChar lightBaseAmbientLabel[] = RWSTRING("Base ambient_A");

    static RwChar lightResetLabel[] = RWSTRING("Reset_R");

    static RwChar lightRadiusLabel[]    = RWSTRING("Radius");
    static RwChar lightConeAngleLabel[] = RWSTRING("Cone angle");

    static RwChar lightRedLabel[]    = RWSTRING("Light red");
    static RwChar lightGreenLabel[]  = RWSTRING("Light green");
    static RwChar lightBlueLabel[]   = RWSTRING("Light blue");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    static const RwChar *lightTypeStrings[] = 
    {
        RWSTRING("Ambient"),
        RWSTRING("Point"),
        RWSTRING("Directional"),
        RWSTRING("Spot"),
        RWSTRING("SpotSoft")
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryInt(lightTypeLabel, &LightTypeIndex,
            LightTypeCallback, 0, 4, 1, lightTypeStrings);
        MenuAddEntryBool(lightLabel, &LightOn, NULL);
        MenuAddEntryBool(lightDrawLabel, &LightDrawOn, LightDrawCallback);
        MenuAddEntryBool(lightBaseAmbientLabel, &BaseAmbientLightOn, 
            BaseAmbientLightCallback);
        MenuAddSeparator();

        MenuAddEntryTrigger(lightResetLabel, LightResetCallback);
        MenuAddSeparator();

        MenuAddEntryReal(lightRadiusLabel, &LightRadius,
            LightRadiusCallback, MIN_LIGHT_RADIUS,
            MAX_LIGHT_RADIUS, STEP_LIGHT_RADIUS);
        MenuAddEntryReal(lightConeAngleLabel, &LightConeAngle,
            LightConeAngleCallback, MIN_LIGHT_CONE_ANGLE * 180.0f / rwPI,
            MAX_LIGHT_CONE_ANGLE * 180.0f / rwPI, STEP_LIGHT_CONE_ANGLE);
        MenuAddSeparator();

        MenuAddEntryReal(lightRedLabel, &LightColor.red,
            LightRedCallback, 0.0f, 1.0f, 0.01f);
        MenuAddEntryReal(lightGreenLabel, &LightColor.green,
            LightGreenCallback, 0.0f, 1.0f, 0.01f);
        MenuAddEntryReal(lightBlueLabel, &LightColor.blue,
            LightBlueCallback, 0.0f, 1.0f, 0.01f);
        MenuAddSeparator();

        MenuAddEntryBool(fpsLabel, &FPSOn, NULL);

        return TRUE;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
Initialize3D(void *param)
{
    if( !RsRwInitialize(param) )
    {
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

    BaseAmbientLight = CreateBaseAmbientLight();
    if( BaseAmbientLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create base ambient light."));

        return FALSE;
    }
    
    AmbientLight = CreateAmbientLight();
    if( AmbientLight == NULL ) 
    {
        RsErrorMessage(RWSTRING("Cannot create ambient light."));

        return FALSE;
    }

    DirectLight = CreateDirectLight();
    if( DirectLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create directional light."));

        return FALSE;
    }

    PointLight = CreatePointLight();
    if( PointLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create point light."));

        return FALSE;
    }

    SpotLight = CreateSpotLight();
    if( SpotLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create spot light."));

        return FALSE;
    }

    SpotSoftLight = CreateSpotSoftLight();
    if( SpotSoftLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create soft spot light."));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create main camera."));

        return FALSE;
    }

    if( !CreateTestScene(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create test scene."));

        return FALSE;
    }

    if( !InitializeMenu() )
    {
        RsErrorMessage(RWSTRING("Cannot initialize menu."));

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
DestroyClump(RpClump *clump, 
             void *data __RWUNUSED__)
{
    RpWorldRemoveClump(World, clump);

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

    RpWorldForAllClumps(World, DestroyClump, NULL);

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        CameraDestroy(Camera);
    }

    LightsDestroy();

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
        RsErrorMessage(RWSTRING("RpWorldPluginAttach failed."));

        return FALSE;
    }

#ifdef RWLOGO
    /* 
     * Attach logo plug-in...
     */
    if( !RpLogoPluginAttach() )
    {
        RsErrorMessage(RWSTRING("RpLogoPluginAttach failed."));

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
    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ | rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            RpWorldRender(World);

            if( LightDrawOn && CurrentLight )
            {
                DrawCurrentLight();
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

    LightsUpdate();

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
            return Initialize()? rsEVENTPROCESSED : rsEVENTERROR;
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
