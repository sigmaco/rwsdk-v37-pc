
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
 * Purpose: To illustrate the usage of memory hints.
 *
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

#include "main.h"
#include "memheap.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

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

static RwUInt32 RwMatrixHeapId;
static RwUInt32 ResizableHeapId;
static RwUInt32 FunctionHeapId;
static RwUInt32 FrameHeapId;
static RwUInt32 EventHeapId;
static RwUInt32 GlobalHeapId;

RwMemoryFunctions OrgMemFuncs;

RpWorld *World = NULL;
RwCamera *Camera = NULL;
RtCharset *Charset = NULL;

/* Navigation */
RwReal CameraPitchRate      = 0.0f;
RwReal CameraTurnRate       = 0.0f;
RwReal CameraMaxSpeed       = 15.0f;
RwReal CameraSpeed          = 0.0f;
RwReal CameraStrafeSpeed    = 0.0f;

/*
 *****************************************************************************
 */
static RpWorld *
CreateWorld(void)
{
    RwStream    *stream;
    RwChar      *path;
    RpWorld     *world;

    /*
     * Attempt to load in the BSP file...
     */
    path = RsPathnameCreate(RWSTRING ("models/dungeon/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING ("models/dungeon.bsp"));
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
        RwCameraSetNearClipPlane(camera, 0.4f);
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics MemHints Example");

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
     * Create an empty world if not loading a BSP...
     */
    World = CreateWorld();
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

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

    if( World )
    {
        RpWorldDestroy(World);
    }

    if( Charset )
    {
        RwRasterDestroy(Charset);
    }

    RsRwTerminate();

    /* Delete the heaps and free all the memory. */
    MemHeapDestroy(RwMatrixHeapId);
    MemHeapDestroy(ResizableHeapId);
    MemHeapDestroy(FunctionHeapId);
    MemHeapDestroy(FrameHeapId);
    MemHeapDestroy(EventHeapId);
    MemHeapDestroy(GlobalHeapId);
            
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
 ******************************************************************************
 */
static RwBool
CameraUpdate(RwReal deltaTime)
{
    if ((CameraSpeed    != 0.0f) || (CameraPitchRate   != 0.0f) ||
        (CameraTurnRate != 0.0f) || (CameraStrafeSpeed != 0.0f) )
    {
        RwFrame  *frame = RwCameraGetFrame(Camera);
        RwMatrix *m = RwFrameGetMatrix(frame);
        RwV3d    *right = RwMatrixGetRight(m);
        RwV3d     pos, invPos, yAxis = {0, 1, 0};

        pos = *RwMatrixGetPos(m);

        /* Move camera to origin for rotation. */
        RwV3dNegate(&invPos, &pos);
        RwFrameTranslate(frame, &invPos, rwCOMBINEPOSTCONCAT);

        /* Rotate camera */
        RwFrameRotate(frame, right, CameraPitchRate * deltaTime,
                      rwCOMBINEPOSTCONCAT);
        RwFrameRotate(frame, &yAxis, CameraTurnRate * deltaTime,
                      rwCOMBINEPOSTCONCAT);
        /* Move to new position */
        RwV3dIncrementScaled(&pos, RwMatrixGetAt(m),
                             CameraSpeed * deltaTime);
        RwV3dIncrementScaled(&pos, RwMatrixGetRight(m),
                             CameraStrafeSpeed * deltaTime);

        RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
    }

    return TRUE;
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
            /*
             * Scene rendering here...
             */
            /* check if it's enough. */
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
     * Update camera
     */
    CameraUpdate(deltaTime);

    lastAnimTime = thisTime;

    Render();

    return;
}


/*
 *****************************************************************************
 */
static RwUInt32
GetHeap(RwUInt32 hint)
{
    RwUInt32    chunkID, duration, flags, heapId;
    
    chunkID     = RwMemoryHintGetChunkID(hint); 
    duration    = RwMemoryHintGetDuration(hint);
    flags       = RwMemoryHintGetFlags(hint);

    /* Set the default heap for all allocation without a hint. */
    heapId = EventHeapId;
    
    /* Choose appropriate heap from the duration info. */
    switch (duration)
    {
        case rwMEMHINTDUR_FUNCTION:
        {
            heapId = FunctionHeapId;
            break;
        }
        case rwMEMHINTDUR_FRAME:
        {
            heapId = FrameHeapId;
            break;
        }
        case rwMEMHINTDUR_EVENT:
        {
            heapId = EventHeapId;
            break;
        }
        case rwMEMHINTDUR_GLOBAL:
        {
            heapId = GlobalHeapId;
            break;
        }
        default:
        {
            break;
        }
    }

    /* Redirect resizable objects to a separate heap. */
    if (flags &  rwMEMHINTFLAG_RESIZABLE)
    {
        heapId = ResizableHeapId;
    }

    /* Redirect RwMatrix objects to a separate heap. */
    if (chunkID == rwID_MATRIX)
    {
        heapId = RwMatrixHeapId;
    }

    return heapId;
}


/*
 *****************************************************************************
 */
static void *
MemHintMalloc(size_t size, RwUInt32 hint)
{
    RwUInt32    heapId;
    void        *mem;

    heapId = GetHeap(hint);
    mem = MemHeapAlloc(heapId, size);

    return mem;
}


/*
 *****************************************************************************
 */
static void
MemHintFree(void *mem)
{
    MemHeapFree(mem);
}


/*
 *****************************************************************************
 */
static void *
MemHintRealloc(void *orgMem, size_t newSize, RwUInt32 hint)
{
    RwUInt32    heapId;
    void        *mem;

    heapId = GetHeap(hint);
    mem = MemHeapRealloc(heapId, orgMem, newSize);

    return mem;
}


/*
 *****************************************************************************
 */
static void *
MemHintCalloc(size_t numObj, size_t sizeObj, RwUInt32 hint)
{
    RwUInt32    heapId;
    void        *mem;

    heapId = GetHeap(hint);
    mem = MemHeapCalloc(heapId, numObj, sizeObj);

    return mem;
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

        case rsSETMEMORYFUNCS:
        {
            RwMemoryFunctions *memFuncs = (RwMemoryFunctions* )param;
            
            /* Store the original ps mem functions which will chain
             * inside the new ones and register the new functions.
             */
            OrgMemFuncs = *memFuncs;
            memFuncs->rwmalloc  = MemHintMalloc;
            memFuncs->rwfree    = MemHintFree;
            memFuncs->rwrealloc = MemHintRealloc;
            memFuncs->rwcalloc  = MemHintCalloc;

            /* Create heaps. Size of the Global heap will be platform.
             * specific and will vary from 5MB to 21MB depending on the
             * platform specific resource arena size
             */
            RwMatrixHeapId  = MemHeapCreate(65536);
#if defined(SOFTRAS_DRVMODEL_H)
            ResizableHeapId = MemHeapCreate(1048576);
#else /* defined(SOFTRAS_DRVMODEL_H) */
            ResizableHeapId = MemHeapCreate(131072);
#endif /* defined(SOFTRAS_DRVMODEL_H) */
            FunctionHeapId  = MemHeapCreate(8192);
            FrameHeapId     = MemHeapCreate(1024);
            EventHeapId     = MemHeapCreate(14680064);
            GlobalHeapId    = MemHeapCreate(rsRESOURCESDEFAULTARENASIZE +
                                            2097152);

            if (!RwMatrixHeapId || !ResizableHeapId || !FunctionHeapId ||
                !FrameHeapId || !EventHeapId || !GlobalHeapId)
            {
                RsErrorMessage(RWSTRING("Out of memory error."));
                return rsEVENTERROR;
            }
            
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
