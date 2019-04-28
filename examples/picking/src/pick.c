
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
 * pick.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics atomic picking example, showing how atomics can be picked 
 *          either by their bounding spheres or triangles.
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpcollis.h"
#include "rtpick.h"

#include "skeleton.h"

#include "pick.h"
#include "main.h"

typedef struct
{
        RpIntersection intersection;
        RpAtomic *pickedAtomic;
        RwReal minDistance;
}
AtomicIntersectParams;

/*
 * Number of points used in drawing a circle... 
 */
#define NUMPOINTS (100)

static RpAtomic *PickedAtomic = NULL;
static RpAtomicCallBackRender DefaultAtomicRender = NULL;

enum PickModes CurrentPickMode = TRIANGLES;


/*
 *****************************************************************************
 */
static void
AtomicRenderBoundingBox(RpAtomic *atomic)
{
    static RwImVertexIndex index[24] = 
    {
        0, 1,  1, 2,  2, 3,  3, 0,  4, 5,  5, 6,
        6, 7,  7, 4,  0, 4,  3, 7,  1, 5,  2, 6
    };

    RwIm3DVertex boxVertices[8];
    RwUInt8 red, green, blue, alpha;

    RpGeometry *geometry;
    RpMorphTarget *morphTarget;
    RwV3d *vertices;
    RwInt32 numVerts;
    RwBBox bBox;
    RwMatrix *ltm;

    /*
     * Get the atomic's vertices to calculate its bounding box...
     */
    geometry = RpAtomicGetGeometry(atomic);
    morphTarget  = RpGeometryGetMorphTarget(geometry, 0);
    vertices  = RpMorphTargetGetVertices(morphTarget);
    numVerts = RpGeometryGetNumVertices(geometry);

    RwBBoxCalculate(&bBox, vertices, numVerts);

    red = green = 196;
    blue = 0;
    alpha = 255;

    RwIm3DVertexSetRGBA(&boxVertices[0], red, green, blue, alpha);
    RwIm3DVertexSetPos(&boxVertices[0], bBox.inf.x, bBox.inf.y, bBox.inf.z);

    RwIm3DVertexSetRGBA(&boxVertices[1], red, green, blue, alpha);
    RwIm3DVertexSetPos(&boxVertices[1], bBox.sup.x, bBox.inf.y, bBox.inf.z);

    RwIm3DVertexSetRGBA(&boxVertices[2], red, green, blue, alpha);
    RwIm3DVertexSetPos(&boxVertices[2], bBox.sup.x, bBox.sup.y, bBox.inf.z);

    RwIm3DVertexSetRGBA(&boxVertices[3], red, green, blue, alpha);
    RwIm3DVertexSetPos(&boxVertices[3], bBox.inf.x, bBox.sup.y, bBox.inf.z);

    RwIm3DVertexSetRGBA(&boxVertices[4], red, green, blue, alpha);
    RwIm3DVertexSetPos(&boxVertices[4], bBox.inf.x, bBox.inf.y, bBox.sup.z);

    RwIm3DVertexSetRGBA(&boxVertices[5], red, green, blue, alpha);
    RwIm3DVertexSetPos(&boxVertices[5], bBox.sup.x, bBox.inf.y, bBox.sup.z);

    RwIm3DVertexSetRGBA(&boxVertices[6], red, green, blue, alpha);
    RwIm3DVertexSetPos(&boxVertices[6], bBox.sup.x, bBox.sup.y, bBox.sup.z);

    RwIm3DVertexSetRGBA(&boxVertices[7], red, green, blue, alpha);
    RwIm3DVertexSetPos(&boxVertices[7], bBox.inf.x, bBox.sup.y, bBox.sup.z);

    ltm = RwFrameGetLTM(RpAtomicGetFrame(atomic));

    if( RwIm3DTransform(boxVertices, 8, ltm, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, index, 24);

        RwIm3DEnd();
    }

    return;
}


/*
 *****************************************************************************
 */
static void
AtomicRenderBoundingCircle(RpAtomic *atomic)
{
    RwIm3DVertex circle[NUMPOINTS + 1];
    RwV3d point;
    RwInt32 i;
    RwSphere *sphere;
    RwMatrix *ltm;
    RwV3d center;

    /*
     * Draw a circle with a radius equal to the radius of the atomic's 
     * bounding sphere...
     */
    
    sphere = RpAtomicGetBoundingSphere(atomic);

    /*
     * Position the circle so its center equals the center of the picked 
     * atomic's bounding sphere...
     */
    ltm = RwFrameGetLTM(RpAtomicGetFrame(atomic));
    RwV3dTransformPoint(&center, &sphere->center, ltm);

    for(i=0; i<NUMPOINTS + 1; i++)
    {
        point.x = center.x + (RwReal)RwCos(i / (NUMPOINTS / 2.0f) * rwPI) *
            sphere->radius;

        point.y = center.y + (RwReal)RwSin(i / (NUMPOINTS / 2.0f) * rwPI) *
            sphere->radius; 

        point.z = center.z;

        RwIm3DVertexSetPos(&circle[i], point.x, point.y, point.z);
        RwIm3DVertexSetRGBA(&circle[i], 196, 196, 0, 255);
    }

    if( RwIm3DTransform(circle, NUMPOINTS + 1, NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPEPOLYLINE);

        RwIm3DEnd();
    }

    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicRenderCallBack(RpAtomic *atomic)
{
    /* 
     * If the atomic has been picked render it depending upon the method of 
     * picking, otherwise use its default callback...
     */
    if( PickedAtomic == atomic )
    {
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

        if( CurrentPickMode == TRIANGLES )
        {
            AtomicRenderBoundingBox(atomic);
        }
        else if( CurrentPickMode == BOUNDINGSPHERE )
        {  
            AtomicRenderBoundingCircle(atomic);
        }

        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
    }
    
    DefaultAtomicRender(atomic);

    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicSetRenderCallBack(RpAtomic *atomic,
                        void *data __RWUNUSED__)
{
    if( DefaultAtomicRender == NULL )
    {
        /*
         * Get the atomic's default render callback so atomics which aren't 
         * picked are render with the default...
         */
        DefaultAtomicRender = RpAtomicGetRenderCallBack(atomic);
    }

    RpAtomicSetRenderCallBack(atomic, AtomicRenderCallBack);

    return atomic;
}


void
ClumpSetRenderCallBack(RpClump *clump)
{
    RpClumpForAllAtomics(clump, AtomicSetRenderCallBack, NULL);

    return;
}


/*
 *****************************************************************************
 */
static RpCollisionTriangle * 
TriangleIntersectionCallback(RpIntersection *intersection __RWUNUSED__,
                             RpCollisionTriangle *triangle, 
                             RwReal distance,
                             void *data)
{
    RwReal *minDistance = (RwReal *)data;

    /*
     * The intersection distance is the distance to the point of 
     * intersection in the collision triangle from the start of the line,
     * normalized to the length of the line. Compare this with the current
     * minimum intersection distance...
     */
    if( distance < *minDistance )
    {
        *minDistance = distance;
    }

    return triangle;
}


static RpAtomic * 
AtomicIntersectLine1(RpIntersection *intersection __RWUNUSED__,
                     RpWorldSector *sector __RWUNUSED__,
                     RpAtomic *atomic, 
                     RwReal distance __RWUNUSED__,
                     void *data)
{
    /*
     * This callback is executed for each atomic bounding-sphere intersected. 
     * For atomics that span more than one world sector, we get a single 
     * callback for the world sector that is nearest to the start of the
     * intersecting line.
     * 
     * Given that we have a bounding-sphere intersection, test the line 
     * for intersections with the atomic's triangles...
     */ 
    AtomicIntersectParams *intersectParams;
    RwReal oldDistance;

    intersectParams = (AtomicIntersectParams *)data;

    oldDistance = intersectParams->minDistance;

    RpAtomicForAllIntersections(atomic, &intersectParams->intersection, 
                                TriangleIntersectionCallback, &intersectParams->minDistance);
    
    if( intersectParams->minDistance < oldDistance )
    {
        intersectParams->pickedAtomic = atomic;
    }

    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic * 
AtomicIntersectLine2(RpIntersection *intersection __RWUNUSED__,
                     RpWorldSector *sector __RWUNUSED__,
                     RpAtomic *atomic, 
                     RwReal distance,
                     void *data)
{
    /*
     * This callback is executed for each atomic bounding-sphere intersected. 
     * For atomics that span more than one world sector, we get a single 
     * callback for the world sector that is nearest to the start of the
     * intersecting line...
     */ 
    AtomicIntersectParams *intersectParams;

    intersectParams = (AtomicIntersectParams *)data;

    /*
     * The intersection distance is the distance of the atomic’s 
     * bounding-sphere center from start of line, projected onto the line, 
     * normalized to length of line. Compare this with the current
     * minimum intersection distance...
     */
    if( distance < intersectParams->minDistance )
    {
        /*
         * This atomic is currently the nearest, so remember it...
         */
        intersectParams->pickedAtomic = atomic;

        /*
         * Update the nearest intersection distance...
         */
        intersectParams->minDistance = distance;
    }

    return atomic;
}


/*
 *****************************************************************************
 */
void
PickNearestAtomic(RwV2d *pixel)
{
    AtomicIntersectParams intersectParams;
    RwLine pixelRay;

    /*
     * An atomic is picked based on the intersection with a line that passes
     * through the camera's view-frustum, defined by the camera's center of
     * projection and the position of the pixel on the view-plane. The line is 
     * delimited by the near clip-plane (its start point) and the far 
     * clip-plane (its end position)...
     */
    RwCameraCalcPixelRay(Camera, &pixelRay, pixel);

    intersectParams.intersection.t.line = pixelRay;
    intersectParams.intersection.type = rpINTERSECTLINE;
    intersectParams.pickedAtomic = NULL;
    intersectParams.minDistance = RwRealMAXVAL;

    /*
     * Pick atomics using the current selected method...
     */
    switch( CurrentPickMode )
    {
        case TRIANGLES:
            {
                /* 
                 * Search for line-atomic intersections that penetrate the 
                 * atomic's triangles...
                 */
                RpWorldForAllAtomicIntersections(World, &intersectParams.intersection, 
                                                 AtomicIntersectLine1, &intersectParams);

                PickedAtomic = intersectParams.pickedAtomic;

                break;
            }

        case BOUNDINGSPHERE:
            {
                /* 
                 * Search for line-atomic intersections that only penetrate the 
                 * atomic's bounding-sphere...
                 */
                RpWorldForAllAtomicIntersections(World, &intersectParams.intersection, 
                                                 AtomicIntersectLine2, &intersectParams);

                PickedAtomic = intersectParams.pickedAtomic;

                break;
            }
            
        default:
            {
                PickedAtomic = NULL;

                break;
            }
    }

    return;
}

/*
*****************************************************************************
*/
