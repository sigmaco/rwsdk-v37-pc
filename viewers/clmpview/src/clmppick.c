
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
 * clmppick.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by: 
 *                                                                         
 * Purpose: Atomic picking tool box.
 *                         
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpcollis.h"
#include "rtpick.h"

#include "skeleton.h"

#include "clmpskin.h"
#include "clmpview.h"
#include "clmppick.h"

#include "scene.h"

typedef struct
{
    RpIntersection intersection;
    RpAtomic *pickedAtomic;
    RwReal minDistance;
    RwInt32 atomicNumber;
}
ClumpIntersectParams;

RpAtomic *AtomicSelected = (RpAtomic *)NULL;

RwInt32 currentAtomicNumber = 0;
RwBBox CurrentAtomicBBox;

RwInt32 AtomicTotalTriangles;
RwInt32 AtomicTotalVertices;
RwInt32 AtomicTotalMorphTargets;

static RwBool passedCurrentAtomic;

/*
 *****************************************************************************
 */
void
AtomicGetBoundingBox(RpAtomic * atomic, RwBBox * bbox)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RwReal interpPos = 0.0f, invInterpPos = 1.0f;
        RpMorphTarget *morphTarget;
        RwInt32 numMorphTargets, numVert, i;
        RwV3d *vertPosStart = (RwV3d *)NULL;
        RwV3d *vertPosEnd = (RwV3d *)NULL;

        numMorphTargets = RpGeometryGetNumMorphTargets(geometry);

        if( numMorphTargets > 1 )
        {
            RpInterpolator *interp;
            RwInt32 startMorphTarget, endMorphTarget;

            interp = RpAtomicGetInterpolator(atomic);

            interpPos =
                RpInterpolatorGetValue(interp) /
                RpInterpolatorGetScale(interp);

            invInterpPos = 1.0f - interpPos;

            startMorphTarget = RpInterpolatorGetStartMorphTarget(interp);
            endMorphTarget = RpInterpolatorGetEndMorphTarget(interp);

            morphTarget = RpGeometryGetMorphTarget(geometry, startMorphTarget);
            vertPosStart = RpMorphTargetGetVertices(morphTarget);

            morphTarget = RpGeometryGetMorphTarget(geometry, endMorphTarget);
            vertPosEnd = RpMorphTargetGetVertices(morphTarget);
        }
        else
        {
            morphTarget = RpGeometryGetMorphTarget(geometry, 0);
            vertPosStart = RpMorphTargetGetVertices(morphTarget);
        }

        numVert = RpGeometryGetNumVertices(geometry);

        bbox->sup.x = bbox->sup.y = bbox->sup.z = -RwRealMAXVAL;
        bbox->inf.x = bbox->inf.y = bbox->inf.z = RwRealMAXVAL;

        for( i = 0; i < numVert; i++ )
        {
            RwV3d vertPos;

            if( numMorphTargets > 1 )
            {
                RwV3d tempVec1, tempVec2;

                RwV3dScale(&tempVec1, &vertPosStart[i], invInterpPos);
                RwV3dScale(&tempVec2, &vertPosEnd[i], interpPos);
                RwV3dAdd(&vertPos, &tempVec1, &tempVec2);
            }
            else
            {
                vertPos = vertPosStart[i];
            }

            RwBBoxAddPoint(bbox, &vertPos);
        }
    }

    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
nextAtomicCB(RpAtomic * atomic, void *data)
{
    if( passedCurrentAtomic )
    {
        /*
         * Select atomic and stop processing remaining atomics...
         */
        *(RpAtomic **)data = atomic;
        return (RpAtomic *)NULL;
    }
    else if( atomic == *(RpAtomic **) data )
    {
        /*
         * Found current atomic, need to select next one
         */
        passedCurrentAtomic = TRUE;
    }

    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic *
RpClumpGetNextAtomic(RpClump * clump, RpAtomic * atomic)
{
    RpAtomic *data;

    if ( !clump || !atomic )
    {
        return (RpAtomic *)NULL;
    }

    passedCurrentAtomic = FALSE;
    data = atomic;
    RpClumpForAllAtomics(clump, nextAtomicCB, &data);
    if( data == atomic )
    {
        return (RpAtomic *)NULL;
    }

    return data;
}


/*
 *****************************************************************************
 */
void
UpdateSelectedStats(void)
{
    if( AtomicSelected )
    {
        RpGeometry *geometry;

        geometry = RpAtomicGetGeometry(AtomicSelected);

        if( geometry )
        {
            AtomicTotalTriangles = RpGeometryGetNumTriangles(geometry);
            AtomicTotalVertices = RpGeometryGetNumVertices(geometry);
            AtomicTotalMorphTargets = RpGeometryGetNumMorphTargets(geometry);
        }
        else
        {
            AtomicTotalTriangles = 0;
            AtomicTotalVertices = 0;
            AtomicTotalMorphTargets = 0;
        }

        AtomicGetBoundingBox(AtomicSelected, &CurrentAtomicBBox);
    }

    return;
}


/*
 *****************************************************************************
 */
RwBool 
SelectNextAtomic(void)
{
    if( !AtomicSelected )
    {
        AtomicSelected = ClumpGetFirstAtomic(Clump);
        currentAtomicNumber = 0;
    }
    else
    {
        AtomicSelected = RpClumpGetNextAtomic(Clump, AtomicSelected);
        currentAtomicNumber++;
    }

    if( !AtomicSelected )
    {
        return FALSE;
    }
    else
    {
        UpdateSelectedStats();

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RpCollisionTriangle *
TriangleIntersectionCallback(RpIntersection * intersection __RWUNUSED__,
                             RpCollisionTriangle *triangle, 
                             RwReal distance, void *data)
{
    if( distance < *(RwReal *)data )
    {
        *(RwReal *)data = distance;
    }

    return triangle;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicIntersectLine(RpAtomic * atomic, void *data)
{
    ClumpIntersectParams *intersectParams;
    RwReal distance;

    intersectParams = (ClumpIntersectParams *)data;
    
    distance = intersectParams->minDistance;

    RpAtomicForAllIntersections(atomic, &intersectParams->intersection,
                                TriangleIntersectionCallback,
                                &intersectParams->minDistance);

    if(intersectParams->minDistance < distance)
    {
        intersectParams->pickedAtomic = atomic;
        currentAtomicNumber = intersectParams->atomicNumber;
    }
    intersectParams->atomicNumber++;

    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic *
CameraPickAtomicOnPixel(RwCamera * camera, RwInt32 screenX, RwInt32 screenY)
{
    RwV2d pixel;
    ClumpIntersectParams intersectParams;

    pixel.x = (RwReal)screenX;
    pixel.y = (RwReal)screenY;

    intersectParams.intersection.type = rpINTERSECTLINE;

    RwCameraCalcPixelRay(camera, &intersectParams.intersection.t.line, &pixel);

    intersectParams.pickedAtomic = (RpAtomic *)NULL;
    intersectParams.minDistance = RwRealMAXVAL;
    intersectParams.atomicNumber = 0;
    RpClumpForAllAtomics(Clump, AtomicIntersectLine, &intersectParams);

    return intersectParams.pickedAtomic;
}


/*
 *****************************************************************************
 */
RwBool 
AtomicSelect(RwInt32 screenX, RwInt32 screenY)
{
    RpGeometry *geometry;
    RpAtomic *picked;

    /*
     * Do the intersection testing on the clump. Could use
     * RwCameraPickAtomicOnPixel or RpWorldPickAtomicOnLine here,
     * but they only test the atomics' bounding-spheres. Here we
     * want to perform the testing down to the polygon level to
     * make it more accurate...
     */
    picked = CameraPickAtomicOnPixel(Camera, screenX, screenY);

    if( !picked )
    {
        AtomicSelected = (RpAtomic *)NULL;
        return FALSE;
    }

    if( AtomicSelected == picked )
    {
        AtomicSelected = (RpAtomic *)NULL;
    }
    else
    {
        AtomicSelected = picked;

        geometry = RpAtomicGetGeometry(AtomicSelected);

        if( geometry )
        {
            AtomicTotalTriangles = RpGeometryGetNumTriangles(geometry);
            AtomicTotalVertices = RpGeometryGetNumVertices(geometry);
            AtomicTotalMorphTargets = RpGeometryGetNumMorphTargets(geometry);
        }
        else
        {
            AtomicTotalTriangles = 0;
            AtomicTotalVertices = 0;
            AtomicTotalMorphTargets = 0;
        }

        AtomicGetBoundingBox(AtomicSelected, &CurrentAtomicBBox);

        return TRUE;
    }

    return FALSE;
}

/*
 *****************************************************************************
 */
