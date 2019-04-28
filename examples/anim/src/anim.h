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
 * Copyright (c) 2002 Criterion Software Ltd.
 * All Rights Reserved.
 *
 ****************************************************************************/

/****************************************************************************
 *
 * anim.h
 *
 * Copyright (C) 2002 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: This Example demonstrates the use of the rtanim toolkit.
 *
 ****************************************************************************/

#ifndef ANIM_H
#define ANIM_H

#include "rwcore.h"
#include "rpworld.h"
#include "rtanim.h"

typedef struct LightKeyFrame LightKeyFrame;
struct LightKeyFrame
{
/* RtAnimKeyFrameHeader */
	void	*prevFrame;		
	RwReal	time;

	RwRGBAReal	color;
	RwReal		radius;
};

typedef struct LightInterpFrame LightInterpFrame;
struct LightInterpFrame
{
/* RtAnimInterpFrameHeader */
    LightKeyFrame   *keyframe1;
    LightKeyFrame   *keyframe2;

	RwRGBAReal	color;
	RwReal		radius;
};

extern RtAnimInterpolator *AnimInterpolator;

/*
 * Key Frame Layout 
 * Light 1 : Directional 1
 * Light 2 : Directional 2
 * Light 3 : Point Light
 * alpha allways == 1.0f;
 * NumKey frame = 21
 *                 0.0f            1.0f            2.0f            3.0f                    
 * L1
 *     color       0.0f 0.0f 0.0f--1.0f 0.0f 0.0f------------------0.0f 1.0f 0.0f--
 *     radius      0.0f------------0.0f----------------------------0.0f------------
 * L2
 *     color       0.0f 0.0f 0.0f--1.0f 1.0f 1.0f--0.0f 0.0f 0.0f--1.0f 1.0f 1.0f--
 *     radius      0.0f------------0.0f------------0.0f------------0.0f------------
 * L3
 *     color       1.0f 1.0f 1.0f--------------------------------------------------
 *     radius      1000.0f---------------------------------------------------------
 * 
 *                 3.0f            4.0f            5.0f            6.0f             
 * L1
 *     color       0.0f 1.0f 0.0f------------------0.0f 0.0f 1.0f------------------
 *     radius      0.0f----------------------------0.0f----------------------------
 * L2
 *     color       1.0f 1.0f 1.0f--0.0f 0.0f 0.0f--1.0f 1.0f 1.0f--0.0f 0.0f 0.0f--
 *     radius      0.0f------------0.0f------------0.0f------------0.0f------------
 * L3
 *     color       --------------------------------1.0f 1.0f 1.0f------------------
 *     radius      --------------------------------0.0f----------------------------
 * 
 *                 6.0f            7.0f            8.0f            9.0f             
 * L1
 *     color       ----------------0.0f 1.0f 0.0f------------------1.0f 0.0f 0.0f--
 *     radius      ----------------0.0f----------------------------0.0f------------
 * L2
 *     color       0.0f 0.0f 0.0f--1.0f 1.0f 1.0f--0.0f 0.0f 0.0f--1.0f 1.0f 1.0f--
 *     radius      0.0f------------0.0f------------0.0f------------0.0f------------
 * L3
 *     color       ----------------------------------------------------------------
 *     radius      ----------------------------------------------------------------
 * 
 *                 9.0f            10.0f           
 * L1
 *     color       1.0f 0.0f 0.0f--0.0f 0.0f 0.0f
 *     radius      0.0f------------0.0f
 * L2
 *     color       1.0f 1.0f 1.0f--0.0f 0.0f 0.0f
 *     radius      0.0f------------0.0f
 * L3
 *     color       ----------------1.0f 1.0f 1.0f
 *     radius      ----------------1000.0f
 */
#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwBool
AnimCreate(void);

extern void
AnimDestroy(void);

extern void
LightAnimApply(RpLight **lights,RtAnimInterpolator *anim);


#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif
