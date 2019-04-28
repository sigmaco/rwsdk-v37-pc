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
 * Purpose: Example to demonstrate the functions of light maps as
   as they are provided in RpLightmap and RtLightmap.
 *
*****************************************************************************/

#ifndef MAIN_H
#define MAIN_H

#if (rwLIBRARYCURRENTVERSION < 0x33000)
#error "Cannot use RW3.3 features w/ pre-RW3.3 LIBs/headers!"
#endif /* (rwLIBRARYCURRENTVERSION < 0x33000) */

#define DEFAULTLIGHTMAPSIZE 512

#define MAXCAMERAROTSPEED   (0.5f) /* Radians per second */

/* These are used (in events.c and main.c) to extend menu functionality */
#define MENUSTRINGARRAYCAMSPEED (0)
#define MENUSTRINGARRAYDENSITY  (1)
#define MENUSTRINGARRAYMAPSIZE  (2)
#define MENUSTRINGARRAYENTRIES  (3)

#define FAKEENUMMINVAL     (0)
#define FAKEENUMMAXVAL     (2)
#define FAKEENUMDEFAULTVAL (1)

extern RwReal   CameraPitchRate;
extern RwReal   CameraTurnRate;
extern RwReal   CameraMaxSpeed;
extern RwReal   CameraSpeed;
extern RwReal   CameraStrafeSpeed;
extern RwReal   LightMapDensity;
extern RwUInt32 LightMapSize;
extern RwInt32  LightMapScale;
extern RwChar   CurrentWorldPath[];
extern RwInt32  gFakeEnums[];

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern RwBool ChangeCameraSpeed(RwBool justCheck);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MAIN_H */

