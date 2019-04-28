
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
 * render.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 *                                                                         
 * Purpose: RenderWare3 BSP viewer.
 *                         
 ****************************************************************************/

#ifndef RENDER_H
#define RENDER_H

#include "rwcore.h"
#include "rpworld.h"

extern RwBool TrianglesOn;
extern RwBool WireFrameOn;
extern RwBool NormalsOn;
extern RwBool WorldSectorsOn;
extern RwBool SingleSectorOn;
extern RwInt32 TriStripLength;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */ 

extern RwBool ResizeIm3DVertexArray(RpWorld *world);

extern void FreeIm3DVertices(void);

extern void WorldRender(RpWorld *world, RwCamera *camera);

extern RpWorldSector* RenderWorldSectorTriStrip(RpWorldSector *sector,
    void *data);

extern RpWorldSector* RenderWorldSectorMesh(RpWorldSector *sector, void *data);

extern RwBool ResizeTriStripVertexArray(RpWorld *world);
extern void FreeTriStripVertices(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */    

#endif  /* RENDER_H */
