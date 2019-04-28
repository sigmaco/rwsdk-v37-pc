
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
 * clmpatch.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Handling patch based geometry
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rppatch.h"

#include "skeleton.h"
#include "menu.h"

#include "clmpview.h"
#include "clmpatch.h"

static RwBool PatchMenuActive = FALSE;

static RpPatchMesh *PatchMesh = (RpPatchMesh *)NULL;

RwBool ClumpHasPatch = FALSE;

RwInt32 AtomicPatchLOD = rpPATCHLODMAXVALUE / 4;

/*
 *****************************************************************************
 */
static RwUInt32
PatchLODCallBack(RpAtomic *atomic __RWUNUSED__, RpPatchLODUserData userData __RWUNUSED__)
{
    return (RwUInt32)AtomicPatchLOD;
}

/*
 *****************************************************************************
 */
static RwBool
PatchLODCB(RwBool justCheck)
{
    if( !ClumpLoaded || !ClumpHasPatch )
    {
        return FALSE;
    }

    if( justCheck )
    {
        return TRUE;
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static void
PatchMenuSetup(void)
{
    static RwChar PathLODLabel[] = RWSTRING("Patch LOD");

    if( !PatchMenuActive )
    {
        MenuAddSeparator();

        MenuAddEntryInt(PathLODLabel,
                        &AtomicPatchLOD,
                        (MenuTriggerCallBack) PatchLODCB,
                        1, rpPATCHLODMAXVALUE, 1,
                        (const RwChar **)NULL);

        PatchMenuActive = TRUE;
    }
    
    return;
}


/*
 *****************************************************************************
 */
static void
PatchMenuDestroy(void)
{
    MenuRemoveEntry(&AtomicPatchLOD);

    PatchMenuActive = FALSE;
    
    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
SetupPatchCB(RpAtomic *atomic, void *data)
{
    /*
     * Check if there is any attached RpPatchMesh data 
     */
    PatchMesh = RpPatchAtomicGetPatchMesh(atomic);
    
    if (PatchMesh)
    {
        RpPatchAtomicSetPatchLODCallBack(atomic, PatchLODCallBack, NULL);

        *((RwBool *)data) = TRUE;
    }

    return atomic;
}

/*
 *****************************************************************************
 */
RwBool 
PatchClumpInitialize(RpClump * clump)
{
    RwBool result = FALSE;
    
    ClumpHasPatch = FALSE;

    RpClumpForAllAtomics(clump, SetupPatchCB, &ClumpHasPatch);

    if( ClumpHasPatch )
    {
        PatchMenuSetup();
    }
    else
    {
        PatchMenuDestroy();
    }

    return result;
}

/*
 *****************************************************************************
 */
static RpAtomic *
RemovePatchMesh(RpAtomic *atomic, void *data __RWUNUSED__)
{
    RpPatchMesh *patchMesh = RpPatchAtomicGetPatchMesh(atomic);

    if (patchMesh != NULL)
    {
        RpPatchMeshDestroy(patchMesh);
    }

    return atomic;
}

/*
 *****************************************************************************
 */
void
PatchDestroy(void)
{
    if (ClumpHasPatch)
    {
        RpClumpForAllAtomics(Clump, RemovePatchMesh, NULL);
    }

    return;
}


/*
 *****************************************************************************
 */
