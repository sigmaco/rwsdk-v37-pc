
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
 * im3d.h.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate RenderWare's 3D immediate mode.
 *****************************************************************************/

#ifndef IM3D_H
#define IM3D_H

#include "rwcore.h"

extern RwCamera *Camera;

extern RwInt32 Im3DPrimType;
extern RwBool Im3DTextured;
extern RwBool Im3DColored;

extern RwRGBA SolidWhite;
extern RwRGBA SolidBlack;
extern RwRGBA SolidRed;
extern RwRGBA SolidGreen;
extern RwRGBA SolidBlue;
extern RwRGBA SolidYellow;
extern RwRGBA SolidPurple;
extern RwRGBA SolidCyan;

#ifdef    __cplusplus
extern "C"
{
#endif

extern void Im3DRender(void);
extern RwBool Im3DInitialize(void);
extern void Im3DTerminate(void);
extern void Im3DRotate(RwReal angleX, RwReal angleY);
extern void Im3DTranslateZ(RwReal zDelta);

/*
 * Line-list functions...
 */
extern void LineListCreate(void);
extern void LineListSetColor(RwBool white);
extern void LineListRender(RwMatrix *transform, RwUInt32 transformFlags);

extern void IndexedLineListCreate(void);
extern void IndexedLineListSetColor(RwBool white);
extern void IndexedLineListRender(RwMatrix *transform, RwUInt32 transformFlags);

/*
 * Poly-line functions...
 */
extern void PolyLineCreate(void);
extern void PolyLineSetColor(RwBool white);
extern void PolyLineRender(RwMatrix *transform, RwUInt32 transformFlags);

extern void IndexedPolyLineCreate(void);
extern void IndexedPolyLineSetColor(RwBool white);
extern void IndexedPolyLineRender(RwMatrix *transform, RwUInt32 transformFlags);

/*
 * Tri-list functions...
 */
extern void TriListCreate(void);
extern void TriListSetColor(RwBool white);
extern void TriListRender(RwMatrix *transform, RwUInt32 transformFlags);

extern void IndexedTriListCreate(void);
extern void IndexedTriListSetColor(RwBool white);
extern void IndexedTriListRender(RwMatrix *transform, RwUInt32 transformFlags);

/*
 * Tri-strip functions...
 */
extern void TriStripCreate(void);
extern void TriStripSetColor(RwBool white);
extern void TriStripRender(RwMatrix *transform, RwUInt32 transformFlags);

extern void IndexedTriStripCreate(void);
extern void IndexedTriStripSetColor(RwBool white);
extern void IndexedTriStripRender(RwMatrix *transform, RwUInt32 transformFlags); 

/*
 * Tri-fan functions...
 */
extern void TriFanCreate(void);
extern void TriFanSetColor(RwBool white);
extern void TriFanRender(RwMatrix *transform, RwUInt32 transformFlags);

extern void IndexedTriFanCreate(void);
extern void IndexedTriFanSetColor(RwBool white);
extern void IndexedTriFanRender(RwMatrix *transform, RwUInt32 transformFlags);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* IM3D_H */
