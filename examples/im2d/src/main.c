
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
 * Purpose: To demonstrate RenderWare's 2D immediate mode.
 * *****************************************************************************/

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

#include "im2d.h"

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
    RwCamera *camera;

    camera = CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);

    if( camera )
    {
        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 10.0f);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics 2D Immediate Mode Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool 
Im2DColoredCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    /*
     * The color for the geometries has been changed, 
     * set the geometries new color...
     */         
    LineListSetColor(!Im2DColored);
    IndexedLineListSetColor(!Im2DColored);
    
    PolyLineSetColor(!Im2DColored); 
    IndexedPolyLineSetColor(!Im2DColored);
    
    TriListSetColor(!Im2DColored);
    IndexedTriListSetColor(!Im2DColored);        
    
    TriStripSetColor(!Im2DColored);
    IndexedTriStripSetColor(!Im2DColored);
    
    TriFanSetColor(!Im2DColored);
    IndexedTriFanSetColor(!Im2DColored);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{    
    static RwChar im2DTypeLabel[] = RWSTRING("Primitive_P");        
    static RwChar texturedLabel[] = RWSTRING("Textured_T");    
    static RwChar coloredLabel[] = RWSTRING("Colored_C");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    static const RwChar *Im2DTypeStrings[] = 
    {
        RWSTRING("Line-list"),
        RWSTRING("Line-list 2"),    
        RWSTRING("Poly-line"),
        RWSTRING("Poly-line 2"),    
        RWSTRING("Tri-list"),
        RWSTRING("Tri-list 2"),    
        RWSTRING("Tri-strip"),
        RWSTRING("Tri-strip 2"),    
        RWSTRING("Tri-fan"),
        RWSTRING("Tri-fan 2")    
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryInt(im2DTypeLabel, &Im2DPrimType, NULL,
            0, 9, 1, Im2DTypeStrings);
        MenuAddEntryBool(texturedLabel, &Im2DTextured, NULL);
        MenuAddEntryBool(coloredLabel,  &Im2DColored, Im2DColoredCallback);
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

    Im2DTerminate();

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
    
    switch( Im2DPrimType )
    {
        case 0:
        case 1:
        {
            RsSprintf(caption, RWSTRING("rwPRIMTYPELINELIST"));
            break;
        }

        case 2:
        case 3:
        {
            RsSprintf(caption, RWSTRING("rwPRIMTYPEPOLYLINE"));
            break;
        }

        case 4:
        case 5:
        {
            RsSprintf(caption, RWSTRING("rwPRIMTYPETRILIST"));
            break;
        }

        case 6:
        case 7:
        {
            RsSprintf(caption, RWSTRING("rwPRIMTYPETRISTRIP"));
            break;
        }

        case 8:
        case 9:
        {
            RsSprintf(caption, RWSTRING("rwPRIMTYPETRIFAN"));
            break;
        }

        default:
        {
            RsSprintf(caption, RWSTRING("undefined"));
            break;
        }
    }

    if( Im2DPrimType % 2 )
    {
        rwstrcat(caption, RWSTRING(" : Indexed"));
    }            
    else
    {
        rwstrcat(caption, RWSTRING(" : Non-indexed"));
    }
    
    RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSBOTTOMRIGHT);
  
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
            Im2DRender();
            
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
