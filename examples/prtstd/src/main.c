
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
 * Original author: Alexandre Hadjdaj.
 * Reviewed by: .
 *
 * Purpose: Illustrate the picking and dragging of 3D immediate vertices.
 *
 ****************************************************************************/

#include "rwcore.h"

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

#include "prtstd.h"

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

static RwBool PrtStdAutoRotate = FALSE;
static RwV3d AutoRotate = {0.0f,0.0f,0.0f};

RwCamera *Camera = NULL;

RpWorld *World = NULL;


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

        RwV3d camPos = {0.0f, 30.0f, 0.0f};
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Particle Standard Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
emtSizeMenuCB(RwBool justCheck)
{
    if( justCheck )
    {
        return TRUE;
    }


    PrtEmitterIm3DCreate(PrtData.EmtStd);

    return TRUE;
}


static RwBool
emtEmitBiasCB(RwBool justCheck)
{
    if( justCheck )
    {
        return TRUE;
    }

    MenuSetRangeInt(&PrtData.EmtStd->emtPrtEmitBias,
                0,
                BATCHSIZE-PrtData.EmtStd->emtPrtEmit,
                1,
                NULL);

    return TRUE;
}


static RwBool
InitializeMenu(void)
{
    static RwChar fpsLabel[]      = RWSTRING("FPS_F");

    static RwChar emtSizeXLabel[] = RWSTRING("Emitter Size X");
    static RwChar emtSizeYLabel[] = RWSTRING("Emitter Size Y");
    static RwChar emtSizeZLabel[] = RWSTRING("Emitter Size Z");

    static RwChar emtPrtEmitLabel[] = RWSTRING("Emitted Particle Count");
    static RwChar emtPrtEmitBiasLabel[] = RWSTRING("Emitted Particle Count Bias");

    static RwChar emtEmitGapLabel[] = RWSTRING("Emittion Gap");
    static RwChar emtEmitGapBiasLabel[] = RWSTRING("Emittion Gap Bias");

    static RwChar prtLifeLabel[] = RWSTRING("Particles Life");
    static RwChar prtLifeBiasLabel[] = RWSTRING("Particles Life Bias");

    static RwChar prtInitVelLabel[] = RWSTRING("Initial Velocity");
    static RwChar prtInitVelBiasLabel[] = RWSTRING("Initial Velocity Bias");

    /* Set the particles emission Direction */
    static RwChar prtInitDirXLabel[] = RWSTRING("Initial Direction X");
    static RwChar prtInitDirYLabel[] = RWSTRING("Initial Direction Y");
    static RwChar prtInitDirZLabel[] = RWSTRING("Initial Direction Z");

    static RwChar prtInitDirBiasXLabel[] = RWSTRING("Initial Direction X Bias");
    static RwChar prtInitDirBiasYLabel[] = RWSTRING("Initial Direction Y Bias");
    static RwChar prtInitDirBiasZLabel[] = RWSTRING("Initial Direction Z Bias");

    static RwChar forceXLabel[] = RWSTRING("Force X");
    static RwChar forceYLabel[] = RWSTRING("Force Y");
    static RwChar forceZLabel[] = RWSTRING("Force Z");

    static RwChar prtStartColRLabel[] = RWSTRING("Start Color Red");
    static RwChar prtStartColGLabel[] = RWSTRING("Start Color Green");
    static RwChar prtStartColBLabel[] = RWSTRING("Start Color Blue");
    static RwChar prtStartColALabel[] = RWSTRING("Start Color Alpha");

    static RwChar prtEndColRLabel[] = RWSTRING("End Color Red");
    static RwChar prtEndColGLabel[] = RWSTRING("End Color Green");
    static RwChar prtEndColBLabel[] = RWSTRING("End Color Blue");
    static RwChar prtEndColALabel[] = RWSTRING("End Color Alpha");

    static RwChar prtStartSizeXLabel[] = RWSTRING("Start Size X");
    static RwChar prtStartSizeYLabel[] = RWSTRING("Start Size Y");
    static RwChar prtStartSizeBiasXLabel[] = RWSTRING("Start Size X Bias");
    static RwChar prtStartSizeBiasYLabel[] = RWSTRING("Start Size Y Bias");

    static RwChar prtEndSizeXLabel[] = RWSTRING("End Size X");
    static RwChar prtEndSizeYLabel[] = RWSTRING("End Size Y");
    static RwChar prtEndSizeBiasXLabel[] = RWSTRING("End Size X Bias");
    static RwChar prtEndSizeBiasYLabel[] = RWSTRING("End Size Y Bias");

    static RwChar renderEmitterLabel[] = RWSTRING("Render Emitter");
    static RwChar resetEmitterLabel[] = RWSTRING("Reset Emitter");

    static RwChar autoRotateBoolLabel[] = RWSTRING("AutoRotate Emitter");

    static RwChar autoRotateXLabel[] = RWSTRING("AutoRotate X");
    static RwChar autoRotateYLabel[] = RWSTRING("AutoRotate Y");
    static RwChar autoRotateZLabel[] = RWSTRING("AutoRotate Z");

    static RwChar vertexAlphaLabel[] = RWSTRING("Alpha Blending");

    static RwChar srcLabel[]  = RWSTRING("Source blend function");
    static RwChar destLabel[] = RWSTRING("Destination blend function");

    static const RwChar *blendFunctionStrings[11] =
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
       MenuAddEntryReal(emtSizeXLabel,
            &PrtData.EmtStd->emtSize.x,
            emtSizeMenuCB,
            0.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(emtSizeYLabel,
            &PrtData.EmtStd->emtSize.y,
            emtSizeMenuCB,
            0.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(emtSizeZLabel,
            &PrtData.EmtStd->emtSize.z,
            emtSizeMenuCB,
            0.0f,
            10.0f,
            0.1f);

        MenuAddSeparator();

        /* AJH: Need security callbakcs */
        MenuAddEntryInt(emtPrtEmitLabel,
                &PrtData.EmtStd->emtPrtEmit,
                emtEmitBiasCB,
                0,
                BATCHSIZE,
                1,
                NULL);

        MenuAddEntryInt(emtPrtEmitBiasLabel,
                    &PrtData.EmtStd->emtPrtEmitBias,
                    NULL,
                    0,
                    0,
                    1,
                    NULL);

        MenuAddEntryReal(emtEmitGapLabel,
            &PrtData.EmtStd->emtEmitGap,
            NULL,
            0.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(emtEmitGapBiasLabel,
            &PrtData.EmtStd->emtEmitGapBias,
            NULL,
            0.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(prtLifeLabel,
            &PrtData.EmtStd->prtLife,
            NULL,
            0.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(prtLifeBiasLabel,
            &PrtData.EmtStd->prtLifeBias,
            NULL,
            0.0f,
            10.0f,
            0.1f);

        MenuAddSeparator();

        MenuAddEntryReal(prtInitVelLabel,
            &PrtData.EmtStd->prtInitVel,
            NULL,
            -10.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(prtInitVelBiasLabel,
            &PrtData.EmtStd->prtInitVelBias,
            NULL,
            0.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(prtInitDirXLabel,
            &PrtData.EmtStd->prtInitDir.x,
            NULL,
            -10.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(prtInitDirYLabel,
            &PrtData.EmtStd->prtInitDir.y,
            NULL,
            -10.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(prtInitDirZLabel,
            &PrtData.EmtStd->prtInitDir.z,
            NULL,
            -10.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(prtInitDirBiasXLabel,
            &PrtData.EmtStd->prtInitDirBias.x,
            NULL,
            0.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(prtInitDirBiasYLabel,
            &PrtData.EmtStd->prtInitDirBias.y,
            NULL,
            0.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(prtInitDirBiasZLabel,
            &PrtData.EmtStd->prtInitDirBias.z,
            NULL,
            0.0f,
            10.0f,
            0.1f);

        MenuAddSeparator();

        MenuAddEntryReal(forceXLabel,
            &PrtData.EmtStd->force.x,
            NULL,
            -10.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(forceYLabel,
            &PrtData.EmtStd->force.y,
            NULL,
            -10.0f,
            10.0f,
            0.1f);

        MenuAddEntryReal(forceZLabel,
            &PrtData.EmtStd->force.z,
            NULL,
            -10.0f,
            10.0f,
            0.1f);

#ifdef COLORANIM
        MenuAddSeparator();

        MenuAddEntryReal(prtStartColRLabel,
            &PrtData.emtPrtCol->prtStartCol.red,
            NULL,
            0,
            255.0f,
            1.0f);

        MenuAddEntryReal(prtStartColGLabel,
            &PrtData.emtPrtCol->prtStartCol.green,
            NULL,
            0,
            255.0f,
            1.0f);

        MenuAddEntryReal(prtStartColBLabel,
            &PrtData.emtPrtCol->prtStartCol.blue,
            NULL,
            0,
            255.0f,
            1.0f);

        MenuAddEntryReal(prtStartColALabel,
            &PrtData.emtPrtCol->prtStartCol.alpha,
            NULL,
            0,
            255.0f,
            1.0f);

        MenuAddEntryReal(prtEndColRLabel,
            &PrtData.emtPrtCol->prtEndCol.red,
            NULL,
            0,
            255.0f,
            1.0f);

        MenuAddEntryReal(prtEndColGLabel,
            &PrtData.emtPrtCol->prtEndCol.green,
            NULL,
            0,
            255.0f,
            1.0f);

        MenuAddEntryReal(prtEndColBLabel,
            &PrtData.emtPrtCol->prtEndCol.blue,
            NULL,
            0,
            255.0f,
            1.0f);

        MenuAddEntryReal(prtEndColALabel,
            &PrtData.emtPrtCol->prtEndCol.alpha,
            NULL,
            0,
            255.0f,
            1.0f);
#endif

#ifdef SIZEANIM
        MenuAddSeparator();

         MenuAddEntryReal(prtStartSizeXLabel,
            &PrtData.emtPrtSize->prtStartSize.x,
            NULL,
            0,
            5.0f,
            0.1f);

         MenuAddEntryReal(prtStartSizeYLabel,
            &PrtData.emtPrtSize->prtStartSize.y,
            NULL,
            0,
            5.0f,
            0.1f);

         MenuAddEntryReal(prtStartSizeBiasXLabel,
            &PrtData.emtPrtSize->prtStartSizeBias.x,
            NULL,
            0,
            5.0f,
            0.1f);

         MenuAddEntryReal(prtStartSizeBiasYLabel,
            &PrtData.emtPrtSize->prtStartSizeBias.y,
            NULL,
            0,
            5.0f,
            0.1f);

         MenuAddEntryReal(prtEndSizeXLabel,
            &PrtData.emtPrtSize->prtEndSize.x,
            NULL,
            0,
            5.0f,
            0.1f);

         MenuAddEntryReal(prtEndSizeYLabel,
            &PrtData.emtPrtSize->prtEndSize.y,
            NULL,
            0,
            5.0f,
            0.1f);

         MenuAddEntryReal(prtEndSizeBiasXLabel,
            &PrtData.emtPrtSize->prtEndSizeBias.x,
            NULL,
            0,
            5.0f,
            0.1f);

         MenuAddEntryReal(prtEndSizeBiasYLabel,
            &PrtData.emtPrtSize->prtEndSizeBias.y,
            NULL,
            0,
            5.0f,
            0.1f);
#endif

        MenuAddSeparator();

        MenuAddEntryBool(renderEmitterLabel,
                 &renderEmitter,
                 NULL);

        MenuAddEntryTrigger(resetEmitterLabel,
                    PrtStdSetDefault);

        MenuAddSeparator();

        MenuAddEntryBool(autoRotateBoolLabel,
         &PrtStdAutoRotate,
         NULL);

        MenuAddEntryReal(autoRotateXLabel,
            &AutoRotate.x,
            NULL,
            -360.0f,
            360.0f,
            1.0f);

        MenuAddEntryReal(autoRotateYLabel,
            &AutoRotate.y,
            NULL,
            -360.0f,
            360.0f,
            1.0f);

        MenuAddEntryReal(autoRotateZLabel,
            &AutoRotate.z,
            NULL,
            -360.0f,
            360.0f,
            1.0f);

        MenuAddSeparator();

        MenuAddEntryBool(vertexAlphaLabel,
                            &VtxAlphaOn,
                            PrtStdSetPtankAlphaBlending);

        MenuAddEntryInt(srcLabel,
                        &SrcBlendID,
                        PrtStdSetPtankAlphaBlending,
                        0,
                        10,
                        1,
                        blendFunctionStrings);

        MenuAddEntryInt(destLabel,
                        &DestBlendID,
                        PrtStdSetPtankAlphaBlending,
                        0,
                        10,
                        1,
                        blendFunctionStrings);

        MenuAddSeparator();

        MenuAddEntryBool(fpsLabel,
                        &FPSOn,
                        PrtStdSetPtankAlphaBlending);

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

    if( !PrtStdInitialize() )
    {
        RsErrorMessage(RWSTRING("Error initializing Im3D."));

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

    PrtStdTerminate();

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
     * Attach PTank plug-in...
     */
    if( !RpPTankPluginAttach() )
    {
        return FALSE;
    }

    /*
     * Attach PrtStd plug-in...
     */
    if ( !RpPrtStdPluginAttach() )
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

    RsSprintf(caption, RWSTRING("Active Particles : %010d"), PrtData.emitter->prtActive);
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
            PrtStdRender();

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


    if( TRUE == PrtStdAutoRotate )
    {
        PrtStdEmitterWorldRotate(AutoRotate.x*deltaTime, AutoRotate.y*deltaTime, AutoRotate.z*deltaTime);
    }

#ifdef SKY
    /*
     * Ensure proper synchronisation...
     */
    _rwDMAWaitQueue();
#endif
    
    /*
     * Update the Particle System...
     */
    RpPrtStdAtomicUpdate(PrtData.atmEmitter, (void *)&deltaTime);

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
