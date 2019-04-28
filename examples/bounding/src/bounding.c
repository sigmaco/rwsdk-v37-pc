
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
 * bounding.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how a clump's bounding-box and bounding-sphere 
 *          can be calculated.
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "bounding.h"

/*
 * Number of points used in drawing a circle... 
 */
#define NUMPOINTS (100)

#define FASTWORLDBOXx
#define FASTWORLDSPHEREx

static RwBBox ClumpLocalBoundingBox;
static RwBBox ClumpWorldBoundingBox;

static RwSphere ClumpLocalBoundingSphere;
static RwSphere ClumpWorldBoundingSphere;

RwInt32 WorldSpace = 0;
RwInt32 BoundingBox = 1;

static RwBool NewSphere = TRUE;



/*
 *****************************************************************************
 */
static RpAtomic *
AtomicAddVertices(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;
    RwV3d *vertsIn = NULL, *vertsOut = NULL;
    RwInt32 numVerts, i;
    RwMatrix transform;
    RwBBox *clumpBBox;

    geometry = RpAtomicGetGeometry(atomic);
    numVerts = RpGeometryGetNumVertices(geometry);

    /*
     * Vertex positions in atomic local-space 
     * (assuming single morph target)...
     */
    vertsIn = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry, 0));

    /*
     * An array to hold the transformed vertices...
     */
    vertsOut = (RwV3d *)RwMalloc(numVerts * sizeof(RwV3d), rwID_NAOBJECT);

    if( WorldSpace )
    {
        /*
         * Create matrix to transform points to world space ...
         */     
        transform = *RwFrameGetLTM(RpAtomicGetFrame(atomic));
    }
    else
    {
        RwMatrix *atomicLTM, *clumpLTM, invClumpLTM;
        
        /*
         * Create matrix to transform points to clump space
         * (atomic --> world --> clump)...
         */     
        atomicLTM = RwFrameGetLTM(RpAtomicGetFrame(atomic));
        clumpLTM = RwFrameGetLTM(RpClumpGetFrame(RpAtomicGetClump(atomic)));
        RwMatrixInvert(&invClumpLTM, clumpLTM);
        
        RwMatrixMultiply(&transform, atomicLTM, &invClumpLTM);
    }
            
    RwV3dTransformPoints(vertsOut, vertsIn, numVerts, &transform);

    /*
     * Add to the clump's bounding-box...
     */
    clumpBBox = (RwBBox *)data;

    for(i=0; i<numVerts; i++)
    {
        RwBBoxAddPoint(clumpBBox, &vertsOut[i]);
    }

    /*
     * We're done with this atomic...
     */
    RwFree(vertsOut);

    return atomic;
}


/*
 *****************************************************************************
 */
static void 
ClumpCalculateWorldBoundingBox(RpClump *clump)
{
#ifdef FASTWORLDBOX

    /*
     * Calculate the world-space box from the clump-space box.
     * Does not give a tight-fitting box, but is faster...
     */
    RwInt32 i;
    RwV3d worldPoints[8];
    RwMatrix *ltm;

    ltm = RwFrameGetLTM(RpClumpGetFrame(clump));
    
    for(i=0; i<8; i++)
    {
        worldPoints[i].x = 
            i & 1 ? ClumpLocalBoundingBox.sup.x : ClumpLocalBoundingBox.inf.x;
        
        worldPoints[i].y = 
            i & 2 ? ClumpLocalBoundingBox.sup.y : ClumpLocalBoundingBox.inf.y;
        
        worldPoints[i].z = 
            i & 4 ? ClumpLocalBoundingBox.sup.z : ClumpLocalBoundingBox.inf.z;        
    }

    RwV3dTransformPoints(worldPoints, worldPoints, 8, ltm);

    RwBBoxCalculate(&ClumpWorldBoundingBox, worldPoints, 8);

#else

    /*
     * Calculate the world-space bounding-box directly from the geometry.
     * Gives a tight-fitting box, but is slower...
     */
    ClumpWorldBoundingBox.sup.x = ClumpWorldBoundingBox.sup.y = 
        ClumpWorldBoundingBox.sup.z = -RwRealMAXVAL;
    
    ClumpWorldBoundingBox.inf.x = ClumpWorldBoundingBox.inf.y = 
        ClumpWorldBoundingBox.inf.z = RwRealMAXVAL;

    RpClumpForAllAtomics(clump, AtomicAddVertices, &ClumpWorldBoundingBox);

#endif

    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicAddBoundingSphere(RpAtomic *atomic, void *data)
{
    RwSphere atomicSphere, *clumpSphere;
    RwReal separation, radius;
    RwV3d center, temp;
    RwMatrix transform;

    /*
     * The clump's bounding-sphere is calculated by first setting the
     * bounding-sphere equal the first atomic's bounding-sphere, then growing
     * the clump's bounding-sphere to enclose the next and future atomic
     * bounding-sphere...
     */

    clumpSphere = (RwSphere *)data;
    atomicSphere = *RpAtomicGetBoundingSphere(atomic);

    if( WorldSpace )
    {
        /*
         * Create matrix to transform points to world space ...
         */     
        transform = *RwFrameGetLTM(RpAtomicGetFrame(atomic));
    }
    else
    {
        RwMatrix *atomicLTM, *clumpLTM, invClumpLTM;
        
        /*
         * Create matrix to transform points to clump space
         * (atomic --> world --> clump)...
         */     
        atomicLTM = RwFrameGetLTM(RpAtomicGetFrame(atomic));
        clumpLTM = RwFrameGetLTM(RpClumpGetFrame(RpAtomicGetClump(atomic)));
        RwMatrixInvert(&invClumpLTM, clumpLTM);
        
        RwMatrixMultiply(&transform, atomicLTM, &invClumpLTM);
    }

    RwV3dTransformPoint(&atomicSphere.center, 
        &atomicSphere.center, &transform);

    if( NewSphere )
    {
        /*
         * Initialize the clump's bounding-sphere with the center 
         * and radius of the first atomic bounding-sphere...
         */
        clumpSphere->center = atomicSphere.center;
        clumpSphere->radius = atomicSphere.radius;

        NewSphere = FALSE;

        return atomic;
    }

    /*
     * Test if the clump's bounding-sphere already encloses this atomic's
     * bounding-sphere, by finding if the distance between the two bounding-
     * sphere centers plus the radius of the atomic's bounding-sphere, is 
     * less than the clump's bounding-sphere radius...
     */
    RwV3dSub(&temp, &atomicSphere.center, &clumpSphere->center);
    separation = RwV3dLength(&temp);

    if( separation + atomicSphere.radius < clumpSphere->radius )
    {
        return atomic;
    }

    /*
     * Calculate the clump's new bounding-sphere radius...
     */
    radius = (separation + atomicSphere.radius + clumpSphere->radius) * 0.5f;

    /*
     * Calculate the clump's new bounding-sphere center...
     */
    RwV3dScale(&temp, &temp, 1.0f/separation);

    RwV3dScale(&temp, &temp, atomicSphere.radius - clumpSphere->radius);
    RwV3dAdd(&center, &clumpSphere->center, &atomicSphere.center);
    RwV3dAdd(&center, &center, &temp);
    
    RwV3dScale(&center, &center, 0.5f);

    clumpSphere->center = center;
    clumpSphere->radius = radius;

    return atomic;
}


/*
 *****************************************************************************
 */
static void
ClumpCalculateWorldBoundingSphere(RpClump *clump)
{
#ifdef FASTWORLDSPHERE

    /*
     * Calculate the world-space bounding-sphere from the clump
     * bounding-sphere; assumes no scale-factors in the clump's frame
     * (faster method)...
     */
    RwMatrix *ltm;

    ltm = RwFrameGetLTM(RpClumpGetFrame(clump));

    RwV3dTransformPoint(&ClumpWorldBoundingSphere.center, 
        &ClumpLocalBoundingSphere.center, ltm);

    ClumpWorldBoundingSphere.radius = ClumpLocalBoundingSphere.radius;

#else

    /*
     * Calculate the world-space bounding-sphere directly from the 
     * atomic bounding-spheres (slower method)...
     */
    NewSphere = TRUE;

    RpClumpForAllAtomics(clump, AtomicAddBoundingSphere, 
        &ClumpWorldBoundingSphere);

#endif

    return;
}


/*
 *****************************************************************************
 */
void
UpdateWorldSpaceBoundingVolumes(RpClump *clump)
{
    ClumpCalculateWorldBoundingBox(clump);

    ClumpCalculateWorldBoundingSphere(clump);

    return;
}


/*
 *****************************************************************************
 */
void 
InitializeLocalSpaceBoundingVolumes(RpClump *clump)
{
    /*
     * Calculate the clump-space bounding-box...
     */
    ClumpLocalBoundingBox.sup.x = ClumpLocalBoundingBox.sup.y = 
        ClumpLocalBoundingBox.sup.z = -RwRealMAXVAL;
    
    ClumpLocalBoundingBox.inf.x = ClumpLocalBoundingBox.inf.y = 
        ClumpLocalBoundingBox.inf.z = RwRealMAXVAL;

    RpClumpForAllAtomics(clump, AtomicAddVertices, &ClumpLocalBoundingBox);

    /*
     * Calculate the clump-space bounding-sphere...
     */
    RpClumpForAllAtomics(clump, AtomicAddBoundingSphere, 
        &ClumpLocalBoundingSphere);

    return;
}


/*
 *****************************************************************************
 */
static void
ClumpRenderBoundingBox(RpClump *clump)
{
    RwInt32 i;
    RwIm3DVertex imVertex[8];
    RwMatrix *ltm = NULL;
    RwBBox *bbox = NULL;
    RwImVertexIndex indices[24] = 
    {
        0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7,
        7, 6, 6, 4, 0, 4, 1, 5, 2, 6, 3, 7
    };

    if( WorldSpace )
    {
        bbox = &ClumpWorldBoundingBox;

        ltm = NULL;
    }
    else
    {
        bbox = &ClumpLocalBoundingBox;

        ltm = RwFrameGetLTM(RpClumpGetFrame(clump));
    }
       
    for(i=0; i<8; i++)
    {
        RwIm3DVertexSetPos(&imVertex[i],
            i & 1 ? bbox->sup.x : bbox->inf.x,
            i & 2 ? bbox->sup.y : bbox->inf.y,
            i & 4 ? bbox->sup.z : bbox->inf.z
        );

        RwIm3DVertexSetRGBA(&imVertex[i], 196, 196, 0, 255);
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

    if( RwIm3DTransform(imVertex, 8, ltm, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, indices, 24);

        RwIm3DEnd();
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

    return;
}


/*
 *****************************************************************************
 */
static void
ClumpRenderBoundingCircle(RpClump *clump)
{
    RwIm3DVertex imVertex[NUMPOINTS + 1];
    RwV3d center;
    RwInt32 i;
    RwReal radius, constant;

    /*
     * Draw a camera-aligned circle with a radius equal to the 
     * radius of the clump's bounding-sphere...
     */

    if( WorldSpace )
    {
        center = ClumpWorldBoundingSphere.center;

        radius = ClumpWorldBoundingSphere.radius;
    }
    else
    {
        RwMatrix *ltm;

        ltm = RwFrameGetLTM(RpClumpGetFrame(clump));

        RwV3dTransformPoint(&center, 
            &ClumpLocalBoundingSphere.center, ltm);

        radius = ClumpLocalBoundingSphere.radius;
    }

    constant = 2.0f * rwPI / NUMPOINTS;

    for(i=0; i<NUMPOINTS+1; i++)
    {
        RwV3d point;

        point.x = center.x + radius * (RwReal)RwCos(i * constant);
        point.y = center.y + radius * (RwReal)RwSin(i * constant); 
        point.z = center.z;

        RwIm3DVertexSetPos(&imVertex[i], point.x, point.y, point.z);
        RwIm3DVertexSetRGBA(&imVertex[i], 196, 196, 0, 255);
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

    if( RwIm3DTransform(imVertex, NUMPOINTS + 1, NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPEPOLYLINE);

        RwIm3DEnd();
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

    return;
}


/*
 *****************************************************************************
 */
void 
ClumpRenderBoundingVolume(RpClump *clump)
{
    if( BoundingBox )
    {
        ClumpRenderBoundingBox(clump);
    }
    else
    {
        ClumpRenderBoundingCircle(clump);
    }

    return;
}

/*
 *****************************************************************************
 */
