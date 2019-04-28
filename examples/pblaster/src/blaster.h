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
 * 
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: The PBlaster example demonstrate a blaster gun effects, using both 
 * 3d matrix based particles to generate the laser beams and screen aligned 
 * particles for the charging effect.
 *
 ****************************************************************************/


#ifndef BLASTER_H
#define BLASTER_H

#define BS_STATE_IDLE                   (0)
#define BS_STATE_PRECHARGING            (1)
#define BS_STATE_CHARGING               (2)
#define BS_STATE_SUPERCHARGING          (3)
#define BS_STATE_DISCHARGING            (4)
#define BS_STATE_CHARGED                (5)
#define BS_STATE_FIRED                  (6)
#define BS_STATE_NUM_STATES             (7)

typedef struct blasterShot blasterShot;

#define SKY_PERF_COUNTERx

#ifdef SKY_PERF_COUNTER

#define SHOTUPDATE 0
#define BURSTUPDATE 1
#define BLASTERSTATE 2
#define TOTALFRAME 5

extern RwUInt32 cycles[16];
extern RwUInt32 dcacheMiss[16];

#endif


struct blasterShot
{
    RwInt32 state;
    RwReal time;

    RwV3d radius;
    RwV3d deltaRadius;
};

extern blasterShot Shot;

extern RpWorld *World;
extern RwCamera *Camera;


#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool   
BlasterCreate(void);

extern void 
BlasterUpdate(RwReal delta);

extern void     
BlasterRender(void);

extern void     
BlasterDestroy(void);

extern void 
BlasterRotate(RwReal xAngle, RwReal yAngle);

extern void 
BlasterShotStart(void);

extern void 
BlasterShotEnd(void);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* BLASTER_H */
