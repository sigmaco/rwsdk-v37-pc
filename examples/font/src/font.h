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
 * font.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics camera example.
 *          can be calculated.
 ****************************************************************************/


#ifndef FONTEXAMPLE_H
#define FONTEXAMPLE_H

#include "rwcore.h"
#include "rpworld.h"

#include "rt2d.h"

#define NUMFONT     6

extern Rt2dBrush   *Brush;

extern Rt2dFont    *Font[NUMFONT];
extern RwChar      FontName[NUMFONT][20];
extern RwInt32     *FontCharSet[NUMFONT];
extern RwInt32     FontCharSetCount[NUMFONT];

extern RwInt32     FontIndex;



#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern void
Render2D(RwReal dTime);

extern RwBool
Initialize2D(void);

extern void
Terminate2D(void);

extern void
PageTranslateInit(void);

extern void
PageTranslate(RwReal x, RwReal y);

extern void
PageRotateInit(RwReal x, RwReal y);

extern void
PageRotate(RwReal x);

extern void
PagePositionSet(RwReal x, RwReal y);



#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif /* FONTEXAMPLE_h */
