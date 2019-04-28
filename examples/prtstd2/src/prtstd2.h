
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
 * Copyright (C) 2002 Criterion Technologies.
 *
 * Original author: Alexandre Hadjdaj.
 *
 * Purpose: Illustrate the use custom property in the Standard Particle
 *          Plugin.
 *
 ****************************************************************************/

#ifndef PRTSTD_H
#define PRTSTD_H

#include "rwcore.h"
#include "rpprtstd.h"

#define BATCHSIZE (140)


#define PARTICLEGLOWCOUNT                               4

#define PRIVATEPROPERTYCODE(_code)                      ((0x02 << 24) | (_code))

#define PARTICLEPROPCODEGLOW                            PRIVATEPROPERTYCODE(0)

#define EMITTERPRTPROPCODEGLOW                          PRIVATEPROPERTYCODE(0)

#define PARTICLEPROP(_ptr, _offset) \
    (((char *)(_ptr)) + (_offset))

extern RwCamera *Camera;
extern RwBool renderEmitter;

typedef struct EmitterPrtGlow EmitterPrtGlow;

typedef struct ParticleGlow ParticleGlow;

typedef struct ParticleGlowTable ParticleGlowTable;

typedef struct PrtStdData PrtStdData;


/* Custom emitter property. This structure represent the custom property
 * that will be included in the emitter.
 *
 * This structure is used by the emitter to initialise the particles custom
 * property during emission.
 *
 * It contains the default values for all the particle's initial color and
 * color animation.
 */
struct EmitterPrtGlow
{
    RwInt32             numGlow;
    ParticleGlowTable   *glowTable;
};

/* Custom particle property. This structure represent the custom property
 * that will be included in the particle.
 *
 * This structure is used by the custom callbacks to animate the particle's
 * color during its lifetime.
 *
 * It contains the present state of the particle's color animation data. It
 * does not hold the particle's color. This is stored in the pTank's particle
 * color property.
 *
 * As the particle age, the animation data is updated. This in turn is used
 * to animate the color.
 *
 * Each particle is given its own animation data so they will appear
 * differently. We could have used a single animation global to all particles,
 * this would mean all particles will look alike and the animation will be
 * synchronised with each other.
 */
struct ParticleGlow
{
    RwInt32     index;
    RwReal      t;
    RwReal      glow0;
    RwReal      glow1;
    RwReal      iDeltaT;
};

/* Table to hold the color variation of the particle.
 */
struct ParticleGlowTable
{
    RwReal                      t;
    RwReal                      glow;
    RwReal                      iDeltaT;
};

/* Data structure to hold commonly used global variables.
 */
struct PrtStdData
{
    RpPrtStdEmitterClass        *eClass;
    RpPrtStdParticleClass       *pClass;

    RpAtomic                    *atmEmitter;

    RpPrtStdEmitter             *emitter;

    /* Standard properties */
    RpPrtStdEmitterStandard     *EmtStd;

    /* Color Interpolation */
    EmitterPrtGlow              *emtPrtGlow;

    RpPrtStdEmitterPTank        *EmtPtank;

    RwTexture                   *PrtTexture;

    RwInt32                     numGlow;
    ParticleGlowTable           *prtGlowTable;

    RwInt32                     offsetEmtStd;
    RwInt32                     offsetEmtPrtGlow;
    RwInt32                     offsetEmtPTank;

    RwInt32                     offsetPrtStd;
    RwInt32                     offsetPrtGlow;
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
extern RwBool PrtStdStreamRead(void);
extern RwBool PrtStdStreamWrite(void);

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


