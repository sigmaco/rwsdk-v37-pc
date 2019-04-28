
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
 * clmpmorf.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Handles morph based animations.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpmorph.h"

#include "skeleton.h"
#include "menu.h"

#include "clmpview.h"
#include "clmppick.h"
#include "clmpmorf.h"

static RwInt32 NumMorphTargets = 0;
static RwBool MorphMenuActive = FALSE;

RwBool MorphOn = FALSE;
RwBool ClumpHasMorphAnimation = FALSE;

RwReal MorphsPerSecond = 1.0f;



/*
 *****************************************************************************
 */
static RpAtomic *
AtomicSetupInterpolator(RpAtomic * atomic, 
                        void * data __RWUNUSED__)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        NumMorphTargets = RpGeometryGetNumMorphTargets(geometry);

        if( NumMorphTargets > 1 )
        {
            RwInt32 i;

            /*
             * Create interpolators...
             */
            RpMorphGeometryCreateInterpolators(geometry, NumMorphTargets);

            /*
             * Link all morph targets...
             */
            for( i=0; i < (NumMorphTargets - 1); i++ )
            {
                RpMorphGeometrySetInterpolator(geometry, i, i, i+1, 1.0f);
            }
            /*
             * Link final morph target to fisrt creating continuos loop...
             */
            RpMorphGeometrySetInterpolator(geometry, i, i, 0, 1.0f);            

            RpMorphAtomicSetCurrentInterpolator(atomic, 0);

            ClumpHasMorphAnimation = TRUE;
        }
    }
    
    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicResetInterpolator(RpAtomic * atomic, 
                        void * data __RWUNUSED__)
{
    RpMorphAtomicSetCurrentInterpolator(atomic, 0);

    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicIncrementMorphTarget(RpAtomic * atomic, 
                           void * data __RWUNUSED__)
{
    RwInt32 newIpIndex;
    RwInt32 curIpIndex = RpMorphAtomicGetCurrentInterpolator(atomic);
    
    newIpIndex = (curIpIndex + 1) % NumMorphTargets;

    RpMorphAtomicSetCurrentInterpolator(atomic, newIpIndex);

    return atomic;
}


/*
 *****************************************************************************
 */
static RwBool
NextMorphTargetCB(RwBool justCheck)
{
    if( justCheck )
    {
        if( !ClumpLoaded || !ClumpHasMorphAnimation )
        {
            return FALSE;
        }
        return TRUE;
    }

    /*
     * Skip to the NEXT morph target...
     */
    if( AtomicSelected )
    {
        AtomicIncrementMorphTarget(AtomicSelected, NULL);

        AtomicGetBoundingBox(AtomicSelected, &CurrentAtomicBBox);
    }
    else
    {
        RpClumpForAllAtomics(Clump, AtomicIncrementMorphTarget, NULL);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
ResetMorphingCB(RwBool justCheck)
{
    if( justCheck )
    {
        if( !ClumpLoaded || !ClumpHasMorphAnimation )
        {
            return FALSE;
        }
        return TRUE;
    }

    /*
     * Skip to the FIRST morph target...
     */
    if( AtomicSelected )
    {
        AtomicResetInterpolator(AtomicSelected, NULL);

        AtomicGetBoundingBox(AtomicSelected, &CurrentAtomicBBox);
    }
    else
    {
        RpClumpForAllAtomics(Clump, AtomicResetInterpolator, NULL);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
MorphOnCB(RwBool justCheck)
{
    if( justCheck )
    {
        if( !ClumpLoaded || !ClumpHasMorphAnimation )
        {
            MorphOn = FALSE;
            return FALSE;
        }
        return TRUE;
    }
    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
MorphsPerSecondCB(RwBool justCheck)
{
    if( justCheck )
    {
        if( !MorphOnCB(justCheck) )
        {
            return FALSE;
        }
        return TRUE;
    }

    MorphOn = TRUE;

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
MorphMenuSetup(void)
{
    static RwChar MorphsPerSecondLabel[] = RWSTRING("Morph Anim Speed");
    static RwChar MorphOnLabel[] = RWSTRING("Morph Animation_A");
    static RwChar NextMorphTargetLabel[] = RWSTRING("Next Morph Tgt_K");
    static RwChar ResetMorphingLabel[] = RWSTRING("First Morph Tgt_Z");

    if( !MorphMenuActive )
    {
        MenuAddEntryBool(MorphOnLabel, &MorphOn, MorphOnCB);
        MenuAddEntryReal(MorphsPerSecondLabel, 
            &MorphsPerSecond, MorphsPerSecondCB, 1.0f, 50.0f, 1.0f);
        MenuAddEntryTrigger(ResetMorphingLabel, ResetMorphingCB);
        MenuAddEntryTrigger(NextMorphTargetLabel, NextMorphTargetCB);

        MorphMenuActive = TRUE;
    }

    return;
}


/*
 *****************************************************************************
 */
static void
MorphMenuDestroy(void)
{
    MenuRemoveEntry(&MorphOn);
    MenuRemoveEntry(&MorphsPerSecond);
    MenuRemoveEntry((void *)ResetMorphingCB);
    MenuRemoveEntry((void *)NextMorphTargetCB);

    MorphMenuActive = FALSE;

    return;
}


/*
 *****************************************************************************
 */
RwBool 
MorphClumpInitialize(RpClump * clump)
{
    /*
     * If any atomics have more than one morph target, initialise the
     * interpolators so we can do morph target animation...
     */
    ClumpHasMorphAnimation = FALSE;
   
    RpClumpForAllAtomics(clump, AtomicSetupInterpolator, NULL);

    if( ClumpHasMorphAnimation )
    {
        MorphOn = TRUE;
        MorphMenuSetup();
    }
    else
    {
        MorphOn = FALSE;
        MorphMenuDestroy();
    }

    MorphsPerSecond = 1.0f;

    return FALSE;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicIncrementInterpolator(RpAtomic * atomic, void *data)
{
    RwReal step = *(RwReal *)data;

    if( step > 0.0f )
    {
        RpMorphAtomicAddTime(atomic, step);
    }   

    return atomic;
}


/*
 *****************************************************************************
 */
void
MorphClumpUpdate( RwReal delta )
{
    if( MorphOn )
    {
        RwReal step;

        /*
         * Animated morphing controlled by a timer...
         */
        step = delta * MorphsPerSecond;

        if( AtomicSelected )
        {
            AtomicIncrementInterpolator(AtomicSelected, &step);
        }
        else
        {
            RpClumpForAllAtomics(Clump, AtomicIncrementInterpolator, &step);
        }
    }

    return;
}

/*
 *****************************************************************************
 */
