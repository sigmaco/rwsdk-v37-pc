
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
 * Copyright (C) 2001 CCriterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how two H-anim sequences can be blended together.
 * *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rphanim.h"
#include "rpskin.h"
#include "skeleton.h"

#include "hanim.h"

/*
 * The total number of animations this example will run...
 */
#define NUMANIMS (2)

static RtAnimAnimation *Anim[NUMANIMS];

static RpHAnimHierarchy *OutHierarchy = NULL;
static RpHAnimHierarchy *InHierarchy2 = NULL;

enum AnimationState CurrentAnimation = FIRST;

/*
 * Blending between the two animations will initially run over 1 second,,,
 */
RwReal BlendDuration = 1.0f;



/*
 *****************************************************************************
 */
static RtAnimAnimation *
LoadAnimationFile(RwChar *file)
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
static RpClump *
LoadClump(RpWorld *world)
{
    RpClump *clump = NULL;
    RwStream *stream = NULL;
    RwChar *path = NULL;

    path = RsPathnameCreate(RWSTRING("models/clump.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            clump = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    if( clump )
    {
        RwV3d yAxis = {0.0f, 1.0f, 0.0f};
        RwV3d pos = {0.0f, 0.0f, 150.0f};

        RwFrameRotate(RpClumpGetFrame(clump), &yAxis, 180.0f,
            rwCOMBINEREPLACE);

        RwFrameTranslate(RpClumpGetFrame(clump), &pos,
            rwCOMBINEPOSTCONCAT);

        RpWorldAddClump(world, clump);
    }

    return clump;
}


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
RpClump *
CreateClump(RpWorld *world)
{
    RpClump *clump = NULL;  
    RpHAnimHierarchy *hierarchy = NULL;
    RpHAnimHierarchyFlag flags;
    RwInt32 i;

    /*
     * Load the clump that contains the skinned atomics and hierarchies, the
     * animations that will be run on the hierarchies, and create the
     * temporary hierarchies that will be used for blending between the two
     * animations...
     */

    clump = LoadClump(world);
    if( clump == NULL )
    {
        return NULL;
    }

    hierarchy = GetHierarchy(clump);

    RpClumpForAllAtomics(clump, SetHierarchyForSkinAtomic, (void *)hierarchy);

    /*
     * ...and then the animation files, assuming they are named "anim0.anm",
     * "anim1.anm" etc...
     */
    for(i=0; i<NUMANIMS; i++)
    {
        RwChar file[256];

        RsSprintf(file, RWSTRING("models/anim%d.anm"), i);

        Anim[i] = LoadAnimationFile(file);

        if( Anim[i] == NULL )
        {
            return NULL;
        }
    }

    /*
     * We will play this animation first...
     */
    RpHAnimHierarchySetCurrentAnim(hierarchy, Anim[0]);

    /*
     * Create the hierarchies that will be used for blending between the two
     * animations...
     */

    flags = (RpHAnimHierarchyFlag)hierarchy->flags;

    OutHierarchy = 
        RpHAnimHierarchyCreateFromHierarchy(hierarchy,
                                            flags, 
                                            hierarchy->currentAnim->maxInterpKeyFrameSize);

    InHierarchy2 = 
        RpHAnimHierarchyCreateFromHierarchy(hierarchy,
                                            flags, 
                                            hierarchy->currentAnim->maxInterpKeyFrameSize);

    /*
     * For the OutHierarchy the app will use the standard key frame callbacks...
     */
    RpHAnimHierarchySetKeyFrameCallBacks(OutHierarchy, rpHANIMSTDKEYFRAMETYPEID);

    /*
     * ...also give this hierarchy a parent frame. This requires the app to
     * set the parentFrame to NULL before a call to RpHAnimHierarchyDestroy...
     */
    OutHierarchy->parentFrame = hierarchy->parentFrame;

    return clump;
}


/*
 *****************************************************************************
 */
void
DestroyClump(RpClump *clump, RpWorld *world)
{
    RwInt32 i;

    if( clump )
    {
        RpWorldRemoveClump(world, clump);

        RpClumpDestroy(clump);
    }

    /*
     * Destroy only the hierarchies and animations the app created...
     */

    for(i=0; i<NUMANIMS; i++)
    {
        if( Anim[i] )
        {
            RtAnimAnimationDestroy(Anim[i]);
        }
    }

    if( OutHierarchy )
    {
        OutHierarchy->parentFrame = NULL;

        RpHAnimHierarchyDestroy(OutHierarchy);
    }

    if( InHierarchy2 )
    {
        RpHAnimHierarchyDestroy(InHierarchy2);
    }

    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
GetSkinHierarchy(RpAtomic *atomic, void *data)
{
    *(void **)data = (void *)RpSkinAtomicGetHAnimHierarchy(atomic);
    
    return NULL;
}


/*
 *****************************************************************************
 */
void
UpdateAnimation(RpClump *clump, RwReal deltaTime)
{
    static RwReal blendDuration = 0.0f;
    static enum AnimationState previousAnimation = FIRST;
    static RpHAnimHierarchy *inHierarchy1 = NULL;
    static RtAnimAnimation *currentAnim1 = NULL;
    static RtAnimAnimation *currentAnim2 = NULL;

    /*
     * Updates in seconds the animation currently running by deltaTime. If
     * the end of the current animation is reached we blend between the end of
     * the current animation with the start of the next animation. Once the
     * blend has finished the next animation will be running.
     */

    if( blendDuration <= 0.0f )
    {
        RwReal duration;

        /*
         * Blending between the animations is not required so run the current
         * animation...
         */

        RpClumpForAllAtomics(clump, GetSkinHierarchy, (void *)&inHierarchy1);

        currentAnim1 = RpHAnimHierarchyGetCurrentAnim(inHierarchy1);
        duration = currentAnim1->duration;

        if( (inHierarchy1->currentAnim->currentTime + deltaTime) >= duration )
        {
            /*
             * Adding deltaTime to the current animation time will cause the
             * animation to loop as it will pass its duration. Adjust
             * deltaTime so the animation will not loop...
             */
            deltaTime = duration - inHierarchy1->currentAnim->currentTime;

            /*
             * Use an optimized version of RpHAnimHierarchyAddAnimTime as the
             * hierarchy has standard key frames...
             */
            RpHAnimHierarchyAddAnimTime(inHierarchy1, deltaTime);

            /*
             * The animation has reached its end so get ready to blend
             * between the animations over BlendDuration seconds...
             */
            blendDuration = BlendDuration;

            previousAnimation = CurrentAnimation;
            CurrentAnimation = BLEND;

            /*
             * Find the next animation, assuming we only have two...
             */
            if( currentAnim1 == Anim[0] )
            {
                RpHAnimHierarchySetCurrentAnim(InHierarchy2, Anim[1]);
            }
            else
            {
                RpHAnimHierarchySetCurrentAnim(InHierarchy2, Anim[0]);
            }
        }
        else
        {
            /*
             * Adding deltaTime to the current animation will not cause 
             * it to loop...
             */

            RpHAnimHierarchyAddAnimTime(inHierarchy1, deltaTime);
        }

        RpHAnimHierarchyUpdateMatrices(inHierarchy1);
    }

    if( blendDuration > 0.0f )
    {
        RwReal alpha;

        /*
         * Currently blending between two animations, storing the results in
         * OutHierarchy.
         */

        /*
         * Calculate the blending parameter, 0.0 to return the current
         * hierarchy and 1.0 to return the next hierarchy. If the
         * total blendDuration does not equal 1 second then scale
         * blendDuration by the total number of seconds the blend is to run,
         * so alpha remains in the range [0,1]...
         */
        alpha = 1.0f - blendDuration / BlendDuration;

        RpClumpForAllAtomics(clump, SetHierarchyForSkinAtomic,
            (void *)OutHierarchy);

        RpHAnimHierarchyBlend(OutHierarchy, inHierarchy1, InHierarchy2,alpha);

        blendDuration -= deltaTime;

        RpHAnimHierarchyUpdateMatrices(OutHierarchy);

        if( blendDuration <= 0.0f )
        {
            /*
             * Blending between the hierarchies has ended so switch to running
             * the next animation...
             */

            RpClumpForAllAtomics(clump, SetHierarchyForSkinAtomic,
               (void *)inHierarchy1);

            currentAnim2 = RpHAnimHierarchyGetCurrentAnim(InHierarchy2);
            RpHAnimHierarchySetCurrentAnim(inHierarchy1,currentAnim2);

            RpHAnimHierarchyUpdateMatrices(inHierarchy1);

            blendDuration = 0.0f;

            if( previousAnimation == FIRST )
            {
                CurrentAnimation = SECOND;
            }
            else
            {
                CurrentAnimation = FIRST;
            }
        }
    }
    
    return;
}

/*
 *****************************************************************************
 */
