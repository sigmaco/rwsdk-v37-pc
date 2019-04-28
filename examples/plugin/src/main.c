
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
 * Purpose: Example to demonstrate how user-plugins can be used to extend
 *          RenderWare objects.
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

#include "physics.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

#define NUM_ACTIVE_CLUMPS (10)

#define START_HEIGHT (20.0f)
#define COLLISION_HEIGHT (-15.0f)
#define CAMERA_OFFSET (60.0f)
#define BUCKY_HEIGHT (2.0f)

/*
 * Compare the horizontal distance between two clumps...
 */
#define COMPAREPOS(a, b)                        \
    (((((a)->x-(b)->x)*((a)->x-(b)->x) +        \
       ((a)->z-(b)->z)*((a)->z-(b)->z)) <       \
      (4*BUCKY_HEIGHT*BUCKY_HEIGHT)) ? 1 : 0)

#define TESTSTREAMx

static RpWorld *World = NULL;
static RwCamera *Camera = NULL;
static RtCharset *Charset = NULL;

static RpClump *Floor = NULL;
static RpClump *Bucky = NULL;
static RpClump *ActiveClumps[NUM_ACTIVE_CLUMPS];

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static RwV3d Xaxis = {1.0f, 0.0f, 0.0f};

RwBool InitClumps(void);



/*
 *****************************************************************************
 */
static RpWorld *
CreateWorld(void)
{
    RpWorld *world;
    RwBBox bb;

    bb.inf.x = bb.inf.y = bb.inf.z = -100.0f;
    bb.sup.x = bb.sup.y = bb.sup.z = 100.f;

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
        RwV3d pos;

        RwFrameRotate(RwCameraGetFrame(camera), 
            &Xaxis, 50.0f, rwCOMBINEREPLACE);

        pos.x = pos.z = 0.0f;
        pos.y = START_HEIGHT * 2.5f;
        RwFrameTranslate(RwCameraGetFrame(camera), 
            &pos, rwCOMBINEPOSTCONCAT);

        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 300.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
static RwBool
CreateLights(RpWorld *world)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTSPOTSOFT);
    if( light )
    {
        RwFrame *frame;
        RwV3d pos;

        RpLightSetRadius(light, 200.0f);
        RpLightSetConeAngle(light, 60.0f / 180.0f * rwPI);

        frame = RwFrameCreate();
        if( frame )
        {
            pos.x = 0.0f;
            pos.y = START_HEIGHT / 2.0f;
            pos.z = CAMERA_OFFSET;

            RwFrameRotate(frame, &Xaxis, 90.0f, rwCOMBINEREPLACE);
            RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
            RpLightSetFrame(light, frame);

            RpWorldAddLight(world, light);
        }
        else
        {
            RpLightDestroy(light);

            return FALSE;
        }
    }
    
    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RpWorldAddLight(world, light);
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
static RwReal 
GetRandomReal(RwReal lower, RwReal upper)
{
    const RwUInt32 random = RpRandom();
    /* 
     * Returns a random number which lies between the 
     * upper and lower bounds...
     */
    
    return lower + (upper-lower) * ((RwReal)random / (RwUInt32MAXVAL>>1));
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
    
    vec->x = 2.0f * ((RwReal)randomX / (RwUInt32MAXVAL>>1)) - 1.0f;
    vec->y = 2.0f * ((RwReal)randomY / (RwUInt32MAXVAL>>1)) - 1.0f;
    vec->z = 2.0f * ((RwReal)randomZ / (RwUInt32MAXVAL>>1)) - 1.0f;

    return;
}


/*
 *****************************************************************************
 */
static RwV3d *
CreateNewRandomPosition(RwV3d * result,
                        RpWorld * world __RWUNUSED__,
                        RwInt32 curClump)
{
    static RwV3d clumpPosition[NUM_ACTIVE_CLUMPS];
    RwInt32 i;
    RwBool overlapped;

    while( TRUE )
    {
        overlapped = FALSE;

        /*
         * Create random position...
         */
        result->x = GetRandomReal(-25.0f, 25.0f);
        result->y = START_HEIGHT;
        result->z = GetRandomReal(-25.0f, 25.0f) + CAMERA_OFFSET;

        /*
         * Compare to all previous positions to check overlap...
         */
        for(i=0; i<curClump; i++)
        {
            if( COMPAREPOS(result, &clumpPosition[i]) )
            {
                overlapped = TRUE;

                break;
            }
        }

        if( !overlapped )
        {
            /* 
             * Success -> end while loop...
             */
            break;
        }

        /*
         * Failure - start again...
         */
    }

    clumpPosition[curClump] = *result;

    return result;
}


/*
 *****************************************************************************
 */
RwBool
InitClumps(void)
{
    RwInt32 i;

    RpRandomSeed(RsTimer());

    for(i=0; i<NUM_ACTIVE_CLUMPS; i++)
    {
        RwFrame *clumpFrame;
        RwV3d axis, pos;
        RwReal angle;
        RpWorld *world;

        clumpFrame = RpClumpGetFrame(ActiveClumps[i]);
        world = RpClumpGetWorld(ActiveClumps[i]);
    
        /*
         * Reset clumps with new random position & rotation...
         */
        GetRandomVector(&axis);
        angle = GetRandomReal(0.0f, 360.0f);
        RwFrameRotate(clumpFrame, &axis, angle, rwCOMBINEREPLACE);
    
        CreateNewRandomPosition(&pos, world, i);
        
        RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

        /*
         * Initialize user plugin data...
         */
        RpClumpPhysicsSetSpeed(ActiveClumps[i], 0.0f);

        RpClumpPhysicsSetBounciness(ActiveClumps[i], 
            GetRandomReal(0.1f, 0.9f));

        RpClumpPhysicsSetActive(ActiveClumps[i], TRUE);
    }

    return TRUE;
}

 
/*
 *****************************************************************************
 */
static RwBool
CreateActiveClumps(RpWorld *world)
{
    RwChar *path;
    RwStream *stream;

    path = RsPathnameCreate(RWSTRING("models/bucky.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            Bucky = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);

        if( Bucky )
        {
            RwInt32 i;

            /*
             * Use bucky as a master to make multiple clones...
             */
            for(i=0; i<NUM_ACTIVE_CLUMPS; i++)
            {
                ActiveClumps[i] = RpClumpClone(Bucky);

                RpWorldAddClump(world, ActiveClumps[i]);
            }

            InitClumps();

            return TRUE;
        }
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
CreateScene(RpWorld *world)
{
    RwChar *path;
    RwStream *stream;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING("models/checker.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            Floor = RpClumpStreamRead(stream);

            RwStreamClose(stream, NULL);
        }

        if( Floor)
        {
            RwFrame *clumpFrame;
            RwV3d pos;

            clumpFrame = RpClumpGetFrame(Floor);

            RwFrameRotate(clumpFrame, &Xaxis, 90.0f, rwCOMBINEREPLACE);

            pos.x = 0.0f;
            pos.y = COLLISION_HEIGHT - BUCKY_HEIGHT;
            pos.z = CAMERA_OFFSET;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);
            
            RpWorldAddClump(world, Floor);
        }
        else
        {
            RsErrorMessage(RWSTRING("Cannot create floor."));

            return FALSE;
        }
    }
    else
    {
        RsErrorMessage(RWSTRING("Cannot open stream for checker.dff"));

        return FALSE;
    }

    if( !CreateActiveClumps(world) )
    {
        RsErrorMessage(RWSTRING("Cannot create clumps"));

        return FALSE;
    }
    
    return TRUE;
}


#ifdef TESTSTREAM
/*
 *****************************************************************************
 */
static void 
TestPluginStream(void)
{
    RwStream *stream;
    RwChar *path;

    /*
     * Fill the extension data with identifiable values...
     */
    RpClumpPhysicsSetSpeed(Bucky, 123.45f);
    RpClumpPhysicsSetBounciness(Bucky, 0.12345f);
    RpClumpPhysicsSetActive(Bucky, TRUE);

    /* 
     * Write the clump (with extension data) to binary stream...
     */
    path = RsPathnameCreate(RWSTRING("models/bucky2.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        RpClumpStreamWrite(Bucky, stream);

        RwStreamClose(stream, NULL);
    }

    /*
     * Read the clump back in and check that the extension
     * data is intact...
     */
    path = RsPathnameCreate(RWSTRING("models/bucky2.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        RpClump *bucky2;
        RwReal speed, bounciness;
        RwBool active;

        RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL);
        bucky2 = RpClumpStreamRead(stream);
        RwStreamClose(stream, NULL);

        speed = RpClumpPhysicsGetSpeed(bucky2);
        bounciness = RpClumpPhysicsGetBounciness(bucky2);
        active = RpClumpPhysicsGetActive(bucky2);

        RpClumpDestroy(bucky2);
    }

    return;
}
#endif /* TESTSTREAM */


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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Plugin Construction Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
ResetClumpsCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    InitClumps();

    return TRUE;
}


static RwBool 
InitializeMenu(void)
{    
    static RwChar resetLabel[] = RWSTRING("Reset_R");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryTrigger(resetLabel, ResetClumpsCallback);

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

    if( !CreateLights(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create lights."));

        return FALSE;
    }

    Camera = CreateCamera(World);
    if( !Camera )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    if( !CreateScene(World) )
    {
        RsErrorMessage(RWSTRING("Cannot create scene."));

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

#ifdef TESTSTREAM
    TestPluginStream();
#endif

    return TRUE;
}

/*
 *****************************************************************************
 */
static RpClump *
DestroyClump(RpClump *clump,
             void *data __RWUNUSED__)
{
    RpWorld *world;

    world = RpClumpGetWorld(clump);
    if( world )
    {
        RpWorldRemoveClump(world, clump);
    }

    RpClumpDestroy(clump);

    return clump;
}


/*
 *****************************************************************************
 */
static RpLight *
DestroyLight(RpLight *light,
             void *data __RWUNUSED__)
{
    RwFrame *frame;
    
    RpWorldRemoveLight(World, light);
    
    frame = RpLightGetFrame(light);
    if( frame )
    {
        RpLightSetFrame(light, NULL);
        RwFrameDestroy(frame);
    }
    
    RpLightDestroy(light);

    return light;
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

    RpWorldForAllClumps(World, DestroyClump, NULL);
    RpClumpDestroy(Bucky);

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        CameraDestroy(Camera);
    }

    RpWorldForAllLights(World, DestroyLight, NULL);

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
     * Attach random number generator plug-in...
     */
    if( !RpRandomPluginAttach() )
    {
        RsErrorMessage(RWSTRING("RpRandomPluginAttach failed."));

        return FALSE;
    }

    /* 
     * Attach our custom plug-in...
     */
    if( !RpClumpPhysicsPluginAttach() )
    {
        RsErrorMessage(RWSTRING("RpClumpPhysicsPluginAttach failed."));

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

    rwstrcpy(caption, RpClumpPhysicsGetCaption());

    RsCharsetPrint(Charset, caption, 0, 1, rsPRINTPOSTOPRIGHT);

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
            /*
             * Scene rendering here...
             */
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
static RpClump *
UpdateClumpDynamics(RpClump *clump, void *data)
{
    RwReal deltaT;
    RwFrame *frame;
    RwV3d translate;
    RwV3d *pos; 

    /* 
     * Exclude the floor or any inactive clumps...
     */
    if (clump == Floor || !RpClumpPhysicsGetActive(clump))
    {
        return clump;
    }

    deltaT = *(RwReal *)data;

    frame = RpClumpGetFrame(clump);

    /*
     * Equation to calculate distance moved is as follows:
     *      s = u*t + 0.5*a*t^2  where   s - distance
     *                                   u - initial speed
     *                                   t - time
     *                                   a - acceleration
     */
    translate.x = translate.z = 0.0f;
    translate.y = RpClumpPhysicsGetSpeed(clump) * deltaT
        + 0.5f * RpClumpPhysicsGetGravity() * deltaT * deltaT;
    
    /*
     * Update clump position...
     */
    RwFrameTranslate(frame, &translate, rwCOMBINEPOSTCONCAT);
    
    /*
     * Update clump's speed...
     */
    RpClumpPhysicsIncSpeed(clump, RpClumpPhysicsGetGravity() * deltaT);
    
    /*
     * Primitive check for collision with ground...
     */
    pos = RwMatrixGetPos(RwFrameGetMatrix(frame));

    if( pos->y < COLLISION_HEIGHT )
    {
        /*
         * Check clump's speed is still worth considering...
         */
        if( RwFabs(RpClumpPhysicsGetSpeed(clump)) >= RpClumpPhysicsGetMinSpeed() )
        {
            /*
             * Clump has bounced -> update variables...
             */
            RwReal newSpeed;
            
            /* 
             * Negate speed and factor in bounciness...
             */
            newSpeed = -RpClumpPhysicsGetSpeed(clump) 
                * RpClumpPhysicsGetBounciness(clump);
            
            /*
             * Update clump's speed...
             */
            RpClumpPhysicsSetSpeed(clump, newSpeed);
            
            /*
             * Make sure there is no 'drift' by reseting clumps height...
             */
            pos->y = COLLISION_HEIGHT;
        }
        else
        {
            /*
             * Clump moving too slowly - deactivate...
             */
            RpClumpPhysicsSetActive(clump, FALSE);
        }
    }

    return clump;
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

    deltaTime = (thisTime - lastAnimTime) * 0.001f;

    /*
     * Update clump positions...
     */
    RpWorldForAllClumps(World, UpdateClumpDynamics, &deltaTime);

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
