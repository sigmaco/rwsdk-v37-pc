
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
 * Copyright (c) 2000 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *                                                                         
 * texdict.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: Display the difference in texture loading times by using a
 *          texture dictionary.
 *
 ****************************************************************************/

#ifndef TEXDICT_H
#define TEXDICT_H

#include "rwcore.h"
#include "rpworld.h"

extern RwReal TimeWithoutTexDict;
extern RwReal TimeWithTexDict;
extern RwBool TexDictCreated;

extern RpLight *AmbientLight;
extern RpWorld *World;
extern RwCamera *Camera;

extern RwBool LoadingWorld;

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern void ReloadWorld(void);
extern void CameraSetPosition(RwCamera *camera, RpWorld *world);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif /* TEXDICT_H */
