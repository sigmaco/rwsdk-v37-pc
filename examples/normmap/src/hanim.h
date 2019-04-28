
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
 * Purpose: To illustrate how two H-anim sequences can be blended together.
 * *****************************************************************************/

#ifndef HANIM1_H
#define HANIM1_H

#include "rwcore.h"
#include "rpworld.h"
#include "rtanim.h"

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RpClump *
LoadClumpAnimation(RpClump *clump, const RwChar *filename);

extern void
UpdateClumpAnimation(RpClump *clump, RwReal deltaTime);

extern void
DestroyClumpAnimation(RpClump *clump);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* HANIM1_H */
