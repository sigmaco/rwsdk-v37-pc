
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
 * clmpmorf.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Handles morph based animations.
 *
 ****************************************************************************/

#ifndef CLMPMORF_H
#define CLMPMORF_H

#include "rwcore.h"
#include "rpworld.h"

extern RwBool MorphOn;
extern RwBool ClumpHasMorphAnimation;
extern RwReal MorphsPerSecond;

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern RwBool MorphClumpInitialize(RpClump *clump);
extern void MorphClumpUpdate(RwReal delta);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* CLMPMORF_H */

