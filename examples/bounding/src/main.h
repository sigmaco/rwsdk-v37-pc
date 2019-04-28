
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
 * Purpose: To illustrate how a clump's bounding-box and bounding-sphere 
 *          can be calculated.
 ****************************************************************************/

#ifndef MAIN_H
#define MAIN_H

#include "rwcore.h"
#include "rpworld.h"

extern RpClump *Clump;
extern RwCamera *Camera;

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern void ClumpRotate(RpClump *clump, RwCamera *camera, 
                        RwReal xAngle, RwReal yAngle);

extern void ClumpTranslateZ(RpClump *clump, RwCamera *camera, RwReal zDelta);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* MAIN_H */
