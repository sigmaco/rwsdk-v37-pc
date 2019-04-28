
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
 * Copyright (c) 2002 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * main.c
 *
 * Copyright (C) 2002 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics Tutorial Four (Writing a Plugin)
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rpcollis.h"
#include "rtcharse.h"
#include "rtpick.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#include "utils.h"

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

static RpAtomic *PickedAtomic = NULL;
static RwBBox PickBox;
static RwV2d OldPos;

static RwReal NearClip = 0.1f;
static RwReal FarClip = 30.0f;
static RwBool ShowPos = FALSE;

RpWorld *World = NULL;
RwCamera *Camera = NULL;
RtCharset *Charset = NULL;
RpLight *Ambient = NULL;
RpLight *Direct = NULL;
RwV2d MousePos = {0,0};

MouseMoveOperation MouseMoveAction = MMNoOperation;

/*
 *****************************************************************************
 */
static void
CreateLights(void)
{
    Ambient = RpLightCreate(rpLIGHTAMBIENT);

    if (Ambient)
    {
        RpWorldAddLight(World, Ambient);
    }

    Direct = RpLightCreate(rpLIGHTDIRECTIONAL);

    if (Direct)
    {
        RwFrame *f = RwFrameCreate();

        RpLightSetFrame(Direct, f);
        RpWorldAddLight(World, Direct);
    }
}

/*
 *****************************************************************************
 */
static void
DestroyLights()
{
    if (Direct)
    {
        RwFrame *f;

        RpWorldRemoveLight(World, Direct);
        f = RpLightGetFrame(Direct);
        RpLightSetFrame(Direct, NULL);
        RwFrameDestroy(f);
        RpLightDestroy(Direct);
        Direct = NULL;
    }

    if (Ambient)
    {
        RpWorldRemoveLight(World, Ambient);
        RpLightDestroy(Ambient);
        Ambient = NULL;
    }
}

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
        RwCameraSetNearClipPlane(camera, NearClip);
        RwCameraSetFarClipPlane(camera, FarClip);

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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Tutorial Four");

        return TRUE;
    }

    return FALSE;
}

/*
 *****************************************************************************
 */
/* InitializeMenu Function */
static RwBool 
InitializeMenu(void)
{    
    static RwChar fpsLabel[] = RWSTRING("FPS_F");
    static RwChar nearLabel[] = RWSTRING("Near Clip");
    static RwChar farLabel[] = RWSTRING("Far Clip");
    static RwChar showPosLabel[] = RWSTRING("Show Position");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryBool(fpsLabel, &FPSOn, NULL);

        MenuAddEntryReal(nearLabel, &NearClip, NULL, 0.1f, 10.0f, 0.1f);
        MenuAddEntryReal( farLabel, & FarClip, NULL, 10.0f, 100.0f, 1.0f);
        MenuAddEntryBool(showPosLabel, &ShowPos, NULL);

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

    /* Create an empty world if not loading a RWS... */

    World = CreateWorld();
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

    /* Create the lights */

    CreateLights();

    /* Create a camera using the democom way... */

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
static RpClump *
_destroyClumpCB(RpClump *c, void *d)
{
    RpWorldRemoveClump(World, c);
    RpClumpDestroy(c);

    return c;
}

/*
 *****************************************************************************
 */
static void 
Terminate3D(void)
{
    /* Close or destroy RenderWare components in the reverse order they */
    /* were opened or created... */

    RpWorldForAllClumps(World, _destroyClumpCB, NULL);

    DestroyLights();

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

    if (!RpCollisionPluginAttach() )
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
/* DisplayOnScreenInfo function */
static void 
DisplayOnScreenInfo(RwCamera *camera)
{
    RwChar caption[256];
    RtCharsetDesc charsetDesc;
    RwInt32 crw, crh;

    crw = RwRasterGetWidth(RwCameraGetRaster(camera));
    crh = RwRasterGetHeight(RwCameraGetRaster(camera));

    RtCharsetGetDesc(Charset, &charsetDesc);

    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint( Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT );
    }

    if (ShowPos)
    {
        if (PickedAtomic)
        {
            RwFrame *f = RpAtomicGetFrame(PickedAtomic);
            RwMatrix *m = RwFrameGetLTM(f);

            RsSprintf(caption, RWSTRING("%4.2f %4.2f %4.2f"), 
                m->pos.x, m->pos.y, m->pos.z);

            RsCharsetPrint( Charset, caption, 0, 2, rsPRINTPOSTOPRIGHT );
        }
    }
    return;
}

/*
 *****************************************************************************
 */
static void
HighlightRender(void)
{
    RwMatrix                  *ltm =
                                RwFrameGetLTM(RpAtomicGetFrame(PickedAtomic));
    
    RwIm3DVertex              vertices[8];
    
    RwInt32                   i;
    
    static RwImVertexIndex    indices[24] = { 0, 1, 1, 3, 3, 2, 2, 0,
                                              4, 5, 5, 7, 7, 6, 6, 4,
                                              0, 4, 1, 5, 2, 6, 3, 7 };
    
    for (i = 0; i < 8; i++)
    {
        RwIm3DVertexSetPos(vertices+i,
            i&1 ? PickBox.sup.x : PickBox.inf.x,
            i&2 ? PickBox.sup.y : PickBox.inf.y,
            i&4 ? PickBox.sup.z : PickBox.inf.z);
        RwIm3DVertexSetRGBA(vertices+i, 255, 0, 0, 255); 
    }

    /* all vertices have opaque colors so we can use the rwIM3D_ALLOPAQUE
     * hint for RwIm3DTransform */
    if (RwIm3DTransform(vertices, 8, ltm, rwIM3D_ALLOPAQUE))
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, indices, 24);
        RwIm3DEnd();
    }
}

/*************************************************************************/
/* RenderVoid */
static void 
Render(void)
{
    RwCameraSetNearClipPlane(Camera, NearClip);
    RwCameraSetFarClipPlane(Camera, FarClip);

    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            /*
             * Scene rendering here...
             */
            RpWorldRender(World);

            DisplayOnScreenInfo(Camera);

            if (PickedAtomic)
            {
                HighlightRender();
            }
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
/* ZTranslateAtomic function */
static void
ZTranslateAtomic(void)
{
    RwReal dy;
    RwFrame *f;
    RwV3d at;

    f = RwCameraGetFrame(Camera);
    dy = OldPos.y - MousePos.y;

    at = *RwMatrixGetAt(RwFrameGetMatrix(f));

    RwV3dScale(&at, &at, dy*0.01f);

    f = RpAtomicGetFrame(PickedAtomic);
    RwFrameTranslate(f, &at, rwCOMBINEPOSTCONCAT);

    OldPos = MousePos;
}

/*
 *****************************************************************************
 */
static void
TranslateAtomic(void)
{
    RwReal dx, dy;
    RwFrame *f;
    RwV3d up, right, trans;

    f = RwCameraGetFrame(Camera);
    dx = OldPos.x - MousePos.x;
    dy = OldPos.y - MousePos.y;

    right = *RwMatrixGetRight(RwFrameGetMatrix(f));
    up = *RwMatrixGetUp(RwFrameGetMatrix(f));

    RwV3dScale(&right, &right, dx*0.01f);
    RwV3dScale(&up, &up, dy*0.01f);
    RwV3dAdd(&trans, &up, &right);

    f = RpAtomicGetFrame(PickedAtomic);
    RwFrameTranslate(f, &trans, rwCOMBINEPOSTCONCAT);

    OldPos = MousePos;
}

/*
 *****************************************************************************
 */
static void
RotateAtomic(void)
{
    RwFrame  *f;
    RwMatrix *worldToLocal, *tmpMatrix;
    RwV3d     up, right;
    RwReal    dx, dy;
 
    f     =  RwCameraGetFrame(Camera);
    right = *RwMatrixGetRight(RwFrameGetMatrix(f));
    up    = *RwMatrixGetUp(RwFrameGetMatrix(f));

    dx = OldPos.x - MousePos.x;
    dy = OldPos.y - MousePos.y;

    f            = RpAtomicGetFrame(PickedAtomic);
    worldToLocal = RwMatrixCreate();
    tmpMatrix    = RwFrameGetLTM(f);
    RwMatrixInvert(worldToLocal, tmpMatrix);

    RwV3dTransformVector(&up, &up, worldToLocal);
    RwV3dNormalize(&up, &up);
    RwV3dTransformVector(&right, &right, worldToLocal);
    RwV3dNormalize(&right, &right);

    RwMatrixDestroy(worldToLocal);

    /* Apply the rotations */
    RwFrameRotate(f, &up,    -dx*0.5f, rwCOMBINEPRECONCAT);
    RwFrameRotate(f, &right,  dy*0.5f, rwCOMBINEPRECONCAT);

    OldPos = MousePos;
}

/*
 *****************************************************************************
 */
/* PickAtomic function */
static void
PickAtomic(MouseMoveOperation nextOp)
{
    PickedAtomic = RwCameraPickAtomicOnPixel(Camera, &MousePos);

    if (PickedAtomic)
    {
        AtomicGetBBox(PickedAtomic, &PickBox);
        OldPos = MousePos;
        MouseMoveAction = nextOp;
    }
    else
    {
        MouseMoveAction = MMNoOperation;
    }
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
     * Update any animations here...
     */

    lastAnimTime = thisTime;

    switch(MouseMoveAction)
    {
    case MMPickAndTranslateObject :
        PickAtomic(MMTranslateObject);
        break;

    case MMTranslateObject :
        TranslateAtomic();
        break;

    case MMPickAndRotateObject :
        PickAtomic(MMRotateObject);
        break;

    case MMRotateObject :
        RotateAtomic();
        break;

    case MMPickAndZTranslateObject :
        PickAtomic(MMZTranslateObject);
        break;

    case MMZTranslateObject :
        ZTranslateAtomic();
        break;

    default:
        PickedAtomic = NULL;
        break;
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

        case rsFILELOAD:
        {
            RwChar  *s     = (RwChar *)param;
            RpClump *clump = DffLoad(s);

            if (clump)
            {
                RwV3d v = {0.0f, 0.0f, 5.0f};

                RwFrameTranslate(RpClumpGetFrame(clump), &v, rwCOMBINEREPLACE);
                RpWorldAddClump(World, clump);
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
