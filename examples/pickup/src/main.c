
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
 * Copyright (c) 2003 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * main.c
 *
 * Copyright (C) 2003 Criterion Technologies.
 *
 * Original author: RenderWare Team.
 *
 * Purpose: demonstrate a health pickup effects, using one ptank atomic per 
 * pickup.
 * 
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpmatfx.h"
#if (defined(SKY))
#include "rppds.h"
#endif /* (defined(SKY)) */

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

#include "pickup.h"

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

static RtCharset *Charset = NULL;

RwCamera *Camera = NULL;

RpWorld *World = NULL;
RpLight *LightDirectional = NULL;
RpLight *LightAmbient = NULL;
RpClump *Clump;
RwFrame *pickupObjectFrame;

const RwV3d YAxis  = {0.0f, 1.0f, 0.0f};

/*
 *****************************************************************************
 */
#define PICKUP_EFFECTS_NUM_PRT      (200)
#define PICKUP_EFFECTS_RADIUS       (30.0f)

RwRGBA      PickUpColor1 =  { 255,0,0,255};
RwRGBA      PickUpColor2 =  { 255,255,255,128};


PickUpObj *PickUp = NULL;

/*
 *****************************************************************************
 */
static RwFrame *
getFirstChildCB(RwFrame *frame, void *data)
{
    RwFrame **aframe = (void*)data;
    
    *aframe  = frame;
    
    RwFrameForAllChildren(frame,getFirstChildCB,data);

    /* 
     * Only looking for the first child so stop it here !
     */
    return NULL;
}


/*
 *****************************************************************************
 */
static RpClump *
ClumpLoad(void)
{
    RwChar *path;
    RpClump *clump = NULL;
    RwStream *stream = NULL;
    
    path = RsPathnameCreate(RWSTRING("models/pickup.dff"));
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
        RwFrameSetIdentity(RpClumpGetFrame(clump));

        getFirstChildCB(RpClumpGetFrame(clump),(void*)&pickupObjectFrame);
        
    }

    
    return clump;
}


/*
 *****************************************************************************
 */
static RpLight *
CreateDirectionalLight(RpWorld *world, RwCamera *camera)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);

    if( light )
    {
        RwFrame *frame;

        /* Attach the light to the camera */
        frame = RwCameraGetFrame(camera);

        if( frame )
        {
            RwRGBAReal color = {1.0f, 1.0f, 1.0f, 1.0f};

            RpLightSetColor(light, &color);

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
static RpLight *
CreateAmbientLight(RpWorld *world)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTAMBIENT);

    if( light )
    {
        RwRGBAReal color = {0.25f, 0.25f, 0.25f, 1.0f};

        RpLightSetColor(light, &color);

        RpWorldAddLight(world, light);

        return light;
    }

    return NULL;
}


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
        RwV3d camPos = {0.0f, 0.0f, -300.0f};
        RwFrame *camFrame = NULL;

        camFrame = RwCameraGetFrame(camera);
        RwFrameSetIdentity(camFrame);

        RwFrameTranslate(camFrame,&camPos, rwCOMBINEPOSTCONCAT);

        RwCameraSetNearClipPlane(camera, 1.0f);
        RwCameraSetFarClipPlane(camera, 500.0f);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Pickup Example");

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
    static RwChar fpsLabel[]      = RWSTRING("FPS_F");


    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryBool(fpsLabel,
                        &FPSOn,
                        NULL);

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
    RwChar *path;
    RwTexture *texture;

    if( !RsRwInitialize(param) )
    {
        RsErrorMessage(RWSTRING("Error initializing RenderWare."));

        return FALSE;
    }
    RwTextureSetAutoMipmapping(FALSE);
    RwTextureSetMipmapping(FALSE);

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

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    /*
     * Create a directional light...
     */
    LightDirectional = CreateDirectionalLight(World,Camera);
    if( LightDirectional == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create directional light."));

        return FALSE;
    }

    
    LightAmbient = CreateAmbientLight(World);
    if( LightAmbient == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create ambient light."));

        return FALSE;
    }
    
    
    /*
     * Load the particle texture...
     */
    path = RsPathnameCreate(RWSTRING("textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    texture = RwTextureRead(RWSTRING("particle"), RWSTRING("particle"));

    if( texture )
    {
        RwTextureSetFilterMode(texture, rwFILTERLINEAR  );
    }

    /*
     * Load the pickup dff...
     */
    Clump = ClumpLoad();

    if( NULL == Clump )
    {
        RsErrorMessage(RWSTRING("Error loading pickup clump."));

        return FALSE;
    }


    /*
     * Create the pickup effect atomic...
     */
    PickUp = PickUpCreate(pickupObjectFrame, 
                            texture, 
                            PICKUP_EFFECTS_RADIUS, 
                            PICKUP_EFFECTS_NUM_PRT,
                            &PickUpColor1,
                            &PickUpColor2);

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
static void
Terminate3D(void)
{
#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    if( PickUp )
    {
        PickUpDestroy(PickUp);
    }

    if( Clump )
    {
        RpClumpDestroy(Clump); 
    }

    if( LightDirectional )
    {
        RpWorldRemoveLight(World,LightDirectional);

        RpLightSetFrame(LightDirectional,NULL);
        
        RpLightDestroy(LightDirectional);
    }
    
    if( LightAmbient )
    {
        RpWorldRemoveLight(World,LightAmbient);

        RpLightDestroy(LightAmbient);
    }
    
    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        CameraDestroy(Camera);
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

    /*
     * Attach world plug-in...
     */
    if( !RpMatFXPluginAttach() )
    {
        return FALSE;
    }

    /* For Sky builds register the matfx PDS pipes */
#if (defined(SKY))
    RpMatfxPipesAttach();
#endif /* (defined(SKY)) */

    /*
     * Attach PTank plug-in...
     */
    if( !RpPTankPluginAttach() )
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
    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            RpClumpRender(Clump);

            PickUpRender(PickUp);

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


    /*
     * Update the Particle System...
     */
    PickUpUpdate(PickUp,deltaTime);

    /*
     * Rotate the red cross, rotation speed is 45 degrees/seconds
     */
    RwFrameRotate(pickupObjectFrame,
                    &YAxis,
                    45.0f*deltaTime,
                    rwCOMBINEPOSTCONCAT);

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

