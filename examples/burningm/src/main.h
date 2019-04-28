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
 * Purpose: Example of a particle system, using a ptank for the rendering, 
 *          using a skinned character as an emitter.
 *
*****************************************************************************/

#ifndef MAIN_H
#define MAIN_H

#define MAXCAMERASPEED      250.0f 
#define MAXCAMERAROTSPEED   80.0f /* Degrees per second */

extern RwReal   CameraPitchRate;
extern RwReal   CameraTurnRate;
extern RwReal   CameraSpeed;
extern RwReal   CameraStrafeSpeed;

extern RtCharset          *Charset;

#endif /* MAIN_H */

