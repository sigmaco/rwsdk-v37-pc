
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
 * Copyright (c) 2002 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * main.h
 *
 * Copyright (C) 2002 Criterion Technologies.
 *
 * Author: RenderWare Team
 *
 * Purpose: A viewer capable of displaying clump's - including bones, skin, 
 *          and animation support
 *
 ****************************************************************************/

#ifndef MAIN_H
#define MAIN_H

#include "rwcore.h"
#include "rpworld.h"

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool CameraZooming;
extern RwReal CameraZoomDelta;

extern RwBool CameraTranslating;
extern RwReal CameraTranslateDelta;

extern void MainLightRotate(RwReal x, RwReal y);

extern void ClumpsRotate(RwReal x, RwReal y);

extern void ClumpsTranslate(RwReal x, RwReal y);

extern void ClumpsZoom(RwReal y);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* MAIN_H */

