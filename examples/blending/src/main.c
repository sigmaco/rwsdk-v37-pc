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
 * Purpose: To illustrate alpha blending between two IM2D 
 *          rendered geometries.
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

#include "blend.h"

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

static RwCamera *Camera = NULL;
static RtCharset *Charset = NULL;



/*
 *****************************************************************************
 */
static RwCamera *
CreateCamera(void)
{
    return CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Alpha Blending Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
SrcColorCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    UpdateSrcGeometryColor(&SrcColor);

    return TRUE;
}


static RwBool
DestColorCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    UpdateDestGeometryColor(&DestColor);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{
    static RwChar srcLabel[]  = RWSTRING("SRC blend");
    static RwChar destLabel[] = RWSTRING("DEST blend");

    static RwChar srcRedLabel[]   = RWSTRING("SRC red");
    static RwChar srcGreenLabel[] = RWSTRING("SRC green");
    static RwChar srcBlueLabel[]  = RWSTRING("SRC blue");
    static RwChar srcAlphaLabel[] = RWSTRING("SRC alpha");

    static RwChar destRedLabel[]   = RWSTRING("DEST red");
    static RwChar destGreenLabel[] = RWSTRING("DEST green");
    static RwChar destBlueLabel[]  = RWSTRING("DEST blue");
    static RwChar destAlphaLabel[] = RWSTRING("DEST alpha");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    static const RwChar *blendFunctionStrings[NUMBLENDFUNCTIONS] = 
    {
        RWSTRING("rwBLENDZERO"),
        RWSTRING("rwBLENDONE"),
        RWSTRING("rwBLENDSRCCOLOR"),
        RWSTRING("rwBLENDINVSRCCOLOR"),
        RWSTRING("rwBLENDSRCALPHA"),
        RWSTRING("rwBLENDINVSRCALPHA"),
        RWSTRING("rwBLENDDESTALPHA"),
        RWSTRING("rwBLENDINVDESTALPHA"),
        RWSTRING("rwBLENDDESTCOLOR"),
        RWSTRING("rwBLENDINVDESTCOLOR"),
        RWSTRING("rwBLENDSRCALPHASAT")
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        /*
         * Source and destination blend functions...
         */
        MenuAddEntryInt(srcLabel, &SrcBlendID, NULL, 
            0, NUMBLENDFUNCTIONS-1, 1, blendFunctionStrings);       

        MenuAddEntryInt(destLabel, &DestBlendID, NULL, 
            0, NUMBLENDFUNCTIONS-1, 1, blendFunctionStrings);

        MenuAddSeparator();

        /*
         * Source color...
         */
        MenuAddEntryReal(srcRedLabel, 
            &SrcColor.red, SrcColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddEntryReal(srcGreenLabel, 
            &SrcColor.green, SrcColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddEntryReal(srcBlueLabel, 
            &SrcColor.blue, SrcColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddEntryReal(srcAlphaLabel, 
            &SrcColor.alpha, SrcColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddSeparator();

        /*
         * Destination color...
         */
        MenuAddEntryReal(destRedLabel, 
            &DestColor.red, DestColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddEntryReal(destGreenLabel, 
            &DestColor.green, DestColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddEntryReal(destBlueLabel, 
            &DestColor.blue, DestColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddEntryReal(destAlphaLabel, 
            &DestColor.alpha, DestColorCallback, 0.0f, 1.0f, 0.01f);

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

    Camera = CreateCamera();
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    if( !Im2DInitialize(Camera) )
    {
        RsErrorMessage(RWSTRING("Error initializing immediate mode geometry."));

        return FALSE;
    }

    /*
     * Menu is initialized just before the first render, where it can
     * establish which blend modes are available by setting
     * renderstates. See function Render() below...
     */

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

    if( Camera )
    {
        CameraDestroy(Camera);
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
    if ( !RpWorldPluginAttach() )
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

    if( !BlendMode[SrcBlendID][DestBlendID] )
    {
        rwstrcpy(caption, RWSTRING("Blend function combination not available"));

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSBOTTOMRIGHT);
    }

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
            /*
             * Query available blending functions...
             */
            QueryBlendFunctionInfo();
    
            /*
             * ...and create a menu from the known blending functions...
             */
            InitializeMenu();
    
            firstCall = FALSE;
        }

        if( MenuGetStatus() != HELPMODE )
        {
            if( BlendMode[SrcBlendID][DestBlendID] )
            {
                Im2DRender();
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
            RwRect *rect;

            rect = (RwRect *)param;

            CameraSize(Camera, rect, DEFAULT_VIEWWINDOW, DEFAULT_ASPECTRATIO);
            
            Im2DSize(Camera, rect->w, rect->h);

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
