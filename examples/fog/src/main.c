
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
 * Purpose: To illustrate fog in RenderWare.
 *                         
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rprandom.h"

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

#define SCREEN_WIDTH (640)
#define SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.7f)

#define MAXCLIPPLANE (100.0f)
#define FOGDISTANCE (10.0f)
#define MINCLIPPLANE (0.01f)

#define NUMFOGTYPES (3)

static RwFogType FogTypes[NUMFOGTYPES] = 
{
    rwFOGTYPELINEAR,
    rwFOGTYPEEXPONENTIAL,
    rwFOGTYPEEXPONENTIAL2
};

static RwBool FogMode[NUMFOGTYPES];
static RwInt32 FogTypeID = 0;
static RwBool FogAvailable;

static RpWorld *World = NULL;
static RpLight *AmbientLight = NULL;
static RpLight *MainLight = NULL;
static RwCamera *Camera = NULL;
static RwRaster *Charset = NULL;

static RwBool FogOn = FALSE;
static RwRGBAReal FogColorReal = {1.0f, 1.0f, 1.0f, 1.0f};
static RwUInt32 FogColor = 0xFFFFFFFF;
static RwReal FogDensity = 0.02f;
static RwReal FogDistance = FOGDISTANCE;

static RwReal NearClipPlane = MINCLIPPLANE;
static RwReal FarClipPlane = MAXCLIPPLANE;

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};
static RwRGBA ClearColor      = { 64,  64,  64,   0};

RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
RwV3d Yaxis = {0.0f, 1.0f, 0.0f};
RwV3d Zaxis = {0.0f, 0.0f, 1.0f};



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
        RwFrame *lightFrame;

        /* 
         * This one needs a frame...
         */
        lightFrame = RwFrameCreate();
        if( lightFrame )
        {
            RwFrameRotate(lightFrame, &Xaxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(lightFrame, &Yaxis, 30.0f, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, lightFrame);

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
        RwCameraSetNearClipPlane(camera, NearClipPlane);
        RwCameraSetFarClipPlane(camera, FarClipPlane);

        RwCameraSetFogDistance(camera, FogDistance);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static void 
GetRandomVector(RwV3d *vec)
{
    const RwUInt32 randomX = RpRandom();
    const RwUInt32 randomY = RpRandom();
    const RwUInt32 randomZ = RpRandom();
    
    vec->x = 2.0f * ((RwReal)(randomX) / (RwUInt32MAXVAL>>1)) - 1.0f;
    vec->y = 2.0f * ((RwReal)(randomY) / (RwUInt32MAXVAL>>1)) - 1.0f;
    vec->z = 2.0f * ((RwReal)(randomZ) / (RwUInt32MAXVAL>>1)) - 1.0f;

    return;
}


/*
 *****************************************************************************
 */
static RwReal 
GetRandomUnitReal(void)
{
    const RwUInt32 random = RpRandom();
    return (RwReal)(random) / (RwUInt32MAXVAL>>1);
}


/*
 *****************************************************************************
 */
static RwBool
CreateTestScene(RpWorld *world)
{
    RwStream *stream;
    RwChar *path;
    RpClump *clump;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING("models/checker.dff"));
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
            RwV3d pos = {0.0f, -5.0f, 40.0f};

            clumpFrame = RpClumpGetFrame(clump);

            RwFrameRotate(clumpFrame, &Xaxis, 90.0f, rwCOMBINEREPLACE);
            RwFrameRotate(clumpFrame, &Yaxis, 45.0f, rwCOMBINEPOSTCONCAT);

            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

            RpWorldAddClump(world, clump);
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    path = RsPathnameCreate(RWSTRING("models/bucky.dff"));
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
            RwV3d pos, axis;
            RwReal angle;
            RpClump *clumpCopy;

            clumpFrame = RpClumpGetFrame(clump);
            GetRandomVector(&axis);
            angle = 360.0f * GetRandomUnitReal();
            RwFrameRotate(clumpFrame, &axis, angle, rwCOMBINEREPLACE);
            pos.x = 15.0f;
            pos.y = -2.0f;
            pos.z = 45.0f;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);
            RpWorldAddClump(world, clump);

            clumpCopy = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clumpCopy);
            GetRandomVector(&axis);
            angle = 360.0f * GetRandomUnitReal();
            RwFrameRotate(clumpFrame, &axis, angle, rwCOMBINEREPLACE);
            pos.x = 7.5f;
            pos.y = 0.0f;
            pos.z = 37.5f;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

            clumpCopy = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clumpCopy);
            GetRandomVector(&axis);
            angle = 360.0f * GetRandomUnitReal();
            RwFrameRotate(clumpFrame, &axis, angle, rwCOMBINEREPLACE);
            pos.x = 0.0f;
            pos.y = 2.0f;
            pos.z = 30.0f;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

            clumpCopy = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clumpCopy);
            GetRandomVector(&axis);
            angle = 360.0f * GetRandomUnitReal();
            RwFrameRotate(clumpFrame, &axis, angle, rwCOMBINEREPLACE);
            pos.x = -7.5f;
            pos.y = 4.0f;
            pos.z = 23.5f;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
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
            RsGlobal.maximumWidth = SCREEN_WIDTH;
        }

        if( !RsGlobal.maximumHeight )
        {
            RsGlobal.maximumHeight = SCREEN_HEIGHT;
        }

        RsGlobal.appName = RWSTRING("RenderWare Graphics Fog Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool 
FogCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return FogAvailable;
    }

    return TRUE;
}


static RwBool 
FogTypeCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return FogOn;
    }

    return TRUE;
}


static RwBool 
FogDistanceCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return (FogTypeID == 0) && FogMode[0] && FogOn;
    }

    if( FogDistance > (FarClipPlane - 0.01f) ) 
    {
        FogDistance = FarClipPlane - 0.01f;
    }

    RwCameraSetFogDistance(Camera, FogDistance);

    return TRUE;
}


static RwBool 
FogDensityCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return (((FogTypeID == 1) && FogMode[1]) || ((FogTypeID == 2) && FogMode[2])) && FogOn;
    }

    return TRUE;
}


static RwBool 
FogColorCallback(RwBool testEnable)
{
    RwUInt32 red, green, blue;

    if( testEnable )
    {
        return FogMode[FogTypeID] && FogOn;
    }

    red   = (RwUInt32)(255.0f * FogColorReal.red);
    green = (RwUInt32)(255.0f * FogColorReal.green);
    blue  = (RwUInt32)(255.0f * FogColorReal.blue);

    FogColor = RWRGBALONG(red, green, blue, 255);

    return TRUE;
}


static RwBool 
NearClipPlaneCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    if( NearClipPlane > (FarClipPlane - 0.01f) ) 
    {
        NearClipPlane = FarClipPlane - 0.01f;
    }

    RwCameraSetNearClipPlane(Camera, NearClipPlane);

    return TRUE;
}


static RwBool 
FarClipPlaneCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    /*
     * For linear fog, the far clip-plane has to be beyond the 
     * fog distance or the near clip-plane, whichever is larger. For
     * other fogs, restrict the far clip-plane to the near plane...
     */ 
    if( (FogTypeID == 0) && (FogDistance > NearClipPlane) )
    {
        /*
         * Test against the fog distance...
         */
        if( FarClipPlane < (FogDistance + 0.01f) )
        {
            FarClipPlane = FogDistance + 0.01f;
        }
    }
    else
    {
        /*
         * Test against the near clip-plane...
         */
        if( FarClipPlane < (NearClipPlane + 0.01f) )
        {
            FarClipPlane = NearClipPlane + 0.01f;
        }
    }

    RwCameraSetFarClipPlane(Camera, FarClipPlane);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{    
    static RwChar fogLabel[]         = RWSTRING("Fog");
    static RwChar fogTypeLabel[]     = RWSTRING("Fog type");
    static RwChar fogDistanceLabel[] = RWSTRING("Fog distance");
    static RwChar fogDensityLabel[]  = RWSTRING("Fog density");
    static RwChar fogRedLabel[]      = RWSTRING("Fog red");
    static RwChar fogGreenLabel[]    = RWSTRING("Fog green");
    static RwChar fogBlueLabel[]     = RWSTRING("Fog blue");

    static RwChar nearClipLabel[] = RWSTRING("Near clip-plane");
    static RwChar farClipLabel[]  = RWSTRING("Far clip-plane");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    static const RwChar *fogTypeStrings[NUMFOGTYPES] = 
    {
        RWSTRING("Linear"),
        RWSTRING("Exponential"),
        RWSTRING("Exponential2")
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryBool(fogLabel, &FogOn, FogCallback);

        MenuAddEntryInt(fogTypeLabel, &FogTypeID, 
            FogTypeCallback, 0, NUMFOGTYPES-1, 1, fogTypeStrings);
    
        MenuAddEntryReal(fogDistanceLabel, &FogDistance, 
            FogDistanceCallback, MINCLIPPLANE, MAXCLIPPLANE, 1.0f);

        MenuAddEntryReal(fogDensityLabel, &FogDensity, 
            FogDensityCallback, 0.0f, 1.0f, 0.01f);

        MenuAddEntryReal(fogRedLabel, &FogColorReal.red, 
            FogColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddEntryReal(fogGreenLabel, &FogColorReal.green, 
            FogColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddEntryReal(fogBlueLabel, &FogColorReal.blue, 
            FogColorCallback, 0.0f, 1.0f, 0.01f);

        MenuAddSeparator();

        MenuAddEntryReal(nearClipLabel, &NearClipPlane, 
            NearClipPlaneCallback, MINCLIPPLANE, MAXCLIPPLANE, 1.0f);

        MenuAddEntryReal(farClipLabel, &FarClipPlane, 
            FarClipPlaneCallback, MINCLIPPLANE, MAXCLIPPLANE, 1.0f);

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
        RsErrorMessage(RWSTRING("Failed to initialize RenderWare."));

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
        RsErrorMessage(RWSTRING("Cannot create main camera."));
        
        return FALSE;
    }

    if( !CreateTestScene(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create test scene."));

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
static RpClump *
DestroyClump(RpClump *clump, void *data)
{
    RpWorldRemoveClump((RpWorld *)data, clump);

    RpClumpDestroy(clump);

    return clump;
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

    RpWorldForAllClumps(World, DestroyClump, World);

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

    if( AmbientLight )
    {
        RpWorldRemoveLight(World, AmbientLight);

        RpLightDestroy(AmbientLight);
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
        RsErrorMessage(RWSTRING("RpWorldPluginAttach failed."));

        return FALSE;
    }

#ifdef RWLOGO
    /* 
     * Attach logo plug-in...
     */
    if( !RpLogoPluginAttach() )
    {
        RsErrorMessage(RWSTRING("RpLogoPluginAttach failed."));

        return FALSE;
    }
#endif

    if( !RpRandomPluginAttach() )
    {
        RsErrorMessage(RWSTRING("RpRandomPluginAttach failed."));
        
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
    RwBool fogEnabled;
    RwChar caption[256];

    RwRenderStateGet(rwRENDERSTATEFOGENABLE, (void *)&fogEnabled);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }

    if( !FogAvailable )
    {
        rwstrcpy(caption, RWSTRING("Fogging is not available"));

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSBOTTOMRIGHT);
    }
    else if( !FogMode[FogTypeID] )
    {
        rwstrcpy(caption, RWSTRING("Fog type not available"));

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSBOTTOMRIGHT);
    }
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)fogEnabled);

    return;
}


/*
 *****************************************************************************
 */
static void 
UpdateFogRenderStates(void)
{
    if( FogAvailable && FogMode[FogTypeID] && FogOn )
    {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)TRUE);

        RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)FogTypes[FogTypeID]);

        RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void *)FogColor);

        RwRenderStateSet(rwRENDERSTATEFOGDENSITY, (void *)&FogDensity);
    }
    else
    {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
    }

    return;
}


/*
 *****************************************************************************
 */
static void
QueryFogTypes(void)
{
    RwInt32 i;

    FogAvailable = FALSE;

    for(i=0; i<NUMFOGTYPES; i++)
    {
        if( RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)FogTypes[i]) )
        {
            FogMode[i] = TRUE;

            FogAvailable = TRUE;
        }
        else
        {
            FogMode[i] = FALSE;
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
    static RwBool firstCall = TRUE;

    if( FogAvailable && FogMode[FogTypeID] && FogOn )
    {
        RwRGBAFromRwRGBAReal(&ClearColor, &FogColorReal);
    }
    else
    {
        ClearColor = BackgroundColor;
    }

    RwCameraClear(Camera, &ClearColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( firstCall )
        {
            /*
             * Query available fog types and initialize...
             */
            QueryFogTypes();
    
            firstCall = FALSE;
        }

        if( MenuGetStatus() != HELPMODE )
        {
            UpdateFogRenderStates();

            RpWorldRender(World);

            DisplayOnScreenInfo();
        }

        if( FogOn )
        {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);
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
