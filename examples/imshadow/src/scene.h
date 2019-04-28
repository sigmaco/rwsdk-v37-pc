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
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example of parallel-projection shadow rendering using
 *          3D immediate mode.
 *
*****************************************************************************/

#ifndef SCENE_H
#define SCENE_H

extern RpWorld *SceneOpen(void);
extern void     SceneClose(void);

extern RwBool   SceneMenuInit(void);

extern RwBool   SceneUpdate(RwReal deltaTime);
extern RwBool   SceneRender(void);

#endif /* SCENE_H */
