
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
 * imlight.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrate the lighting of 3D immediate vertices.
 *
*****************************************************************************/

#ifndef IMLIGHT_H
#define IMLIGHT_H

#include "rwcore.h"

extern RwCamera *Camera;

typedef enum
{
    LIGHTDIRECTIONAL,
    LIGHTPOINT,
    LIGHTAMBIENT,
    NUMLIGHTMODES,

    LIGHTENUMFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
}
LightEnum;

extern RwInt32 LightMode;

typedef enum
{
    SHADEGOURAUD,
    SHADEFLAT,
    NUMSHADEMODE,

    SHADEMODEENUMFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
}
ShadeModeEnum;

extern RwInt32 ShadeMode;


#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool Im3DInitialize(void);
extern void Im3DTerminate(void);
extern void Im3DRender(void);

extern void Im3DMeshRotate(RwReal angleX, RwReal angleY);
extern void Im3DMeshTranslateZ(RwReal zDelta);

extern void Im3DLightRotate(RwReal xAngle, RwReal yAngle);
extern void Im3DLightTranslateZ(RwReal zDelta);
extern void Im3DLightTranslateXY(RwReal xDelta, RwReal yDelta);

extern void Im3DLightReset(void);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* IMLIGHT_H */


