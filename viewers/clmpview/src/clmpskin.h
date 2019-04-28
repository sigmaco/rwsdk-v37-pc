
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
 * clmpskin.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Handling skin based animations
 *
 ****************************************************************************/

#ifndef CLMPSKIN_H
#define CLMPSKIN_H

#include "rwcore.h"
#include "rpworld.h"

extern RwBool SkinOn;
extern RwBool ClumpHasSkinAnimation;

extern RwInt32 AtomicTotalSkinBones;
extern RwInt32 AtomicTotalAtomicTotalKeyFrame;

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern RwBool SkinLoadSKA(RpClump *clump, RwChar *skaPath);
extern RwBool SkinClumpInitialize(RpClump *clump, RwChar *clumpFileName);
extern void SkinClumpUpdate(RwReal delta);
extern void SkinDestroy(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* CLMPSKIN_H */

