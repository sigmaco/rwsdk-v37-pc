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
 ****************************************************************************/

/****************************************************************************
 *
 * main.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the use of PAK files for the storing and 
 *          retrieval of RenderWare data.
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

#include "pakfile.h"

#include "rtfsyst.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif


#if (defined (SKY))
#define READ_BUFFER_SIZE (64 * 1024)
static RwChar hBuffer[PAKMAXOPENFILES * READ_BUFFER_SIZE]  __attribute__ ((aligned(64)));
#elif (defined (DOLPHIN))
#define READ_BUFFER_SIZE GCN_DVD_SECTORSIZE
static RwChar hBuffer[PAKMAXOPENFILES * READ_BUFFER_SIZE] __attribute__ ((aligned(32)));
#endif


#define DEFAULT_VIEWWINDOW (0.7f)

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static RtCharset *Charset = NULL;
static RpWorld *World = NULL;
static RwCamera *Camera = NULL;
static RpLight *AmbientLight = NULL;
static RpLight *MainLight = NULL;

static RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
static RwV3d Yaxis = {0.0f, 1.0f, 0.0f};

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
static RpLight *
CreateAmbientLight(RpWorld *world)
{
    RpLight *light = NULL;

    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RpWorldAddLight(world, light);
    }

    return light;
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

        /* 
         * This one needs a frame...
         */
        frame = RwFrameCreate();
        if( frame )
        {
            RwFrameRotate(frame, &Xaxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &Yaxis, 30.0f, rwCOMBINEPOSTCONCAT);

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
        RwFrame *cameraFrame;

        cameraFrame = RwCameraGetFrame(camera);

        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 100.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}

/*
 *****************************************************************************
 * This function is used for platforms for which the RsPathnameCreate function
 * modifies the pathname is such a way that it becomes invalid for the pakfs.
 * It returns a valid pak pathname.
 *
 * On Xbox we need to strip out the media path that is attached to this 
 * path by the RsPathnameCreate function, i.e. remove everything before
 * the PAKFSDEVICENAME.
 */
static RwChar *
GetValidPakPath(RwChar *path)
{
    RwChar *validPath;

#if (defined (_XBOX))
    validPath = rwstrstr(path, PAKFSDEVICENAME);
#elif(defined (SKY) || defined (DOLPHIN))
    {
        /* 
         * Here reverse the conversion of ':' to '/' done by 
         * the RsPathnameDestroy function for SKY and GCN.
         */
        RwChar *pak;
        validPath = path;
        pak = rwstrchr(validPath, '/');
        *pak = ':';
    }
#else
    validPath = path;
#endif

    return (validPath);
}

/*
 *****************************************************************************
 */
static RwBool
CreateScene(RpWorld *world)
{
    RwStream *stream;
    RwChar *path, tempPath[50];
    RpClump *clump;
    
    const RwChar *texture = "./models/textures/";
    const RwChar *checker = "./models/checker.dff";
    const RwChar *bucky   = "./models/bucky.dff";

    /* 
     * These files must be opened using the pak file system.
     * Therefore append the pakfilesystem device name in front
     * of the path names.
     */

    /* Texture */
    rwstrcpy(tempPath, PAKFSDEVICENAME);
    rwstrcat(tempPath, texture);
    
    path = RsPathnameCreate(tempPath);
    
    RwImageSetPath(GetValidPakPath(path));
    RsPathnameDestroy(path);
    
    /* Checker */
    rwstrcpy(tempPath, PAKFSDEVICENAME);
    rwstrcat(tempPath, checker);

    path = RsPathnameCreate(tempPath);

    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, GetValidPakPath(path));
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

    /* 
     * Same for this one, it must be opened using the pak file system: 
     * append the pack file system device name 
     *
     * Bucky
     */
    rwstrcpy(tempPath, PAKFSDEVICENAME);
    rwstrcat(tempPath, bucky);

    path = RsPathnameCreate(tempPath);

    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, GetValidPakPath(path));
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
            RwV3d pos;
            RpClump *clumpCopy;

            clumpFrame = RpClumpGetFrame(clump);
            pos.x = 15.0f;
            pos.y = -2.0f;
            pos.z = 45.0f;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEREPLACE);
            RpWorldAddClump(world, clump);

            clumpCopy = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clumpCopy);
            pos.x = 7.5f;
            pos.y = 0.0f;
            pos.z = 37.5f;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEREPLACE);

            clumpCopy = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clumpCopy);
            pos.x = 0.0f;
            pos.y = 2.0f;
            pos.z = 30.0f;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEREPLACE);

            clumpCopy = RpClumpClone(clump);
            clumpFrame = RpClumpGetFrame(clumpCopy);
            pos.x = -7.5f;
            pos.y = 4.0f;
            pos.z = 23.5f;
            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEREPLACE);
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
            RsGlobal.maximumWidth = DEFAULT_SCREEN_WIDTH;
        }

        if( !RsGlobal.maximumHeight )
        {
            RsGlobal.maximumHeight = DEFAULT_SCREEN_HEIGHT;
        }

        RsGlobal.appName = RWSTRING("RenderWare Graphics Packed File System Example");

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
OpenPakFile(void)
{
    RwChar *path1, *path2;
    RwChar *prefixPath;
    RwBool opened = FALSE;

    /* 
     * This file will be opened using the default file system 
     */
    path1 = RsPathnameCreate(RWSTRING("misc/pakdemo.pak"));

    /* 
     * Now get the pak file system device name in order to open the pak file
     * on the pak file system
     */
    prefixPath = RwMalloc(sizeof(RwChar) * (rwstrlen(PAKFSDEVICENAME) + 3), rwID_NAOBJECT);
    
    rwstrcpy(prefixPath, PAKFSDEVICENAME);
    rwstrcat(prefixPath, "./");

    path2 = RsPathnameCreate(prefixPath);

    /* Here we only want the path starting from the PAKFSDEVICENAME,
     * so call GetValidPath as on certain platforms, e.g. Xbox
     * RsPathnameCreate adds a media path in front of the path.
     */
    opened = PakFileOpenExt(path1, GetValidPakPath(path2));

    RsPathnameDestroy(path2);
    RsPathnameDestroy(path1);
    
    RwFree(prefixPath);
    return opened;
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

    /* 
     * Here install the Pak File System 
     */
    {
        RwChar pakFsName[RTFS_MAX_NAME_LENGTH];
        RtFileSystem *pakFS;
        
        RwChar pakDeviceName[5];
        
        rwstrcpy(pakFsName, "pkfs"); 
        rwstrcpy(pakDeviceName, PAKFSDEVICENAME);
#if (defined (WIN32))
        pakFS = PakFSystemInit(PAKMAXOPENFILES, PAKFSDEVICENAME, pakFsName);
#else
        pakFS = PakFSystemInit(PAKMAXOPENFILES, hBuffer, READ_BUFFER_SIZE, pakDeviceName, pakFsName);
#endif        
        if (pakFS)
        {
            /* Now register it */
            if (RtFSManagerRegister(pakFS) == FALSE)
            {
                return (FALSE);
            }
        }
        else
        {
            return (FALSE);
        }
    }

    if( !OpenPakFile() )
    {
        RsErrorMessage(RWSTRING("Cannot open PAK file."));

        return FALSE;
    }

    Charset = RtCharsetCreate(&ForegroundColor, &BackgroundColor);
    if( Charset == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create raster charset."));
    
        return FALSE;
    }

    World = CreateWorld();
    if (World == NULL)
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

    if( !CreateScene(World) )
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
DestroyClump(RpClump * clump, void *data)
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

    /* 
     * Close PAK file...
     */
    PakFileClose();

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
