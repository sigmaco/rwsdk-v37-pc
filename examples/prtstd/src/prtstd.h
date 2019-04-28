
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
 * Original author: Alexandre Hadjdaj.
 *
 * Purpose: Illustrate the use of the Standard Particle Plugin.
 *
 ****************************************************************************/

#ifndef PRTSTD_H
#define PRTSTD_H

#include "rwcore.h"
#include "rpprtstd.h"

#define COLORANIM
#define SIZEANIM
#define BATCHSIZE (140)

extern RwCamera *Camera;

typedef struct PrtStdData PrtStdData;

extern RwBool renderEmitter;

struct PrtStdData
{
    RpPrtStdEmitterClass        *eClass;
    RpPrtStdParticleClass       *pClass;

    RpAtomic                    *atmEmitter;

    RpPrtStdEmitter             *emitter;

    /* Standard properties */
    RpPrtStdEmitterStandard     *EmtStd;


#ifdef COLORANIM
    /* Color Interpolation */
    RpPrtStdEmitterPrtColor     *emtPrtCol;
#endif

#ifdef SIZEANIM
    RpPrtStdEmitterPrtSize      *emtPrtSize;
#endif

    RpPrtStdEmitterPTank        *EmtPtank;

    RwTexture *PrtTexture;



};

extern PrtStdData PrtData;

extern RwInt32 SrcBlendID;
extern RwInt32 DestBlendID;

extern RwBool VtxAlphaOn;

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool PrtStdInitialize(void);
extern void PrtStdTerminate(void);
extern void PrtStdRender(void);

extern RwBool PrtStdSetDefault(RwBool justcheck);

extern void PrtEmitterIm3DCreate( RpPrtStdEmitterStandard *emtStd );

extern void PrtStdCameraRotate(RwReal xAngle, RwReal yAngle);
extern void PrtStdEmitterRotate(RwReal xAngle, RwReal yAngle);
extern void PrtStdEmitterWorldRotate(RwReal xAngle, RwReal yAngle, RwReal zAngle);

extern RwBool PrtStdSetPtankAlphaBlending( RwBool justCheck );

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* PRTSTD_H */


