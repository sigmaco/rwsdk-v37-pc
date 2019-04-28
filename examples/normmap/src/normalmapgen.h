
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
 * Copyright (c) 2002 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * normalmapgen.h
 *
 * Copyright (C) 2002 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose : RenderWare 3.4 demo.
 *
 ****************************************************************************/

#ifndef NORMALMAPGEN_H
#define NORMALMAPGEN_H

#include "rwcore.h"

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwTexture *
NormalMapTextureSpaceCreateFromTexture(RwTexture *texture, RwReal bumpness);

extern RwRaster *
NormalMapTextureSpaceCreateFromImage(RwImage *image, RwUInt32 rasterFlags, RwBool clamp, RwReal bumpness);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* NORMALMAPGEN_H */