
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
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 * Reviewed by: John Irwin (with substantial edits).
 *
 * Purpose: RenderWare3 BSP viewer.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#if (defined(SKY2_DRVMODEL_H))
#include "rppds.h"
#endif /* (defined(SKY2_DRVMODEL_H)) */

#include <ctype.h> /* isprint */

#include "rpcollis.h"
#include "rppvs.h"
#include "rpltmap.h"
#include "rpmatfx.h"

#include "rprandom.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#if (defined (XBOX_DRVMODEL_H))
#include "rpanisot.h"
#include "rpnormmap.h"
#endif

#include "rtcharse.h"
#include "rtfsyst.h"
#include "rtworld.h"
#include "rtpng.h"
#include "rtbmp.h"
#include "rttiff.h"
#include "rtras.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "main.h"
#include "world.h"
#include "spline.h"
#include "render.h"
#include "pvsgen.h"
#include "movement.h"

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

#define DEGTORAD(x) ((x) * rwPI / 180.0f)

/*
 * Minimum distance between the near and far clip planes...
 */
#define MINNEARTOFAR (0.05f)

/*
 * Initial camera horizontal field-of-view in degrees...
 */
#define INITHFOV (60.0f)

/*
 * Initial camera tilt and turn in degrees...
 */
#define INITTILT (-30.0f)
#define INITTURN (30.0f)

/* 
 * Factors of the world's sphere radius which give the 
 * initial values of the far-clip plane, the length of the 
 * rendered normals, the camera translate sensitivity, and
 * the camera flying speed, respectively...
 */
#define CLIPFACTOR (2.0f)
#define NORMALSFACTOR (0.01f)
#define TRANSLATEFACTOR (0.001f)
#define SPEEDFACTOR (0.05f)

/*
 * Factors of the initial values which give the maximum 
 * allowable in the corresponding menu entries.
 * Minmium values are all 0.01f...
 */
#define MAXCLIPFACTOR (2.0f)
#define MAXNORMALSFACTOR (10.0f)
#define MAXTRANSLATEFACTOR (2.0f)
#define MAXSPEEDFACTOR (10.0f)

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static RtCharset *Charset = (RtCharset *)NULL;
static RwRaster *Pointer = (RwRaster *)NULL;

static RwReal FieldOfView = INITHFOV;
static RwReal CurrentViewWindow = 0.3f;
static RwReal NearClip = 1.0f;
static RwReal FarClip = 100.0f;

static RwBool OnScreenInfoOn = TRUE;
static RwBool HaveIm3DVertices;

static RwReal AmbientIntensity = 0.25f;
static RwReal MainIntensity = 0.75f;

static RwCullMode FaceCullMode;
static RwInt32 FaceCullIndex;
static RwBool NewFaceCullMode = FALSE;

static RwBool HaveTriStripIm3DVertices = FALSE;
static RwBool TriStripNoDegOn = FALSE;
static RwBool MeshesOn = FALSE;
static RwBool TriStripOn = FALSE;
static RwBool IsTriStrip = FALSE;

#ifdef SKY2_DRVMODEL_H
static RwBool TSClipperOn = FALSE;
#endif /* SKY2_DRVMODEL_H */

RwCamera *Camera = (RwCamera *)NULL;
RpLight *AmbientLight = (RpLight *)NULL;
RpLight *MainLight = (RpLight *)NULL;

RwInt32 NumCameraWorldSectors;
RwInt32 NumCameraTriangles;
RwInt32 NumPVSWorldSectors;
RwInt32 NumPVSTriangles;

RwReal NormalsScaleFactor;
RwReal TranslateScaleFactor;

RwBool AmbientLightOn = FALSE;
RwBool MainLightOn = FALSE;
extern RwBool WorldHasCollData;

static RwUInt32 LtMapRenderStyle = rpLTMAPSTYLERENDERBASE|rpLTMAPSTYLERENDERLIGHTMAP;
static RwBool   LtMapFiltering = FALSE;


/*
 *****************************************************************************
 */
static RpWorld *
CreateWorld(void)
{
    RpWorld *world = (RpWorld *)NULL;
    RwBBox bb;

    bb.inf.x = bb.inf.y = bb.inf.z = -10.0f;
    bb.sup.x = bb.sup.y = bb.sup.z = 10.0f;

    world = RpWorldCreate(&bb);
    if( world )
    {
        /* 
         * Determine the world's bounding-sphere to aid the
         * initial positioning of the camera...
         */
        WorldGetBoundingSphere(world, &WorldSphere);

        CurrentWorldSector = WorldGetFirstWorldSector(world);
        CurrentWorldSectorIndex = 1;
        SingleSectorOn = FALSE;

        /*
         * Adjust the size of the IMM vertex array, used for 
         * bounding-box/wire-frame/normals rendering...
         */
        if( ResizeIm3DVertexArray(world) )
        {
            HaveIm3DVertices = TRUE;
        }
        else
        {
            HaveIm3DVertices = FALSE;
        }
    }

    return world;
}


/*
 *****************************************************************************
 */
static RpLight *
CreateAmbientLight(void)
{
    RpLight *light = (RpLight *)NULL;

    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RwRGBAReal color;

        color.red = color.green = color.blue = AmbientIntensity;
        color.alpha = 1.0f;

        RpLightSetColor(light, &color);

        return light;
    }

    return (RpLight *)NULL;
}


/*
 *****************************************************************************
 */
static RpLight *
CreateMainLight(void)
{
    RpLight *light = (RpLight *)NULL;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    if( light )
    {
        RwFrame *frame = (RwFrame *)NULL;
        RwRGBAReal color;

        color.red = color.green = color.blue = MainIntensity;
        color.alpha = 1.0f;

        RpLightSetColor(light, &color);

        frame = RwFrameCreate();
        if( frame )
        {
            RwV3d xAxis = {1.0f, 0.0f, 0.0f};
            RwV3d yAxis = {0.0f, 1.0f, 0.0f};

            RwFrameRotate(frame, &xAxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &yAxis, 30.0f, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, frame);

            return light;
        }

        RpLightDestroy(light);
    }

    return (RpLight *)NULL;
}


/*
 *****************************************************************************
 */
static void
CameraReset(RwCamera *camera, RwSphere *worldSphere)
{
    RwFrame *cameraFrame, *baseFrame;
    RwMatrix *baseMatrix;
    RwV3d yAxis = {0.0f, 1.0f, 0.0f};

    /*
     * Set the camera's view window so that it gives
     * a HFoV of INITHFOV degrees...
     */
    CurrentViewWindow = (RwReal)RwTan(DEGTORAD(INITHFOV * 0.5f));
    RsEventHandler(rsCAMERASIZE, NULL);
    FieldOfView = INITHFOV;

    /*
     * Reset to stop asserts when setting in clip planes...
     */
    RwCameraSetNearClipPlane(camera, 1.0f);
    RwCameraSetFarClipPlane(camera, 10.0f);

    FarClip = WorldSphere.radius * CLIPFACTOR;
    if( FarClip < NearClip + MINNEARTOFAR )
    {
        FarClip = NearClip + MINNEARTOFAR;
    }

    NearClip = FarClip * 0.01f;
    if( NearClip < 0.01f )
    {
        NearClip = 0.01f;
    }

    RwCameraSetFarClipPlane(camera, FarClip);
    RwCameraSetNearClipPlane(camera, NearClip);

    /*
     * Make the camera's frame coincident with the base-frame...
     */
    cameraFrame = RwCameraGetFrame(camera);
    RwFrameSetIdentity(cameraFrame);

    baseFrame = RwFrameGetParent(cameraFrame);
    baseMatrix = RwFrameGetMatrix(baseFrame);

    if( WorldHasSpline )
    {
        /*
         * Position the camera at the start of the spline...
         */
        RpSplineFindFrame(WayPointSpline, 
            rpSPLINEPATHNICEENDS, 0.0f, &yAxis, baseMatrix);

        RwMatrixUpdate(baseMatrix);
        RwFrameUpdateObjects(baseFrame);

        SplinePos = 0.0f;
    }
    else
    {
        RwV3d *right, at;
        RwReal cameraDistance;

        /*
         * Orient the camera...
         */
        RwFrameSetIdentity(baseFrame);
        right = RwMatrixGetRight(baseMatrix);

        RwFrameRotate(baseFrame, right, -INITTILT, rwCOMBINEREPLACE);
        RwFrameRotate(baseFrame, &yAxis, INITTURN, rwCOMBINEPOSTCONCAT);

        /* ...and pull it back, from the center of the sphere, along the
         * -ve at-axis so that the world's bounding-sphere subtends 
         * twice the HFoV...
         */
        cameraDistance = 0.5f * worldSphere->radius / CurrentViewWindow;

        at = *RwMatrixGetAt(baseMatrix);
        RwV3dScale(&at, &at, -cameraDistance);
        RwV3dAdd(&at, &at, &worldSphere->center);
        RwFrameTranslate(baseFrame, &at, rwCOMBINEPOSTCONCAT);
    }

    TotalTilt = INITTILT;

    return;
}


/*
 *****************************************************************************
 */
static void
CameraResetFrame(RwCamera *camera)
{
    /*
     * Set the camera's frame coincident with the base frame...
     */
    RwFrameSetIdentity(RwCameraGetFrame(camera));

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
        RwFrame *baseFrame = (RwFrame *)NULL;

        /* 
         * Create a base-frame and make it the camera frame's parent.
         * When the world is loaded with a spline, the base-frame is
         * restricted to move along the spline but the camera is allowed
         * to move away from it relative to it. If no spline is available,
         * both frames remain coincident and all transforms are applied
         * to the base-frame...
         */
        baseFrame = RwFrameCreate();
        RwFrameAddChild(baseFrame, RwCameraGetFrame(camera));

        RpWorldAddCamera(world, camera);

        /*
         * Initialize the camera's position, orientation and
         * view frustum according to the initial (empty) world,
         * artificially increasing the world radius so that we can
         * see all of the world's bounding-box...
         */
        WorldSphere.radius *= 4.0f;
        CameraReset(camera, &WorldSphere);

        NormalsScaleFactor = WorldSphere.radius * NORMALSFACTOR;
        TranslateScaleFactor = WorldSphere.radius * TRANSLATEFACTOR;
        FlyingSpeed = WorldSphere.radius * SPEEDFACTOR;

        return camera;
    }

    return (RwCamera *)NULL;
}


/*
 *****************************************************************************
 */
static RwBool
FileLoad(RwChar *filename)
{
    RwChar *path = (char *)NULL;
    RwBool loaded = FALSE;
    RwChar message[256];

    path = RsPathnameCreate(filename);

    if( rwstrstr(path, RWSTRING(".bsp")) || rwstrstr(path, RWSTRING(".BSP")) )
    {
        loaded = LoadWorld(path);
    }
    else
    {
        RsSprintf(message, RWSTRING("The file %s is not a BSP file."), path);
        RsErrorMessage(message);

        RsPathnameDestroy(path);

        return FALSE;
    }

    if( loaded )
    {
        rwstrcpy(message, RsGlobal.appName);
        rwstrcat(message, RWSTRING(" - "));
        rwstrcat(message, path);
        RsWindowSetText(message);

        /*
         * Initialize the camera's view-frustum and position to give
         * a reasonable first view of the new world...
         */
        WorldGetBoundingSphere(World, &WorldSphere);
        CameraReset(Camera, &WorldSphere);

        /*
         * Adjust the clip-plane menu items to give a
         * reasonable range...
         */
        MenuSetRangeReal(&NearClip, 0.01f, MAXCLIPFACTOR * FarClip, 0.01f);
        MenuSetRangeReal(&FarClip, 0.01f, MAXCLIPFACTOR * FarClip, 0.01f);

        /*
         * Set the initial scale factor used in the rendering of the 
         * vertex normals...
         */
        NormalsScaleFactor = WorldSphere.radius * NORMALSFACTOR;
        MenuSetRangeReal(&NormalsScaleFactor, 0.01f, 
            MAXNORMALSFACTOR * NormalsScaleFactor, 0.01f);

        /*
         * Set the initial scale factor for translating the camera...
         */
        TranslateScaleFactor = WorldSphere.radius * TRANSLATEFACTOR;
        MenuSetRangeReal(&TranslateScaleFactor, 0.01f, 
            MAXTRANSLATEFACTOR * TranslateScaleFactor, 0.01f);

        /*
         * Set the initial flying speed...
         */
        FlyingSpeed = WorldHasSpline ? 0.01f : WorldSphere.radius * SPEEDFACTOR;
        MenuSetRangeReal(&FlyingSpeed, 0.01f, 
            MAXSPEEDFACTOR * FlyingSpeed, 0.01f);

        /*
         * Adjust the size of the IMM vertex array, used for 
         * bounding-box/wire-frame/normals rendering...
         */
        if( ResizeIm3DVertexArray(World) )
        {
            HaveIm3DVertices = TRUE;
        }
        else
        {
            rwstrcpy(message, RWSTRING("Out of memory: No im3D."));
            RsErrorMessage(message);

            HaveIm3DVertices = FALSE;
        }

        IsTriStrip = (RpWorldGetFlags(World) & rpWORLDTRISTRIP)? TRUE: FALSE;
        if( IsTriStrip )
        {
            if( ResizeTriStripVertexArray(World) )
            {
                HaveTriStripIm3DVertices = TRUE;
            }
            else
            {
                rwstrcpy(message,
                    RWSTRING("Out of memory: No TriStrip rendering."));

                RsErrorMessage(message);

                HaveTriStripIm3DVertices = FALSE;
            }
        }

        TrianglesOn = TRUE;
        WireFrameOn = FALSE;
        NormalsOn = FALSE;
        WorldSectorsOn = FALSE;
    }
    else
    {
        RsSprintf(message, RWSTRING("The file %s could not be loaded."), path);

        RsErrorMessage(message);
    }

    RsPathnameDestroy(path);

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
LoadStartupFile(void)
{
    void *fp = NULL;
    RwChar *path = (RwChar *)NULL;

    path = RsPathnameCreate(RWSTRING("./wrldview.ini"));
    fp = RwFopen(path, RWSTRING("r"));
    RsPathnameDestroy(path);

    if( fp )
    {
        RwChar buffer[256];

        while( RwFgets(buffer, 256, fp) )
        {
            /*
             * Lines may be commented out...
             */
            if( buffer[0] != '#' )
            {
                RwChar *src, *dst;


                /*
                 * Hopefully a BSP path...
                 */
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

                FileLoad(buffer);

                /*
                 * Only the first file is loaded...
                 */
                break;
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics World Viewer");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
FOVCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    CurrentViewWindow = (RwReal)RwTan(DEGTORAD(FieldOfView * 0.5f));

    RsEventHandler(rsCAMERASIZE, NULL);

    return TRUE;
}


static RwBool
NearClipCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    if( NearClip > FarClip - MINNEARTOFAR )
    {
        NearClip = FarClip - MINNEARTOFAR;
    }

    RwCameraSetNearClipPlane(Camera, NearClip);

    return TRUE;
}


static RwBool
FarClipCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    if( FarClip < NearClip + MINNEARTOFAR )
    {
        FarClip = NearClip + MINNEARTOFAR;
    }

    RwCameraSetFarClipPlane(Camera, FarClip);

    return TRUE;
}


static RwBool
ResetCameraCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    CameraReset(Camera, &WorldSphere);

    return TRUE;
}


static RwBool
ResetCameraFrameCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldHasSpline;
    }

    CameraResetFrame(Camera);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
TrianglesOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldLoaded;
    }

    return TRUE;
}


static RwBool
WireFrameOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldLoaded && HaveIm3DVertices;
    }

    return TRUE;
}


static RwBool
NormalsOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldLoaded && WorldHasNormals && HaveIm3DVertices;
    }

    return TRUE;
}


static RwBool
NormalsScaleFactorCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return NormalsOn;
    }

    return TRUE;
}


static RwBool
MeshesOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return IsTriStrip && HaveTriStripIm3DVertices;
    }

    TriStripOn = FALSE;
    TriStripNoDegOn = FALSE;
    
    return TRUE;
}

static RwBool
TriStripNoDegOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return IsTriStrip && HaveTriStripIm3DVertices;
    }

    TriStripOn = FALSE;
    MeshesOn = FALSE;

    return TRUE;
}

static RwBool
TriStripOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return IsTriStrip && HaveTriStripIm3DVertices;
    }

    TriStripNoDegOn = FALSE;
    MeshesOn = FALSE;

    return TRUE;
}

static RwBool
TriStripLengthCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return IsTriStrip && TriStripOn && HaveTriStripIm3DVertices;
    }

    return TRUE;
}


static RwBool 
WorldSectorsOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return HaveIm3DVertices;
    }

    return TRUE;
}


static RwBool 
SingleSectorOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return HaveIm3DVertices;
    }

    return TRUE;
}


static RwBool 
SplineOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldHasSpline;
    }

    return TRUE;
}


static RwBool 
FaceCullCallback(RwBool testEnable)
{
    if( testEnable )
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
DumpWorldCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldLoaded && !PVSGenerating;
    }

    if( !WorldHasCollData )
    {
        /* There should be no collision data, if there, destroy it -
         * it must have been generated by PVS! */
        if (RpCollisionWorldQueryData(World))
            RpCollisionWorldDestroyData(World);
    }

    SaveWorld();

    return TRUE;
}


static RwBool 
DumpTexDictCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldLoaded && WorldHasTextures && !PVSGenerating;
    }

    SaveTextureDictionary();

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
AmbientLightCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldLoaded;
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


static RwBool
AmbientLightIntensityCallback(RwBool testEnable)
{
    RwRGBAReal color;

    if( testEnable )
    {
        return AmbientLightOn;
    }

    color.red = color.green = color.blue = AmbientIntensity;
    color.alpha = 1.0f;

    RpLightSetColor(AmbientLight, &color);

    return TRUE;
}


static RwBool
MainLightCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldLoaded && WorldHasNormals;
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


static RwBool
MainLightIntensityCallback(RwBool testEnable)
{
    RwRGBAReal color;

    if( testEnable )
    {
        return MainLightOn;
    }

    color.red = color.green = color.blue = MainIntensity;
    color.alpha = 1.0f;

    RpLightSetColor(MainLight, &color);

    return TRUE;
}


#ifdef SKY2_DRVMODEL_H
static RwBool 
TSClipperCallBack(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    RpSkySelectTrueTSClipper(TSClipperOn);

    return TRUE;
}
#endif /* SKY2_DRVMODEL_H */

/*
 *****************************************************************************
 */
static RwBool
LtMapRenderStyleCallBack(RwBool justCheck)
{
    if( justCheck )
    {
        return(WorldHasLightmaps);
    }

    /* Toggle the current render style */
    RpLtMapSetRenderStyle(
        (LtMapRenderStyle & (rpLTMAPSTYLERENDERBASE|rpLTMAPSTYLERENDERLIGHTMAP)) |
        (LtMapFiltering ? 0 : rpLTMAPSTYLEPOINTSAMPLE),
        World);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool 
InitializeMenu(void)
{
    static RwChar resetCameraLabel[]       = RWSTRING("Reset camera_V");
    static RwChar resetCameraSplineLabel[] = RWSTRING("Reset on spline_C");
    static RwChar fovLabel[]               = RWSTRING("Field of view");
    static RwChar farClipLabel[]           = RWSTRING("Far clip plane");
    static RwChar nearClipLabel[]          = RWSTRING("Near clip plane");

#ifdef SKY2_DRVMODEL_H
    static RwChar tsClipperLabel[] = RWSTRING("Tristrip clipper");
#endif /* SKY2_DRVMODEL_H */

    static RwChar cameraFlyLabel[]  = RWSTRING("Camera fly_R");
    static RwChar flySpeedLabel[]   = RWSTRING("Flying speed");
    static RwChar transSensLabel[]  = RWSTRING("Translate sensitivity");

    static RwChar trianglesOnLabel[]    = RWSTRING("Triangles_T");
    static RwChar wireFrameOnLabel[]    = RWSTRING("Wire-frame_W");
    static RwChar normalsOnLabel[]      = RWSTRING("Normals_N");
    static RwChar normalScaleLabel[]    = RWSTRING("Scale normals");

    static RwChar meshOnLabel[] = RWSTRING("Mesh");
    
    static RwChar triStripNoDegOnLabel[] =
        RWSTRING("Tristrip without degenerates");

    static RwChar tristripOnLabel[]    = RWSTRING("Highlight tristrip");
    static RwChar triStripLengthLabel[] = RWSTRING("Tristrip length");

    static RwChar worldSectorsOnLabel[] = RWSTRING("World-sectors_B");
    static RwChar singleSectorOnLabel[] = RWSTRING("Single sector_S");
    static RwChar splineOnLabel[]       = RWSTRING("Spline");
    static RwChar faceCullLabel[]       = RWSTRING("Face culling");

    static RwChar generatePVSLabel[] = RWSTRING("Generate PVS_G");
    static RwChar repairPVSLabel[]   = RWSTRING("Repair PVS_R");
    static RwChar usePVSLabel[]      = RWSTRING("Use PVS_P");

    static RwChar dumpWorldLabel[]   = RWSTRING("Dump world");
    static RwChar dumpTexDictLabel[] = RWSTRING("Dump tex dict");

    static RwChar ambientLightLabel[]     = RWSTRING("Ambient light_A");
    static RwChar ambientIntensityLabel[] = RWSTRING("Ambient intensity");
    static RwChar mainLightLabel[]        = RWSTRING("Main light_M");
    static RwChar mainIntensityLabel[]    = RWSTRING("Main intensity");

    static RwChar styleLabel[]        = RWSTRING("LightMap Render Style_S");
    static const RwChar *styleNames[] = {RWSTRING("Base texture only"),
                                         RWSTRING("Lightmap only"),
                                         RWSTRING("Base & lightmap") };
    static RwChar sampleLabel[]       = RWSTRING("LightMap Filtering_P");
    
    static RwChar screenInfoLabel[] = RWSTRING("On screen info_I");
    static RwChar fpsLabel[]        = RWSTRING("FPS_F");

    static const RwChar *faceCullStrings[] = 
    {
        RWSTRING("none"),
        RWSTRING("back"),
        RWSTRING("front")
    };

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        /*
         * Camera frustum controls...
         */
        MenuAddEntryTrigger(resetCameraLabel, ResetCameraCallback);

        MenuAddEntryTrigger(resetCameraSplineLabel, ResetCameraFrameCallback);

        MenuAddEntryReal(fovLabel, &FieldOfView, 
            FOVCallback, 0.1f, 179.9f, 1.0f);

        MenuAddEntryReal(nearClipLabel, &NearClip, NearClipCallback, 
            0.01f, MAXCLIPFACTOR * FarClip, 0.01f);

        MenuAddEntryReal(farClipLabel, &FarClip, FarClipCallback, 
            0.01f, MAXCLIPFACTOR * FarClip, 0.01f);

        MenuAddSeparator();

#ifdef SKY2_DRVMODEL_H
        MenuAddEntryBool(tsClipperLabel, &TSClipperOn, TSClipperCallBack); 

        MenuAddSeparator();
#endif /* SKY2_DRVMODEL_H */
        
        /*
         * Camera movement controls....
         */
        MenuAddEntryBool(cameraFlyLabel, &CameraFlying, (MenuTriggerCallBack) NULL);

        MenuAddEntryReal(flySpeedLabel, &FlyingSpeed, 
                         (MenuTriggerCallBack)NULL, 
                         0.01f, MAXSPEEDFACTOR * FlyingSpeed, 0.01f);

        MenuAddEntryReal(transSensLabel, &TranslateScaleFactor,
                         (MenuTriggerCallBack)NULL, 
                         0.01f, MAXTRANSLATEFACTOR * TranslateScaleFactor, 0.01f);

        MenuAddSeparator();

        /*
         * Rendering controls...
         */
        MenuAddEntryBool(trianglesOnLabel, &TrianglesOn, TrianglesOnCallback);

        MenuAddEntryBool(wireFrameOnLabel, &WireFrameOn, WireFrameOnCallback);

        MenuAddEntryBool(normalsOnLabel, &NormalsOn, NormalsOnCallback);

        MenuAddEntryReal(normalScaleLabel, &NormalsScaleFactor, 
            NormalsScaleFactorCallback, 
            0.01f, MAXNORMALSFACTOR * NormalsScaleFactor, 0.01f);

        MenuAddEntryBool(meshOnLabel, &MeshesOn, MeshesOnCallback);

        MenuAddEntryBool(triStripNoDegOnLabel, &TriStripNoDegOn,
            TriStripNoDegOnCallback);

        MenuAddEntryBool(tristripOnLabel, &TriStripOn,
            TriStripOnCallback);

        MenuAddEntryInt(triStripLengthLabel, &TriStripLength,
            TriStripLengthCallback, 1, 100, 1, (const RwChar**)NULL);

        MenuAddEntryBool(worldSectorsOnLabel, 
            &WorldSectorsOn, WorldSectorsOnCallback);

        MenuAddEntryBool(singleSectorOnLabel, 
            &SingleSectorOn, SingleSectorOnCallback);

        MenuAddEntryBool(splineOnLabel, &SplineOn, SplineOnCallback);

        MenuAddEntryInt(faceCullLabel, &FaceCullIndex, 
            FaceCullCallback, 0, 2, 1, faceCullStrings);

        MenuAddSeparator();

        /*
         * PVS controls...
         */
        MenuAddEntryTrigger(generatePVSLabel, GeneratePVSCallback);

        MenuAddEntryTrigger(repairPVSLabel, RepairPVSCallback);

        MenuAddEntryBool(usePVSLabel, &PVSOn, PVSOnCallback);

        MenuAddSeparator();

        /*
         * Dump controls...
         */
        MenuAddEntryTrigger(dumpWorldLabel, DumpWorldCallback);

        MenuAddEntryTrigger(dumpTexDictLabel, DumpTexDictCallback);

        MenuAddSeparator();

        /*
         * Light controls...
         */
        MenuAddEntryBool(ambientLightLabel, &AmbientLightOn, 
            AmbientLightCallback);

        MenuAddEntryReal(ambientIntensityLabel, &AmbientIntensity,
            AmbientLightIntensityCallback, 0.0f, 1.0f, 0.05f);

        MenuAddEntryBool(mainLightLabel, &MainLightOn, MainLightCallback);

        MenuAddEntryReal(mainIntensityLabel, &MainIntensity,
            MainLightIntensityCallback, 0.0f, 1.0f, 0.05f);

        MenuAddSeparator();

        /*
         * LightMaps controls...
         */
        MenuAddEntryInt(styleLabel, (RwInt32 *)&LtMapRenderStyle, LtMapRenderStyleCallBack,
            rpLTMAPSTYLERENDERBASE,
            rpLTMAPSTYLERENDERBASE|rpLTMAPSTYLERENDERLIGHTMAP, 1, styleNames);
        MenuAddEntryBool(sampleLabel, &LtMapFiltering, LtMapRenderStyleCallBack);

        MenuAddSeparator();

        /*
         * Miscellaneous controls...
         */
        MenuAddEntryBool(screenInfoLabel, &OnScreenInfoOn, 
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

    World = CreateWorld();
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

    AmbientLight = CreateAmbientLight();
    if( AmbientLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create ambient light."));

        return FALSE;
    }

    MainLight = CreateMainLight();
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

    /*
     * Create a camera pointer, which aids world navigation...
     */
    Pointer = CameraCreateCrossHair();
    if( Pointer == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera pointer."));

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

    LoadStartupFile();

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

    if( Pointer )
    {
        RwRasterDestroy(Pointer);
    }

    if( Camera )
    {
        RwFrame *baseFrame = (RwFrame *)NULL;

        RpWorldRemoveCamera(World, Camera);

        baseFrame = RwFrameGetParent(RwCameraGetFrame(Camera));
        RwFrameRemoveChild(RwCameraGetFrame(Camera));
        RwFrameDestroy(baseFrame);

        CameraDestroy(Camera);
    }

    if( MainLight )
    {
        RwFrame *frame = (RwFrame *)NULL;

        if( MainLightOn )
        {
            RpWorldRemoveLight(World, MainLight);
        }

        frame = RpLightGetFrame(MainLight);
        if( frame )
        {
            RpLightSetFrame(MainLight, (RwFrame *)NULL);

            RwFrameDestroy(frame);
        }

        RpLightDestroy(MainLight);
    }

    if( AmbientLight )
    {
        if( AmbientLightOn )
        {
            RpWorldRemoveLight(World, AmbientLight);
        }

        RpLightDestroy(AmbientLight);
    }

    FreeIm3DVertices();

    if( HaveTriStripIm3DVertices )
    {
        FreeTriStripVertices();
    }

    DestroySpline();

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
     * Attach PVS plug-in...
     */
    if( !RpPVSPluginAttach() )
    {
        return FALSE;
    }

    /* 
     * Attach collision plug-in, used by PVS...
     */
    if( !RpCollisionPluginAttach() )
    {
        return FALSE;
    }

    /* 
     * Attach spline plug-in...
     */
    if( !RpSplinePluginAttach() )
    {
        return FALSE;
    }

    /* 
     * Attach material effects plug-in...
     */
    if( !RpMatFXPluginAttach() )
    {
        return FALSE;
    }

    if( !RpLtMapPluginAttach() )
    {
        return FALSE;
    }

#if (defined(SKY2_DRVMODEL_H))
    RpLtMapPipesAttach();
    RpMatfxPipesAttach();
#endif /* (defined(SKY2_DRVMODEL_H)) */

    if( !RpRandomPluginAttach() )
    {
        return FALSE;
    }

#if (defined (XBOX_DRVMODEL_H))
    /* 
     * Attach anisotrpic filtering plug-in...
     */
    if (!RpAnisotPluginAttach())
    {
        return FALSE;
    }

    /* 
     * Attach normal map plug-in...
     */
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
DisplayOnScreenInfo(RwCamera *camera)
{
    RwChar caption[256];

    if( PVSGenerating )
    {
        RsSprintf(caption, 
            RWSTRING("Generating PVS data : %3.1f%% completed."),
            PVSProgressDone);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSMIDDLE);
    }
    else
    {
        if( FPSOn )
        {
            RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

            RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
        }

        /*
         * Don't want on-screen info appearing when the menu is on...
         */
        if( OnScreenInfoOn && (MenuGetStatus() != MENUMODE) )
        {
            RwFrame *frame = (RwFrame *)NULL;
            RwV3d pos;

            if( SingleSectorOn )
            {
                RsSprintf(caption, RWSTRING("World sector ID: WS%03d"), 
                    CurrentWorldSectorIndex);

                RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPLEFT);

                RsSprintf(caption, RWSTRING("World sector vertices: %d"), 
                    RpWorldSectorGetNumVertices(CurrentWorldSector));

                RsCharsetPrint(Charset, caption, 0, 1, rsPRINTPOSTOPLEFT);

                RsSprintf(caption, RWSTRING("World sector triangles: %d"), 
                    RpWorldSectorGetNumTriangles(CurrentWorldSector));

                RsCharsetPrint(Charset, caption, 0, 2, rsPRINTPOSTOPLEFT);
            }
            else
            {
                RsSprintf(caption, RWSTRING("World sectors: %d/%d/%d"),
                    NumPVSWorldSectors, NumCameraWorldSectors,
                    RtWorldGetNumWorldSectors(World));

                RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPLEFT);

                RsSprintf(caption, RWSTRING("World triangles: %d/%d/%d"),
                    NumPVSTriangles, NumCameraTriangles,
                    RtWorldGetNumPolygons(World));

                RsCharsetPrint(Charset, caption, 0, 1, rsPRINTPOSTOPLEFT);
            }

            frame = RwCameraGetFrame(camera);
            pos = *RwMatrixGetPos(RwFrameGetLTM(frame));

            RsSprintf(caption, 
                RWSTRING("Camera pos: [%.1f, %.1f, %.1f]"), pos.x, pos.y, pos.z);

            RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSBOTTOMRIGHT);
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
void 
Render(void)
{
    static RwBool firstCall = TRUE;

    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

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
            if( World && !PVSGenerating )
            {
                WorldRender(World, Camera);

                if( WorldHasSpline && SplineOn )
                {
                    RenderSpline();
                }

                if( IsTriStrip )
                {
                    if( TriStripOn )
                    {
                        RpWorldForAllWorldSectors(World, RenderWorldSectorTriStrip,
                            (void *)TRUE);
                    }
                    else if( TriStripNoDegOn )
                    {
                        RpWorldForAllWorldSectors(World, RenderWorldSectorTriStrip,
                            (void *)FALSE);
                    }
                    else if( MeshesOn )
                    {
                        RpWorldForAllWorldSectors(World, RenderWorldSectorMesh, NULL);
                    }
                }
            }

            DisplayOnScreenInfo(Camera);
        }

        MenuRender(Camera, (RwRaster *)NULL);

#ifdef RWMETRICS
        RsMetricsRender();
#endif

        RwCameraEndUpdate(Camera);
    }

    if( MenuGetStatus() != HELPMODE && !PVSGenerating && 
        (CameraPointing || CameraTranslating || CameraFlying) )
    {
        RwRaster *camRas;

        camRas = RwCameraGetRaster(Camera);

        RwRasterPushContext(camRas);

        RwRasterRender(Pointer, 
            (RwRasterGetWidth(camRas) - RwRasterGetWidth(Pointer)) >> 1, 
            (RwRasterGetHeight(camRas) - RwRasterGetHeight(Pointer)) >> 1);

        RwRasterPopContext();
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

    deltaTime = (thisTime - lastAnimTime) * 0.001f;

    if( CameraFlying )
    {
        CameraFly(Camera, deltaTime);
    }

    if( CameraTranslating )
    {        
        TranslateCameraZ(CameraTranslateDelta);
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
            CameraSize(Camera, (RwRect *)param, 
                CurrentViewWindow, DEFAULT_ASPECTRATIO);

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

        case rsREGISTERIMAGELOADER:
        {
            /*
             * Register the image readers clump viewer will support...
             */
            if( !RwImageRegisterImageFormat(RWSTRING("ras"), RtRASImageRead,
                    0) )
            {
                return rsEVENTERROR;
            }

            if( !RwImageRegisterImageFormat(RWSTRING("tif"),
                    RtTIFFImageRead, 0) )
            {
                return rsEVENTERROR;
            }

            if( !RwImageRegisterImageFormat(RWSTRING("bmp"),
                    RtBMPImageRead, 0) )
            {
                return rsEVENTERROR;
            }

            if( !RwImageRegisterImageFormat(RWSTRING("png"),
                    RtPNGImageRead, 0) )
            {
                return rsEVENTERROR;
            }

            return rsEVENTPROCESSED;
        }

        case rsFILELOAD:
        {
            return FileLoad((RwChar *)param) ? rsEVENTPROCESSED : rsEVENTERROR;
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
