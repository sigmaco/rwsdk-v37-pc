
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
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 *                                                                         
 * Purpose: RenderWare3 BSP viewer.
 *                         
 ****************************************************************************/

#ifndef MAIN_H
#define MAIN_H

#include "rwcore.h"
#include "rpworld.h"

extern RwCamera *Camera;
extern RpLight *AmbientLight;
extern RpLight *MainLight;

extern RwInt32 NumCameraWorldSectors;
extern RwInt32 NumCameraTriangles;
extern RwInt32 NumPVSWorldSectors;
extern RwInt32 NumPVSTriangles;

extern RwReal NormalsScaleFactor;
extern RwReal TranslateScaleFactor;

extern RwBool AmbientLightOn;
extern RwBool MainLightOn;

extern RwBool CameraPointing;
extern RwBool CameraTranslating;
extern RwReal CameraTranslateDelta;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */ 

extern void Render(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */    

#endif  /* MAIN_H */
