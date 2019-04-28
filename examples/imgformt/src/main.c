
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
 * Purpose: Demonstrates how different image file formats can be registered 
 *          and used with RenderWare.
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

#include "tga.h"

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

static RpWorld *World = NULL;
static RpClump *Clump = NULL;
static RwCamera *Camera = NULL;
static RpLight *AmbientLight = NULL;
static RpLight *MainLight = NULL;
static RtCharset *Charset = NULL;

static RwChar TestedTGAReadWrite = FALSE;
static RwChar TestedTGAMessage[4][256];


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
        RwCameraSetNearClipPlane(camera,  0.1f);
        RwCameraSetFarClipPlane(camera, 300.0f);

        RpWorldAddCamera(world, camera);

        return camera;
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
        RpWorldAddLight(world, light);

        return light;
    }

    return NULL;
}

/*
 *****************************************************************************
 */
static RpLight *
CreateMainLight(RpWorld *world)
{
    RpLight *light;

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

            return light;
        }

        RpLightDestroy(light);
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static RpClump *
CreateClump(RpWorld *world)
{
    RpClump *clump;
    RwStream *stream;
    RwChar *path;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING("models/cube.dff"));
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
            RwFrame *clumpFrame;
            RwV3d pos = {0.0f, 0.0f, 30.0f};

            clumpFrame = RpClumpGetFrame(clump);

            RwFrameRotate(clumpFrame, &Xaxis, 90.0f, rwCOMBINEREPLACE);
            RwFrameRotate(clumpFrame, &Yaxis, 45.0f, rwCOMBINEPOSTCONCAT);

            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);
            
            RpWorldAddClump(world, clump);

            return clump;
        }
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Image Format Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
TestTGAImageReadWriteCallback(RwBool testEnable)
{
    RwImage *tgaImage = NULL;  
    RwChar *imagePath = NULL;

    if( testEnable )
    {
        return TRUE;
    }
    
    /* 
     * Set the image path...
     */
    imagePath = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(imagePath);
    RsPathnameDestroy(imagePath);

    /* 
     * Read in a 24-bit uncompressed TGA image...
     */
    tgaImage = RwImageRead(RWSTRING("24bit.tga"));

    if( tgaImage )
    {
        RsSprintf(TestedTGAMessage[0], 
            RWSTRING("(1) 24-bit uncompressed read successful."));

        /* 
         * Write out a 24-bit uncompressed TGA image...
         */
        imagePath = RsPathnameCreate(RWSTRING("models/textures/24bit2.tga"));
        if( RwImageWrite(tgaImage, imagePath) )
        {
            RsSprintf(TestedTGAMessage[1], 
                RWSTRING("(2) 24-bit uncompressed write successful."));
        }
        else
        {
            RsSprintf(TestedTGAMessage[1], 
                RWSTRING("(2) 24-bit uncompressed write failed."));
        }
        RsPathnameDestroy(imagePath);

        RwImageDestroy(tgaImage);
    }
    else
    {  
        RsSprintf(TestedTGAMessage[0], 
            RWSTRING("(1) 24-bit uncompressed read failed."));
    }

    /* 
     * Read in a 24-bit RLE TGA image...
     */
    tgaImage = RwImageRead(RWSTRING("24bitRLE.tga"));

    if( tgaImage )
    {
        RsSprintf(TestedTGAMessage[2], 
            RWSTRING("(3) 24-bit RLE read successful."));

        /* 
         * Write out a 24-bit uncompressed TGA image...
         */

        imagePath = RsPathnameCreate(RWSTRING("models/textures/24bit2.tga"));
        if( RwImageWrite(tgaImage, imagePath) )
        {
            RsSprintf(TestedTGAMessage[3], 
                RWSTRING("(4) 24-bit uncompressed write successful."));
        }
        else
        {
            RsSprintf(TestedTGAMessage[3], 
                RWSTRING("(4) 24-bit uncompressed write failed."));
        }
        RsPathnameDestroy(imagePath);

        RwImageDestroy(tgaImage);
    }
    else
    {  
        RsSprintf(TestedTGAMessage[2], 
            RWSTRING("(3) 24-bit RLE read failed."));
    }

    /* 
     * Test carried out...
     */
    TestedTGAReadWrite = TRUE;

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{
    static RwChar testLabel[] = RWSTRING("Test TGA read/write");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryTrigger(testLabel, TestTGAImageReadWriteCallback);
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

    /*
     * Register TGA image file format...
     */
    if( !RwImageRegisterImageFormat(RWSTRING("tga"), ImageReadTGA, ImageWriteTGA) )
    {
        RsErrorMessage(RWSTRING("Cannot register TGA image file format"));

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
        RsErrorMessage(RWSTRING("Cannot create world"));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if( !Camera)
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    AmbientLight = CreateAmbientLight(World);
    if( AmbientLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create light."));

        return FALSE;
    }
    
    MainLight = CreateMainLight(World);
    if( MainLight == NULL )
    {
        RsErrorMessage(RWSTRING ("Cannot create main light."));

        return FALSE;
    }

    Clump = CreateClump(World);
    if( Clump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create scene"));

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
#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    if( Clump )
    {
        RpWorldRemoveClump(World, Clump);

        RpClumpDestroy(Clump);
    }
    
    if( MainLight )
    {
        RwFrame *frame;

        RpWorldRemoveLight(World, MainLight);

        frame = RpLightGetFrame(MainLight);
        RpLightSetFrame(MainLight, NULL);
        RwFrameDestroy(frame);

        RpLightDestroy(MainLight);
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
    RwInt32 i;

    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }

    if( TestedTGAReadWrite )
    {
        /* 
         * Display messages from TGA tests...
         */
        for(i = 0;i < 4;i++)
        {
            RsCharsetPrint(Charset, TestedTGAMessage[i], 0, -(6 - i), rsPRINTPOSBOTTOMRIGHT);
        }
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
UpdateClump(RwReal deltaTime)
{
    RwFrame *frame;
    RwV3d *up, *at;
    RwReal angle;
    
    frame = RpClumpGetFrame(Clump);

    up = RwMatrixGetUp(RwFrameGetMatrix(frame));
    at = RwMatrixGetAt(RwFrameGetMatrix(frame));

    angle = 45.0f * deltaTime;

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
