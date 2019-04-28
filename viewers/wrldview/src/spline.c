
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
 * spline.c
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

#include "skeleton.h"
#include "rtfsyst.h"

#include "main.h"
#include "spline.h"

#define NUMWAYPOINTS (1000)

static RwIm3DVertex WayPoints[NUMWAYPOINTS];

RpSpline *WayPointSpline = (RpSpline *)NULL;

RwReal SplinePos = 0.0f;

RwBool SplineOn = TRUE;


/*
 *****************************************************************************
 */
static void
InitializeImmPath(RpSpline *spline)
{
    RwInt32 i;

    for(i=0; i<NUMWAYPOINTS; i++)
    {
        RwV3d pos;
        RwReal value;

        value = (RwReal)i / (NUMWAYPOINTS-1);

        RpSplineFindPosition(spline, 
            rpSPLINEPATHNICEENDS, value, &pos, (RwV3d *)NULL);

        RwIm3DVertexSetPos(&WayPoints[i], pos.x, pos.y, pos.z);

        if( i%2 )
        {
            RwIm3DVertexSetRGBA(&WayPoints[i], 255, 255, 255, 255);
        }
        else
        {
            RwIm3DVertexSetRGBA(&WayPoints[i], 255, 0, 0, 255);
        }
    }

    return;
}


/*
 *****************************************************************************
 */
void
DestroySpline(void)
{
    if( WayPointSpline )
    {
        RpSplineDestroy(WayPointSpline);
    }

    return;
}


/*
 *****************************************************************************
 */
static void
GetSplineName(RwChar *dst, const RwChar *src)
{
    /*
     * Returns in "dst" the name of the spline for the world, assuming only
     * the BSP file and SPL file have the same name and are located in the 
     * same folder...
     */
    RwInt32 i, len;

    len = rwstrlen(src);

    for(i=0; i<len - 4; i++)
    {
        /*
         * Copy across the file name until we reach the file extension...
         */
        *dst++ = *src++;
    }
    
    *dst = '\0';

    /*
     * ...then give it the spline file extension "SPL"...
     */
    rwstrcat(dst, RWSTRING(".spl"));

    return;
}


/*
 *****************************************************************************
 */
RwBool 
LoadSpline(RwChar *file, RpWorld *world __RWUNUSED__)
{
    RpSpline *spline = (RpSpline *)NULL;
    RwChar *path = (RwChar *)NULL;
    RwChar splineName[256];

    GetSplineName(splineName, file);

    path = RsPathnameCreate(splineName);

    if( RwFexist(path) )
    {
        spline = RpSplineRead(path);
    }

    RsPathnameDestroy(path);

    if( spline )
    {
        InitializeImmPath(spline);

        if( WayPointSpline )
        {
            RpSplineDestroy(WayPointSpline);
        }

        WayPointSpline = spline;

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
void
RenderSpline(void)
{
    /*
     * Draw the waypoint path as a 3D immediate mode polyline...
     */
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

    if( RwIm3DTransform(WayPoints, 
            NUMWAYPOINTS, (RwMatrix *)NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPEPOLYLINE);

        RwIm3DEnd();
    }

    return;
}

/*
 *****************************************************************************
 */
