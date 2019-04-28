
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
 * hanim.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how to compress H-anim sequences.
 * *****************************************************************************/

#ifndef HANIM1_H
#define HANIM1_H

#include "rwcore.h"
#include "rpworld.h"
#include "rtanim.h"

#define NUM_REDUCTION_METHODS 6
#define NUM_COMPRESSION_METHODS 2

extern RtAnimAnimation *Anims[NUM_REDUCTION_METHODS][NUM_COMPRESSION_METHODS];
extern RwInt32          Sizes[NUM_REDUCTION_METHODS][NUM_COMPRESSION_METHODS];
extern RtAnimAnimation *CurrentAnim;

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RpClump *CreateClump(RpWorld *world);
extern void DestroyClump(RpClump *clump, RpWorld *world);

extern void UpdateAnimation(RpClump *clump, RwReal deltaTime);

extern RpHAnimHierarchy *GetHierarchy(RpClump *clump);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* HANIM1_H */
