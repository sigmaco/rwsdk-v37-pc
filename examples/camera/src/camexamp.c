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
 * camexamp.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics camera example.
 *          can be calculated.
 ****************************************************************************/


#include "rwcore.h"
#include "rpworld.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif
#include "skeleton.h"
#include "menu.h"

#include "main.h"
#include "camexamp.h"
#include "viewer.h"


#if (defined(SKY))
#define __RWUNUSEDUNLESSSKY__ /* No op */
#endif /* (defined(SKY)) */

#if (!defined(__RWUNUSEDUNLESSSKY__))
#define __RWUNUSEDUNLESSSKY__ __RWUNUSED__
#endif /* (!defined(__RWUNUSEDUNLESSSKY__)) */

#define TEXSIZE (256)

static RwCamera *MainCamera = NULL;
static RwCamera *SubCamera = NULL;

static RwRaster *SubCameraRaster = NULL;
static RwRaster *SubCameraZRaster = NULL;
static RwRaster *SubCameraMainCameraSubRaster = NULL;
static RwRaster *SubCameraMainCameraSubZRaster = NULL;

static TextureCamera CameraTexture;

RwUInt32 CameraSelected = 0;
RwUInt32 ProjectionIndex = 0;
RwBool SubCameraMiniView = TRUE;

CameraData SubCameraData;


/*
 *****************************************************************************
 */
RwBool
CameraSelectCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    if( Rendering )
    {
        return FALSE;
    }

    switch( CameraSelected )
    {
        case 0:
        {
            SubCameraMiniViewSelect(TRUE);

            break;
        }

        case 1:
        {
            SubCameraMiniViewSelect(FALSE);

            break;
        }

        default:
        {
            break;
        }
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static CameraData *
CameraQueryData(CameraData *data, 
                CameraDataType dataType, 
                RwCamera *camera)
{
    /* lets remember the camera for updates */
    data->camera = camera;

    /* do anything else - no then lets return */
    if( dataType == NONE )
    {
        return data;
    }

    if( dataType & FARCLIPPLANE )
    {
        data->farClipPlane = RwCameraGetFarClipPlane(camera);
    }

    if( dataType & NEARCLIPPLANE )
    {
        data->nearClipPlane = RwCameraGetNearClipPlane(camera);
    }

    if( dataType & PROJECTION )
    {
        data->projection = RwCameraGetProjection(camera);
        switch(data->projection)
        {
            case rwPERSPECTIVE:
            {
                ProjectionIndex = 0;
                break;
            }

            case rwPARALLEL:
            {
                ProjectionIndex = 1;
                break;
            }

            default:
            {
                RsErrorMessage(RWSTRING(
                    "RwCameraGetProjection returned rwNACAMERAPROJECTION"));
            }
        }
    }

    if( dataType & OFFSET )
    {
        data->offset = *RwCameraGetViewOffset(camera);
    }

    if( dataType & VIEWWINDOW )
    {
        data->viewWindow = *RwCameraGetViewWindow(camera);
    }

    if( dataType & MATRIX )
    {
        data->matrix = RwFrameGetMatrix(RwCameraGetFrame(camera));
    }

    return data;
}


/*
 *****************************************************************************
 */
static CameraData *
CameraSetData(CameraData *data, CameraDataType dataType)
{
    if( dataType & FARCLIPPLANE )
    {
        RwCameraSetFarClipPlane(data->camera, data->farClipPlane);
    }

    if( dataType & NEARCLIPPLANE )
    {
        RwCameraSetNearClipPlane(data->camera, data->nearClipPlane);
    }

    if( dataType & PROJECTION )
    {
        RwCameraSetProjection(data->camera, data->projection);
    }

    if( dataType & OFFSET )
    {
        RwCameraSetViewOffset(data->camera, &(data->offset));
    }

    if( dataType & VIEWWINDOW )
    {
        RwCameraSetViewWindow(data->camera, &(data->viewWindow));
    }

    return data;
}

/*
 *****************************************************************************
 */
RwBool
FarClipPlaneCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    MenuSetRangeReal(&SubCameraData.nearClipPlane, 
        VIEWERNEARCLIPPLANEMIN,
        SubCameraData.farClipPlane-VIEWERNEARCLIPPLANESTEP,
        VIEWERNEARCLIPPLANESTEP);

    CameraSetData(&SubCameraData, FARCLIPPLANE);

    return TRUE;
}


/*
 *****************************************************************************
 */
RwBool
NearClipPlaneCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    MenuSetRangeReal(&SubCameraData.farClipPlane,
            SubCameraData.nearClipPlane+VIEWERFARCLIPPLANESTEP,
            VIEWERFARCLIPPLANEMAX, VIEWERFARCLIPPLANESTEP);

    CameraSetData(&SubCameraData, NEARCLIPPLANE);

    return TRUE;
}


/*
 *****************************************************************************
 */
RwBool
ProjectionCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    switch( ProjectionIndex )
    {
        case 0:
        {
            SubCameraData.projection = rwPERSPECTIVE;
            break;
        }

        case 1:
        {
            SubCameraData.projection = rwPARALLEL;
            break;
        }
    }

    CameraSetData(&SubCameraData, PROJECTION);

    return TRUE;
}


/*
 *****************************************************************************
 */
RwCamera *
ChangeViewOffset(RwReal deltaX, RwReal deltaY)
{
    SubCameraData.offset.x += deltaX;
    SubCameraData.offset.y += deltaY;

    if( SubCameraData.offset.x > 5.0f )
    {
        SubCameraData.offset.x = 5.0f;
    }

    if( SubCameraData.offset.x < -5.0f )
    {
        SubCameraData.offset.x = -5.0f;
    }

    if( SubCameraData.offset.y > 5.0f )
    {
        SubCameraData.offset.y = 5.0f;
    }

    if( SubCameraData.offset.y < -5.0f )
    {
        SubCameraData.offset.y = -5.0f;
    }

    CameraSetData(&SubCameraData, OFFSET);

    return SubCameraData.camera;
}


/*
 *****************************************************************************
 */
RwCamera *
ChangeViewWindow(RwReal deltaX, RwReal deltaY)
{
    SubCameraData.viewWindow.x += deltaX;
    SubCameraData.viewWindow.y += deltaY;

    if( SubCameraData.viewWindow.x > 5.0f )
    {
        SubCameraData.viewWindow.x = 5.0f;
    }

    if( SubCameraData.viewWindow.x < 0.01f )
    {
        SubCameraData.viewWindow.x = 0.01f;
    }

    if( SubCameraData.viewWindow.y > 5.0f )
    {
        SubCameraData.viewWindow.y = 5.0f;
    }

    if( SubCameraData.viewWindow.y < 0.01f )
    {
        SubCameraData.viewWindow.y = 0.01f;
    }

    CameraSetData(&SubCameraData, VIEWWINDOW);

    return SubCameraData.camera;
}


/*
 *****************************************************************************
 */
RwCamera *
CamerasCreate(RpWorld *world)
{
    RwV3d offset = { 3.0f, 0.0f, 8.0f };
    RwReal rotate = -90.0f;
  
    SubCamera = ViewerCreate(world);
    ViewerMove(SubCamera, &offset);
    ViewerRotate(SubCamera, rotate, 0.0f);

    MainCamera = ViewerCreate(world);

    CameraQueryData(&SubCameraData, ALL, SubCamera);

    SubCameraData.nearClipPlane = 0.3f;
    CameraSetData(&SubCameraData, NEARCLIPPLANE);

    SubCameraData.farClipPlane = 5.0f;
    CameraSetData(&SubCameraData, FARCLIPPLANE);

    CameraTexture.camera = SubCamera;
    CameraTextureInit(&CameraTexture);

    SubCameraData.cameraTexture = &CameraTexture;

    SubCameraMainCameraSubRaster = RwRasterCreate(0, 0, 0, rwRASTERTYPECAMERA);
    SubCameraMainCameraSubZRaster = RwRasterCreate(0, 0, 0, rwRASTERTYPEZBUFFER);

    return MainCamera;
}


/*
 *****************************************************************************
 */
RwBool
CamerasDestroy(RpWorld *world)
{
    SubCameraMiniViewSelect(FALSE);

    if( MainCamera )
    {
        ViewerDestroy(MainCamera, world);
    }

    if( SubCamera )
    {
        ViewerDestroy(SubCamera, world);
    }

    CameraTextureTerm(&CameraTexture);

    if( SubCameraMainCameraSubRaster )
    {
        RwRasterDestroy(SubCameraMainCameraSubRaster);
    }

    if( SubCameraMainCameraSubZRaster )
    {
        RwRasterDestroy(SubCameraMainCameraSubZRaster);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
RwCamera *
GetSubCamera(void)
{
    return SubCamera;
}


RwCamera *
GetMainCamera(void)
{
    return MainCamera;
}




/*
 *****************************************************************************
 */
static RwCamera *
UpdateSubRasters(RwCamera *camera, RwRect *rect)
{
    RwRect subRect;

    subRect.x = (RwInt32)(rect->w * 0.75f);
    subRect.y = 0;

    subRect.w = (RwInt32)(rect->w * 0.25f);
    subRect.h = (RwInt32)(rect->h * 0.25f);

    RwRasterSubRaster(SubCameraMainCameraSubRaster, 
        RwCameraGetRaster(camera), &subRect);
    
    RwRasterSubRaster(SubCameraMainCameraSubZRaster, 
        RwCameraGetZRaster(camera), &subRect);

    return camera;
}


/*
 *****************************************************************************
 */
void
CameraSizeUpdate(RwRect *rect, RwReal viewWindow, RwReal aspectRatio)
{
    static RwBool RasterInit = FALSE;

    ViewerSize(MainCamera, rect, viewWindow, aspectRatio);

    /* 
     * We need to wait for the first camera resize to 
     * update the sub-raster...
     */
    UpdateSubRasters(MainCamera, rect);

    /*
     * Switch the SubCamera raster back to the original raster...
     */
    if( RasterInit )
    {
        SubCameraMiniViewSelect(FALSE);
    }

    /*
     * Resize the SubCamera with its own raster...
     */
    ViewerSize(SubCamera, rect, viewWindow, aspectRatio);

    /* 
     * Remember the SubCameraSubRasters...
     */
    SubCameraRaster = RwCameraGetRaster(SubCamera);
    SubCameraZRaster = RwCameraGetZRaster(SubCamera);

    RasterInit = TRUE;

    /*
     * If we are using the SubCamera then lets set the SubCamera raster
     * back to the original SubCamera raster
     */
    /* If we are using the MainCamera then lets set the SubCamera sub-raster
     * to the MainCamera raster if the MiniView is on and the SubCamera raster
     * if the MiniView is off.
     */
    SubCameraMiniViewSelect(!CameraSelected);

    CameraQueryData(&SubCameraData, VIEWWINDOW, SubCamera);

    return;
}


/*
 *****************************************************************************
 */
void
RenderSubCamera(RwRGBA *backgroundColor, RwInt32 clearMode, RpWorld *world)
{
    RwCameraClear(SubCamera, backgroundColor, clearMode);

    if( RwCameraBeginUpdate(SubCamera) )
    {
        RpWorldRender(world);

        RwCameraEndUpdate(SubCamera);
    }

    return;
}


/*
 *****************************************************************************
 */
void
RenderTextureCamera(RwRGBA *foregroundColor, RwInt32 clearMode, RpWorld *world)
{ 
    RwRaster *saveRaster = NULL;
    RwRaster *saveZRaster = NULL;
    
    saveRaster = RwCameraGetRaster(CameraTexture.camera);
    saveZRaster = RwCameraGetZRaster(CameraTexture.camera);
    
    (void)RwCameraSetRaster(CameraTexture.camera, CameraTexture.raster); 
    (void)RwCameraSetZRaster(CameraTexture.camera, CameraTexture.zRaster); 

    RwCameraClear(CameraTexture.camera, foregroundColor, clearMode); 
 
    if( RwCameraBeginUpdate(CameraTexture.camera) ) 
    { 
        RpWorldRender(world); 
 
        RwCameraEndUpdate(CameraTexture.camera);
    }

#ifdef DOLPHIN
    RwGameCubeCameraTextureFlush(
        RwCameraGetRaster(SubCamera),
        FALSE);
#endif

    (void)RwCameraSetRaster(CameraTexture.camera, saveRaster); 
    (void)RwCameraSetZRaster(CameraTexture.camera, saveZRaster); 
     
    return; 
} 


/*
 *****************************************************************************
 */
void
SubCameraMiniViewSelect(RwBool select)
{
    if( select )
    {
        /* 
         * Change the SubCamera SubRaster to the MainCamera Raster...
         */
        RwCameraSetRaster(SubCamera, SubCameraMainCameraSubRaster);
        RwCameraSetZRaster(SubCamera, SubCameraMainCameraSubZRaster);
    }
    else
    {
        /* 
         * Return the SubCamera SubRaster to the SubCamera Raster...
         */
        RwCameraSetRaster(SubCamera, SubCameraRaster);
        RwCameraSetZRaster(SubCamera, SubCameraZRaster);
    }

    return;
}

/*
 *****************************************************************************
 */
void
LockRaster(RwBool lock __RWUNUSEDUNLESSSKY__)
{
#ifdef SKY
    skyTexCacheRasterLock(CameraTexture.raster, lock);
#endif

    return;
}    


/*
 *****************************************************************************
 */
TextureCamera * 
CameraTextureInit(TextureCamera *ct) 
{ 
    ct->raster = 
        RwRasterCreate(TEXSIZE, TEXSIZE, 0, rwRASTERTYPECAMERATEXTURE); 
 
    if( ct->raster ) 
    { 
        ct->zRaster = 
            RwRasterCreate(TEXSIZE, TEXSIZE, 0, rwRASTERTYPEZBUFFER); 
 
        if( ct->zRaster ) 
        { 
            RwTexture *checkTexture;
            
            /* 
             * Create a texture from this camera's raster so we 
             * can use it on the clump... 
             */ 
            ct->texture = RwTextureCreate(ct->raster); 
            checkTexture = RwTextureSetFilterMode(ct->texture, 
                                                  rwFILTERLINEAR); 

            if (checkTexture)
            {
                ct->texture = checkTexture;
            }
                    
            return ct; 
        }
        
        RwRasterDestroy(ct->raster); 
    } 
 
    return NULL; 
}


/*
 *****************************************************************************
 */
TextureCamera * 
CameraTextureTerm(TextureCamera *ct) 
{ 
    if( ct )
    {
        if( ct->raster )
        {
            RwRasterDestroy(ct->raster); 
        }

        if( ct->zRaster )
        {
            RwRasterDestroy(ct->zRaster); 
        }

        if( ct->texture )
        {
            RwTextureSetRaster(ct->texture, NULL);
            RwTextureDestroy(ct->texture);
        }
    }

    return ct; 
}


/*
 *****************************************************************************
 */
void
DrawCameraFrustum(CameraData *c)
{
    static RwRGBA yellow = {255, 255, 0,  64};
    static RwRGBA red    = {255,   0, 0, 255};

    /*
     * 0:                Camera origin (center of projection)
     * 1,  2,  3,  4:    View plane
     * 5,  6,  7,  8:    Near clip-plane
     * 9,  10, 11, 12:   Far clip-plane
     */
    RwIm3DVertex frustum[13];

    /* Line index */
    static RwImVertexIndex indicesL[] =
    {
        1,  2,  2,  3,  3,  4,  4,  1,
        5,  6,  6,  7,  7,  8,  8,  5,
        9, 10, 10, 11, 11, 12, 12,  9,
        5,  9,  6, 10,  7, 11,  8, 12,
        0,  0
    };

    /* Triangle index */
    static RwImVertexIndex indicesT[] =
    {
         5,  6, 10,
        10,  9,  5,
         6,  7, 11,
        11, 10,  6,
         7,  8, 12,
        12, 11,  7,
         8,  5,  9,
         9, 12,  8,

         7,  6,  5,
         5,  8,  7,
         9, 10, 11,
        11, 12,  9
    };

    static RwReal signs[4][2] = 
    {
        {  1,  1 },
        { -1,  1 },
        { -1, -1 },
        {  1, -1 }
    };

    RwInt32 i = 0;
    RwInt32 j = 0;
    RwInt32 k = 0;
    RwMatrix *LTM;

    /*
     * Ok we're going to draw a camera frustum.
     * All we need is:
     *    Vertices of the View plane,
     *    Vertices of the Near Clip plane,
     *    Vertices of the Far Clip plane,
     * then we're in business.
     */
    RwReal depth[3];
    depth[0] = 1.0f;
    depth[1] = c->nearClipPlane;
    depth[2] = c->farClipPlane;

    /* Origin */
    RwIm3DVertexSetPos(&frustum[k], c->offset.x, c->offset.y, 0.0f);
    k++;

    /* View Window */
    for(i=0; i<3; i++)
    {
        for(j=1; j<5; j++)
        {
            if( c->projection == rwPERSPECTIVE )
            {
                RwIm3DVertexSetPos(&frustum[k],
                    -c->offset.x + depth[i] * 
                        ((signs[j-1][0]*c->viewWindow.x) + c->offset.x),
                    c->offset.y + depth[i] * 
                        ((signs[j-1][1]*c->viewWindow.y) - c->offset.y),
                    depth[i]);
            }
            else if( c->projection == rwPARALLEL )
            {
                RwIm3DVertexSetPos(&frustum[k],
                    -c->offset.x + (signs[j-1][0] * c->viewWindow.x)
                        + (depth[i] * c->offset.x),
                    c->offset.y + (signs[j-1][1] * c->viewWindow.y)
                        - (depth[i] * c->offset.y),
                    depth[i]);
            }

            k++;
        }
    }

    /*
     * Set color & alpha for the lines...
     */
    for(i=0; i<5; i++)
    {
        RwIm3DVertexSetRGBA(&frustum[i], 
                            red.red, red.green, red.blue, red.alpha);
    }

    for(i=5; i<13; i++)
    {
        RwIm3DVertexSetRGBA(&frustum[i], 
                            yellow.red, yellow.green, yellow.blue, 255);
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

    LTM = RwFrameGetLTM(RwCameraGetFrame(c->camera));

    /*
     * Draw Lines...
     */
    if( RwIm3DTransform(frustum, 13, LTM, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, indicesL, 34);
        
        RwIm3DEnd();
    }

    /*
     * Set color & alpha for the triangles...
     */
    for (i=5; i<13; i++)
    {
        RwIm3DVertexSetRGBA(&frustum[i], 
            yellow.red, yellow.green, yellow.blue, yellow.alpha);
    }

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);

    /*
     * Draw triangles...
     */
    if( RwIm3DTransform(frustum, 13, LTM, 0) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indicesT, 36);
        
        RwIm3DEnd();
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

    return;
}


/*
 *****************************************************************************
 */
void
DrawCameraViewplaneTexture(CameraData *c)
{
    static RwRGBA white = {255, 255, 255, 128};
    
    /*
     * 0,  1,  2,  3:    View plane
     */
    RwIm3DVertex frustum[4];

    /* View plane index */
    static RwImVertexIndex indicesV[] =
    { 
        2, 1, 0,
        0, 3, 2,
        0, 1, 2,
        2, 3, 0
    }; 

    /* U V texture values */
    static RwReal uvValues[4][2] = 
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };

    static RwReal signs[4][2] = 
    {
        {  1,  1 },
        { -1,  1 },
        { -1, -1 },
        {  1, -1 }
    };

    RwInt32 i = 0;
    RwInt32 j = 0;
    RwInt32 k = 0;
    RwMatrix *LTM;

    /* View Window */
    for(j=1; j<5; j++)
    {
        RwIm3DVertexSetPos(&frustum[k],
            signs[j-1][0]*c->viewWindow.x,
            signs[j-1][1]*c->viewWindow.y,
            1.0f);

        k++;
    }

    for(i=0; i<4; i++)
    {
        RwIm3DVertexSetRGBA(&frustum[i], 
            white.red, white.green, white.blue, white.alpha);

        RwIm3DVertexSetU(&frustum[i], uvValues[i][0]);
        RwIm3DVertexSetV(&frustum[i], uvValues[i][1]);
    }

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 
        (void *)RwTextureGetRaster(c->cameraTexture->texture));

    LTM = RwFrameGetLTM(RwCameraGetFrame(c->camera));

    /*
     * Draw Lines and triangles...
     */
    if( RwIm3DTransform(frustum, 4, LTM, rwIM3D_VERTEXUV) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indicesV, 12);

        RwIm3DEnd();
    }

    return;
}


/*
 *****************************************************************************
 */
RwBool
ResetCameraAndClumpCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }
    else
    {
        RwV3d position = { 3.0f, 0.0f, 8.0f };
        RwV3d point    = { 0.0f, 0.0f, 8.0f };

        SubCameraData.nearClipPlane = 0.3f;
        SubCameraData.farClipPlane = 5.0f;
        SubCameraData.projection = rwPERSPECTIVE;
        SubCameraData.offset.x = 0.0f;
        SubCameraData.offset.y = 0.0f;
        SubCameraData.viewWindow.x = 0.5f;
        SubCameraData.viewWindow.y = 0.38f;

        CameraSetData(&SubCameraData, ALL);

        ProjectionIndex = 0;

        ViewerSetPosition(SubCameraData.camera, &position);
        ViewerRotate(SubCamera, -90.0f, 0.0f);

        ClumpSetPosition(Clump, &point);
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
