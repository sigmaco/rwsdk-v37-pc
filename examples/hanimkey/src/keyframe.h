
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
 * keyframe.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the usage of custom RpHAnimAnimation 
 *          keyframe interpolation schemes.
 * *****************************************************************************/

#ifndef KEYFRAME_H
#define KEYFRAME_H

#include "rwcore.h"
#include "rpworld.h"
#include "rtquat.h"
#include "rphanim.h"

#define HANIMROTKEYFRAMEID  0x100

/*
 *****************************************************************************
 */
typedef struct HAnimRotKeyFrame  HAnimRotKeyFrame;
struct HAnimRotKeyFrame
{
    HAnimRotKeyFrame   *prevFrame;
    RwReal              time;
    RtQuat              q;
};

typedef struct HAnimRotInterpFrame  HAnimRotInterpFrame;
struct HAnimRotInterpFrame
{
    HAnimRotKeyFrame   *keyFrame1;
    HAnimRotKeyFrame   *keyFrame2;
    RtQuat              q;
};

/*
 ******************************************************************************
 */
#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RwBool
HAnimRotKeyFrameRegister(void);

extern RtAnimAnimation *
HAnimExtractRotAnimFromStdAnim(RtAnimAnimation *stdAnim);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* KEYFRAME_H */
