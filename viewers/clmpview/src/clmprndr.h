
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
 * clmprndr.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Hierarchy, WireFrame, Normals and Bounding Boxes
 *          rendering call backs
 *
 ****************************************************************************/
 
#ifndef CLMPRNDR_H
#define CLMPRNDR_H

#include "rwcore.h"
#include "rpworld.h"

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern RpAtomic *AtomicRenderWireMesh(RpAtomic *atomic, void *data);
extern RpAtomic *AtomicRenderSkeleton(RpAtomic *atomic,void *data);
extern RpAtomic *AtomicRenderVertexNormals(RpAtomic *atomic, void *data);
extern RpAtomic *AtomicRenderTriStrip(RpAtomic *atomic, void *data);
extern RpAtomic *AtomicRenderMeshes(RpAtomic *atomic, void *data);
extern void AtomicRenderBoundingBox(RpAtomic *atomic, RwBBox *bbox);

extern void ResizeIm3DVertexArray(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* CLMPRNDR_H */

