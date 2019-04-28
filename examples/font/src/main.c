
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
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Colin Ho.
 * Reviewed by: .
 *
 * Purpose: Illustrate the basic usage of font reading and display.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#if defined( SKY2_DRVMODEL_H )
#include "rppds.h"
#endif /* !defined( SKY2_DRVMODEL_H ) */


#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "font.h"

#include "rt2d.h"

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
static RwRGBA BackgroundColor = { 100,  80,  100,   0};

static RtCharset *Charset = NULL;

RwCamera    *Camera = NULL;

RpWorld     *World = NULL;

RwInt32     WinWidth = DEFAULT_SCREEN_WIDTH;
RwInt32     WinHeight = DEFAULT_SCREEN_HEIGHT;
RwBBox      WinBBox;

static RwBool   filtered = FALSE;

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
        RwV3d Xaxis = {1.0f, 0.0f, 0.0f};

        RwV3d camPos = {0.0f, 20.0f, 0.0f};
        RwFrame *camFrame = NULL;

        camFrame = RwCameraGetFrame(camera);
        RwFrameSetIdentity(camFrame);

        RwFrameRotate(camFrame, &Xaxis,   90.0f, rwCOMBINEREPLACE);
        RwFrameTranslate(camFrame,&camPos, rwCOMBINEPOSTCONCAT);

        RwCameraSetNearClipPlane(camera, 1.0f);
        RwCameraSetFarClipPlane(camera, 100.0f);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Font Example");

        return TRUE;
    }

    return FALSE;
}

/*
 *****************************************************************************
 */
static RwTexture *
TextureSetFilterModeCB( RwTexture *tex, void * data)
{
    RwTextureFilterMode filter = (RwTextureFilterMode)data;


    RwTextureSetFilterMode( tex, filter );

    return tex;
}

/*
 *****************************************************************************
 */
static RwBool
textureFilterCB( RwBool testEnable )
{
    RwTexDictionary *texDict;


    if ( FALSE != testEnable )
    {
        return TRUE;
    }

    /* Change the filtermode in the font's texture. */
    texDict = Rt2dFontTexDictionaryGet();

    if ( FALSE == filtered )
    {
        RwTexDictionaryForAllTextures(texDict,
            TextureSetFilterModeCB, (void *) rwFILTERNEAREST);
    }
    else
    {
        RwTexDictionaryForAllTextures(texDict,
            TextureSetFilterModeCB, (void *) rwFILTERLINEAR);
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
InitializeMenu(void)
{
    static RwChar fontLabel[] = RWSTRING("Font_I");
    static RwChar filterLabel[] = RWSTRING("Filtered");
    static RwChar fpsLabel[]  = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryInt(fontLabel, &FontIndex,
            NULL, 0, NUMFONT - 1, 1, NULL);

        MenuAddEntryBool( filterLabel, &filtered, textureFilterCB );

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
    if( !InitializeMenu() )
    {
        RsErrorMessage(RWSTRING("Error initializing menu."));

        return FALSE;
    }

#ifdef RWMETRICS
    RsMetricsOpen(Camera);
#endif

    Initialize2D();

    /* initialize the default filtering settings */
    textureFilterCB(FALSE);

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
Terminate3D(void)
{

    Terminate2D();

#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

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

#if defined( SKY2_DRVMODEL_H )

    Rt2DPipesAttach();

#endif /* !defined( SKY2_DRVMODEL_H ) */


    return TRUE;
}


/*
 *****************************************************************************
 */
static void
DisplayOnScreenInfo(void)
{
    RwChar caption[256];
    RwInt32 lineNumber = 0;
    RwChar value[16];

    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }

    /* Char set count */
    if (Font[FontIndex])
    {
        rwsprintf(value, "%d", FontCharSetCount[FontIndex]);

        RsSprintf(caption, RWSTRING("Char Count .........."));
        caption[rwstrlen(caption) - rwstrlen(value)] = RWSTRING('\0');
        RsSprintf(caption, RWSTRING("%s %s"), caption, value);

        RsCharsetPrint(Charset, caption, 0, -lineNumber, rsPRINTPOSBOTTOMRIGHT);
        lineNumber++;

        /* Unicode or ASCII ? */
        if (Rt2dFontIsUnicode(Font[FontIndex]))
            rwsprintf(value, "Unicode");
        else
            rwsprintf(value, "ASCII");

        RsSprintf(caption, RWSTRING("Encoding ............"));
        caption[rwstrlen(caption) - rwstrlen(value)] = RWSTRING('\0');
        RsSprintf(caption, RWSTRING("%s %s"), caption, value);

        RsCharsetPrint(Charset, caption, 0, -lineNumber, rsPRINTPOSBOTTOMRIGHT);
        lineNumber++;

        /* Outline or bitmap ? */
        if (Rt2dFontIsOutline(Font[FontIndex]))
            rwsprintf(value, "Outline");
        else
            rwsprintf(value, "Bitmap");

        RsSprintf(caption, RWSTRING("Type ................"));
        caption[rwstrlen(caption) - rwstrlen(value)] = RWSTRING('\0');
        RsSprintf(caption, RWSTRING("%s %s"), caption, value);

        RsCharsetPrint(Charset, caption, 0, -lineNumber, rsPRINTPOSBOTTOMRIGHT);
        lineNumber++;

        /* Texture name. */
        RsSprintf(caption, RWSTRING("Name ................"));
        caption[rwstrlen(caption) - rwstrlen(FontName[FontIndex])] = RWSTRING('\0');
        RsSprintf(caption, RWSTRING("%s %s"), caption, FontName[FontIndex]);

        RsCharsetPrint(Charset, caption, 0, -lineNumber, rsPRINTPOSBOTTOMRIGHT);
        lineNumber++;
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
        Render2D(0.0f);

        if( MenuGetStatus() != HELPMODE )
        {
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

            WinWidth = RwRasterGetWidth(RwCameraGetRaster(Camera));
            WinHeight = RwRasterGetHeight(RwCameraGetRaster(Camera));

            Rt2dDeviceSetCamera(Camera);

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
