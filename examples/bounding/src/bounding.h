
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
 * bounding.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how a clump's bounding-box and bounding-sphere 
 *          can be calculated.
 ****************************************************************************/

#ifndef BOUNDING_H
#define BOUNDING_H

#include "rwcore.h"
#include "rpworld.h"

extern RwInt32 WorldSpace;
extern RwInt32 BoundingBox;

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern void UpdateWorldSpaceBoundingVolumes(RpClump *clump);
extern void InitializeLocalSpaceBoundingVolumes(RpClump *clump);

extern void ClumpRenderBoundingVolume(RpClump *clump);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* BOUNDING_H */
