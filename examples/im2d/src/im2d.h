
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
 * im2d.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate RenderWare's 2D immediate mode.
 * *****************************************************************************/

#ifndef IM2D_H
#define IM2D_H

#include "rwcore.h"

extern RwInt32 Im2DPrimType;
extern RwBool Im2DTextured;
extern RwBool Im2DColored;

extern RwV2d ScreenSize;
extern RwReal Scale;

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

extern void Im2DRender(void);
extern RwBool Im2DInitialize(RwCamera *camera);
extern void Im2DSize(RwCamera *camera, RwInt32 width, RwInt32 height);
extern void Im2DTerminate(void);

/*
 * Line-list functions...
 */
extern void LineListCreate(RwCamera *camera);
extern void LineListSetColor(RwBool white);
extern void LineListRender(void);

extern void IndexedLineListCreate(RwCamera *camera);
extern void IndexedLineListSetColor(RwBool white);
extern void IndexedLineListRender(void);

/*
 * Poly-line functions...
 */
extern void PolyLineCreate(RwCamera *camera);
extern void PolyLineSetColor(RwBool white);
extern void PolyLineRender(void);

extern void IndexedPolyLineCreate(RwCamera *camera);
extern void IndexedPolyLineSetColor(RwBool white);
extern void IndexedPolyLineRender(void);

/*
 * Tri-list functions...
 */
extern void TriListCreate(RwCamera *camera);
extern void TriListSetColor(RwBool white);
extern void TriListRender(void);

extern void IndexedTriListCreate(RwCamera *camera);
extern void IndexedTriListSetColor(RwBool white);
extern void IndexedTriListRender(void);

/*
 * Tri-strip functions...
 */
extern void TriStripCreate(RwCamera *camera);
extern void TriStripSetColor(RwBool white);
extern void TriStripRender(void);

extern void IndexedTriStripCreate(RwCamera *camera);
extern void IndexedTriStripSetColor(RwBool white);
extern void IndexedTriStripRender(void); 

/*
 * Tri-fan functions...
 */
extern void TriFanCreate(RwCamera *camera);
extern void TriFanSetColor(RwBool white);
extern void TriFanRender(void);

extern void IndexedTriFanCreate(RwCamera *camera);
extern void IndexedTriFanSetColor(RwBool white);
extern void IndexedTriFanRender(void);

#ifdef    __cplusplus
}
#endif

#endif /* IM2D_H */
