
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
 * hanimsub.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the usage of sub-hierarchical animations.
 * *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpskin.h"
#include "rphanim.h"
#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"
#include "main.h"
#include "hanimsub.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#ifdef RWMETRICS
#include "metrics.h"
#endif

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
     * Find the first hierarchy in the clump... 
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
RwBool
LoadAnimations(void)
{
    static const RwChar *files[MAX_ANIMATIONS] =
    {
        RWSTRING("models/tophalf.anm"),
        RWSTRING("models/lleg.anm"),
        RWSTRING("models/rleg.anm")
    };

    RwInt32 i;

    for(i=0; i<MAX_ANIMATIONS; i++)
    {
        RwChar *path = NULL;

        path = RsPathnameCreate(files[i]);
        Anim[i] = RtAnimAnimationRead(path);
        RsPathnameDestroy(path);

        if( Anim[i] == NULL )
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

/*
 ****************************************************************************
 */
static RwInt32
FindFrameFromBoneID(RpHAnimHierarchy *hierarchy, RwInt32 boneID)
{
    RwInt32 i;
    RwInt32 frameID = -1;

    /*
     * Find the frame index for the specified bone index in the model...
     */
    for(i=0; i<hierarchy->numNodes; i++)
    {
        if( boneID == hierarchy->pNodeInfo[i].nodeID )
        {
            frameID = i;
        }
    }

    /* 
     * Check the frame index was successfully found... 
     */
    if( frameID == -1 )
    {
        RwChar msg[256];
        
        RsSprintf(msg, RWSTRING("Could not find frame ID for boneID: %d, check\
exported number matches boneID"), boneID);

        RsErrorMessage(msg);
    }

    return frameID;
}

/*
 ****************************************************************************
 */
static RpHAnimHierarchy *
CreateSubHierarchy(RpHAnimHierarchy *parentHierarchy, RwInt32 boneID)
{
    RpHAnimHierarchy *subHierarchy;
    RpHAnimHierarchyFlag flags;
    RwInt32 startFrame;

    flags = (RpHAnimHierarchyFlag)RpHAnimHierarchyGetFlags(parentHierarchy);

    /* 
     * Find the frame to start the sub-hierarchy at...
     */
    startFrame = FindFrameFromBoneID(parentHierarchy, boneID);

    subHierarchy = RpHAnimHierarchyCreateSubHierarchy(parentHierarchy,
        startFrame, flags, -1);

    return subHierarchy;
}

/*
 ****************************************************************************
 */
void
SetupHierarchies(RpClump* clump)
{
    RwInt32 i;
    RpHAnimHierarchy *hierarchy = NULL;

    /*
     * These values are the bone IDs that are outputted during export. The first
     * entry is for the base hierarchy and is used as a place holder. For your 
     * own models these values may be different...
     */
    RwInt32 boneID[MAX_ANIMATIONS] = {0, 1023, 1077};

    hierarchy = GetHierarchy(clump);

    RpClumpForAllAtomics(clump, SetHierarchyForSkinAtomic, (void *)hierarchy);

    /* 
     * Set the base hierarchy... 
     */
    Hierarchies[0] = hierarchy;

    /* 
     * Attach the RwFrame hierarchy to the base hierarchy, so the frames will
     * be updated when the hierarchy is...
     */
    RpHAnimHierarchyAttach(Hierarchies[0]);

    /* 
     * Set the animations for each hierarchy. First the base...
     */
	RtAnimInterpolatorSetCurrentAnim( Hierarchies[ 0 ]->currentAnim, Anim[0]);

    /* 
     * ...then the sub-hierarchies...
     */
    for(i=0; i<MAX_SUBHIERARCHIES; i++)
    {
        /* 
         * Create the sub-hierarchy...
         */
        Hierarchies[i+1] = CreateSubHierarchy(hierarchy, boneID[i+1]);

        /* 
         * and set its animation...
         */
		RtAnimInterpolatorSetCurrentAnim( Hierarchies[ i+1 ]->currentAnim, Anim[i+ 1]);
    }

    /* 
     * Set up the initial status of the animations, the base animation will be
     * running...
     */
    PlayAnim[0] = TRUE;

    /*
     * ...the sub-animations will not...
     */
    for(i=1; i<MAX_ANIMATIONS; i++)
    {
        PlayAnim[i] = FALSE;
    }

    return;
}


/*
 ****************************************************************************
 */
void
UpdateAnimations(RpHAnimHierarchy** hierarchy, RwReal deltaTime)
{
    RwInt32 i;
    
    /* 
     * Loop over all the possible animations and update there time... 
     */
    for(i=0; i<MAX_ANIMATIONS; i++)
    {
        /* 
         * ...only if the animation is enabled... 
         */
        if( PlayAnim[i] == TRUE )
        {
			RtAnimInterpolatorAddAnimTime( Hierarchies[ i ]->currentAnim, deltaTime);
        }

        /* 
         * Update the matrices if this is the base animation
         * or the given sub-hierarchical animation is enabled 
         */
        if( (PlayAnim[i] == TRUE) || (i == 0) )
        {
            RpHAnimUpdateHierarchyMatrices(hierarchy[i]);
        }
    }

    return;
}





/*
 *****************************************************************************
 */
