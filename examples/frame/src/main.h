
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
 * main.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the frame hierarchy of a series of atomics 
 * in a clump.
 *****************************************************************************/

#ifndef MAIN_H
#define MAIN_H

#include "rwcore.h"
#include "rpworld.h"

#define ATOMICNUM (11)

#define POS_X (0.0f)
#define POS_Y (30.0f)
#define POS_Z (50.0f)

extern RpWorld *World;
extern RwCamera *Camera;
extern RpClump *Clump;
extern RpAtomic *Atomics[ATOMICNUM];

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern void HighlightAtomic(void);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* MAIN_H */
