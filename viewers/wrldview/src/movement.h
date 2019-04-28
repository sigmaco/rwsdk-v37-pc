
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
 * movement.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 *                                                                         
 * Purpose: RenderWare3 BSP viewer.
 *                         
 ****************************************************************************/

#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "rwcore.h"

extern RwReal TotalTilt;

extern RwBool CameraFlying;
extern RwReal FlyingSpeed;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */ 

extern void CameraPoint(RwReal tilt, RwReal turn);
extern void TranslateCameraZ(RwReal dist);
extern void CameraFly(RwCamera *camera, RwReal deltaTime);

#ifdef __cplusplus
}
#endif  /* __cplusplus */    

#endif  /* MOVEMENT_H */
