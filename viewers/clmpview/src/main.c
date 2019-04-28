
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
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: A viewer capable of displaying clumps - including bones, skin, 
 *          and animation support.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> /* isprint */

#include "rwcore.h"
#include "rpworld.h"
#if (defined(SKY))
#include "rppds.h"
#endif /* (defined(SKY)) */

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rpcollis.h"
#include "rplodatm.h"
#include "rpmorph.h"
#include "rpskin.h"
#include "rphanim.h"
#include "rpdmorph.h"
#include "rprandom.h"
#include "rpmatfx.h"
#include "rppatch.h"

#include "rtcharse.h"
#include "rtfsyst.h"
#include "rtworld.h"
#include <rtbmp.h>
#include <rtras.h>
#include <rtpng.h>

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "main.h"
#include "scene.h"
#include "clmpview.h"
#include "clmpskin.h"
#include "clmphanm.h"
#include "clmppick.h"
#include "clmpcntl.h"

#if (defined (XBOX_DRVMODEL_H))
#include "rpanisot.h"
#include "rpnormmap.h"
#endif

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

typedef struct IdleState IdleState;
struct IdleState
{
    RwUInt32 Calls;
    RwUInt32 LastFrameTime;
    RwUInt32 LastAnimTime;
};

static RwBool FPSOn = TRUE;
static RwBool OnScreenInfoOn = TRUE;

static RwCullMode FaceCullMode;
static RwInt32 FaceCullIndex;
static RwBool NewFaceCullMode = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

RtCharset *Charset = (RwRaster *)NULL;

RwReal NormalsScaleFactor = 1.0f;
RwBool NormalsOn = FALSE;

RwBool SpinOn = FALSE;

RwInt32 RenderMode = RENDERSOLID;
RwInt32 NumTriStripAngles = 1;



/*
 *****************************************************************************
 */
static void
ParseColor(RwChar * buffer)
{
    RwInt32 r, g, b;
    RwChar dummy[8];

    rwsscanf(buffer, RWSTRING("%s %d %d %d"), dummy, &r, &g, &b);

    switch( buffer[2] )
    {
        case '1':
        {
            TopColor.red = (RwUInt8)r;
            TopColor.green = (RwUInt8)g;
            TopColor.blue = (RwUInt8)b;

            break;
        }

        case '2':
        {
            BottomColor.red = (RwUInt8)r;
            BottomColor.green = (RwUInt8)g;
            BottomColor.blue = (RwUInt8)b;

            break;
        }

        default:
        {
            BottomColor.red = TopColor.red = (RwUInt8)r;
            BottomColor.green = TopColor.green = (RwUInt8)g;
            BottomColor.blue = TopColor.blue = (RwUInt8)b;

            break;
        }
    }

    return;
}


/*
 *****************************************************************************
 */
static void
ProcessSetupFile(RwChar *filename)
{
    void *fp;
    RwChar *path;
    RwChar buffer[256];

    path = RsPathnameCreate(filename);
    fp = RwFopen(path, RWSTRING("r"));
    RsPathnameDestroy(path);

    if( fp )
    {
        while( RwFgets(buffer, 256, fp) )
        {
            /*
             * Ignore commented lines...
             */
            if( buffer[0] != '#' )
            {
                RwChar *src, *dst;

                src = dst = buffer;
                while( *src != '\0' )
                {
                    /*
                     * Is character displayable?
                     */
                    if( isprint(*src) )
                    {
                        *dst++ = *src;
                    }
                    src++;
                }

                /*
                 * File names are zero terminated....
                 */
                *dst = '\0';

                if( buffer[0] == '@' )
                {
                    /*
                     * it's a color...
                     */
                    ParseColor(buffer);
                }
                else
                {
                    /*
                     * it's a file...
                     */
                    HandleFileLoad(buffer);
                }
            }
        }

        RwFclose(fp);
    }

    return;
}


/*
 *****************************************************************************
 */
static RwBool
Initialize (void)
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Clump Viewer");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
RenderModeCB(RwBool justCheck)
{
    if( justCheck )
    {
        return TRUE;
    }

    if( ClumpHasHAnimAnimation || ClumpHasSkinAnimation )
    {
        switch( RenderMode )
        {
            case RENDERWIRE:
            {
                RenderMode = RENDERSKEL;

                break;
            }

            case RENDERWIRESKEL:
            case RENDERWIRESOLID:
            {
                RenderMode = RENDERSOLIDSKEL;

                break;

            }

            case RENDERALL:
            {
                RenderMode = RENDERSOLID;

                break;
            }
        }
    }

    if( ClumpStats.maxFramePerAtomic <= 1 )
    {
        switch( RenderMode )
        {
            case RENDERSKEL:
            case RENDERWIRESKEL:
            {
                RenderMode = RENDERWIRESOLID;

                break;
            }

            case RENDERSOLIDSKEL:
            {
                RenderMode = RENDERTRISTRIP;

                break;
            }

            case RENDERALL:
            {
                RenderMode = RENDERSOLID;

                break;
            }
        }
    }

    return TRUE;
}


static RwBool 
FaceCullCallback(RwBool justCheck)
{
    if( justCheck )
    {
        return FaceCullIndex != -1;
    }

    switch( FaceCullIndex )
    {
        case 0:
        {
            FaceCullMode = rwCULLMODECULLNONE;

            break;
        }

        case 1:
        {
            FaceCullMode = rwCULLMODECULLBACK;

            break;
        }

        case 2:
        {
            FaceCullMode = rwCULLMODECULLFRONT;

            break;
        }
    }

    NewFaceCullMode = TRUE;

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
AmbientLightOnCB(RwBool justCheck)
{
    if( justCheck )
    {
        return TRUE;
    }

    if( AmbientLightOn )
    {
        RpWorldAddLight(World, AmbientLight);
    }
    else
    {
        RpWorldRemoveLight(World, AmbientLight);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
MainLightOnCB(RwBool justCheck)
{
    if( justCheck )
    {
        return TRUE;
    }

    if( MainLightOn )
    {
        RpWorldAddLight(World, MainLight);
    }
    else
    {
        RpWorldRemoveLight(World, MainLight);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
NormalsOnCB(RwBool justCheck)
{
    if( justCheck )
    {
        return ClumpLoaded /* && (ClumpHasSkinAnimation || ClumpHasHAnimAnimation) */ ;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
ResetClumpCB(RwBool justCheck)
{
    if( justCheck )
    {
        return ClumpLoaded;
    }

    ClumpReset(Clump);

    ClumpControlReset();

    SpinOn = FALSE;

    return TRUE;
}


static RwBool
SpinOnCB(RwBool justCheck)
{
    if( justCheck )
    {
        return ClumpLoaded;
    }
    
    return TRUE;
}

static RwBool 
DumpClumpCB(RwBool justCheck)
{
    if( justCheck )
    {
        return ClumpLoaded;
    }

    SaveDFF();

    return TRUE;
}


static RwBool 
DumpTexDictCB(RwBool justCheck)
{
    if( justCheck )
    {
        return ClumpLoaded && ClumpHasTextures;
    }

    SaveTextureDictionary();

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
InitializeMenu(void)
{
    static RwChar ambientLightOnLabel[] = RWSTRING("Ambient light");
    static RwChar ambientLightLabel[]   = RWSTRING("Ambient intensity");
    static RwChar mainLightOnLabel[]    = RWSTRING("Main light ");
    static RwChar mainLightLabel[]      = RWSTRING("Main light intensity");

    static RwChar renderModeLabel[] = RWSTRING("Render mode_R");
    static const RwChar *renderModeEnumStrings[NUMRENDERMODES] =
    {
        RWSTRING("Solid"),
        RWSTRING("Wireframe"),
        RWSTRING("Skeleton"),
        RWSTRING("Wire & skeleton"),
        RWSTRING("Wire & solid"),
        RWSTRING("Solid & skeleton"),
        RWSTRING("TriStrip lengths"),
        RWSTRING("TriStrips"),
        RWSTRING("Meshes"),
        RWSTRING("All")
    };

    static RwChar normalsLabel[]            = RWSTRING("Normals_N");
    static RwChar normalsScaleFactorLabel[] = RWSTRING("Normals length");
    
    static RwChar faceCullLabel[] = RWSTRING("Face culling");
    static const RwChar *faceCullStrings[] = 
    {
        RWSTRING("none"),
        RWSTRING("back"),
        RWSTRING("front")
    };
    static RwChar triStripAnglesLabel[] = RWSTRING("TriStrip length");

    static RwChar fieldOfViewLabel[] = RWSTRING("Field of view");
    static RwChar farClipLabel[]     = RWSTRING("Far clip plane");
    static RwChar nearClipLabel[]    = RWSTRING("Near clip plane");

    static RwChar resetClumpLabel[]  = RWSTRING("Reset clump_C");
    static RwChar spinOnLabel[]      = RWSTRING("Spin clump_S");
    static RwChar dumpClumpLabel[]   = RWSTRING("Dump clump");
    static RwChar dumpTexDictLabel[] = RWSTRING("Dump tex dict");

    static RwChar onScreenInfoLabel[] = RWSTRING("On screen info_O");
    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        /*
         * Lights control...
         */
        MenuAddEntryBool(ambientLightOnLabel, &AmbientLightOn, AmbientLightOnCB);
        
        MenuAddEntryReal(ambientLightLabel, &AmbientIntensity, 
                         (MenuTriggerCallBack)NULL, 0.0f, 1.0f, 0.1f);

        MenuAddEntryBool(mainLightOnLabel, &MainLightOn, MainLightOnCB);
        
        MenuAddEntryReal(mainLightLabel, &MainIntensity, 
                         (MenuTriggerCallBack)NULL, 0.0f, 1.0f, 0.1f);

        MenuAddSeparator();

        /*
         * Render modes...
         */
        MenuAddEntryInt(renderModeLabel, &RenderMode, 
            RenderModeCB, 0, NUMRENDERMODES - 1, 1, renderModeEnumStrings);
        
        MenuAddEntryBool(normalsLabel, &NormalsOn, NormalsOnCB);
        
        MenuAddEntryReal(normalsScaleFactorLabel,
            &NormalsScaleFactor, NormalsOnCB, 0.1f, 10.0f, 0.1f);

        MenuAddEntryInt(faceCullLabel, &FaceCullIndex, 
            FaceCullCallback, 0, 2, 1, faceCullStrings);

        MenuAddEntryInt(triStripAnglesLabel, &NumTriStripAngles,
            NULL, 1, 100, 1, NULL);

        MenuAddSeparator();

        /*
         * Camera frustum control...
         */
        MenuAddEntryReal(fieldOfViewLabel, &FieldOfView, 
                         (MenuTriggerCallBack)NULL, 0.1f, 179.9f, 1.0f);
        
        MenuAddEntryReal(farClipLabel, &FarClip, 
                         (MenuTriggerCallBack)NULL, 
                         NearClip + MINNEARTOFARCLIP, FARMAXCLIP, 0.1f);

        MenuAddEntryReal(nearClipLabel, &NearClip, 
                         (MenuTriggerCallBack)NULL, 
                         NEARMINCLIP, FarClip - MINNEARTOFARCLIP, 1.0f);

        MenuAddSeparator();

        /*
         * Clump control...
         */
        MenuAddEntryTrigger(resetClumpLabel, ResetClumpCB);

        MenuAddEntryBool(spinOnLabel, &SpinOn, SpinOnCB);

        MenuAddEntryTrigger(dumpClumpLabel, DumpClumpCB);

        MenuAddEntryTrigger(dumpTexDictLabel, DumpTexDictCB);

        MenuAddSeparator();

        MenuAddEntryBool(onScreenInfoLabel, &OnScreenInfoOn, 
                         (MenuTriggerCallBack)NULL);
        MenuAddEntryBool(fpsLabel, &FPSOn, 
                         (MenuTriggerCallBack)NULL);

        MenuSetStatus(MENUOFF);

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

    if( !SceneInit() )
    {
        RsErrorMessage(RWSTRING("Error initializing the scene."));

        return FALSE;
    }

    if( !InitializeMenu() )
    {
        RsErrorMessage(RWSTRING("Error initializing menu."));

        return FALSE;
    }

    if( !ClumpViewInit() )
    {
        RsErrorMessage(RWSTRING("Error initializing clumpview."));

        return FALSE;
    }

    /*
     * Load Setup File
     */
    {
        RwChar iniFile[] = RWSTRING("./clmpview.ini");
        ProcessSetupFile(iniFile);
    }

#ifdef RWMETRICS
    RsMetricsOpen (Camera);
#endif

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
Terminate3D (void)
{

#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    SceneDestroy();

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
    if( !RpWorldPluginAttach() )
    {
        return FALSE;
    }

#ifdef RWLOGO
    if( !RpLogoPluginAttach() )
    {
        return FALSE;
    }
#endif

    if( !RpMorphPluginAttach() )
    {
        RsErrorMessage(RWSTRING("Morph plugin attach failed."));

        return FALSE;
    }

    if( !RpSkinPluginAttach() )
    {
        RsErrorMessage(RWSTRING("Skin plugin attach failed."));

        return FALSE;
    }
/* For Sky builds register the skinfx PDS pipes */
#if (defined(SKY))
    RpSkinfxPipesAttach();
#endif /* (defined(SKY)) */

    if( !RpDMorphPluginAttach() )
    {
        RsErrorMessage(RWSTRING("DMorph plugin attach failed."));

        return FALSE;
    }

    if( !RpLODAtomicPluginAttach() )
    {
        RsErrorMessage(RWSTRING("LOD Atomic plugin attach failed."));

        return FALSE;
    }

    if( !RpCollisionPluginAttach() )
    {
        RsErrorMessage(RWSTRING("Collision plugin attach failed."));

        return FALSE;
    }

    if( !RtAnimInitialize() )
    {
        RsErrorMessage(RWSTRING("animation toolkit attach failed."));

        return FALSE;
    }

    if( !RpHAnimPluginAttach() )
    {
        RsErrorMessage(RWSTRING("H animation plugin attach failed."));

        return FALSE;
    }

    if( !RpRandomPluginAttach() )
    {
        RsErrorMessage(RWSTRING("Random plugin attach failed."));

        return FALSE;
    }

    if( !RpMatFXPluginAttach() )
    {
        RsErrorMessage(RWSTRING("Material effects plugin attach failed."));

        return FALSE;
    }
/* For Sky builds register the matfx PDS pipes */
#if (defined(SKY))
    RpMatfxPipesAttach();
#endif /* (defined(SKY)) */

    if( !RpPatchPluginAttach() )
    {
        RsErrorMessage(RWSTRING("Patch plugin attach failed."));

        return FALSE;
    }
/* For Sky builds register the patchskinfx PDS pipes */
#if (defined(SKY))
    RpPatchSkinfxPipesAttach();
#endif /* (defined(SKY)) */

#if (defined (XBOX_DRVMODEL_H))
    if (!RpAnisotPluginAttach())
    {
        return FALSE;
    }

    if (!RpNormMapPluginAttach())
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
DisplayOnScreenInfo(RwCamera * camera)
{
    RwV3d pos;
    RwChar caption[128];

    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }

    if ( OnScreenInfoOn && (MenuGetStatus() == MENUOFF) )
    {
        if( ClumpLoaded )
        {    
            pos = *RwMatrixGetPos(RwFrameGetLTM(RpClumpGetFrame(Clump)));

            RsSprintf(caption,
                      RWSTRING("Clump Pos: [%6.1f, %6.1f, %6.1f]"),
                      pos.x, pos.y, pos.z);

            RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSBOTTOMRIGHT);

            ClumpDisplayOnScreenInfo(RwCameraGetRaster(camera));
        }
        else
        {
            /* Print the start-up screen... */
            RsCharsetPrint(Charset, RWSTRING("No clump loaded"),
                           0, 0, rsPRINTPOSTOPLEFT);
        }
    }

    return;
}


/*
 *****************************************************************************
 */
static void 
QueryDefaultFaceCullMode(void)
{
    RwCullMode cullMode;

    if( !RwRenderStateGet(rwRENDERSTATECULLMODE, (void *)&cullMode) )
    {
        FaceCullIndex = -1;
    }

    switch( cullMode )
    {
        case rwCULLMODECULLNONE:
        {
            FaceCullIndex = 0;

            break;
        }

        case rwCULLMODECULLBACK:
        {
            FaceCullIndex = 1;

            break;
        }

        case rwCULLMODECULLFRONT:
        {
            FaceCullIndex = 2;

            break;
        }

        default:
        {
            FaceCullIndex = -1;

            break;
        }
    }

    return;
}


/*
 *****************************************************************************
 */
static void
Render3D(void)
{
    static RwBool firstCall = TRUE;

    RwCameraClear(Camera, &BackgroundColor,
                  rwCAMERACLEARZ | rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( firstCall )
        {
            QueryDefaultFaceCullMode();

            firstCall = FALSE;
        }

        if( NewFaceCullMode )
        {
            RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)FaceCullMode);

            NewFaceCullMode = FALSE;
        }

        if( MenuGetStatus() != HELPMODE )
        {
            /*
             * Scene rendering here...
             */
            SceneRender();

            DisplayOnScreenInfo(Camera);
        }

        MenuRender(Camera, (RwRaster *)NULL);

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
    static IdleState state =    {
        (RwUInt32) 0 /*  Calls */,
        (RwUInt32) 0 /*  LastFrameTime */,
        (RwUInt32) 0 /*  LastAnimTime */
    };

    RwUInt32 thisTime;
    RwReal delta;

    if(!state.Calls++ )
    {
        /*
         * Initialize the timers variables that control clump animation and
         * estimating the number of rendered frames per second...
         */
        state.LastFrameTime = RsTimer();
        state.LastAnimTime = state.LastFrameTime;
    }

    thisTime = RsTimer();

    delta = (thisTime - state.LastAnimTime) * 0.001f;
    state.LastAnimTime = thisTime;

    /*
     * Has a second elapsed since we last updated the FPS...
     */
    if( thisTime > (state.LastFrameTime + 1000) )
    {
        /*
         * Capture the frame counter...
         */
        FramesPerSecond = FrameCounter;

        /*
         * ...and reset...
         */
        FrameCounter = 0;

        state.LastFrameTime = thisTime;
    }

    SceneCameraUpdate();

    SceneLightsUpdate();

    ClumpDeltaUpdate(delta);

    ClumpControlUpdate(delta);

    Render3D();

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

        case rsREGISTERIMAGELOADER:
            RwImageRegisterImageFormat("ras", RtRASImageRead, RtRASImageWrite);
            RwImageRegisterImageFormat("bmp", RtBMPImageRead, RtBMPImageWrite);
            RwImageRegisterImageFormat("png", RtPNGImageRead, RtPNGImageWrite);

            return (rsEVENTPROCESSED);




        case rsCAMERASIZE:
        {
            CameraSize(Camera, 
                (RwRect *)param, CurrentViewWindow, DEFAULT_ASPECTRATIO);

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

        case rsFILELOAD:
        {
            return HandleFileLoad(param) ? rsEVENTPROCESSED : rsEVENTERROR;
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
