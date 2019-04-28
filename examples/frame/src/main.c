
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
 * Purpose: To illustrate the frame hierarchy of a series of atomics 
 * in a clump.
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
#include "frame.h"

typedef struct 
{
    /*
     * Clump containing the atomic to be cloned...
     */
    RpClump *clump;

    /* 
     * The number of atomic clones to create...
     */
    RwInt32 numClones;
}
CloneParameters;

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
static RpLight *AmbientLight = NULL;
static RpLight *MainLight = NULL;
static RwInt32 AtomicIndex = 0;
static RwBool AtomicsOn = TRUE;

RpClump *Clump = NULL;
RpWorld *World = NULL;
RwCamera *Camera = NULL;
RpAtomic *Atomics[ATOMICNUM];




/*
 *****************************************************************************
 */
static RpWorld *
CreateWorld(void)
{
    RpWorld *world;
    RwBBox bBox;

    bBox.sup.x = bBox.sup.y = bBox.sup.z = 100.0f;
    bBox.inf.x = bBox.inf.y = bBox.inf.z = -100.0f;

    world = RpWorldCreate(&bBox);

    return world;
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
            RwV3d xAxis = {1.0f, 0.0f, 0.0f};
            RwV3d yAxis = {0.0f, 1.0f, 0.0f};

            RwFrameRotate(frame, &xAxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &yAxis, 30.0f, rwCOMBINEPOSTCONCAT);

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
static RwCamera *
CreateCamera(RpWorld *world)
{
    RwCamera *camera;

    camera = CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);
    if( camera )
    {
        RwFrame *frame;
        RwV3d pos;

        frame = RwCameraGetFrame(camera);

        pos = *RwMatrixGetAt(RwFrameGetMatrix(frame));
        RwV3dScale(&pos, &pos, -100.0f);

        RwFrameTranslate(frame, &pos, rwCOMBINEREPLACE);

        RwCameraSetFarClipPlane(camera, 250.0f);
        RwCameraSetNearClipPlane(camera, 50.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static RpClump *
LoadClump(const RwChar *file)
{
    RpClump *clump = NULL;
    RwStream *stream;
    RwChar *path;

    path = RsPathnameCreate(file);
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

    return clump;
}


/*
 ****************************************************************************
 */
static RpAtomic *
SetRootAtomic(RpAtomic *atomic, 
              void *data __RWUNUSED__)
{
    Atomics[0] = atomic;

    AtomicIndex = 1;

    return NULL;
}


/*
 ****************************************************************************
 */
static RpAtomic *
CloneAtomics(RpAtomic *atomic, void *data)
{
    RpAtomic *clone;
    RwInt32 i;
    
    for(i=0; i<((CloneParameters *)data)->numClones; i++)
    {
        RwFrame *frame;

        clone = RpAtomicClone(atomic);
        if( clone == NULL )
        {
            RsErrorMessage(RWSTRING("Cannot clone atomic."));

            return NULL;
        }

        frame = RwFrameCreate();
        RpAtomicSetFrame(clone, frame);

        RpClumpAddAtomic(((CloneParameters *)data)->clump, clone);

        Atomics[AtomicIndex++] = clone;
    }

    return NULL;           
}


/*
 ****************************************************************************
 */
static RpClump *
CreateClump(RpWorld *world)
{
    RpClump *clump, *greenClump, *blueClump, *yellowClump;
    RwV3d pos;
    CloneParameters param;

    /*
     * Use this clump to assemble our atomics; use this clump's
     * atomic as the root...
     */
    clump = LoadClump(RWSTRING("models/redcube.dff"));
    if( clump == NULL )
    {
        return NULL;
    }

    RpClumpForAllAtomics(clump, SetRootAtomic, NULL);

    /*
     * Use the atomic in greenClump for the immediate children 
     * of the root... 
     */
    greenClump = LoadClump(RWSTRING("models/greencub.dff"));
    if( greenClump == NULL )
    {
        return NULL;
    }

    param.clump = clump;
    param.numClones = 3;
    RpClumpForAllAtomics(greenClump, CloneAtomics, &param);

    /*
     * Use the atomic in blueClump for the grandchildren 
     * of the root... 
     */
    blueClump = LoadClump(RWSTRING("models/bluecube.dff"));
    if( blueClump == NULL )
    {
        return NULL;
    }

    param.numClones = 5;
    RpClumpForAllAtomics(blueClump, CloneAtomics, &param);

    /*
     * Use the atomic in yellowClump for the great-grandchildren 
     * of the root... 
     */
    yellowClump = LoadClump(RWSTRING("models/yellcube.dff"));
    if( yellowClump == NULL )
    {
        return NULL;
    }

    param.numClones = 2;
    RpClumpForAllAtomics(yellowClump, CloneAtomics, &param);

    /*
     * Delete them there temporary clumps...
     */
    RpClumpDestroy(greenClump);
    RpClumpDestroy(blueClump);
    RpClumpDestroy(yellowClump);

    /*
     * Position and add clump to world...
     */
    pos.x = POS_X;
    pos.y = POS_Y;
    pos.z = POS_Z;
    RwFrameTranslate(RpClumpGetFrame(clump), &pos, rwCOMBINEREPLACE);

    RpWorldAddClump(world, clump);

    /*
     * Assemble the frame hierarchy...
     */
    LinkFrameHierarchy();

    return clump;
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Frame Example");

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
    static RwChar resetLabel[]  = RWSTRING("Reset_R");
    static RwChar atomicLabel[] = RWSTRING("Atomics_A");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryTrigger(resetLabel, ResetFrameHierarchyCallback);

        MenuAddEntryBool(atomicLabel, &AtomicsOn, NULL);

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

    AmbientLight = CreateAmbientLight(World);
    if( AmbientLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create ambient light."));

        return FALSE;
    }

    MainLight = CreateMainLight(World);
    if( MainLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create main light."));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    Clump = CreateClump(World);
    if( Clump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create clump."));

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
        RwInt32 i;

        RpWorldRemoveClump(World, Clump);

        for(i=1; i<ATOMICNUM; i++)
        {
            RpClumpRemoveAtomic(Clump, Atomics[i]);
        
            RpAtomicDestroy(Atomics[i]);
        }

        RpClumpDestroy(Clump);
    }

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        CameraDestroy(Camera);
    }

    if( AmbientLight )
    {
        RpWorldRemoveLight(World, AmbientLight);

        RpLightDestroy(AmbientLight);
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
    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            if( AtomicsOn )
            {
                RpWorldRender(World);
            }

            RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

            /*
             * Draw a wireframe representation of the frame hierarchy...
             */
            RenderFrameHierarchy();

            /*
             * Draw a wireframe box around the currently selected atomic...
             */
            HighlightAtomic();

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
