
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
 * Copyright (c) 2000 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * morph.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: Example to demonstrate the morph plugin.
 *
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpmorph.h"

#include "skeleton.h"
#include "menu.h"

#include "morph.h"

RwInt32 NumInterpolators;

RwReal MorphSpeed = 1.0f;
RwBool MorphOn = TRUE;


/*
 *****************************************************************************
 */
static RwReal
InterpolatorCallback(RpAtomic *atomic, RwReal position)
{
    /*
     * NOTE: The behavior of the default callback is simply to execute
     * the interpolator following this one (or the first if this is the
     * last). For instructional purposes we explicitly reproduce this 
     * behavior in a custom callback...
     */

    RwInt32 curIpIndex, newIpIndex;
    RwReal newPosition, scale;

    scale = RpInterpolatorGetScale(RpAtomicGetInterpolator(atomic));
    newPosition = position - scale;

    curIpIndex = RpMorphAtomicGetCurrentInterpolator(atomic);

    newIpIndex = (curIpIndex + 1) % NumInterpolators;

    RpMorphAtomicSetCurrentInterpolator(atomic, newIpIndex);

    return newPosition;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicSetupInterpolators(RpAtomic *atomic, 
                         void *data __RWUNUSED__)
{
    RpGeometry *geometry;
    
    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RwInt32 i, interpolatorNum, numMorphTargets;
        
        numMorphTargets = RpGeometryGetNumMorphTargets(geometry);

        NumInterpolators = (numMorphTargets-1) * 2;
        
        /*
         * Create interpolators...
         */
        RpMorphGeometryCreateInterpolators(geometry, NumInterpolators);
        
        /*
         * Initialize interpolators with different morph scales:
         * Morph target 0 is the base;
         * [1 -> numMorphTargets] are the targets;
         * After every target we return to base.
         */
        interpolatorNum = 0;
        for(i=1; i<numMorphTargets; i++)
        {
            RpMorphGeometrySetInterpolator(geometry, interpolatorNum++, 
                0, i, i/2.0f);

            RpMorphGeometrySetInterpolator(geometry, interpolatorNum++, 
                i, 0, (i + 1)/2.0f);
        }

        /*
         * Begin with the first interpolator...
         */
        RpMorphAtomicSetCurrentInterpolator(atomic, 0);
        
        /*
         * Set our own interpolator callback...
         */
        RpMorphGeometrySetCallBack(geometry, InterpolatorCallback);
    }

    return NULL;
}


/*
 *****************************************************************************
 */
void 
ClumpSetupInterpolators(RpClump *clump)
{
    RpClumpForAllAtomics(clump, AtomicSetupInterpolators, NULL);

    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicMorphUpdate(RpAtomic *atomic, void *data)
{
    RwReal step = *(RwReal*)data;

    if( step > 0.0f )
    {
        RpMorphAtomicAddTime(atomic, step);
    }

    return NULL;
}


/*
 *****************************************************************************
 */
void 
ClumpAdvanceMorph(RpClump *clump, RwReal delta)
{
    delta *= MorphSpeed;

    RpClumpForAllAtomics(clump, AtomicMorphUpdate, &delta);

    return;
}

/*
 *****************************************************************************
 */
