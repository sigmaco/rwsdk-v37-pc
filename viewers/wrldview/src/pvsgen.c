
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
 * pvsgen.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 * Reviewed by: John Irwin (with substantial edits).
 *
 * Purpose: RenderWare3 BSP viewer.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpspline.h"
#include "rppvs.h"
#include "rtsplpvs.h"
#include "rtworld.h"
#include "rpcollis.h"

#include "skeleton.h"
#include "menu.h"

#include "main.h"
#include "world.h"
#include "pvsgen.h"

RwReal PVSProgressDone = 0.0f;
RwBool PVSGenerating = FALSE;
RwBool PVSOn = FALSE;

extern RpSpline *WayPointSpline;

/*
 *****************************************************************************
 */
RwBool
PVSProgressCallback(RwInt32 __RWUNUSED__ message, 
                    RwReal value)
{
    PVSProgressDone = value;

    /*
     * Update the PVSCreate info screen...
     */
    Render();

    /*
     * Code to terminate PVS generation can be placed here setting
     * PVSGenerating = FALSE...
     */

    return PVSGenerating;
}


/*
 *****************************************************************************
 */
static void
GeneratePVSFromGeneric(RpWorld *world, 
                       RwReal  __RWUNUSED__ maxDist)
{
    /*
     * The percentage along the diagonal of the world sector bounding box to
     * take sample points...
     */
    RwReal t = 1.0f;
    /*
     * Generate PVS data using the generic (free-form) method...
     */

    /* Build collision data for PVS generation */
    if (!RpCollisionWorldQueryData(world))
    {
        RpCollisionWorldBuildData(world,
                              (RpCollisionBuildParam *) NULL);
    }

    RpPVSConstruct(world, 
        RpPVSGeneric, &t);

    /* don't destroy coll data yet */
    return;
}

#if (defined(RtSplinePVSCreate))
/* RtSplinePVSCreate is now macro in which maxDist is not used */
#define __RWUNUSEDMAXDIST__ __RWUNUSED__
#endif /* (defined(RtSplinePVSCreate)) */

#if (!defined(__RWUNUSEDMAXDIST__))
#define __RWUNUSEDMAXDIST__ /* No op */
#endif /* (!defined(__RWUNUSEDMAXDIST__)) */


static void
GeneratePVSFromWayPoints(RpWorld *world, 
                         RwReal __RWUNUSEDMAXDIST__ maxDist)
{
    RtSplinePVSCreate(world,
                      RwCameraGetRaster(Camera),
                      RwCameraGetZRaster(Camera),
                      RwCameraGetNearClipPlane(Camera),
                      maxDist, WayPointSpline, 1000);
}

/*
 *****************************************************************************
 */
RwBool
GeneratePVSCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldLoaded && !PVSGenerating;
    }

    MenuSetStatus(MENUOFF);

    PVSProgressDone = 0.0f;

    /*
     * Remove any existing PVS data, if necessary...
     */
    if( RpPVSQuery(World) )
    {
        RpPVSDestroy(World);
    }

    PVSGenerating = TRUE;

    if( WayPointSpline != NULL )
    {
        GeneratePVSFromWayPoints(World, RwCameraGetFarClipPlane(Camera));
    }
    else {
        GeneratePVSFromGeneric(World, RwCameraGetFarClipPlane(Camera));
    }

    /*
     * If we completed PVS data creation, use it!...
     */
    PVSOn = RpPVSQuery(World);

    PVSGenerating = FALSE;

    return TRUE;
}


/*
 *****************************************************************************
 */
RwBool
RepairPVSCallback(RwBool testEnable)
{
    RwMatrix *matrix = (RwMatrix *)NULL;
    RwFrame *frame = (RwFrame *)NULL;

    if( testEnable )
    {
        return WorldLoaded && !PVSGenerating && PVSOn;
    }

    frame = RwCameraGetFrame(Camera);
    matrix = RwFrameGetLTM(frame);

    if( RpPVSQuery(World) )
    {
        /* Build collision data for PVS generation */
        if (!RpCollisionWorldQueryData(World))
        {
            RpCollisionWorldBuildData(World,
                                  (RpCollisionBuildParam *) NULL);
        }
        RpPVSSamplePOV(&matrix->pos, FALSE); 
        /* Don't destroy coll data yet */
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
RwBool
PVSOnCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return WorldLoaded && RpPVSQuery(World);
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
