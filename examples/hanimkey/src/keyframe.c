
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
 * keyframe.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the usage of custom RtAnimAnimation 
 *          keyframe interpolation schemes.
 * *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rtquat.h"
#include "rphanim.h"

#include "keyframe.h"

#define EPSILON (0.001f)

/*
 ******************************************************************************
 */
static void
HAnimRotKeyFrameToMatrix(void *mtx, void *voidIFrame)
{
    RwMatrix *matrix = (RwMatrix *)mtx;
    HAnimRotInterpFrame *iFrame = (HAnimRotInterpFrame *)(voidIFrame);

    /*
     *  Convert the interpolated keyframe to a modeling matrix for the
     *  bone hierarchy.
     */
    RtQuatUnitConvertToMatrix(&iFrame->q, matrix);

    return;
}

/*
 ******************************************************************************
 */
static void
HAnimRotKeyFrameBlend(void *voidOut, void *voidIn1, void *voidIn2, RwReal alpha)
{
    HAnimRotInterpFrame   *out = (HAnimRotInterpFrame *) voidOut;
    HAnimRotInterpFrame   *in1 = (HAnimRotInterpFrame *) voidIn1;
    HAnimRotInterpFrame   *in2 = (HAnimRotInterpFrame *) voidIn2;
    RwReal              beta, cosTheta;

    /*
     *  Perform a spherical linear interpolation of quaternions (or SLERP).
     *  More information can be found in "Advanced Animation and Rendering
     *  Techniques" by Alan Watt and Mark Watt.
     */

    /*
     *  Compute 4D dot product. This is equal to the cosine of the angle
     *  between quaternions on a hypersphere.
     */
    cosTheta = RwV3dDotProduct(&in1->q.imag, &in2->q.imag) 
                + in1->q.real * in2->q.real;

    /*
     *  If quaternions are in opposite hemispheres, then flip one.
     */
    if (cosTheta < 0.0f)
    {
        cosTheta = - cosTheta;
        RwV3dNegate(&in2->q.imag, &in2->q.imag);
        in2->q.real = - in2->q.real;
    }

    /*
     *  Set factors to do linear interpolation, as a special case where the
     *  quaternions are close together. 
     */
    beta = 1.0f - alpha;

    /*
     *  If the quaternions aren't close, proceed with spherical interpolation.
     */
    if (cosTheta < (1.0f - EPSILON))
    {
        RwReal  theta, cosecTheta;

        theta = (RwReal) RwACos(cosTheta);
        cosecTheta = 1.0f / (RwReal) RwSin(theta);
        beta  = cosecTheta * (RwReal) RwSin(beta * theta);
        alpha = cosecTheta * (RwReal) RwSin(alpha * theta);
    }

    /*
     *  Do the interpolation
     */
    out->q.imag.x = beta * in1->q.imag.x + alpha * in2->q.imag.x;
    out->q.imag.y = beta * in1->q.imag.y + alpha * in2->q.imag.y;
    out->q.imag.z = beta * in1->q.imag.z + alpha * in2->q.imag.z;
    out->q.real   = beta * in1->q.real   + alpha * in2->q.real;

    return;
}

/*
 ******************************************************************************
 */
static void
HAnimRotKeyFrameInterpolate(void *voidOut, void *voidIn1, void *voidIn2, 
                            RwReal time, void *customData __RWUNUSED__)
{
    /* HAnimRotKeyFrame *out = (HAnimRotKeyFrame *) voidOut; */
    HAnimRotKeyFrame *in1 = (HAnimRotKeyFrame *) voidIn1;
    HAnimRotKeyFrame *in2 = (HAnimRotKeyFrame *) voidIn2;
    const RwReal fraction = ( (time - in1->time) / 
                              (in2->time - in1->time) );

    /* Treat keyframes as interp frames, since they have the same structure
     * apart from the header data, which is not involved here. 
     */
    HAnimRotKeyFrameBlend(voidOut, voidIn1, voidIn2, fraction);

    return;
}

/*
 ******************************************************************************
 */
static RwBool
HAnimRotKeyFrameStreamWrite(const RtAnimAnimation *animation, RwStream *stream)
{
    RwInt32             i;
    HAnimRotKeyFrame   *frames = (HAnimRotKeyFrame *)animation->pFrames;

    /*
     *  Write the keyframes of the given animation to the binary stream.
     */
    for (i=0; i<animation->numFrames; i++)
    {
        RwInt32     temp;

        /*
         *  Calculate memory offset for previous frame
         */
        temp = (RwInt32)frames[i].prevFrame - (RwInt32)frames;

        /*
         *  Write out time, quaternion, and offset.
         */
        if (   !RwStreamWriteReal(
                    stream, (RwReal *)&frames[i].time, 5 * sizeof(RwReal))
            || !RwStreamWriteInt(
                    stream, (RwInt32 *)&temp, sizeof(RwInt32)) )
        {
            return(FALSE);
        }
    }

    return (TRUE);
}

/*
 ******************************************************************************
 */
static RtAnimAnimation *
HAnimRotKeyFrameStreamRead(RwStream *stream, RtAnimAnimation *animation)
{
    RwInt32             i;
    HAnimRotKeyFrame  *frames;

    /*
     *  Read keyframes from a binary stream. 
     */
    frames = (HAnimRotKeyFrame *) animation->pFrames;

    for (i = 0; i < animation->numFrames; i++)
    {
        RwInt32     temp;

        /*
         *  Read the keyframe time and quaternion, then the previous frame
         *  memory offset.
         */
        if (    !RwStreamReadReal(
                    stream, (RwReal *) &frames[i].time, 5 * sizeof(RwReal))
            ||  !RwStreamReadInt(
                    stream, (RwInt32 *) &temp, sizeof(RwInt32)))
        {
            return((RtAnimAnimation *)NULL);
        }

        /*
         *  Fix up the previous frame pointer.
         */
        frames[i].prevFrame = (HAnimRotKeyFrame *) ((RwInt32)frames + temp);
    }

    return (animation);
}

/*
 ******************************************************************************
 */
static RwInt32
HAnimRotKeyFrameStreamGetSize(const RtAnimAnimation *animation)
{
    /*
     *  Stream size for keyframes of the given animation.
     */
    return (animation->numFrames * (5 * sizeof(RwReal) + sizeof(RwInt32)));
}

/*
 ******************************************************************************
 */
static void
HAnimRotKeyFrameMulRecip(void *voidAnimFrame, void *voidStartFrame)
{
    HAnimRotKeyFrame   *animFrame = (HAnimRotKeyFrame *)voidAnimFrame;
    HAnimRotKeyFrame   *startFrame = (HAnimRotKeyFrame *)voidStartFrame;
    RtQuat              recip, temp;

    /*  
     *  Multiply the first keyframe (in place) by the reciprocal of the second.
     *  This callback will be used when genereting a delta animation.
     */
    temp = animFrame->q;
    RtQuatReciprocal(&recip, &startFrame->q);
    RtQuatMultiply(&animFrame->q, &recip, &temp);

    return;
}

/*
 ******************************************************************************
 */
static void
HAnimRotKeyFrameAdd(void *voidOut, void *voidIn1, void *voidIn2)
{
    HAnimRotInterpFrame *out = (HAnimRotInterpFrame *)voidOut;
    HAnimRotInterpFrame *in1 = (HAnimRotInterpFrame *)voidIn1;
    HAnimRotInterpFrame *in2 = (HAnimRotInterpFrame *)voidIn2;

    /*
     *  Add two keyframes together (this is equivalent to multiplicative
     *  combination of quaternions). This callback is used for blending
     *  between hierarchies.
     */
    RtQuatMultiply(&out->q, &in1->q, &in2->q);

    return;
}

/*
 ******************************************************************************
 */
RwBool
HAnimRotKeyFrameRegister(void)
{
    RtAnimInterpolatorInfo     info;

    /*
     *  Register the custom keyframe interpolation scheme.
     */
    info.typeID = HANIMROTKEYFRAMEID;
    info.animKeyFrameSize = sizeof(HAnimRotKeyFrame);
    info.interpKeyFrameSize = sizeof(HAnimRotKeyFrame);
    info.keyFrameApplyCB           = HAnimRotKeyFrameToMatrix;
    info.keyFrameBlendCB           = HAnimRotKeyFrameBlend;
    info.keyFrameInterpolateCB     = HAnimRotKeyFrameInterpolate;
    info.keyFrameAddCB             = HAnimRotKeyFrameAdd;
    info.keyFrameMulRecipCB        = HAnimRotKeyFrameMulRecip;
    info.keyFrameStreamReadCB      = HAnimRotKeyFrameStreamRead;
    info.keyFrameStreamWriteCB     = HAnimRotKeyFrameStreamWrite;
    info.keyFrameStreamGetSizeCB   = HAnimRotKeyFrameStreamGetSize;
    info.customDataSize            = 0;

    if (!RtAnimRegisterInterpolationScheme(&info))
    {
        return FALSE;
    }

    return TRUE;
}

/*
 ******************************************************************************
 */
RtAnimAnimation *
HAnimExtractRotAnimFromStdAnim(RtAnimAnimation *stdAnim)
{
    RtAnimAnimation    *rotAnim;

    /*
     *  Extract the quaternions from a standard animation into an animation
     *  of our custom type which contains only the rotation components of the
     *  bones.
     */
    rotAnim = RtAnimAnimationCreate(HANIMROTKEYFRAMEID, stdAnim->numFrames,
                    stdAnim->flags, stdAnim->duration);

    if (rotAnim)
    {
        RwInt32    i;
        RpHAnimKeyFrame *stdFrames = (RpHAnimKeyFrame *)stdAnim->pFrames;
        HAnimRotKeyFrame   *rotFrames = (HAnimRotKeyFrame *)rotAnim->pFrames;

        for (i=0; i<stdAnim->numFrames; i++)
        {
            /* 
             *  Set the previous frame pointer.
             */
            rotFrames[i].prevFrame = rotFrames + 
                    ((RwInt32)stdFrames[i].prevFrame - (RwInt32)stdFrames) /
                        sizeof(RpHAnimKeyFrame);

            /*
             *  Copy time and quaternion rotation.
             */
            rotFrames[i].time = stdFrames[i].time;
            rotFrames[i].q = stdFrames[i].q;
        }
    }                   

    return rotAnim;
}
