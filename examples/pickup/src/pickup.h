
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
 * Copyright (c) 2003 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * main.h
 *
 * Copyright (C) 2003 Criterion Technologies.
 *
 * Original author: RenderWare Team.
 *
 * Purpose: demonstrate a health pickup effects, using one ptank atomic per 
 * pickup.
 * 
 ****************************************************************************/

#ifndef PICKUP_H
#define PICKUP_H

#include "rwcore.h"
#include "rpptank.h"

#define COLORANIM
#define SIZEANIM
#define BATCHSIZE (140)

extern RwCamera *Camera;

typedef struct PickUpObj PickUpObj;

struct PickUpObj
{
    RpAtomic    *pTank;

    RwRGBAReal  deltaColor;
    RwRGBAReal  color;

    RwBool      direction;
    RwReal      time;
    RwInt32     step;

};

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool 
PickUpInit(void);

extern PickUpObj *
PickUpCreate(RwFrame *frame,
             RwTexture *prtTexture,
             RwReal radius,
             RwInt32 numPrt,
             RwRGBA *color1,
             RwRGBA *color2);

extern void 
PickUpDestroy(PickUpObj *pickup);

extern void
PickUpUpdate(PickUpObj *pickup,RwReal deltaTime);

extern void 
PickUpRender(PickUpObj *pickup);

extern void 
PickUpCameraRotate(RwReal xAngle, RwReal yAngle);


#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* PICKUP_H */


