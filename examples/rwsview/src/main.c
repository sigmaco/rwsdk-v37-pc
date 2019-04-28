
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
 * Copyright (c) 2003 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * main.c
 *
 * Copyright (C) 2003 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrating loading RWS assets
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rtcharse.h"
#include "rphanim.h"
#include "rtanim.h"
#include "rtpitexd.h"
#include "rttoc.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

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

static RwChar rwsFilename[] = "models/scene.rws";

RtCharset *Charset = NULL;

/* structure for storing clumps and a cached pointer to the HAnim
 * hierarchy of their first contained object, e.g. RpAtomic or
 * RpLight in this example */
typedef struct ClumpHierarchy ClumpHierarchy;
struct ClumpHierarchy
{
    RpClump             *clump;

    RpHAnimHierarchy    *hierarchy;
};

/* Used in sky/events.c */
RwCamera *currentCamera = (RwCamera *)NULL;

static RwUInt32 numWorlds = 0;
static RwUInt32 numClumps = 0;
static RwUInt32 numAnims = 0;
static RwUInt32 numPITexDicts = 0;

static RpWorld **worldList = (RpWorld **)NULL;
static ClumpHierarchy *clumpList = (ClumpHierarchy *)NULL;
static RtAnimAnimation **animList = (RtAnimAnimation **)NULL;
static RwTexDictionary  **texDictList = (RwTexDictionary **)NULL;


/*
 *****************************************************************************
 */
static RwBool 
LoadRWSAssets( void )
{    
    RwChar          *pathname;

    RwBool          success;
    
    RwStream        *rwsStream;

    RwTexDictionary *defaultTexDict;


    pathname = RsPathnameCreate( rwsFilename );
    if ( NULL == pathname )
    {
        return FALSE;
    }

    success = TRUE;
    numWorlds = 0;
    numClumps = 0;
    numAnims = 0;
    numPITexDicts = 0;

    /* if a TOC exists, use it to predetermine how many objects we need to load,
     * otherwise, peek at the chunk IDs directly */
    rwsStream = RwStreamOpen( rwSTREAMFILENAME,
                              rwSTREAMREAD,
                              pathname );
    if ( NULL == rwsStream )
    {
        RsPathnameDestroy( pathname );
        return FALSE;
    }
    else
    {
        if ( FALSE != RwStreamFindChunk( rwsStream,
                                         rwID_TOC,
                                         NULL,
                                         NULL ) )
        {
            RtTOC   *toc;


            toc = RtTOCStreamRead( rwsStream );
            if ( NULL != toc )
            {
                RwInt32    idx;


                for ( idx = 0; idx < RtTOCGetNumEntries( toc ); idx += 1 )
                {
                    switch ( RtTOCGetEntry( toc, idx )->id )
                    {
                    case rwID_WORLD:
                        numWorlds += 1;
                        break;

                    case rwID_CLUMP:
                        numClumps += 1;
                        break;

                    case rwID_HANIMANIMATION:
                        numAnims += 1;
                        break;

                    case rwID_PITEXDICTIONARY:
                        numPITexDicts += 1;
                        break;

                    default:
                        break;
                    }
                }

                RtTOCDestroy( toc );
            }
        }
        else
        {
            RwChunkHeaderInfo   chunkHdrInfo;

        
            RwStreamClose( rwsStream, NULL );
            rwsStream = RwStreamOpen( rwSTREAMFILENAME,
                                      rwSTREAMREAD,
                                      pathname );
            while ( NULL != RwStreamReadChunkHeaderInfo( rwsStream, &chunkHdrInfo ) )
            {
                switch ( chunkHdrInfo.type )
                {
                case rwID_WORLD:
                    numWorlds += 1;
                    break;

                case rwID_CLUMP:
                    numClumps += 1;
                    break;

                case rwID_HANIMANIMATION:
                    numAnims += 1;
                    break;

                case rwID_PITEXDICTIONARY:
                    numPITexDicts += 1;
                    break;

                default:
                    break;
                }
                RwStreamSkip( rwsStream, chunkHdrInfo.length );
            }
        }
        RwStreamClose( rwsStream, NULL );
    }

    /* can only handle one world in this example */
    if ( numWorlds > 1 )
    {
        RsErrorMessage( RWSTRING( "Unable to load more than one world." ) );
        return FALSE;
    }

    /* allocate storage */
    worldList = (RpWorld **)RwMalloc( numWorlds * sizeof(RpWorld *), rwID_NAOBJECT );
    if ( NULL == worldList )
    {
        return FALSE;
    }

    clumpList = (ClumpHierarchy *)RwMalloc( numClumps * sizeof(ClumpHierarchy), rwID_NAOBJECT );
    if ( NULL == clumpList )
    {
        return FALSE;
    }

    animList = (RtAnimAnimation **)RwMalloc( numAnims * sizeof(RtAnimAnimation *), rwID_NAOBJECT);
    if ( NULL == animList )
    {
        return FALSE;
    }

    texDictList = (RwTexDictionary **)RwMalloc( numPITexDicts * sizeof(RwTexDictionary *), rwID_NAOBJECT);
    if ( NULL == texDictList )
    {
        return FALSE;
    }
    /* clear all RwTexDictionary pointers to NULL to we don't look at uninitialized memory */
    memset( (void *)texDictList, 0, numPITexDicts * sizeof(RwTexDictionary *) );

    /* ensure that there is no default texture dictionary so that all texture dictionaries are searched */
    defaultTexDict = RwTexDictionaryGetCurrent();
    RwTexDictionarySetCurrent( NULL );

    /* now stream the assets */
    rwsStream = RwStreamOpen( rwSTREAMFILENAME,
                              rwSTREAMREAD,
                              pathname );
    if ( NULL != rwsStream )
    {
        RwChunkHeaderInfo   chunkHdrInfo;

        RwUInt32            currentWorld = 0;

        RwUInt32            currentClump = 0;

        RwUInt32            currentAnim = 0;

        RwUInt32            currentPITexDict = 0;


        while ( NULL != RwStreamReadChunkHeaderInfo( rwsStream, &chunkHdrInfo ) )
        {
            switch ( chunkHdrInfo.type )
            {
            case rwID_WORLD:
                {
                    worldList[currentWorld] = RpWorldStreamRead( rwsStream );
                    if ( NULL == worldList[currentWorld] )
                    {
                        RsErrorMessage( RWSTRING( "Unable to load world." ) );
                        success = FALSE;
                    }
                    else
                    {
                        currentWorld += 1;
                    }
                }
                break;

            case rwID_CLUMP:
                {
                    clumpList[currentClump].clump = RpClumpStreamRead( rwsStream );
                    if ( NULL == clumpList[currentClump].clump )
                    {
                        RsErrorMessage( RWSTRING( "Unable to load clump." ) );
                        success = FALSE;
                    }
                    else
                    {
                        clumpList[currentClump].hierarchy = (RpHAnimHierarchy *)NULL;

                        currentClump += 1;
                    }
                }
                break;

            case rwID_HANIMANIMATION:
                {
                    animList[currentAnim] = RtAnimAnimationStreamRead( rwsStream );
                    if ( NULL == animList[currentAnim] )
                    {
                        RsErrorMessage( RWSTRING( "Unable to load animation." ) );
                        success = FALSE;
                    }
                    else
                    {
                        RwInt32 typeID;


                        typeID = RtAnimAnimationGetTypeID( animList[currentAnim] );

                        if ( rpHANIMSTDKEYFRAMETYPEID != typeID )
                        {
                            RsErrorMessage( RWSTRING( "Unable to handle animation types other than HAnim." ) );
                            success = FALSE;
                        }
                        else
                        {
                            currentAnim += 1;
                        }
                    }
                }
                break;

            case rwID_PITEXDICTIONARY:
                {
                    texDictList[currentPITexDict] = RtPITexDictionaryStreamRead( rwsStream );
                    if ( NULL == texDictList[currentPITexDict] )
                    {
                        RsErrorMessage( RWSTRING( "Unable to load platform independent texture dictionary." ) );
                        success = FALSE;
                    }
                    else
                    {
                        currentPITexDict += 1;
                    }
                }
                break;

            default:
                RwStreamSkip( rwsStream, chunkHdrInfo.length );
            }

            if ( FALSE == success )
            {
                break;
            }
        }

        RwStreamClose( rwsStream, NULL );
    }

    /* restore the default texture dictionary */
    RwTexDictionarySetCurrent( defaultTexDict );

    RsPathnameDestroy( pathname );
    pathname = (RwChar *)NULL;

    return success;
}


/*
 *****************************************************************************
 */
static RpAtomic *
FirstAtomicGetFrame( RpAtomic *atomic,
                     void *data )
{
    RwFrame **frame;


    frame = (RwFrame **)data;
    *frame = RpAtomicGetFrame( atomic );

    return NULL;
}


/*
 *****************************************************************************
 */
static RpLight *
FirstLightGetFrame( RpLight *light,
                    void *data )
{
    RwFrame **frame;


    frame = (RwFrame **)data;
    *frame = RpLightGetFrame( light );

    return NULL;
}


/*
 *****************************************************************************
 */
static RwBool 
SetDefaultAssets( void )
{
    RwUInt32    i;


    if ( NULL == worldList[0] )
    {
        RsErrorMessage( RWSTRING( "No world has been loaded." ) );
        return FALSE;
    }

    /* add each clump to the world */
    for ( i = 0; i < numClumps; i += 1 )
    {
        RwFrame             *frame;


        RpWorldAddClump( worldList[0], clumpList[i].clump );

        if ( 0 == i ) /* light */
        {
            RpClumpForAllLights( clumpList[i].clump, FirstLightGetFrame, (void *)&frame );
            if ( NULL == frame )
            {
                return FALSE;
            }
        }
        else if ( 1 == i ) /* atomic */
        {
            RpClumpForAllAtomics( clumpList[i].clump, FirstAtomicGetFrame, (void *)&frame );
            if ( NULL == frame )
            {
                return FALSE;
            }
        }
        else
        {
            /* unsupported clump */
            return FALSE;
        }

        clumpList[i].hierarchy = RpHAnimFrameGetHierarchy( frame );
        if ( NULL == clumpList[i].hierarchy )
        {
            return FALSE;
        }

        RpHAnimHierarchyAttach( clumpList[i].hierarchy );
        RpHAnimHierarchySetCurrentAnim( clumpList[i].hierarchy, animList[i] );
        RpHAnimHierarchySetCurrentAnimTime( clumpList[i].hierarchy, 0.0f );
        (void)RpHAnimHierarchySetFlags( clumpList[i].hierarchy,
                                        RpHAnimHierarchyGetFlags( clumpList[i].hierarchy ) |
                                        rpHANIMHIERARCHYUPDATEMODELLINGMATRICES |
                                        rpHANIMHIERARCHYUPDATELTMS );
    }

    return TRUE;
}



/*
 *****************************************************************************
 */
static void
CameraInitPosition(RwCamera *camera, RpWorld *world)
{
    const RwBBox *bBox;
    const RwV2d *viewWindow;
    RwV3d pos, right, at;
    RwFrame *frame;
    RwReal size, distance, rTemp;
    
    /* How big's the world? */
    bBox = RpWorldGetBBox(world);
    RwV3dSub(&pos, &(bBox->sup), &(bBox->inf));
    size = RwV3dLength(&pos);
    
    /* Rotate the camera so it looks straight down... */
    frame = RwCameraGetFrame(camera);
    RwFrameSetIdentity(frame);
    right = *RwMatrixGetRight(RwFrameGetMatrix(frame));
    RwFrameRotate(frame, &right, 90.0f, rwCOMBINEREPLACE);
    
    /* Move it to the center of the world... */
    RwV3dSub(&pos, &(bBox->sup), &(bBox->inf));
    RwV3dScale(&pos, &pos, 0.5f);
    RwV3dAdd(&pos, &pos, &(bBox->inf));
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
    
    /* Back it up till it can see the whole world... */
    viewWindow = RwCameraGetViewWindow(camera);
    rTemp = viewWindow->x;
    if (viewWindow->y < rTemp) rTemp = viewWindow->y;
    distance = (1.0f*size) / rTemp;
    at = *RwMatrixGetAt(RwFrameGetMatrix(frame));
    RwV3dScale(&at, &at, -distance);
    RwFrameTranslate(frame, &at, rwCOMBINEPOSTCONCAT);
    
    /* Set the clip planes to give good precision... */
    RwCameraSetNearClipPlane(camera, RwRealMINVAL);
    RwCameraSetFarClipPlane( camera, (1.5f*size));
    RwCameraSetNearClipPlane(camera, (1.5f*size) / 1500.0f);
    
    return;
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
        RpWorldAddCamera(world, camera);
        CameraInitPosition( camera, world );

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics RWSView Example");

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

    /* load RWS assets */
    if ( FALSE == LoadRWSAssets() )
    {
        RsErrorMessage( RWSTRING( "Unable to load RWS assets." ) );

        return FALSE;
    }
   
    /* set up default assets */
    if ( FALSE == SetDefaultAssets() )
    {
        RsErrorMessage( RWSTRING( "Unable to set default assets." ) );

        return FALSE;
    }

    currentCamera = CreateCamera( worldList[0] );
    if( NULL == currentCamera )
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
    RsMetricsOpen( currentCamera );
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

    if( currentCamera )
    {
        RpWorldRemoveCamera( worldList[0], currentCamera);
        
        CameraDestroy(currentCamera);
    }

    /* destroy anims */
    if ( numAnims > 0 )
    {
        RwUInt32    i;


        for ( i = 0; i < numAnims; i += 1 )
        {
            RtAnimAnimationDestroy( animList[i] );
            animList[i] = (RtAnimAnimation *)NULL;
        }

        RwFree( animList );
    }
    
    /* destroy clumps */
    if ( numClumps > 0 )
    {
        RwUInt32    i;


        for ( i = 0; i < numClumps; i += 1 )
        {
            RpWorldRemoveClump( worldList[0], clumpList[i].clump );
            RpClumpDestroy( clumpList[i].clump );
            clumpList[i].clump = (RpClump *)NULL;
            clumpList[i].hierarchy = (RpHAnimHierarchy *)NULL;
        }

        RwFree( clumpList );
    }

    /* destroy the world */
    if ( numWorlds > 0 )
    {
        RwUInt32    i;


        for ( i = 0; i < numWorlds; i += 1 )
        {
            RpWorldDestroy( worldList[i] );
            worldList[i] = (RpWorld *)NULL;
        }

        RwFree( worldList );
    }

    /* destroy texture dictionaries */
    if ( numPITexDicts > 0 )
    {
        RwUInt32    i;


        RwTexDictionarySetCurrent( NULL );
        for ( i = 0; i < numPITexDicts; i += 1 )
        {
            RwTexDictionaryDestroy( texDictList[i] );
            texDictList[i] = (RwTexDictionary *)NULL;
        }

        RwFree( texDictList );
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

    if ( FALSE == RtAnimInitialize() )
    {
        return FALSE;
    }

    if ( FALSE == RpHAnimPluginAttach() )
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
    RwCameraClear( currentCamera,
                   &BackgroundColor,
                   rwCAMERACLEARZ | rwCAMERACLEARIMAGE );

    if ( NULL != RwCameraBeginUpdate( currentCamera ) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            RpWorldRender( worldList[0] );
            
            DisplayOnScreenInfo();
        }

        MenuRender( currentCamera, NULL );

#ifdef RWMETRICS
        RsMetricsRender();
#endif

        RwCameraEndUpdate( currentCamera );
    }

    /* 
     * Display camera's raster...
     */
    RsCameraShowRaster( currentCamera );

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

    RwUInt32    idx;


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
     * Update any animations here...
     */
    for ( idx = 0; idx < numClumps; idx += 1 )
    {
        RpHAnimHierarchyAddAnimTime( clumpList[idx].hierarchy, deltaTime );
        RpHAnimHierarchyUpdateMatrices( clumpList[idx].hierarchy );
    }

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
            CameraSize( currentCamera,
                        (RwRect *)param, 
                        DEFAULT_VIEWWINDOW,
                        DEFAULT_ASPECTRATIO );

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
