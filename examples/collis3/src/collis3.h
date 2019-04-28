
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
 * collis3.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrates the detection of collisions with atomics and
 *          the creation of geometry collision data to speed up the
 *          intersection tests.
 *
 ****************************************************************************/

#ifndef COLLIS3_H
#define COLLIS3_H

#include "rwcore.h"
#include "rpworld.h"

extern RwBool CollisionDataGenerated;

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool CollisionObjectsCreate(RpWorld *world);
extern void CollisionObjectsDestroy(RpWorld *world);

extern RwBool CollisionObjectsUpdate(RpWorld *world, RwReal deltaTime);

extern RwBool CollisionDataBuildCallback(RwBool testEnable);
extern RwBool CollisionDataSaveCallback(RwBool testEnable);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* COLLIS3_H */
