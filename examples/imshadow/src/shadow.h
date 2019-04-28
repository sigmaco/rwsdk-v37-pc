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
 * shadow.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example of parallel-projection shadow rendering using
 *          3D immediate mode.
 *
*****************************************************************************/

#ifndef SHADOW_H
#define SHADOW_H

extern RwUInt32     ShadowNumTriangles;

/*
 *  Shadow camera
 */
extern RwCamera    *ShadowCameraCreate(RwInt32 size);

extern void         ShadowCameraDestroy(RwCamera *shadowCamera);

extern RwCamera    *ShadowCameraSetFrustum(RwCamera *shadowCamera, 
                                           RwReal objectRadius);

extern RwCamera    *ShadowCameraSetLight(RwCamera *shadowCamera, 
                                         RpLight *light);

extern RwCamera    *ShadowCameraSetCenter(RwCamera *shadowCamera, 
                                          RwV3d *center);

extern RwCamera    *ShadowCameraUpdate(RwCamera *shadowCamera,
                                       RpClump *clump,
                                       RwBool invertRaster);

/*
 *  Shadow raster
 */
extern RwRaster    *ShadowRasterCreate(RwUInt32 size);

extern void         ShadowRasterDestroy(RwRaster *shadowRaster);

extern RwBool       ShadowRasterResample(RwRaster *destRaster,
                                         RwRaster *sourceRaster,
                                         RwCamera *camera);

extern RwBool       ShadowRasterBlur(RwRaster *shadowRaster,
                                     RwRaster *tempRaster,
                                     RwCamera *camera,
                                     RwUInt32 numPass);

extern RwRaster    *ShadowRasterRender(RwRaster *shadowRaster,
                                       RwV2d *verts);

extern RwBool       ShadowRasterFade(RwCamera *camera, RwRaster *raster,
                                     RwReal shadowStrength,
                                     RwReal fadeDist,
                                     RwReal objectRadius,
                                     RwBool enableFading);

/*
 *  Shadow rendering
 */
extern RwBool       ShadowRender(RwCamera *shadowCamera, 
                                 RwRaster *shadowRaster,
                                 RpWorld *world, 
                                 RpIntersection *shadowZone,
                                 RwReal shadowStrength, 
                                 RwReal fadeDist,
                                 RwBool enableFading);

#endif /* SHADOW_H */
