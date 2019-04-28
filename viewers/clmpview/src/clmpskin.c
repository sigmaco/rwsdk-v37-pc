
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
 * clmpskin.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Handling skin based animations
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpskin.h"

#include "skeleton.h"
#include "menu.h"

#include "clmppick.h"
#include "clmpview.h"
#include "clmpskin.h"

static RwBool SkinMenuActive = FALSE;
static RwReal SkinPerSecond = 1.0f;
 
RwBool SkinOn = FALSE;
RwBool ClumpHasSkinAnimation = FALSE;

RwInt32 AtomicTotalSkinBones = 0;
RwInt32 AtomicTotalAtomicTotalKeyFrame = 0;

/*
 *****************************************************************************
 */
static RwBool
SkinOnCB(RwBool justCheck)
{
    if( justCheck )
    {    
        if( !ClumpLoaded || !ClumpHasSkinAnimation )
        {
            SkinOn = FALSE;
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
SkinPerSecondCB(RwBool justCheck)
{
    if( justCheck )
    {
        if( !SkinOnCB(justCheck) )
        {
            return FALSE;
        }
        return TRUE;
    }

    SkinOn = TRUE;

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
SkinMenuSetup(void)
{
    static RwChar SkinPerSecondLabel[] = RWSTRING("Skin Anim Speed");
    static RwChar SkinOnLabel[] = RWSTRING("Skin Animation_A");

    if( !SkinMenuActive )
    {
        MenuAddEntryBool(SkinOnLabel, &SkinOn, SkinOnCB);
        MenuAddEntryReal(SkinPerSecondLabel,
                         &SkinPerSecond,
                         SkinPerSecondCB, -10.0f, 10.0f, 0.1f);
        SkinMenuActive = TRUE;
    }
    
    return;
}


/*
 *****************************************************************************
 */
static void
SkinMenuDestroy(void)
{
    MenuRemoveEntry(&SkinOn);
    MenuRemoveEntry(&SkinPerSecond);

    SkinMenuActive = FALSE;
    
    return;
}

/*
 *****************************************************************************
 */
RwBool 
SkinLoadSKA(RpClump * clump __RWUNUSED__, 
            RwChar * skaPath __RWUNUSED__)
{
    return FALSE;
}


/*
 *****************************************************************************
 */
RwBool 
SkinClumpInitialize(RpClump * clump, RwChar * clumpFileName)
{
    RwChar *skaFileName = (char *)NULL;
    RwBool result = FALSE;
    
    ClumpHasSkinAnimation = FALSE;

    /*
     * Try to load <clumpName>.ska file...
     */
    skaFileName = (RwChar *) 
        RwMalloc(sizeof(RwChar) * 
                 (rwstrlen(clumpFileName) + 1), rwID_NAOBJECT);

    if (skaFileName == (char *)NULL)
    {
        ClumpHasSkinAnimation = FALSE;
        SkinMenuDestroy();
        RwFree (skaFileName);
        return FALSE;
    }

    rwstrcpy(skaFileName, clumpFileName);
    skaFileName[rwstrlen(skaFileName) - 3] = 0;
    rwstrcat(skaFileName, RWSTRING("ska"));

    /*
     * Check if there is any attached RpSkin data ...
     */
    result = SkinLoadSKA(clump, skaFileName);

    if( result )
    {
        SkinMenuSetup();
    }
    else
    {
        SkinMenuDestroy();
    }

    RwFree(skaFileName);

    return result;
}


/*
 *****************************************************************************
 */
static RpAtomic *
UpdateSkinAnimation (RpAtomic * atomic, 
                     void * pData __RWUNUSED__)
{

    return atomic;
}


/*
 *****************************************************************************
 */
void
SkinClumpUpdate (RwReal delta)
{
    if (SkinOn)
    {
        RwReal inc;

        /*
         * Animated skin controlled by a timer...
         */
        inc = delta * SkinPerSecond;

        if (AtomicSelected)
        {
            UpdateSkinAnimation(AtomicSelected, &inc);
        }
        else
        {
            RpClumpForAllAtomics(Clump, UpdateSkinAnimation, &inc);
        }
    }

    return;
}


/*
 *****************************************************************************
 */
void
SkinDestroy(void)
{
    return;
}


/*
 *****************************************************************************
 */
