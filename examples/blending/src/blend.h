
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
 * blend.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: To illustrate alpha blending between two IM2D 
 *          rendered geometries.
 *                         
 ****************************************************************************/

#ifndef BLEND_H
#define BLEND_H

#include "rwcore.h"

#define NUMBLENDFUNCTIONS (11)

extern RwRGBAReal SrcColor;
extern RwRGBAReal DestColor;

extern RwInt32 SrcBlendID;
extern RwInt32 DestBlendID;

extern RwBool BlendMode[NUMBLENDFUNCTIONS][NUMBLENDFUNCTIONS];


#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern void QueryBlendFunctionInfo(void);

extern void UpdateSrcGeometryColor(RwRGBAReal *color);
extern void UpdateDestGeometryColor(RwRGBAReal *color);

extern void Im2DRender(void);
extern RwBool Im2DInitialize(RwCamera *camera);
extern void Im2DSize(RwCamera *camera, RwInt32 width, RwInt32 height);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* BLEND_H */
