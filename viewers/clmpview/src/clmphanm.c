
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
 * clmphanm.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Handling HAnim based animations
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpskin.h"
#include "rphanim.h"

#include "skeleton.h"
#include "menu.h"

#include "clmppick.h"
#include "clmpview.h"
#include "clmphanm.h"

#define NUMHANIMSEQUENCESMAX (100)

static RtAnimAnimation *HAnimSequences[NUMHANIMSEQUENCESMAX];
static RwInt32 HAnimNumSequences = 0;
static RwInt32 HAnimCurrentSequences = -1;
static RwReal HAnimPerSecond = 1.0f;

static RwBool HAnimMenuActive = FALSE;

RwBool HAnimOn = FALSE;
RwBool ClumpHasHAnimAnimation = FALSE;



/*
 *****************************************************************************
 */
static RwFrame*
GetChildFrameHierarchy(RwFrame *frame, void *data)
{    
    RpHAnimHierarchy **pHierarchy = (RpHAnimHierarchy **)data;

    *pHierarchy = RpHAnimGetHierarchy(frame);

    if (*pHierarchy == NULL)
    {
        RwFrameForAllChildren(frame, GetChildFrameHierarchy, data);
        return frame;
    }

    return (RwFrame *)NULL;
}


/*
 *****************************************************************************
 */
static RpHAnimHierarchy*
GetHierarchy(void)
{    
    RpHAnimHierarchy *hierarchy = (RpHAnimHierarchy *)NULL;

    hierarchy = RpHAnimGetHierarchy(RpClumpGetFrame(Clump));
    
    if (hierarchy == NULL)
    {
        RwFrameForAllChildren(RpClumpGetFrame(Clump), GetChildFrameHierarchy, &hierarchy );
    }

    return hierarchy;
}


/*
 *****************************************************************************
 */
static RwBool
CurrentHAnimCB(RwBool justCheck)
{
    RpHAnimHierarchy *hierarchy = (RpHAnimHierarchy *)NULL;

    if( !ClumpLoaded || !ClumpHasHAnimAnimation )
    {
        HAnimCurrentSequences = -1;
        return FALSE;
    }

    if( justCheck )
    {
        return TRUE;
    }

    hierarchy = GetHierarchy();

    if( hierarchy )
    {
        /*
         * Setup the current animation ...
         */
        RpHAnimHierarchySetCurrentAnim(hierarchy,
            HAnimSequences[HAnimCurrentSequences]);

        /*
         * Attach the hierarchy to the RwFrame hierarchy 
         */
        RpHAnimHierarchyAttach(hierarchy);        

        RpHAnimUpdateHierarchyMatrices(hierarchy);   

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*
 *****************************************************************************
 */
static RwBool
HAnimOnCB(RwBool justCheck)
{
    if( justCheck )
    {    
        if( !ClumpLoaded || !ClumpHasHAnimAnimation )
        {
            HAnimOn = FALSE;
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
HAnimPerSecondCB(RwBool justCheck)
{
    if( justCheck )
    {
        if( !HAnimOnCB(justCheck) )
        {
            return FALSE;
        }
        return TRUE;
    }

    HAnimOn = TRUE;

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
HAnimMenuSetup(void)
{
    static RwChar hAnimPerSecondLabel[] = RWSTRING("HAnim Anim Speed");
    static RwChar hAnimOnLabel[] = RWSTRING("HAnim Animation_A");
    static RwChar hAnimCurrentLabel[] = RWSTRING("Current HAnim");

    if( !HAnimMenuActive )
    {
        MenuAddEntryBool(hAnimOnLabel,
                         &HAnimOn,
                         HAnimOnCB);
        MenuAddEntryReal(hAnimPerSecondLabel,
                         &HAnimPerSecond,
                         HAnimPerSecondCB,
                         -10.0f, 10.0f, 0.1f);
        MenuAddEntryInt (hAnimCurrentLabel,
                         &HAnimCurrentSequences,
                         (MenuTriggerCallBack) CurrentHAnimCB,
                         0, 0, 1,
                         (const RwChar **)NULL);
        HAnimMenuActive = TRUE;
    }
    
    return;
}


/*
 *****************************************************************************
 */
static void
HAnimMenuDestroy(void)
{
    MenuRemoveEntry(&HAnimOn);
    MenuRemoveEntry(&HAnimPerSecond);
    MenuRemoveEntry(&HAnimCurrentSequences);
    
    HAnimMenuActive = FALSE;
    
    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
SetupHAnimCB(RpAtomic * atomic, void *data)
{
    RpHAnimHierarchy *hierarchy = (RpHAnimHierarchy *)data;

    if( RpSkinAtomicGetSkin(atomic) )
    {     
        RpSkinAtomicSetHAnimHierarchy(atomic,
                                      hierarchy);
    }
       
    return atomic;
}


/*
 *****************************************************************************
 */
RwBool 
HAnimLoadANM(RpClump *clump, RwChar *skaPath)
{
    RpHAnimHierarchy *hierarchy = (RpHAnimHierarchy *)NULL;
    RwChar *pathName;

    if( clump == NULL )
    {
        clump = Clump;
    }

    if( HAnimNumSequences < NUMHANIMSEQUENCESMAX )
    {
        pathName = RsPathnameCreate(skaPath);

        HAnimSequences[HAnimNumSequences] = RtAnimAnimationRead(pathName);        

        RsPathnameDestroy (pathName);

        HAnimOn = FALSE;
        HAnimPerSecond = 1.0f;

        if( HAnimSequences[HAnimNumSequences] != NULL )
        {
            hierarchy = GetHierarchy();

            if( !hierarchy )
            {
                return FALSE;
            }

            /*
             * Make any skinned objects point at the hierarchy 
             */        
            RpClumpForAllAtomics(clump, SetupHAnimCB, hierarchy);           

            /*
             * Set flags to update all matrices 
             */
            RpHAnimHierarchySetFlags(hierarchy,
                                     (RpHAnimHierarchyFlag)
                                     ( RpHAnimHierarchyGetFlags(hierarchy) | 
                                       rpHANIMHIERARCHYUPDATELTMS |
                                       rpHANIMHIERARCHYUPDATEMODELLINGMATRICES) );

            /*
             * Setup the current animation 
             */
            RpHAnimHierarchySetCurrentAnim(hierarchy, 
                HAnimSequences[HAnimNumSequences]);

            /* 
             * Attach the hierarchy to the RwFrame hierarchy 
             */
            RpHAnimHierarchyAttach(hierarchy);        
    
            RpHAnimUpdateHierarchyMatrices(hierarchy);   

            HAnimOn = TRUE;
            HAnimCurrentSequences = HAnimNumSequences;

            ClumpHasHAnimAnimation = TRUE;
            HAnimMenuSetup();

            HAnimNumSequences++;
            MenuSetRangeInt(&HAnimCurrentSequences,
                            0, HAnimNumSequences-1, 1,
                            (const RwChar **) NULL);
        
        }
    }

    return HAnimOn;
}


/*
 *****************************************************************************
 */
RwBool 
HAnimClumpInitialize (RpClump *clump, RwChar *fileName)
{
    RwChar *anmFileName = (char *)NULL;
    RwBool result = FALSE;

    ClumpHasHAnimAnimation = FALSE;
    HAnimNumSequences = 0;

    /*
     * Create the .anm file name 
     */
    anmFileName = (RwChar *)
        RwMalloc(sizeof(RwChar) * 
                 (rwstrlen(fileName) + 1), rwID_NAOBJECT);

    if( anmFileName == (char *)NULL )
    {
        ClumpHasHAnimAnimation = FALSE;
        HAnimMenuDestroy();
        RwFree(anmFileName);
        return FALSE;
    }

    rwstrcpy(anmFileName, fileName);
    anmFileName[rwstrlen(anmFileName) - 3] = 0;
    rwstrcat(anmFileName, RWSTRING("anm"));

    result = HAnimLoadANM(clump, anmFileName);
    
    RwFree(anmFileName);

    if( result == FALSE)
    {
        HAnimMenuDestroy();
    }

    return result;
}


/*
 *****************************************************************************
 */
void
HAnimClumpUpdate(RwReal delta)
{
    if( ClumpHasHAnimAnimation )
    {
        RpHAnimHierarchy *hierarchy = (RpHAnimHierarchy *)NULL;
        hierarchy = GetHierarchy();

        if( !hierarchy )
        {
            return;
        }

        if( HAnimOn )
        {
            RwReal inc;

            /*
             * Animated HAnim controlled by a timer...
             */
            inc = delta * HAnimPerSecond;

            if (inc < 0.0f)
            {
                RpHAnimHierarchySubAnimTime(hierarchy, -inc);
            }
            else
            {
                /* check if we've been playing backwards */
                if (!hierarchy->currentAnim->pNextFrame)
                {
                    /* we have so reset the animation for forward playback */
                    RwReal targetTime;
                    RtAnimAnimation *currentAnim;

                    targetTime = hierarchy->currentAnim->currentTime + inc;
                    currentAnim = RpHAnimHierarchyGetCurrentAnim(hierarchy);
                    RpHAnimHierarchySetCurrentAnim(hierarchy, currentAnim);
                    RpHAnimHierarchySetCurrentAnimTime(hierarchy, targetTime);
                }
                else
                {
                    RpHAnimHierarchyAddAnimTime(hierarchy, inc);
                }
            }          
        }

        RpHAnimUpdateHierarchyMatrices(hierarchy);   
    }

    return;
}


/*
 *****************************************************************************
 */
void
HAnimDestroy(void)
{
    RwInt32 i;

    for( i=0;i<NUMHANIMSEQUENCESMAX;i++ )
    {
        if( HAnimSequences[i] )
        {
            RtAnimAnimationDestroy(HAnimSequences[i]);
            HAnimSequences[i] = (RtAnimAnimation *)NULL;
        }
    }
    return;
}

/*
 *****************************************************************************
 */
