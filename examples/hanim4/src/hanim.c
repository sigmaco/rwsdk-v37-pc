
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
 * Purpose: To illustrate how to compress H-anim sequences.
 * *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rphanim.h"
#include "rpskin.h"
#include "rtcmpkey.h"
#include "rtcmpkey.h"
#include "skeleton.h"

#include "hanim.h"

RtAnimAnimation *Anims[NUM_REDUCTION_METHODS][NUM_COMPRESSION_METHODS];
RwInt32          Sizes[NUM_REDUCTION_METHODS][NUM_COMPRESSION_METHODS];
RtAnimAnimation *CurrentAnim;

static const RwChar *filenames[NUM_REDUCTION_METHODS][NUM_COMPRESSION_METHODS] =
{
    {
        RWSTRING("models/uncompressed.anm"),
        RWSTRING("models/compressed.anm")
    },
    {
        RWSTRING("models/duplicates.anm"),
        RWSTRING("models/duplicatesthencomp.anm")
    },
    {
        RWSTRING("models/interpolates.anm"),
        RWSTRING("models/interpolatesthencomp.anm")
    },
    {
        RWSTRING("models/interpolatesloop.anm"),
        RWSTRING("models/interpolatestloophencomp.anm")
    },
    {
        RWSTRING("models/interpolatesloophier.anm"),
        RWSTRING("models/interpolatesloophierthencomp.anm")
    },
    {
        RWSTRING("models/interpolatesloophiercheck.anm"),
        RWSTRING("models/interpolatesloophiercheckthencomp.anm")
    }
};


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


RpHAnimHierarchy *
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
    RwChar           *path = NULL;
    RpClump          *clump = NULL;  
    RpHAnimHierarchy *hierarchy = NULL;
    RwInt32          index;
    RwUInt32         pushPopFlags[128], nodeIndex;


    /*
     * Load the clump that contains the skinned atomics and hierarchies, the
     * animations that will be run on the hierarchies
     */

    clump = LoadClump(world);
    if( clump == NULL )
    {
        return NULL;
    }

    hierarchy = GetHierarchy(clump);

    RpClumpForAllAtomics(clump, SetHierarchyForSkinAtomic, (void *)hierarchy);

    /*
     * load the animations
     */

    /* First load the original animation for use */
    path = RsPathnameCreate(filenames[0][0]);
    Anims[0][0] = LoadAnimationFile(path);
    RsPathnameDestroy(path);

    if(Anims[0][0] == NULL)
    {
        return NULL;
    }

    /* Need to have push pop flags. */
    nodeIndex = hierarchy->numNodes;
    while(nodeIndex--)
    {
        pushPopFlags[nodeIndex] = hierarchy->pNodeInfo[nodeIndex].flags;
    }

    /* Now try the reduced anims. */
    for(index = 1; index < NUM_REDUCTION_METHODS; index++)
    {
        /* Try to load the reduced animation */
        path = RsPathnameCreate(filenames[index][0]);
        Anims[index][0] = LoadAnimationFile(path);

        /* If it failed to load, recreate it from the uncompressed anim. */
        if(!Anims[index][0])
        {
            RwStream *stream = NULL;

            switch(index)
            {
                case 1:
                    /* Try the next reduction method: Duplicates. */
                    Anims[index][0] = RpHAnimRemoveDuplicates(Anims[0][0],
                        hierarchy->numNodes, 0.01f, 4.f);
                    break;

                case 2:
                    /* Try the next reduction method: Interpolates. */
                    Anims[index][0] = RpHAnimRemoveInterpolate(Anims[0][0],
                        hierarchy->numNodes, 0.00005f, 4.f);
                    break;

                case 3:
                    /* Try the next reduction method: Interpolates Loop. */
                    Anims[index][0] = RpHAnimRemoveInterpolateLoop(Anims[0][0],
                        hierarchy->numNodes, 0.0005f, 4.f);
                    break;

                case 4:
                    /* Try the next reduction method: Interpolates Loop
                       Hierarchy length check. */
                    Anims[index][0] = RpHAnimRemoveInterpolatesLoopLength(Anims[0][0],
                        pushPopFlags, hierarchy->numNodes, 0.001f, 0.25f);
                    break;

                case 5:
                    /* Try the next reduction method: Leaves. */
                    Anims[index][0] = RpHAnimRemoveNoLeafChange(Anims[0][0],
                        pushPopFlags, hierarchy->numNodes, 0.1f, 0.25f);
                    break;
            }

            /* And stream it out. */
            stream = RwStreamOpen(rwSTREAMWRITE, rwSTREAMFILENAME, path);
            if(stream != NULL)
            {
                RtAnimAnimationStreamWrite(Anims[index][0], stream);
                RwStreamClose(stream, NULL);
            }
        }

        RsPathnameDestroy(path);
    }

    /* Now try the compressed version from above. */
    for(index = 0; index < NUM_REDUCTION_METHODS; index++)
    {
        /* Try to load the compressed animation */
        path = RsPathnameCreate(filenames[index][1]);
        Anims[index][1] = LoadAnimationFile(path);

        /* If it failed to load, recreate it from the uncompressed anim. */
        if(!Anims[index][1])
        {
            RwStream *stream = NULL;

            /* Compress the animation */
            Anims[index][1] = RtCompressedKeyConvertFromStdKey(Anims[index][0]);

            /* And stream it out. */
            stream = RwStreamOpen(rwSTREAMWRITE, rwSTREAMFILENAME, path);
            RtAnimAnimationStreamWrite(Anims[index][1], stream);
            RwStreamClose(stream, NULL);
        }
        RsPathnameDestroy(path);
    }

    /* Calculate the animation sizes, bailing if an anim failed to load. */
    //TODO : work out exact
    {
        int i, j;
        for(i = 0; i < NUM_REDUCTION_METHODS; i++)
        {
            for(j = 0; j < NUM_COMPRESSION_METHODS; j++)
            {
                if(Anims[i][j] == NULL)
                {
                    return NULL;
                }

                Sizes[i][j] = RtAnimAnimationStreamGetSize(Anims[i][j]);
            }
        }
    }

    /*
     * We will play this animation first...
     */
    CurrentAnim = Anims[0][0];
    RpHAnimHierarchySetCurrentAnim(hierarchy, CurrentAnim);

    return clump;
}


/*
 *****************************************************************************
 */
void
DestroyClump(RpClump *clump, RpWorld *world)
{
    int i, j;
    if( clump )
    {
        RpWorldRemoveClump(world, clump);

        RpClumpDestroy(clump);
    }

    /*
     * Destroy only the hierarchies and animations the app created...
     */

    for(i = 0; i < NUM_REDUCTION_METHODS; i++)
    {
        for(j = 0; j < NUM_COMPRESSION_METHODS; j++)
        {
            RtAnimAnimationDestroy(Anims[i][j]);
        }
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
    static RpHAnimHierarchy *hierarchy = NULL;

    RpClumpForAllAtomics(clump, GetSkinHierarchy, (void *)&hierarchy);

    RpHAnimHierarchyAddAnimTime(hierarchy, deltaTime);
    
    RpHAnimHierarchyUpdateMatrices(hierarchy);

   
    return;
}

