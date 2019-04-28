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
 * main.c
 *
 * Copyright (C) 2002 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: This Example demonstrates the use of the rtanim toolkit. *
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rtanim.h"

#include "skeleton.h"

#include "anim.h"

#include "string.h"

typedef struct NodesKeyframes NodesKeyframes;

struct NodesKeyframes
{
    void *Keyframes;
    RwInt32 numKeyFrames;
};


static RtAnimAnimation *LightAnimation;

RtAnimInterpolator *AnimInterpolator;

static void
LightKeyFrameApply(void *light, void *IFrame)
{
    RpLight *result = (RpLight*)light;
    LightInterpFrame *app = (LightInterpFrame *)IFrame;

    RpLightSetColor(result,&app->color);
    RpLightSetRadius(result,app->radius);

    return;
}

static void
LightKeyFrameInterpolate(void * Out, void * In1, void * In2,
                         RwReal time, void *customData __RWUNUSED__)
{
    LightInterpFrame *out = (LightInterpFrame *)Out;
    LightKeyFrame *in1 = (LightKeyFrame *)In1;
    LightKeyFrame *in2 = (LightKeyFrame *)In2;
    RwReal fAlpha = ((time - in1->time) /(in2->time - in1->time));

    out->color.red   = in1->color.red   + fAlpha * (in2->color.red   - in1->color.red);
    out->color.green = in1->color.green + fAlpha * (in2->color.green - in1->color.green);
    out->color.blue  = in1->color.blue  + fAlpha * (in2->color.blue  - in1->color.blue);
    out->color.alpha = in1->color.alpha + fAlpha * (in2->color.alpha - in1->color.alpha);

    out->radius = in1->radius + fAlpha * (in2->radius - in1->radius);
    return;
}

static void
LightKeyFrameBlend(void * Out, void * In1, void * In2, RwReal fAlpha)
{
    LightInterpFrame *out = (LightInterpFrame *)Out;
    LightInterpFrame *in1 = (LightInterpFrame *)In1;
    LightInterpFrame *in2 = (LightInterpFrame *)In2;

    out->color.red   = in1->color.red   + fAlpha * (in2->color.red   - in1->color.red);
    out->color.green = in1->color.green + fAlpha * (in2->color.green - in1->color.green);
    out->color.blue  = in1->color.blue  + fAlpha * (in2->color.blue  - in1->color.blue);
    out->color.alpha = in1->color.alpha + fAlpha * (in2->color.alpha - in1->color.alpha);

    out->radius = in1->radius + fAlpha * (in2->radius - in1->radius);

    return;
}

static void
LightKeyFrameAdd(void * Out, void * In1, void * In2)
{
    LightInterpFrame *out = (LightInterpFrame *)Out;
    LightInterpFrame *in1 = (LightInterpFrame *)In1;
    LightInterpFrame *in2 = (LightInterpFrame *)In2;

    RwRGBARealAdd(&out->color, &in1->color, &in2->color);
    out->radius = in1->radius + in2->radius;

    return;
}

static RwBool
AnimOrderAndSetKeyframes(RtAnimAnimation *dst, NodesKeyframes *nodes, RwInt32 numNodes)
{
    RwInt32     *numFrames;
    RwUInt32    **prevKey;
    RwInt32     i,j;
    RwChar      msg[256];
    RwUInt32    *destination;
    RwUInt32    **currentKey;
    RwInt32     keyframeSize;
    RwInt32     keyframesLeft;
    RwInt32 minNodes;
    RwReal minTime;

    for(i=0;i<numNodes;i++)
    {
        if(nodes[i].numKeyFrames <2)
        {
            RsSprintf(msg,"Error : node %d got less than 2 keyframes",i);
            RsErrorMessage(msg);

            return FALSE;
        }
    }

    prevKey = (RwUInt32 **)RwMalloc(sizeof(RwUInt32*)*numNodes,
                                    rwID_NAOBJECT);
    currentKey = (RwUInt32 **)RwMalloc(sizeof(RwUInt32*)*numNodes,
                                       rwID_NAOBJECT);
    numFrames = (RwInt32*)RwMalloc(sizeof(RwInt32)*numNodes,
                                   rwID_NAOBJECT);

    memset(prevKey,0,sizeof(RwUInt32*)*numNodes);
    keyframesLeft = 0;

    for(i=0;i<numNodes;i++)
    {
        currentKey[i] = nodes[i].Keyframes;
        numFrames[i] = nodes[i].numKeyFrames;
        keyframesLeft += nodes[i].numKeyFrames;
    }
    destination = dst->pFrames;
    keyframeSize = dst->interpInfo->animKeyFrameSize;

    /* Copy the first two key frames to the animation... */
    for(j=0;j<2;j++)
    {
        for(i=0;i<numNodes;i++)
        {
            RwChar  *destLvalue;

            RwChar  *curKeyLvalue;


            memcpy(destination,currentKey[i],keyframeSize);
            ((RtAnimKeyFrameHeader*)destination)->prevFrame = prevKey[i];
            prevKey[i] = destination;

            destLvalue = (RwChar *)destination;
            destLvalue += keyframeSize;
            destination = (RwUInt32 *)destLvalue;

            curKeyLvalue = (RwChar *)(currentKey[i]);
            curKeyLvalue += keyframeSize;
            currentKey[i] = (RwUInt32 *)curKeyLvalue;

            numFrames[i]--;
            keyframesLeft--;
        }
    }

    for(j=0;j<keyframesLeft;j++)
    {
        RwChar  *destLvalue;

        RwChar  *curKeyLvalue;


        minNodes = -1;
        minTime = RwRealMAXVAL;
        /* Search for the recently processed keyframe with the smallest time */
        for(i=0;i<numNodes;i++)
        {
            if((((RtAnimKeyFrameHeader*)prevKey[i])->time < minTime)
                && (numFrames[i] != 0))
            {
                minTime = ((RtAnimKeyFrameHeader*)prevKey[i])->time;
                minNodes = i;
            }
        }

        /* Copy the finded keyframe to the animation */
        memcpy(destination,currentKey[minNodes],keyframeSize);
        ((RtAnimKeyFrameHeader*)destination)->prevFrame = prevKey[minNodes];
        prevKey[minNodes] = destination;

        destLvalue = (RwChar *)destination;
        destLvalue += keyframeSize;
        destination = (RwUInt32 *)destLvalue;

        curKeyLvalue = (RwChar *)(currentKey[minNodes]);
        curKeyLvalue += keyframeSize;
        currentKey[minNodes] = (RwUInt32 *)curKeyLvalue;

        numFrames[minNodes]--;
    }

    /* A bit of clean up */
    RwFree(prevKey);
    RwFree(currentKey);
    RwFree(numFrames);

    return TRUE;
}

#define SetColor(k,r,g,b)\
    k->color.red = r;\
    k->color.green = g;\
    k->color.blue = b;\
    k->color.alpha = 1.0f;\

RwBool
AnimCreate(void)
{
    RwBool result = TRUE;

    RtAnimInterpolatorInfo interpInfo;
    RtAnimAnimation *anim;
    LightKeyFrame *frames;

    NodesKeyframes keyframes[3];
    /* Create interpolation scheme */

    interpInfo.typeID = 0x0900CAFE;

    interpInfo.animKeyFrameSize = sizeof(LightKeyFrame);
    interpInfo.interpKeyFrameSize = sizeof(LightKeyFrame);

    interpInfo.keyFrameApplyCB = LightKeyFrameApply;
    interpInfo.keyFrameBlendCB = LightKeyFrameBlend;
    interpInfo.keyFrameInterpolateCB = LightKeyFrameInterpolate;
    interpInfo.keyFrameAddCB = LightKeyFrameAdd;
    interpInfo.keyFrameMulRecipCB = NULL; /* no Mulrecip callback */
    interpInfo.keyFrameStreamReadCB = NULL; /* no streaming callback */
    interpInfo.keyFrameStreamWriteCB = NULL; /* no streaming callback */
    interpInfo.keyFrameStreamGetSizeCB = NULL; /* no streaming callback */
    interpInfo.customDataSize = 0;

    /* Registering the scheme */
    result = RtAnimRegisterInterpolationScheme(&interpInfo);

    if( TRUE == result )
    {
        /* Create Animation */
        anim = RtAnimAnimationCreate(0x0900CAFE,21,0,10.0f);

        if( NULL != anim )
        {
            /* Light 1 Keyframes */

            keyframes[0].Keyframes = (LightKeyFrame*)RwMalloc(sizeof(LightKeyFrame)*7,
                                                              rwID_NAOBJECT);
            keyframes[0].numKeyFrames = 7;

            frames = keyframes[0].Keyframes;

            frames->time = 0.0f;
            SetColor(frames,0.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 1.0f;
            SetColor(frames,1.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 3.0f;
            SetColor(frames,0.0f,1.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 5.0f;
            SetColor(frames,0.0f,0.0f,1.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 7.0f;
            SetColor(frames,0.0f,1.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 9.0f;
            SetColor(frames,1.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 10.0f;
            SetColor(frames,0.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            /* Light 2 Animation */
            keyframes[1].Keyframes = RwMalloc(sizeof(LightKeyFrame)*11,
                                              rwID_NAOBJECT);
            keyframes[1].numKeyFrames = 11;

            frames = keyframes[1].Keyframes;

            frames->time = 0.0f;
            SetColor(frames,0.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 1.0f;
            SetColor(frames,1.0f,1.0f,1.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 2.0f;
            SetColor(frames,0.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 3.0f;
            SetColor(frames,1.0f,1.0f,1.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 4.0f;
            SetColor(frames,0.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 5.0f;
            SetColor(frames,1.0f,1.0f,1.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 6.0f;
            SetColor(frames,0.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 7.0f;
            SetColor(frames,1.0f,1.0f,1.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 8.0f;
            SetColor(frames,0.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 9.0f;
            SetColor(frames,1.0f,1.0f,1.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 10.0f;
            SetColor(frames,0.0f,0.0f,0.0f);
            frames->radius = 0.0f;
            frames++;

            /* Light 3 Animation */
            keyframes[2].Keyframes = RwMalloc(sizeof(LightKeyFrame)*3,
                                              rwID_NAOBJECT);
            keyframes[2].numKeyFrames = 3;

            frames = keyframes[2].Keyframes;

            frames->time = 0.0f;
            SetColor(frames,1.0f,1.0f,1.0f);
            frames->radius = 1000.0f;
            frames++;

            frames->time = 5.0f;
            SetColor(frames,1.0f,1.0f,1.0f);
            frames->radius = 0.0f;
            frames++;

            frames->time = 10.0f;
            SetColor(frames,1.0f,1.0f,1.0f);
            frames->radius = 1000.0f;
            frames++;

            AnimOrderAndSetKeyframes(anim, keyframes, 3);


            RwFree(keyframes[0].Keyframes);
            RwFree(keyframes[1].Keyframes);
            RwFree(keyframes[2].Keyframes);

            LightAnimation = anim;

            AnimInterpolator = RtAnimInterpolatorCreate(3,sizeof(LightKeyFrame));

            RtAnimInterpolatorSetCurrentAnim(AnimInterpolator,LightAnimation);

            RtAnimInterpolatorSetCurrentTime(AnimInterpolator,0.0f);
        }
    }

    return result;
}


void
AnimDestroy(void)
{
    if( LightAnimation )
    {
        RtAnimAnimationDestroy(LightAnimation);
    }

    if ( AnimInterpolator )
    {
        RtAnimInterpolatorDestroy(AnimInterpolator);
    }
}


void
LightAnimApply(RpLight **lights,RtAnimInterpolator *animI)
{
    LightInterpFrame *iframes;
    RwInt32 i;
    RtAnimKeyFrameApplyCallBack callback;
    RwInt32 typeID;
    RtAnimAnimation *currentAnim;

    currentAnim = RtAnimInterpolatorGetCurrentAnim(animI);
    typeID = RtAnimAnimationGetTypeID(currentAnim);

    callback = RtAnimGetInterpolatorInfo(typeID)->keyFrameApplyCB;


    for(i=0;i<animI->numNodes;i++)
    {
        iframes = (LightInterpFrame *)rtANIMGETINTERPFRAME(animI,i);
        (callback)((void*)(*lights),(void*)iframes);
        lights++;
    }
}
