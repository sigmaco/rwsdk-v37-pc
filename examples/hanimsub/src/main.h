
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
 * main.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the usage of sub-hierarchical animations.
 * *****************************************************************************/

#ifndef MAIN_H
#define MAIN_H

#include "rwcore.h"
#include "rpworld.h"

#define MAX_ANIMATIONS (3)
#define MAX_SUBHIERARCHIES (MAX_ANIMATIONS - 1)

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RtAnimAnimation *Anim[MAX_ANIMATIONS];
extern RwBool PlayAnim[MAX_ANIMATIONS];
extern RpHAnimHierarchy *Hierarchies[MAX_ANIMATIONS];
extern RpClump *Clump;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* MAIN_H */
