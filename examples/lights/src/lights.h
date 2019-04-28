
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
 * Purpose: To illustrate the different lights that are available for use in
 *          RenderWare.
 *
*****************************************************************************/

#ifndef LIGHTS_H
#define LIGHTS_H

#include "rwcore.h"
#include "rpworld.h"

#define AMBIENT_LIGHT (0)
#define POINT_LIGHT (1)
#define DIRECT_LIGHT (2)
#define SPOT_LIGHT (3)
#define SOFTSPOT_LIGHT (4)

#define MAX_LIGHT_RADIUS (500.0f)
#define MIN_LIGHT_RADIUS (0.1f)
#define STEP_LIGHT_RADIUS (5.0f)

#define MAX_LIGHT_CONE_ANGLE  rpLIGHTMAXCONEANGLE
#define MIN_LIGHT_CONE_ANGLE  rpLIGHTMINCONEANGLE
#define STEP_LIGHT_CONE_ANGLE (5.0f)

extern RpLight *BaseAmbientLight;
extern RwBool BaseAmbientLightOn;

extern RpLight *CurrentLight;
extern RpLight *AmbientLight;
extern RpLight *PointLight;
extern RpLight *DirectLight;
extern RpLight *SpotLight;
extern RpLight *SpotSoftLight;

extern RwReal LightRadius;
extern RwReal LightConeAngle;
extern RwRGBAReal LightColor;
extern RwRGBA LightDrawColor;

extern RwBool LightOn;
extern RwBool LightDrawOn;
extern RwV3d LightPos;

extern RwInt32 LightTypeIndex;

extern RwReal AdvanceLightSpeed;

extern RwBBox RoomBBox;


#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RpLight *CreateBaseAmbientLight(void);
extern RpLight *CreateAmbientLight(void);
extern RpLight *CreateDirectLight(void);
extern RpLight *CreatePointLight(void);
extern RpLight *CreateSpotLight(void);
extern RpLight *CreateSpotSoftLight(void);

extern void LightsDestroy(void);

extern RwBool LightResetCallback(RwBool testEnable);
extern void LightsUpdate(void);

extern void DrawCurrentLight(void);

extern void LightRotate(RwReal xAngle, RwReal yAngle);
extern void LightTranslateXY(RwReal xDelta, RwReal yDelta);
extern void LightTranslateZ(RwReal delta);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* LIGHTS_H */
