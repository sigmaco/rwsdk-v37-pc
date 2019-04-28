
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
 * Purpose: RenderWare Graphics texture addressing example.
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

#include "texadrss.h"

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

static RwInt32 ModeIndex  = 0;
static RwInt32 ModeIndexU = 0;
static RwInt32 ModeIndexV = 0;

RpClump *Clump = NULL;
RwCamera *Camera = NULL;

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
    if (light)
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
    if (light)
    {
        RwFrame *frame;

        frame = RwFrameCreate();
        if (frame)
        {
            RwFrameRotate(frame, &Xaxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &Yaxis, 30.0f, rwCOMBINEPOSTCONCAT);

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
        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 50.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static void
UpdateTextureAddressMode(TextureAddressType addressMode)
{
    /*
     * We set the clump's textures to the new texture addressing mode.
     * Then we query the texture addressing modes of the both, u, v,
     * texture types, in case one of them has changed or even become 
     * invalid. This is then converted into menu index...
     */
    switch( addressMode )
    {
        case TextureAddressBoth:
        {
            if( !ModeIndex ) 
            {
                ModeIndex++;
            }

            ClumpSetTextureAddressMode(Clump, addressMode,
                AllModeInfo.both.mode[ModeIndex]);

            break;
        }

        case TextureAddressU:
        {
            if( !ModeIndexU ) 
            {
                ModeIndexU++;
            }

            ClumpSetTextureAddressMode(Clump, addressMode,
                AllModeInfo.u.mode[ModeIndexU]);

            break;
        }

        case TextureAddressV:
        {
            if( !ModeIndexV )
            {
                ModeIndexV++;
            }

            ClumpSetTextureAddressMode(Clump, addressMode,
                AllModeInfo.v.mode[ModeIndexV]);

            break;
        }

        default:
        {
            break;
        }
    }

    ModeIndex  = SetModeIndex(&AllModeInfo.both, 
        QueryTextureAddressMode(Clump, TextureAddressBoth));
    
    ModeIndexU = SetModeIndex(&AllModeInfo.u,
        QueryTextureAddressMode(Clump, TextureAddressU));
    
    ModeIndexV = SetModeIndex(&AllModeInfo.v,
        QueryTextureAddressMode(Clump, TextureAddressV));

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Texture Addressing Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool 
TextureAddressCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    UpdateTextureAddressMode(TextureAddressBoth);

    return TRUE;
}


static RwBool 
TextureAddressUCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    UpdateTextureAddressMode(TextureAddressU);

    return TRUE;
}


static RwBool 
TextureAddressVCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    UpdateTextureAddressMode(TextureAddressV);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{
    static RwChar texAdrssLabel[]  = RWSTRING("UV mode");
    static RwChar texAdrssLabelU[] = RWSTRING("U mode");
    static RwChar texAdrssLabelV[] = RWSTRING("V mode");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryInt(texAdrssLabel, &ModeIndex, TextureAddressCallback,
            0, AllModeInfo.both.number-1, 1, AllModeInfo.both.strings);

        MenuAddSeparator();
        
        MenuAddEntryInt(texAdrssLabelU, &ModeIndexU, TextureAddressUCallback,
            0, AllModeInfo.u.number-1, 1, AllModeInfo.u.strings);
        
        MenuAddEntryInt(texAdrssLabelV, &ModeIndexV, TextureAddressVCallback,
            0, AllModeInfo.v.number-1, 1, AllModeInfo.v.strings);

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

    if( !CreateLights(World))
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

    Clump = ClumpCreate(World);
    if( Clump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create clump"));

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
    static RwBool firstRender = TRUE;

    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( firstRender )
        {
            /*
             * Determine which addressing modes are available.
             * We do this by trying to set renderstates and seeing if
             * they succeed or fail. Then we build up a menu list
             * using the available modes. This process is only performed
             * once, just before the first rendering...
             */
            QueryTextureAddressAllInfo();

            UpdateTextureAddressMode(TextureAddressNone);

            InitializeMenu();

            firstRender = FALSE;
        }
         
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

