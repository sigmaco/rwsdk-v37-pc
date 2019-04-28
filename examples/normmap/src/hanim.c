
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

static RpHAnimHierarchy *OutHierarchy = NULL;
static RpHAnimHierarchy *InHierarchy2 = NULL;

static RtAnimAnimation *Anim = NULL;

/*
 *****************************************************************************
 */
static RtAnimAnimation *
LoadAnimationFile(const RwChar *file)
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
LoadClumpAnimation(RpClump *clump, const RwChar *filename)
{
    RpHAnimHierarchy *hierarchy = NULL;
    RwChar file[256];

    hierarchy = GetHierarchy(clump);

    RpClumpForAllAtomics(clump, SetHierarchyForSkinAtomic, (void *)hierarchy);

    /*
     * ...and then the animation files, assuming they are named "anim0.anm",
     * "anim1.anm" etc...
     */
        
    RsSprintf(file, RWSTRING("models/%s.anm"), filename);

    Anim = LoadAnimationFile(file);

    if( Anim == NULL )
    {
        return NULL;
    }

    RpHAnimHierarchySetCurrentAnim(hierarchy, Anim);
    RpHAnimHierarchySetCurrentAnimTime(hierarchy, 0.0f);

    return clump;
}

/*
 *****************************************************************************
 */
void
DestroyClumpAnimation(RpClump *clump)
{
    /*
     * Destroy only the hierarchies and animations the app created...
     */

    if( Anim )
    {
        RtAnimAnimationDestroy(Anim);

        Anim = NULL;
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
UpdateClumpAnimation(RpClump *clump, RwReal deltaTime)
{
    RpHAnimHierarchy *hierarchy = NULL;

    RpClumpForAllAtomics(clump, GetSkinHierarchy, (void *)&hierarchy);

    RpHAnimHierarchyAddAnimTime(hierarchy, deltaTime);

    RpHAnimHierarchyUpdateMatrices(hierarchy);

    return;
}

/*
 *****************************************************************************
 */
