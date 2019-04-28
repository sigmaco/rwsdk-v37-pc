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
 * Purpose: To illustrate how to construct a RenderWare BSP world
 *          procedurally.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rtcharse.h"
#include "rtimport.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "gcond.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

static RwBool FPSOn = FALSE;
static RwBool WireFrame = TRUE;

static RwBool GeometryConditioning = FALSE;
static RwBool LastGeometryConditioning = FALSE;
static RwBool Wrapping = FALSE;
static RwBool LastWrapping = FALSE;
static RwInt32 UVLimit = 16;
static RwInt32 LastUVLimit = 16;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static RwV3d Xaxis = {1.0f, 0.0f, 0.0f};

static RwCamera *Camera = NULL;
static RtCharset *Charset = NULL;

static RpLight *MainLight = NULL;
static RpWorld *World = NULL;

static void ReloadWorld(void);
static void CameraSetPosition(RwCamera *camera, RpWorld *world);

static RwIm3DVertex *Im3DVertices = (RwIm3DVertex *)NULL;


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
            RwFrameRotate(frame, &Xaxis, 50.0f, rwCOMBINEREPLACE);

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
        CameraSetPosition(camera, World);

        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 1000.0f);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Geometry Conditioning Example");

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
    static RwChar conditionLabel[] = RWSTRING("Condition Geometry_G");
    static RwChar wireFrameLabel[] = RWSTRING("Draw WireFrame_W");
    static RwChar wrappingLabel[] = RWSTRING("UV Translation_T");
    static RwChar uvLabel[] = RWSTRING("UV Limits");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryBool(wireFrameLabel, &WireFrame, NULL);
        MenuAddSeparator();
        MenuAddEntryBool(conditionLabel, &GeometryConditioning, NULL);
        MenuAddEntryBool(wrappingLabel, &Wrapping, NULL);
        MenuAddEntryInt(uvLabel, &UVLimit, NULL, 1, 16, 1, NULL);
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

    World = CreateWorld(GeometryConditioning, Wrapping, UVLimit);
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

    MainLight = CreateMainLight(World);
    if( MainLight == NULL )
    {
        RsErrorMessage(RWSTRING ("Cannot create main light."));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING ("Cannot create camera."));

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

    if (Im3DVertices)
    {
        RwFree(Im3DVertices);
    }

    MenuClose();

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        CameraDestroy(Camera);
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

    /* 
     * Initialize RtGCond toolkit...
     */
    if( !RtGCondInitialize() )
    {
        return FALSE;
    }

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

static void
RenderWorldSectorWireMesh(RpWorldSector *worldSector)
{
    RwInt32 numTriangles, i;
    RwRGBA color = {255, 255, 255, 128};
    RwInt32 vertsPerPoly = 6;
    const RpTriangle *worldTriangles;
    const RwV3d *worldVertices;
    RwIm3DVertex *vertex;

    worldVertices = RpWorldSectorGetVertices(worldSector);
    worldTriangles = RpWorldSectorGetTriangles(worldSector);

    numTriangles = RpWorldSectorGetNumTriangles(worldSector);
    if( numTriangles == 0 )
    {
        return;
    }

    vertex = Im3DVertices;
    for(i = 0; i < numTriangles; i++)
    {
        RwV3d vert[3];

        vert[0] = worldVertices[worldTriangles->vertIndex[0]];
        vert[1] = worldVertices[worldTriangles->vertIndex[1]];
        vert[2] = worldVertices[worldTriangles->vertIndex[2]];

        RwIm3DVertexSetPos(vertex, vert[0].x, vert[0].y, vert[0].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;
        RwIm3DVertexSetPos(vertex, vert[1].x, vert[1].y, vert[1].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;

        RwIm3DVertexSetPos(vertex, vert[1].x, vert[1].y, vert[1].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;
        RwIm3DVertexSetPos(vertex, vert[2].x, vert[2].y, vert[2].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;

        RwIm3DVertexSetPos(vertex, vert[2].x, vert[2].y, vert[2].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;
        RwIm3DVertexSetPos(vertex, vert[0].x, vert[0].y, vert[0].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;

        worldTriangles++;

    }
    if( RwIm3DTransform(Im3DVertices, numTriangles * vertsPerPoly,
            (RwMatrix *)NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

        RwIm3DEnd();
    }
    return;
}

static RpWorldSector *
WFWorldSectorRenderCallback(RpWorldSector *worldSector)
{
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    RenderWorldSectorWireMesh(worldSector);

    return worldSector;
}
/*
 *****************************************************************************
 */
static void 
Render(void)
{
    RpWorldSectorCallBackRender OrigWorldSectorRenderCallback;

    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( World && RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            if (WireFrame)
            {
                if (!Im3DVertices)
                {
                    Im3DVertices = (RwIm3DVertex *)RwMalloc((NumEdges - 1) *
                        (NumEdges - 1) * 3 * 2 * 2 *
                        sizeof(RwIm3DVertex), rwID_NAOBJECT);
                }
            }
            RpWorldRender(World);

            if (WireFrame)
            {
                OrigWorldSectorRenderCallback = RpWorldGetSectorRenderCallBack(World);
                RpWorldSetSectorRenderCallBack(World, WFWorldSectorRenderCallback);                         
                RpWorldRender(World);
                RpWorldSetSectorRenderCallBack(World, OrigWorldSectorRenderCallback);
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

    if ((GeometryConditioning != LastGeometryConditioning) ||
        (Wrapping != LastWrapping) ||
        (UVLimit != LastUVLimit))
    {
        if (World)
        {
            ReloadWorld();
        }
        LastGeometryConditioning = GeometryConditioning;
        LastWrapping = Wrapping;
        LastUVLimit = UVLimit;
    }

    return;
}

static void
CameraSetPosition(RwCamera *camera, RpWorld *world)
{
    RwFrame *frame;
    RwV3d pos, right, at;
    const RwBBox *bbox;

    frame = RwCameraGetFrame(camera);

    /* 
     * Rotate the camera so it looks straight down...
     */
    right = *RwMatrixGetRight(RwFrameGetMatrix(frame));
    RwFrameRotate(frame, &right, 90.0f, rwCOMBINEREPLACE);

    /* 
     * Move the camera to the center of the world...
     */
    bbox = RpWorldGetBBox(world);
    RwV3dSub(&pos, &(bbox->sup), &(bbox->inf));
    RwV3dScale(&pos, &pos, 0.5f);
    RwV3dAdd(&pos, &pos, &(bbox->inf));
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    /* 
     * And back it up a little...
     */
    at = *RwMatrixGetAt(RwFrameGetMatrix(frame));
    RwV3dScale(&at, &at, -210.0f);
    RwFrameTranslate(frame, &at, rwCOMBINEPOSTCONCAT);
    
    return;
}

static void
ReloadWorld(void)
{
    /*
     * Firstly we remove the light and camera from
     * the present world...
     */
    RpWorldRemoveLight(World, MainLight);
    RpWorldRemoveCamera(World, Camera);

    /* 
     * Now destroy the present world...
     */
    RpWorldDestroy(World);
    World = NULL;

    /*
     * Load in a new world and add the old light
     * and camera from the previous world...
     */
    World = CreateWorld(GeometryConditioning, Wrapping, UVLimit);
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return;
    }

    CameraSetPosition(Camera, World);

    /*
     * Add the light and camera to the new world...
     */
    RpWorldAddLight(World, MainLight);
    RpWorldAddCamera(World, Camera);

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
