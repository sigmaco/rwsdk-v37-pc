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
 * Copyright (c) 2002 Criterion Software Ltd.
 * All Rights Reserved.
 *
 ****************************************************************************/

/****************************************************************************
 *
 * main.c
 *
 * Copyright (C) 2002 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: This Example demonstrates the use of the rtanim toolkit. *
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rtanim.h"

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
#include "anim.h"

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

RpWorld *World = NULL;
RwCamera *Camera = NULL;
RtCharset *Charset = NULL;

#define NEARCLIP 100.0f
#define FARCLIP 3000.0f


RpLight *Light[3] = {NULL, NULL, NULL};
RwFrame *LightFrame[3] = {NULL, NULL, NULL};


/*
 *****************************************************************************
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

    path = RsPathnameCreate(RWSTRING ("models/streamtest.bsp"));
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
        RwCameraSetFarClipPlane(camera, FARCLIP);

        RwCameraSetNearClipPlane(camera, NEARCLIP);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Anim Example");

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

    /*
     * Load our anim world
     */
    World = CreateWorld();
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }


    /*
     * Here's the light 0
     */
    Light[0] = RpLightCreate(rpLIGHTDIRECTIONAL);
    LightFrame[0] = RwFrameCreate();
    {
        RwV3d   xAxis = {1,0,0},
                yAxis = {0,1,0};
		RwRGBAReal color = {0.0f, 0.0f, 0.0f, 1.0f};

		RpLightSetColor(Light[0],&color);
        RwFrameRotate(LightFrame[0], &xAxis, -150.0f, rwCOMBINEREPLACE);
        RwFrameRotate(LightFrame[0], &yAxis, 150.0f, rwCOMBINEPOSTCONCAT);
		
    }
    (void)RpLightSetFrame(Light[0], LightFrame[0]);
    RpWorldAddLight(World, Light[0]);

    /*
     * Here's the light 1
     */
    Light[1] = RpLightCreate(rpLIGHTDIRECTIONAL);
    LightFrame[1] = RwFrameCreate();
    {
        RwV3d   xAxis = {1,0,0},
                yAxis = {0,1,0};
		RwRGBAReal color = {0.0f, 0.0f, 0.0f, 1.0f};

		RpLightSetColor(Light[1],&color);
        RwFrameRotate(LightFrame[1], &xAxis, 150.0f, rwCOMBINEREPLACE);
        RwFrameRotate(LightFrame[1], &yAxis, 150.0f, rwCOMBINEPOSTCONCAT);

    }
    (void)RpLightSetFrame(Light[1], LightFrame[1]);
    RpWorldAddLight(World, Light[1]);

    /*
     * Here's the light 2
     */
    Light[2] = RpLightCreate(rpLIGHTPOINT);
    LightFrame[2] = RwFrameCreate();
    {
        RwV3d   xAxis = {1.0f,0.0f,0.0f},
                yAxis = {0.0f,1.0f,0.0f},
				yTranslate = {00.0f,-200.0f,00.0f};
		RwRGBAReal color = {1.0f, 1.0f, 1.0f, 1.0f};

		RpLightSetColor(Light[2],&color);
		
		RpLightSetRadius(Light[2],1000.0f);

        RwFrameRotate(LightFrame[2], &xAxis, 150.0f, rwCOMBINEREPLACE);
        RwFrameRotate(LightFrame[2], &yAxis, -150.0f, rwCOMBINEPOSTCONCAT);
		RwFrameTranslate(LightFrame[2], &yTranslate, rwCOMBINEPOSTCONCAT);

    }
    (void)RpLightSetFrame(Light[2], LightFrame[2]);
    RpWorldAddLight(World, Light[2]);
	
    /*
     * Create Animation data...
     */
    if( FALSE == AnimCreate() )
    {
        RsErrorMessage(RWSTRING("Cannot create anim."));

        return FALSE;
    }


    /*
     * Create a camera using the democom way...
     */
    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

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
	RwInt32 i;

    /*
     * Close or destroy RenderWare components in the reverse order they
     * were opened or created...
     */

#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        /*
         * This assumes the camera was created the democom way...
         */
        CameraDestroy(Camera);
    }

	for(i=0;i<3;i++)
	{
		RpWorldRemoveLight(World, Light[i]);
		(void)RpLightSetFrame(Light[i], NULL);
		RwFrameDestroy(LightFrame[i]);
		RpLightDestroy(Light[i]);
	}

    if( World )
    {
        RpWorldDestroy(World);
    }

    if( Charset )
    {
        RwRasterDestroy(Charset);
    }

	AnimDestroy();

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

    if ( !RtAnimInitialize() )
    {
        return FALSE;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static void 
DisplayOnScreenInfo(RwCamera * camera __RWUNUSED__)
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
    RwInt32 i;

	RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            /*
             * Scene rendering here...
             */

            RpWorldRender(World);

			for(i=0;i<3;i++)
			{
				DrawLight(Light[i]);
			}

            DisplayOnScreenInfo(Camera);
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
static RwReal deltaTime = 0.0f;

static void 
Idle(void)
{
    RwUInt32 thisTime;

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
     * Update any animations here...
     */
	RtAnimInterpolatorAddAnimTime(AnimInterpolator, deltaTime);
	LightAnimApply(Light,AnimInterpolator);


	

    lastAnimTime = thisTime;

    Render();

    return;
}


/*
 *****************************************************************************
 */
void
CameraTranslateZ(RwReal zDelta)
{
    RwFrame *frame;
    RwV3d at = { 0, 0, 1};

    frame = RwCameraGetFrame(Camera);
    at = *RwMatrixGetAt(RwFrameGetMatrix(frame));

    RwV3dScale(&at, &at, zDelta );
    RwFrameTranslate(frame, &at, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
void 
CameraLook(RwReal turn, RwReal tilt)
{
    RwFrame *cameraFrame;
    RwV3d delta, pos, *right;

    static RwReal totalTilt = 0.0f;
    static RwV3d Yaxis = {0.0f, 1.0f, 0.0f};
    
    /*
     * Limit the camera's tilt so that it never quite reaches
     * exactly +90 or -90 degrees...
     */
    if( totalTilt + tilt > 89.0f )
    {
        tilt = 89.0f - totalTilt;

    }
    else if( totalTilt + tilt < -89.0f )
    {
        tilt = -89.0f - totalTilt;
    }

    totalTilt += tilt;

    cameraFrame = RwCameraGetFrame(Camera);

    /*
     * Remember where the camera is...
     */
    pos = *RwMatrixGetPos(RwFrameGetMatrix(cameraFrame));

    /*
     * Remove the translation component...
     */
    RwV3dScale(&delta, &pos, -1.0f);
    RwFrameTranslate(cameraFrame, &delta, rwCOMBINEPOSTCONCAT);

    /*
     * Rotate to the new direction...
     */
    right = RwMatrixGetRight(RwFrameGetMatrix(cameraFrame));
    RwFrameRotate(cameraFrame, right, tilt, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(cameraFrame, &Yaxis, -turn, rwCOMBINEPOSTCONCAT);

    /*
     * Put the camera back to where it was...
     */
    RwFrameTranslate(cameraFrame, &pos, rwCOMBINEPOSTCONCAT);

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

