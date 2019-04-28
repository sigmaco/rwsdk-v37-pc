
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
 * hanim.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how two H-anim sequences can be run together,
 *          with the second animation being a delta of the first.
 * *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rphanim.h"
#include "rpskin.h"
#include "skeleton.h"
#include "hanim.h"

#define DELTA_SPEED (2.0f)

static RpHAnimHierarchy *BaseHierarchy = NULL;
static RpHAnimHierarchy *DeltaHierarchy = NULL;

static RtAnimAnimation *BaseAnim = NULL;
static RtAnimAnimation *DeltaAnim = NULL;

/*
 * The hierarchy that will store the results of adding the base and delta
 * hierarchies and is the hierarchy that is attached to the skinned atomics
 * that are rendered to show the result...
 */
static RpHAnimHierarchy *OutHierarchy = NULL;

/*
 * The animation that will be made a delta animation...
 */
static RtAnimAnimation *OutAnim = NULL;

RwInt32 CurrentAnimation = BASE_AND_DELTA_ANIM;


/*
 *****************************************************************************
 */
static RwFrame *
GetChildFrameHierarchy(RwFrame *frame, void *data)
{    
    RpHAnimHierarchy *hierarchy = *(RpHAnimHierarchy **)data;

    /*
     * Return the first hierarchy found that is attached to one of the atomic
     * frames...
     */

    hierarchy = RpHAnimFrameGetHierarchy(frame);
    if( hierarchy == NULL )
    {
        RwFrameForAllChildren(frame, GetChildFrameHierarchy, data);

        return frame;
    }

    *(void **)data = (void *)hierarchy;

    return NULL;
}


static RpHAnimHierarchy *
GetHierarchy(RpClump *clump)
{
    RpHAnimHierarchy *hierarchy = NULL;
    
    /*
     * Return the hierarchy for this model...
     */

    RwFrameForAllChildren(RpClumpGetFrame(clump), GetChildFrameHierarchy,
        (void *)&hierarchy);

    return hierarchy;
}


/*
 *****************************************************************************
 */
static RpAtomic *
SetHierarchyForSkinAtomic(RpAtomic *atomic, void *data)
{
    RpSkinAtomicSetHAnimHierarchy(atomic, (RpHAnimHierarchy *)data);
    
    return atomic;
}


/*
 *****************************************************************************
 */
void
CreateHierarchies(RpClump *baseClump, RpClump *deltaClump, RpClump *outClump)
{
    /*
     * For normal use the app does not need outClump, baseClump or deltaClump
     * can be used, nor does the app need to attach the BaseHierarchy or
     * DeltaHierarchy to any skinned atomics, only needs to attach
     * OutHierarchy to either the skinned atomics of baseClump or deltaClump
     * whichever one is going to be rendered...
     */

    BaseHierarchy = GetHierarchy(baseClump);

    RpClumpForAllAtomics(baseClump, SetHierarchyForSkinAtomic,
        (void *)BaseHierarchy);

    DeltaHierarchy = GetHierarchy(deltaClump);

    RpClumpForAllAtomics(deltaClump, SetHierarchyForSkinAtomic,
        (void *)DeltaHierarchy);

    OutHierarchy = GetHierarchy(outClump);

    /*
     * For this hierarchy the app will use the standard key frame
     * callbacks...
     */
    OutHierarchy->currentAnim->keyFrameApplyCB = RpHAnimKeyFrameApply;
    OutHierarchy->currentAnim->keyFrameAddCB = RpHAnimKeyFrameAdd;

    RpClumpForAllAtomics(outClump, SetHierarchyForSkinAtomic,
        (void *)OutHierarchy);

    return;
}


/*
 *****************************************************************************
 */
static RtAnimAnimation *
LoadAnimFile(const RwChar *file)
{
    RtAnimAnimation *anim = NULL;
    RwChar *path = NULL;

    path = RsPathnameCreate(file);
    anim = RtAnimAnimationRead(path);
    RsPathnameDestroy(path);

    return anim;
}


/*
 *****************************************************************************
 */
RwBool
CreateAnims(void)
{
    /*
     * The base animation is a figure which is running on the spot...
     */
    BaseAnim = LoadAnimFile(RWSTRING("models/base.anm"));
    if( BaseAnim == NULL )
    {
        return FALSE;
    }

    RpHAnimHierarchySetCurrentAnim(BaseHierarchy, BaseAnim);
    RpHAnimHierarchySetCurrentAnimTime(BaseHierarchy, 0.0f);

    /*
     * The delta animation is a figure turning its head to its left...
     */
    DeltaAnim = LoadAnimFile(RWSTRING("models/delta.anm"));
    if( DeltaAnim == NULL )
    {
        return FALSE;
    }

    /*
     * The app loads the delta animation a second time because the app allows
     * the switching between the different animations, we need to maintain the
     * original animation...
     */
    OutAnim = LoadAnimFile(RWSTRING("models/delta.anm"));
    if( OutAnim == NULL )
    {
        return FALSE;
    }

    /*
     * ...else you would use DeltaAnim as the parameter to
     * RpHAnimAnimationMakeDelta and not create OutAnim...
     */
    RtAnimAnimationMakeDelta(OutAnim, OutHierarchy->numNodes, 0.0f);

    RpHAnimHierarchySetCurrentAnim(DeltaHierarchy, OutAnim);
    RpHAnimHierarchySetCurrentAnimTime(DeltaHierarchy, 0.0f);

    return TRUE;
}


/*
 *****************************************************************************
 */
void
DestroyAnims(void)
{
    if( BaseAnim )
    {
        RtAnimAnimationDestroy(BaseAnim);
    }

    if( DeltaAnim )
    {
        RtAnimAnimationDestroy(DeltaAnim);
    }

    if( OutAnim )
    {
        RtAnimAnimationDestroy(OutAnim);
    }

    return;
}


/*
 *****************************************************************************
 */
RwBool
ChangeAnimationCallBack(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    switch( CurrentAnimation )
    {
        /*
         * When switching between which animations to show, the app needs to
         * make sure the correct delta animation is used...
         */

        case DELTA_ANIM:
        {
            /*
             * The user wants to see the delta animation...
             */
            RpHAnimHierarchySetCurrentAnim(DeltaHierarchy, DeltaAnim);

            break;
        }

        case BASE_AND_DELTA_ANIM:
        {
            /*
             * The user wants to see the result of adding the base and delta
             * hierarchies...
             */
            RpHAnimHierarchySetCurrentAnim(DeltaHierarchy, OutAnim);

            break;
        }
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
void
UpdateAnimation(RwReal deltaTime)
{
    switch( CurrentAnimation )
    {
        case BASE_ANIM:
        {
            /*
             * Just run the base animation, the figure running on the spot...
             */
            RpHAnimHierarchyAddAnimTime(BaseHierarchy, deltaTime);

            RpHAnimHierarchyUpdateMatrices(BaseHierarchy);

            break;
        }

        case DELTA_ANIM:
        {
            /*
             * Just run the delta animation, the figure turning its head...
             */
            RpHAnimHierarchyAddAnimTime(DeltaHierarchy,
                deltaTime * DELTA_SPEED);

            RpHAnimHierarchyUpdateMatrices(DeltaHierarchy);

            break;
        }

        case BASE_AND_DELTA_ANIM:
        {
            /*
             * Run the animation that is the result of adding the base and
             * delta hierarchies...
             */
            RpHAnimHierarchyAddAnimTime(BaseHierarchy, deltaTime);

            RpHAnimHierarchyAddAnimTime(DeltaHierarchy,
                deltaTime * DELTA_SPEED);

            RpHAnimHierarchyAddTogether(OutHierarchy, BaseHierarchy,
                DeltaHierarchy);

            RpHAnimHierarchyUpdateMatrices(OutHierarchy);

            break;
        }
    }

    return;
}

/*
 *****************************************************************************
 */
