
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
 * morph.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: Example to demonstrate the morph plugin.
 *
 *****************************************************************************/

#ifndef MORPH_H
#define MORPH_H

#include "rwcore.h"
#include "rpworld.h"

typedef struct
{
    RwInt32 numTargets;
    RwInt32 numInterpolators;
    RwInt32 interpolator;
    RwInt32 startTarget;
    RwInt32 endTarget;
    RwReal value;
    RwReal scale;
}
MorphParams;

extern RwCamera *Camera;
extern RpClump *Clump;

extern RwReal MorphSpeed;
extern RwBool MorphOn;
extern RwInt32 NumInterpolators;

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RpClump *CreateClump(RpWorld *world);
extern void ClumpSetupInterpolators(RpClump *clump);
extern void ClumpAdvanceMorph(RpClump *clump, RwReal delta);
extern void ClumpRotate(RpClump *clump, RwCamera *camera, 
                        RwReal xAngle, RwReal yAngle);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* MORPH_H */
