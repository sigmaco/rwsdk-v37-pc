
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
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrates simple collision detection between an RpWorld and
 *          a line or sphere.
 ****************************************************************************/

#ifndef MOVEMENT_H
#define MOVEMENT_H

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwReal CameraFloatHeight;

extern void
MoveCameraStart(RwBool forward);

extern void
MoveCameraStop(void);

extern void
MoveCameraUpdate(RwCamera *cam, RpWorld *wor);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */
#endif  /* MOVEMENT_H */