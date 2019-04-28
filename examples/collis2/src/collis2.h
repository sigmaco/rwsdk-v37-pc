
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
 * collis2.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the use of the RpCollision plugin to collide 
 *          an atomic with the static geometry in a world.
 ****************************************************************************/

#ifndef COLLIS2_H
#define COLLIS2_H

#include "rwcore.h"
#include "rpworld.h"

extern RwBool GravityOn;
extern RwBool DampingOn;

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RwBool CreateBallAtomic(RpWorld *world);
extern RwBool DestroyBallAtomic(void);
extern void ResetBall(void);
extern void ResetGravity(void);

extern void Run(RwReal deltaTime);

extern void TranslateCameraZ(RwReal dist);
extern void CameraPoint(RwReal turn, RwReal tilt);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* COLLIS2_H */
