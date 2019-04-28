
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
 * scene.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Scene management.
 *
 ****************************************************************************/

#ifndef SCENE_H
#define SCENE_H

#include "rwcore.h"
#include "rpworld.h"

#define NEARMINCLIP (0.1f)
#define MINNEARTOFARCLIP (1.0f)
#define FARMAXCLIP (9999.0f)

extern RpWorld *World;

extern RwCamera *Camera;
extern RwReal NearClip;
extern RwReal FarClip;
extern RwReal FieldOfView;
extern RwReal CurrentViewWindow;

extern RpLight *AmbientLight;
extern RwReal AmbientIntensity;

extern RpLight *MainLight;
extern RwReal MainIntensity;

extern RwBool AmbientLightOn;
extern RwBool MainLightOn;

extern RwRGBA TopColor;
extern RwRGBA BottomColor;

/*
 * Functions
 */
#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern RwBool HandleFileLoad(void *param);

extern RwBool SceneInit(void);
extern void SceneCameraReset(void);
extern void SceneCameraUpdate(void);
extern void SceneLightsUpdate(void);
extern void SceneDestroy(void);
extern void SceneRender(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* SCENE_H */

