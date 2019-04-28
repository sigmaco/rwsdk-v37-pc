
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
 * lodatom.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the use of the level-of-detail plugin.
 *
*****************************************************************************/

#ifndef LODATOM_H
#define LODATOM_H

#include "rwcore.h"
#include "rpworld.h"

extern RpAtomic *LODAtomic;

extern RwIm3DVertex *Im3DVertexBuffer;

extern RwBool WireFrameOn;

extern RwV3d LocalSpaceCameraPosition;

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RpClump *CreateLODClump(RpWorld *world);
extern void FreeIm3DVertexBuffer(void);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* LODATOM_H */
