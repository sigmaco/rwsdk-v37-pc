
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
 * Purpose :To illustrate the effects of different global and local lights on
 *          a landscape.
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

#include "main.h"
#include "lights.h"

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

RwCamera *Camera = NULL;
RtCharset *Charset = NULL;
RpWorld *World = NULL;



/*
 ****************************************************************************
 */
static RpWorld * 
CreateWorld(void)
{
    RwStream *stream;
    RwChar *path;
    RpWorld *world;

    path = RsPathnameCreate(RWSTRING ("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING ("models/world.bsp"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    world = NULL;

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL) )
        {
            world = RpWorldStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

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
        RwFrame *cameraFrame;
        RwV3d pos;

        cameraFrame = RwCameraGetFrame(camera);
        RwFrameRotate(cameraFrame, &Xaxis, 45.0f, rwCOMBINEREPLACE);
        RwFrameRotate(cameraFrame, &Yaxis, 45.0f, rwCOMBINEPOSTCONCAT);

        pos = *RwMatrixGetAt(RwFrameGetMatrix(cameraFrame));
        RwV3dScale(&pos, &pos, -900.0f);
        RwFrameTranslate(cameraFrame, &pos, rwCOMBINEPOSTCONCAT);

        pos = *RwMatrixGetUp(RwFrameGetMatrix(cameraFrame));
        RwV3dScale(&pos, &pos, -70.0f);
        RwFrameTranslate(cameraFrame, &pos, rwCOMBINEPOSTCONCAT);

        RwCameraSetFarClipPlane(camera, 1500.0f);
        RwCameraSetNearClipPlane(camera, 1.0f);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Lights2 Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool 
PointRadiusCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return PointLightOn;
    }

    return TRUE;
}


static RwBool 
InitializeMenu(void)
{    
    static RwChar directLightLabel[] = RWSTRING("Directional light_D");

    static RwChar pointLightLabel[] = RWSTRING("Point light_P");
    static RwChar radiusLabel[]     = RWSTRING("Light radius");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if (MenuOpen(TRUE, &ForegroundColor, &BackgroundColor))
    {
        MenuAddEntryBool(directLightLabel, &DirectLightOn, NULL);
        MenuAddSeparator();

        MenuAddEntryBool(pointLightLabel, &PointLightOn, NULL);
        MenuAddEntryReal(radiusLabel, &PointRadius, 
            PointRadiusCallback, 5.0f, 500.0f, 5.0f);
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

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));
    
        return FALSE;
    }

    if( !CreateLights(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create lights."));
    
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
static void 
Terminate3D(void)
{
    RwFrame *frame;

#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    if( PointLight )
    {
        if( PointLightOn )
        {
            RpWorldRemoveLight(World, PointLight);
        }

        frame = RpLightGetFrame(PointLight);
        RpLightSetFrame(PointLight, NULL);
        RwFrameDestroy(frame);

        RpLightDestroy(PointLight);
    }

    if( DirectionalLight )
    {
        if( DirectLightOn )
        {
            RpWorldRemoveLight(World, DirectionalLight);
        }

        frame = RpLightGetFrame(DirectionalLight);
        RpLightSetFrame(DirectionalLight, NULL);
        RwFrameDestroy(frame);

        RpLightDestroy(DirectionalLight);
    }

    if( AmbientLight )
    {
        RpWorldRemoveLight(World, AmbientLight);

        RpLightDestroy(AmbientLight);
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
DrawBox(const RwBBox *bBox, RwRGBA *color)
{
    RwIm3DVertex vertex[8];

    static RwImVertexIndex index[24] = {
        0, 1,  1, 2,  2, 3,  3, 0,  4, 5,  5, 6,
        6, 7,  7, 4,  0, 4,  3, 7,  1, 5,  2, 6
    };

    RwIm3DVertexSetRGBA(&vertex[0], 
        color->red, color->green, color->blue, color->alpha);
    RwIm3DVertexSetPos(&vertex[0], bBox->inf.x, bBox->inf.y, bBox->inf.z);

    RwIm3DVertexSetRGBA(&vertex[1], 
        color->red, color->green, color->blue, color->alpha);
    RwIm3DVertexSetPos(&vertex[1], bBox->sup.x, bBox->inf.y, bBox->inf.z);

    RwIm3DVertexSetRGBA(&vertex[2], 
        color->red, color->green, color->blue, color->alpha);
    RwIm3DVertexSetPos(&vertex[2], bBox->sup.x, bBox->sup.y, bBox->inf.z);

    RwIm3DVertexSetRGBA(&vertex[3], 
        color->red, color->green, color->blue, color->alpha);
    RwIm3DVertexSetPos(&vertex[3], bBox->inf.x, bBox->sup.y, bBox->inf.z);

    RwIm3DVertexSetRGBA(&vertex[4], 
        color->red, color->green, color->blue, color->alpha);
    RwIm3DVertexSetPos(&vertex[4], bBox->inf.x, bBox->inf.y, bBox->sup.z);

    RwIm3DVertexSetRGBA(&vertex[5], 
        color->red, color->green, color->blue, color->alpha);
    RwIm3DVertexSetPos(&vertex[5], bBox->sup.x, bBox->inf.y, bBox->sup.z);

    RwIm3DVertexSetRGBA(&vertex[6], 
        color->red, color->green, color->blue, color->alpha);
    RwIm3DVertexSetPos(&vertex[6], bBox->sup.x, bBox->sup.y, bBox->sup.z);

    RwIm3DVertexSetRGBA(&vertex[7], 
        color->red, color->green, color->blue, color->alpha);
    RwIm3DVertexSetPos(&vertex[7], bBox->inf.x, bBox->sup.y, bBox->sup.z);

    if( RwIm3DTransform(vertex, 8, NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, index, 24);

        RwIm3DEnd();
    }
}


static RpWorldSector *
DrawSectorBox(RpWorldSector *sector, 
              void *data __RWUNUSED__)
{
    RwRGBA color;

    color.red = 196, color.green = 196, color.blue = 0, color.alpha = 255;

    DrawBox(RpWorldSectorGetBBox(sector), &color);

    return sector;
}


static RpWorldSector *
DrawPointLightSectorBox(RpWorldSector *sector, 
                        void *data __RWUNUSED__)
{
    RwRGBA color;

    color.red = 196, color.green = 0, color.blue = 0, color.alpha = 255;

    DrawBox(RpWorldSectorGetBBox(sector), &color);

    return sector;
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

            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
            RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);
            
            if( PointLightOn )
            {
                /*
                 * Draw all the world sector boxes first in one color...
                 */
                RpWorldForAllWorldSectors(World, DrawSectorBox, NULL);

                /*
                 * ...and then the world sector boxes that the point light
                 * influences in another color...
                 */
                RpLightForAllWorldSectors(PointLight, DrawPointLightSectorBox,
                    NULL);

                DrawLightRadius();     
            }
            
            if( DirectLightOn )
            {
                DrawLightDirection();
            }

            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
            RwRenderStateSet(rwRENDERSTATESHADEMODE,
                (void *)rwSHADEMODEGOURAUD);

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

    UpdateLights();

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
