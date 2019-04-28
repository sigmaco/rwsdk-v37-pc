
/****************************************************************************
 *
 * vecfont.h
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
 * Copyright (c) 1999, 2000 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

#if (!defined(_VECFONT_H))
#define _VECFONT_H

#include "rwcore.h"

typedef struct RsVectorFont RsVectorFont;

struct RsVectorFont
{
    RwV2d         size;
    RwRGBA        color;
    RwReal        recipZ;
    RwIm2DVertex *lineVertBuffer;
    RwUInt32      lineVertBufferSize;
};


#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern struct RsVectorFont *RsVecFontCreate(const RwCamera *camera,
                                             const RwRGBA *color,
                                             const RwV2d *size);

extern RwBool RsVecFontDestroy(struct RsVectorFont *font);

extern void RsVecFontPrint(struct RsVectorFont *font,
                           const RwV2d *pos,
                           const RwChar *string);

extern RwBool RsVecFontOpen(void);
extern void RsVecFontClose(void);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* (!defined(_VECFONT_H)) */
