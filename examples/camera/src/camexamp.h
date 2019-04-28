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
 * camexamp.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics camera example.
 *          can be calculated.
 ****************************************************************************/


#ifndef CAMEXAMP_H
#define CAMEXAMP_H

#include "rwcore.h"
#include "rpworld.h"

typedef struct TextureCamera TextureCamera;
struct TextureCamera
{
    RwRaster *raster;
    RwRaster *zRaster;
    RwCamera *camera;
    RwTexture *texture;
};

typedef struct CameraData  CameraData;
struct CameraData
{
    RwReal farClipPlane;
    RwReal nearClipPlane;
    RwCameraProjection projection;
    RwV2d offset;
    RwV2d viewWindow;
    RwCamera *camera;
    TextureCamera *cameraTexture;
    RwMatrix *matrix;
};


enum CameraDataType
{
    NONE            = 0x00,
    FARCLIPPLANE    = 0x01,
    NEARCLIPPLANE   = 0x02,
    PROJECTION      = 0x04,
    OFFSET          = 0x08,
    VIEWWINDOW      = 0x10,
    MATRIX          = 0x20,
    ALL             = 0xFF,

    CAMERADATATYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

typedef enum CameraDataType CameraDataType;

extern RwUInt32 CameraSelected;
extern RwUInt32 ProjectionIndex;
extern RwBool SubCameraMiniView;

extern CameraData SubCameraData;


#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */


extern RwBool CameraSelectCallback(RwBool testEnable);
extern RwBool FarClipPlaneCallback(RwBool testEnable);
extern RwBool NearClipPlaneCallback(RwBool testEnable);
extern RwBool ProjectionCallback(RwBool testEnable);
extern RwBool ResetCameraAndClumpCallback(RwBool testEnable);

extern RwCamera *ChangeViewOffset(RwReal deltaX, RwReal deltaY);
extern RwCamera *ChangeViewWindow(RwReal deltaX, RwReal deltaY);

extern RwCamera *GetMainCamera(void);
extern RwCamera *GetSubCamera(void);

extern RwCamera *CamerasCreate(RpWorld *world);
extern RwBool CamerasDestroy(RpWorld *world);

extern void CameraSizeUpdate(RwRect *rect, RwReal viewWindow, RwReal aspectRatio);
extern void SubCameraMiniViewSelect(RwBool select);
extern void RenderSubCamera(RwRGBA *backgroundColor,
                            RwInt32 clearMode, RpWorld *world);
extern void RenderTextureCamera(RwRGBA *foregroundColor,
                                RwInt32 clearMode, RpWorld *world);
extern void LockRaster(RwBool lock);

extern TextureCamera *CameraTextureInit(TextureCamera *ct);
extern TextureCamera *CameraTextureTerm(TextureCamera *ct);

extern void DrawCameraFrustum(CameraData *c);
extern void DrawCameraViewplaneTexture(CameraData *c);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif /* CAMEXAMP_H */
