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
 * collis1.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrates simple collision detection between an RpWorld and
 *          a line or sphere.
 ****************************************************************************/

#ifndef COLLIS1_H
#define COLLIS1_H

#include "rwcore.h"
#include "rpworld.h"

enum CollisionModes 
{
    LINE = 0, 
    SPHERE,

    COLLISIONMODESFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

enum MovementModes
{
    FORWARDS = 0,
    BACKWARDS,
    STOPPED,

    MOVEMENTMODESFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

extern RwCamera *Camera;
extern RwReal CameraRadius;

extern enum CollisionModes CurrentCollisionMode;
extern enum MovementModes CurrentMovementMode;

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern void CameraPoint(RwCamera *camera, RwReal turn, RwReal tilt);

extern void CameraUpdate(RwCamera *camera, RpWorld *world, RwReal deltaTime);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* COLLIS1_H */
