
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
 * lights.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose :To illustrate the effects of different global and local lights on
 *          a landscape.
 *
 *****************************************************************************/

#ifndef LIGHTS2_H
#define LIGHTS2_H

#include "rwcore.h"
#include "rpworld.h"

extern RpLight *AmbientLight;
extern RpLight *DirectionalLight;
extern RpLight *PointLight;

extern RwBool DirectLightOn;
extern RwBool PointLightOn;
extern RwReal PointRadius;

#ifdef    __cplusplus
extern "C"
{
#endif	/* __cplusplus */

extern RwBool CreateLights(RpWorld *world);
extern void UpdateLights(void);
extern void DrawLightRadius(void);
extern void TranslatePointLight(RwReal dX, RwReal dY);
extern void RotateDirectLight(RwReal angleX, RwReal angleY);
extern void DrawLightDirection(void);

#ifdef    __cplusplus
}
#endif	/* __cplusplus */

#endif	/* LIGHTS2_H */
