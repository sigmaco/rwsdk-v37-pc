
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
 * hanimsub.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the usage of sub-hierarchical animations.
 * *****************************************************************************/

#ifndef HANIMSUB_H
#define HANIMSUB_H

#include "rwcore.h"
#include "rpworld.h"
#include "rtanim.h"

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RwBool LoadAnimations(void);
extern void SetupHierarchies(RpClump* clump);
extern void UpdateAnimations(RpHAnimHierarchy** hierarchy, RwReal deltaTime);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* HANIMSUB_H */

