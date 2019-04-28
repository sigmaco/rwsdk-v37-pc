
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
 * clmpdmrf.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Matt Thorman
 * Reviewed by:
 *
 * Purpose: Handles delta morph based animations.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "rpdmorph.h"

#include "skeleton.h"
#include "menu.h"

#include "clmpview.h"
#include "clmpdmrf.h"

RwBool ClumpHasDMorphData = FALSE;

static RpDMorphAnimation   *DMorphAnimation = NULL;
static RwBool               DMorphMenuActive = FALSE;
static RwReal               DMorphPerSecond = 1.0f;

/*
 *****************************************************************************
 */
static RpAtomic *
DMorphAtomicQueryData(RpAtomic *atomic, void *data)
{
    RwBool      *hasDMorphData = (RwBool *) data;
    RpGeometry	*geom = RpAtomicGetGeometry(atomic);

    if (geom && (RpDMorphGeometryGetNumDMorphTargets(geom) > 0) )
    {
        /* Found DMorph data - terminate iteration */
        *hasDMorphData = TRUE;
        return NULL;
    }
    
    return atomic;
}

/*
 *****************************************************************************
 */
static RpAtomic *
DMorphAtomicSetAnimation(RpAtomic *atomic, 
                         void * data __RWUNUSED__)
{
    RpGeometry	*geom = RpAtomicGetGeometry(atomic);

    if( geom && (RpDMorphGeometryGetNumDMorphTargets(geom) > 0) )
    {
        RpDMorphAtomicSetAnimation(atomic, DMorphAnimation);
    }
    
    return atomic;
}

/*
 *****************************************************************************
 */
static RpAtomic *
DMorphAtomicAddTime(RpAtomic *atomic, void *data)
{
    RwReal deltaTime = *(RwReal *)data;
    RpGeometry *geometry = RpAtomicGetGeometry(atomic);

    if( geometry && (RpDMorphGeometryGetNumDMorphTargets(geometry) > 0) )
    {
        RpDMorphAtomicAddTime(atomic, deltaTime);
    }

    return atomic;
}


/*
 *****************************************************************************
 */
static RwBool
DMorphPerSecondCB(RwBool justCheck __RWUNUSED__)
{
    return (DMorphAnimation != NULL);
}
 
/*
 *****************************************************************************
 */
static void
DMorphMenuSetup(void)
{
    static RwChar DMorphPerSecondLabel[] = RWSTRING("DMorph Anim Speed");

    if( !DMorphMenuActive )
    {
        MenuAddEntryReal(DMorphPerSecondLabel,
                         &DMorphPerSecond,
                         DMorphPerSecondCB, 0.0f, 10.0f, 0.1f);

        DMorphMenuActive = TRUE;
    }
    
    return;
}


/*
 *****************************************************************************
 */
static void
DMorphMenuDestroy(void)
{
    MenuRemoveEntry(&DMorphPerSecond);

    DMorphMenuActive = FALSE;
    
    return;
}

/*
 *****************************************************************************
 */
RwBool 
DMorphLoadDMA(RpClump *clump, RwChar *dmaPath)
{
    if (ClumpHasDMorphData)
    {
        RwChar              *pathName;

        /*
         * Destroy any existing animation 
         */
        if (DMorphAnimation)
        {
            RpDMorphAnimationDestroy(DMorphAnimation);
            DMorphAnimation = (RpDMorphAnimation *)NULL;
        }

        /*
         * Read the delta morph animation file... 
         */
        pathName = RsPathnameCreate(dmaPath);
        DMorphAnimation = RpDMorphAnimationRead(pathName);
        RsPathnameDestroy(pathName);

        if (DMorphAnimation)
        {
            RpClumpForAllAtomics(clump, DMorphAtomicSetAnimation, NULL);
        }
    }

    return (DMorphAnimation != NULL);
}


/*
 *****************************************************************************
 */
RwBool 
DMorphClumpInitialize(RpClump * clump, RwChar * clumpFileName)
{
    RpClumpForAllAtomics(clump, DMorphAtomicQueryData, &ClumpHasDMorphData);

    if (ClumpHasDMorphData)
    {
        RwChar *dmaFileName = (RwChar *) NULL;

        /*
         * Try to load <clumpName>.dma delta morph animation file...
         */
        dmaFileName = (RwChar *) RwMalloc(sizeof(RwChar) * 
                     (rwstrlen(clumpFileName) + 1), rwID_NAOBJECT);

        rwstrcpy(dmaFileName, clumpFileName);
        dmaFileName[rwstrlen(dmaFileName) - 3] = 0;
        rwstrcat(dmaFileName, RWSTRING("dma"));

        DMorphLoadDMA(clump, dmaFileName);

        RwFree(dmaFileName);

        DMorphMenuSetup();
    }
    else
    {
        DMorphMenuDestroy();
    }

    return ClumpHasDMorphData;
}

/*
 *****************************************************************************
 */
void
DMorphClumpUpdate( RwReal delta )
{
    if( ClumpHasDMorphData && DMorphAnimation && (delta > 0.0f))
    {
        delta *= DMorphPerSecond;

		RpClumpForAllAtomics(Clump, DMorphAtomicAddTime, &delta);
    }

    return;
}

/*
 *****************************************************************************
 */
void 
DMorphDestroy(void)
{
    if (DMorphAnimation)
    {
       RpDMorphAnimationDestroy(DMorphAnimation);

       DMorphAnimation = NULL;
    }

    ClumpHasDMorphData = FALSE;

    return;
}
