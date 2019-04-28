
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
 * hanim.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how two H-anim sequences can be run together,
 *          with the second animation being a delta of the first.
 * *****************************************************************************/

#ifndef HANIM2_H
#define HANIM2_H

#include "rwcore.h"
#include "rpworld.h"
#include "rtanim.h"

#define BASE_ANIM (0)
#define DELTA_ANIM (1)
#define BASE_AND_DELTA_ANIM (2)

extern RwInt32 CurrentAnimation;

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern void CreateHierarchies(RpClump *baseClump, RpClump *deltaClump,
                RpClump *outClump);

extern RwBool CreateAnims(void);
extern void DestroyAnims(void);

extern void UpdateAnimation(RwReal deltaTime);

extern RwBool ChangeAnimationCallBack(RwBool testEnable);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* HANIM2_H */
