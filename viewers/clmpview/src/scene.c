
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
 * scene.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Scene management.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"
#include "camera.h"

#include "main.h"

#include "clmpcntl.h"
#include "clmphanm.h"
#include "clmpskin.h"
#include "clmpdmrf.h"
#include "clmpview.h"
#include "clmppick.h"
#include "clmprndr.h"

#include "scene.h"

#define INITHFOV (90.0f)

#define DegToRad(x) ( (x) * ( rwPI / 180.0f ) )

RpWorld *World = (RpWorld *)NULL;

RwCamera *Camera = (RwCamera *)NULL;
RwReal NearClip = NEARMINCLIP;
RwReal FarClip = FARMAXCLIP;
RwReal FieldOfView = INITHFOV;

RwReal CurrentViewWindow = 0.3f;

RpLight *AmbientLight = (RpLight *)NULL;
RwReal AmbientIntensity = 0.75f;

RpLight *MainLight = (RpLight *)NULL;
RwReal MainIntensity = 1.0f;

RwBool AmbientLightOn = TRUE;
RwBool MainLightOn = FALSE;

RwRGBA TopColor = { 0, 0, 0, 255 };
RwRGBA BottomColor = { 96, 96, 96, 255 };

static RwV3d Xaxis = { 1.0f, 0.0f, 0.0f };
static RwV3d Yaxis = { 0.0f, 1.0f, 0.0f };


/*
 *****************************************************************************
 */
static RpLight*
CreateMainLight(RpWorld * world __RWUNUSED__)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);

    if( light )
    {
        RwFrame *frame;
        RwRGBAReal color;

        color.red = color.green = color.blue = MainIntensity;
        color.alpha = 1.0f;

        RpLightSetColor(light, &color);

        frame = RwFrameCreate();

        if( frame )
        {
            RwFrameRotate(frame, &Xaxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &Yaxis, -150.0f, rwCOMBINEPOSTCONCAT);

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
static RpLight*
CreateAmbientLight(RpWorld *world)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTAMBIENT);

    if( light )
    {
        RwRGBAReal color;

        color.red = color.green = color.blue = AmbientIntensity;
        color.alpha = 1.0f;

        RpLightSetColor(light, &color);

        RpWorldAddLight(world, light);

        return light;
    }

    return (RpLight *)NULL;
}


/*
 *****************************************************************************
 */
static RpWorld*
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
static RwCamera*
CreateCamera(RpWorld *world)
{
    RwCamera *camera;

    /*
     * Create a camera
     */
    camera = CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);

    if( camera )
    {
        /*
         * Add the camera to the world
         */
        RpWorldAddCamera(world, camera);

        return camera;
    }

    return (RwCamera *)NULL;
}


/*
 *****************************************************************************
 */
RwBool
SceneInit(void)
{
    /*
     * Create an empty world
     */
    World = CreateWorld();
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

    /*
     * Create an Ambient Light
     */
    AmbientLight = CreateAmbientLight(World);
    if( AmbientLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create Ambient Light."));
        return FALSE;
    }

    /*
     * Create Main Light
     */
    MainLight = CreateMainLight(World);
    if( MainLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create Ambient Light."));
        return FALSE;
    }

    /*
     * Create a camera using the democom way...
     */
    Camera = CreateCamera(World);
    if ( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));
        return FALSE;
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
void
SceneCameraUpdate(void)
{
    static RwReal PreviousFieldOfView = -1;

    /*
     * Update field of view if necessary
     */
    if( FieldOfView != PreviousFieldOfView )
    {
        CurrentViewWindow = (RwReal)RwTan(DegToRad(FieldOfView * 0.5f));
        RsEventHandler(rsCAMERASIZE, NULL);
        PreviousFieldOfView = FieldOfView;
    }

    /*
     * Update near clipping plane if necessary
     */
    if( NearClip != RwCameraGetNearClipPlane(Camera) )
    {
        RwCameraSetNearClipPlane(Camera, NearClip);
        MenuSetRangeReal(&FarClip,
                         NearClip + MINNEARTOFARCLIP,
                         FARMAXCLIP, 1.0f);

        if( FarClip < NearClip + MINNEARTOFARCLIP )
        {
            FarClip = NearClip + MINNEARTOFARCLIP;
        }
    }

    /*
     * Update far clipping plane if necessary
     */
    if( FarClip != RwCameraGetFarClipPlane(Camera) )
    {
        RwCameraSetFarClipPlane(Camera, FarClip);
        MenuSetRangeReal(&NearClip,
                         NEARMINCLIP,
                         FarClip - MINNEARTOFARCLIP, 1.0f);

        if( NearClip > FarClip + MINNEARTOFARCLIP )
        {
            NearClip = NearClip + MINNEARTOFARCLIP;
        }
    }
    return;
}


/*
 *****************************************************************************
 */
void
SceneLightsUpdate(void)
{
    static RwReal PrevAmbientIntensity = 0.0f;
    static RwReal PrevMainIntensity = 0.0f;

    if( AmbientIntensity != PrevAmbientIntensity )
    {
        RwRGBAReal color;

        color.red = color.green = color.blue = AmbientIntensity;
        color.alpha = 1.0f;

        PrevAmbientIntensity = AmbientIntensity;

        RpLightSetColor(AmbientLight, &color);

        PrevMainIntensity = MainIntensity;
    }

    if( MainIntensity != PrevMainIntensity )
    {
        RwRGBAReal color;

        color.red = color.green = color.blue = MainIntensity;
        color.alpha = 1.0f;

        RpLightSetColor(MainLight, &color);

        PrevMainIntensity = MainIntensity;
    }
    return;
}

/*
 *****************************************************************************
 */
RwBool
HandleFileLoad(void *param)
{
    RwChar *path = (RwChar *)NULL;
    RwBool loaded = FALSE;

    path = RsPathnameCreate((RwChar *) param);

    if( rwstrstr(path, RWSTRING (".dff")) ||
        rwstrstr(path, RWSTRING (".DFF")) )
    {
        loaded = ClumpLoadDFF(path);
    }
    else if( rwstrstr(path, RWSTRING(".ska")) ||
             rwstrstr(path, RWSTRING(".SKA")) )
    {
        if( Clump )
        {
            loaded = SkinLoadSKA(Clump, path);
        }
    }
    else if( rwstrstr(path, RWSTRING(".dma")) ||
             rwstrstr(path, RWSTRING(".DMA")) )
    {
        if( Clump )
        {
            loaded = DMorphLoadDMA(Clump, path);
        }
    }
    else if( rwstrstr(path, RWSTRING (".anm")) ||
             rwstrstr(path, RWSTRING (".ANM")) )
    {
        if( Clump )
        {
            loaded = HAnimLoadANM(Clump, path);
        }
    }
    else if( rwstrstr(path, RWSTRING (".bsp")) ||
             rwstrstr(path, RWSTRING (".BSP")) )
    {
        RsErrorMessage(
            RWSTRING("This is a BSP file,\nPlease use World View(wrldview)."));
        loaded = FALSE;
    }

    if( loaded == TRUE )
    {
        RwChar windowText[1024];

        rwstrcpy(windowText, RsGlobal.appName);
        rwstrcat(windowText, RWSTRING (" - "));
        rwstrcat(windowText, path);

        RsWindowSetText(windowText);
    }

    RsPathnameDestroy(path);

    SceneCameraReset();

    ClumpControlReset();

    /* RpPatchSetAtomicLODRange( (RwReal)2.0, (RwReal)10.0);  */

    return TRUE;
}

/*
 *****************************************************************************
 */
void
SceneDestroy(void)
{
    RwFrame *frame;
    RpWorld *world;

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        /*
         * This assumes the camera was created the democom way...
         */
        CameraDestroy(Camera);
    }

    /*
     * Remove & destroy lights
     */
    world = RpLightGetWorld(AmbientLight);

    if( world )
    {
        RpWorldRemoveLight(world, AmbientLight);
    }

    world = RpLightGetWorld(MainLight);

    if( world )
    {
        RpWorldRemoveLight(world, MainLight);
    }

    RpLightDestroy(AmbientLight);

    frame = RpLightGetFrame(MainLight);

    RpLightDestroy(MainLight);

    if( frame )
    {
        RwFrameDestroy(frame);
    }

    /*
     * Remove & destroy Clump
     */
    ClumpViewTerminate();

    /*
     * Destroy the world
     */
    if( World )
    {
        RpWorldDestroy(World);
    }

    return;
}

/*
 *****************************************************************************
 */
void
SceneCameraReset(void)
{
    RwFrame *cameraFrame;
    RwMatrix *matrix;
    RwReal cameraDistance;
    RwV3d temp;

    /*
     * Set the camera's view window so that it gives
     * a *horizontal* field of view of INITHFOV degrees...
     */
    FieldOfView = INITHFOV;
    CurrentViewWindow = (RwReal)RwTan(DegToRad(FieldOfView * 0.5f));
    RsEventHandler(rsCAMERASIZE, NULL);

    /*
     * Set far and near clipping planes
     */
    FarClip = FARMAXCLIP;

    NearClip = NEARMINCLIP;
    RwCameraSetNearClipPlane(Camera, NearClip);
    RwCameraSetFarClipPlane(Camera, FarClip);

    /*
     * Initialise the camera position
     */
    cameraFrame = RwCameraGetFrame(Camera);

    RwFrameSetIdentity(cameraFrame);
    RwFrameRotate(cameraFrame, &Yaxis, 180.0f, rwCOMBINEREPLACE);
    RwFrameTranslate(cameraFrame, &ClumpSphere.center, rwCOMBINEPOSTCONCAT);

    /*
     * Pull the camera backwards...
     */
    cameraDistance = 2.0f * ClumpSphere.radius / CurrentViewWindow;

    matrix = RwFrameGetLTM(cameraFrame);
    RwV3dScale(&temp, RwMatrixGetAt(matrix), -cameraDistance);
    RwFrameTranslate(cameraFrame, &temp, rwCOMBINEPOSTCONCAT);

    if( cameraDistance )
    {
        FarClip = 5.0f * cameraDistance;
    }

    SpinOn = FALSE;
}

#if (defined(RENDERBACKGROUND))
/*
 *****************************************************************************
 */
static void
RenderBackground (void)
{
    RwIm2DVertex vertex[4];
    RwReal crw, crh;

    /*
     * Render the cool Background using 2D immediates
     */

    RwRaster *camRas = RwCameraGetRaster(Camera);

    crw = (RwReal)RwRasterGetWidth(camRas);
    crh = (RwReal)RwRasterGetHeight(camRas);

    /*
     *  Set top left vertex position
     */
    RwIm2DVertexSetScreenX(&vertex[0], 0.0f);
    RwIm2DVertexSetScreenY(&vertex[0], 0.0f);

    /*
     *  Set ScreenZ in order to get the background behind our scene
     */
    RwIm2DVertexSetScreenZ(&vertex[0], RwIm2DGetFarScreenZ());
    RwIm2DVertexSetRecipCameraZ(&vertex[0],
        1.0f / RwCameraGetFarClipPlane(Camera));

    /*
     *  Set color
     */
    RwIm2DVertexSetIntRGBA(&vertex[0],
        TopColor.red, TopColor.green, TopColor.blue, TopColor.alpha);

    /*
     *  Set bottom left vertex position
     */
    RwIm2DVertexSetScreenX(&vertex[1], 0.0f);
    RwIm2DVertexSetScreenY(&vertex[1], crh);
    RwIm2DVertexSetScreenZ(&vertex[1], RwIm2DGetFarScreenZ ());
    RwIm2DVertexSetRecipCameraZ(&vertex[1],
        1.0f / RwCameraGetFarClipPlane (Camera));
    RwIm2DVertexSetIntRGBA(&vertex[1],
        BottomColor.red, BottomColor.green, BottomColor.blue, BottomColor.alpha);

    /*
     *  Set top right vertex position
     */
    RwIm2DVertexSetScreenX(&vertex[2], crw);
    RwIm2DVertexSetScreenY(&vertex[2], 0.0f);
    RwIm2DVertexSetScreenZ(&vertex[2], RwIm2DGetFarScreenZ() );
    RwIm2DVertexSetRecipCameraZ(&vertex[2],
        1.0f / RwCameraGetFarClipPlane(Camera));
    RwIm2DVertexSetIntRGBA(&vertex[2],
        TopColor.red, TopColor.green, TopColor.blue, TopColor.alpha);

    /*
     *  Set bottom right vertex position
     */
    RwIm2DVertexSetScreenX(&vertex[3], crw);
    RwIm2DVertexSetScreenY(&vertex[3], crh);
    RwIm2DVertexSetScreenZ(&vertex[3], RwIm2DGetFarScreenZ ());
    RwIm2DVertexSetRecipCameraZ(&vertex[3],
        1.0f / RwCameraGetFarClipPlane(Camera));
    RwIm2DVertexSetIntRGBA(&vertex[3],
        BottomColor.red, BottomColor.green, BottomColor.blue, BottomColor.alpha);

    /*
     * Set Render State : No Texture, Gouraud Shading, No Test Z, Write Z
     */
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

    /*
     * Send vertices
     */
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, vertex, 4);

    return;
}
#endif /* (defined(RENDERBACKGROUND)) */

/*
 *****************************************************************************
 */
void
SceneRender (void)
{
#if (defined(RENDERBACKGROUND))
    RenderBackground();
#endif /* (defined(RENDERBACKGROUND)) */

    /*
     * Render clump
     */
    if( ClumpLoaded )
    {
        if( RenderMode == RENDERSOLID || RenderMode == RENDERWIRESOLID ||
            RenderMode == RENDERSOLIDSKEL || RenderMode == RENDERALL )
        {
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

            RpClumpRender(Clump);
        }
        else
        {
            if( LodRoot )
            {
                RpLODAtomicSelectLOD(LodRoot);
            }
        }

        if( RenderMode == RENDERSKEL || RenderMode == RENDERWIRESKEL ||
            RenderMode == RENDERSOLIDSKEL || RenderMode == RENDERALL )
        {
            RpClumpForAllAtomics (Clump, AtomicRenderSkeleton, NULL);
        }

        if( RenderMode == RENDERWIRE || RenderMode == RENDERWIRESKEL ||
            RenderMode == RENDERWIRESOLID || RenderMode == RENDERALL )
        {
            RpClumpForAllAtomics (Clump, AtomicRenderWireMesh, NULL);
        }

        if( RenderMode == RENDERTRISTRIP )
        {
            RpClumpForAllAtomics (Clump, AtomicRenderTriStrip, (void *)TRUE);
        }

        if( RenderMode == RENDERTRISTRIPS )
        {
            RpClumpForAllAtomics (Clump, AtomicRenderTriStrip, (void *)FALSE);
        }

        if( RenderMode == RENDERMESHES )
        {
            RpClumpForAllAtomics (Clump, AtomicRenderMeshes, NULL);
        }

        if( NormalsOn )
            RpClumpForAllAtomics(Clump, AtomicRenderVertexNormals, NULL);

        if( AtomicSelected )
        {
            AtomicRenderBoundingBox(AtomicSelected, &CurrentAtomicBBox);
        }

    }
}

/*
 *****************************************************************************
 */
