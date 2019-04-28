
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
 * main.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrate the picking and dragging of 3D immediate vertices.
 *
*****************************************************************************/

#ifndef IMPICK_H
#define IMPICK_H

#include "rwcore.h"

extern RwCamera *Camera;

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool Im3DCreateGrid(void);
extern RwBool Im3DCreateCube(void);

extern RwBool Im3DInitialize(void);
extern void Im3DTerminate(void);
extern void Im3DRender(void);

extern RwBool Im3DPickVertices(RwV2d *screenPos);
extern void Im3DResetPickedVertexColor(void);

extern void Im3DSetVertexXY(RwV2d *screenPos);

extern void Im3DRotate(RwReal angleX, RwReal angleY);
extern void Im3DTranslateZ(RwReal zDelta);

#if (defined(SKY) || defined(_XBOX) || defined(DOLPHIN))
extern void InitializeMouseCursor(void);
extern void UpdateMouseCursor(void);
#endif  /* (defined(SKY) || defined(_XBOX) || defined(DOLPHIN)) */

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* IMPICK_H */


