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
 * burst.h
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

#ifndef BURST_H
#define BURST_H

#include "rwcore.h"

typedef struct burstObj burstObj;

struct burstObj 
{
    RwMatrix orientation;   /* 4 */

    RwV3d   radius;         /* 3 */
    RwReal  time;           /* 1 */
    
};

extern RpAtomic *BurstPTank;
extern RwInt32 PrtPerBurst;
extern RwInt32 MaxBurst;
extern RwInt32 BurstInRenderList;

extern RwBool            InRenderList;
extern RpPTankLockStruct BurstPosLock;

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool   
BurstsInitialize(RwInt32 maxBurstCount, RwInt32 numPrtPerBurst);

extern void     
BurstsDestroy(void);

extern burstObj *
BurstGetNewBurst(void);

extern void     
BurstBeginRenderList(void);

extern void     
BurstAddToRenderList(burstObj **burstList, RwInt32 numBurst);

extern void     
BurstEndRenderList(void);

extern void     
BurstsRender(void);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* BURST_H */


