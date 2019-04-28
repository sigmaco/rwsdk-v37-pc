
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
 * geom.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the DMorph plugin
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "geom.h"
#include "skeleton.h"

/*
 * Number of squares along the edge of the surface...
 */
#define edges 12

 /*
 * Number of triangles that make up the surface...
 */
#define NumTri ((edges-1)*(edges-1)*2)

/*
 * Total number of vertices...
 */
#define NumVert (NumTri*3)


static RwV3d CubeVertexList[edges*edges];
static RwUInt16 TriangleList[NumVert];

/*
 *****************************************************************************
 */
RpGeometry * 
CreateSurface(int function)
{
    /*
     * Create on of the surfaces, i.e. the base or a delta...
     */
    RpGeometry *geometry;
    RpMorphTarget *morphTarget;
    RwV3d *vlist, *nlist;
    RwRGBA *plist, col1={0, 0, 100, 255}, col2={255, 255, 255, 255};
    RpTriangle *tlist;
    RpMaterial *triangleMaterial;
    RwSurfaceProperties surfProp;
    RwInt32 j;
    RwSphere boundingSphere;
    int rows, cols, tricount=0;

    triangleMaterial = RpMaterialCreate();
    for (rows=0; rows<edges-1; rows++)
    {
        for (cols=0; cols<edges-1; cols++)
        {
            TriangleList[tricount++] = (RwUInt16)(cols + rows*edges);
            TriangleList[tricount++] = (RwUInt16)(1 + cols + rows*edges);
            TriangleList[tricount++] = (RwUInt16)(1 + edges + cols + rows*edges);
            TriangleList[tricount++] = (RwUInt16)(cols + rows*edges);
            TriangleList[tricount++] = (RwUInt16)(1 + edges + cols + rows*edges);
            TriangleList[tricount++] = (RwUInt16)(edges + cols + rows*edges);
        }
    }
    for (rows=0; rows<edges; rows++)
    {
        for (cols=0; cols<edges; cols++)
        {
            float rcol = cols - ((edges-1) / 2.0f);
            float rrow = rows - ((edges-1) / 2.0f);

            CubeVertexList[cols + rows*edges].x = ((RwReal)rows)*40/(edges-1) - 20;
            CubeVertexList[cols + rows*edges].z = ((RwReal)cols)*40/(edges-1) - 20;

            if (function==1)
            {
                CubeVertexList[cols + rows*edges].y=75.0f * (((RwReal)rcol/(RwReal)(edges-1))*((RwReal)rcol/(RwReal)(edges-1)));
            }
            else if (function==2)
            {
                CubeVertexList[cols + rows*edges].y=75.0f * (((RwReal)rrow/(RwReal)(edges-1))*((RwReal)rrow/(RwReal)(edges-1)));
            }
            else
            {
                CubeVertexList[cols + rows*edges].y=0.0f;
            }
        }
    }


    /*
     * Create a geometry that will be lit, have vertex normals, 
     * and have texture coordinates. This geometry is LOCKED. It also
     * has a reference count of 1, meaning this application has ownership.
     * Subsequently, when the geometry is associated with atomics
     * the reference count is incremented for each atomic...
     */
    geometry = RpGeometryCreate(NumVert, NumTri, 
        rpGEOMETRYPOSITIONS | rpGEOMETRYLIGHT | rpGEOMETRYNORMALS | rpGEOMETRYPRELIT);

    if( geometry == NULL )
    {
        return NULL;
    }

    /*
     * Set the surface reflection coefficients...
     */
    surfProp.ambient = 1.0f;
    surfProp.diffuse = 0.7f;
    surfProp.specular = 0.3f;
    RpMaterialSetSurfaceProperties(triangleMaterial, &surfProp);

    /*
     * There's only one morph target, with index 0...
     */
    morphTarget = RpGeometryGetMorphTarget(geometry, 0);

    /* 
     * Construct the triangle and vertex lists by converting the 
     * pentagons and hexagons to triangles.
     * Each pentagon and hexagon has its own vertices and normals
     * so that the faces can be rendered flat...
     */
    vlist = RpMorphTargetGetVertices(morphTarget);
    tlist = RpGeometryGetTriangles(geometry);
    nlist = RpMorphTargetGetVertexNormals(morphTarget);
    plist = RpGeometryGetPreLightColors(geometry);

    for(j=0; j<NumVert; j+=3)
    {
        /*
         * Calculate the face normal...
         */
        RwV3d a, b, normal;

        RwV3dSub(&a, &CubeVertexList[TriangleList[j+2]],
            &CubeVertexList[TriangleList[j+1]]);

        RwV3dSub(&b, &CubeVertexList[TriangleList[j+0]], 
            &CubeVertexList[TriangleList[j+1]]);

        RwV3dCrossProduct(&normal, &a, &b);

        RwV3dNormalize(&normal, &normal);
       

        /*
         * Initialize each vertex of the face with a position, a normal
         * and texture coordinates...
         */
        *vlist++ = CubeVertexList[TriangleList[j]];
        *nlist++ = normal;
        *vlist++ = CubeVertexList[TriangleList[j+1]];
        *nlist++ = normal;
        *vlist++ = CubeVertexList[TriangleList[j+2]];
        *nlist++ = normal;

        if ((((j+1)/2)%2)==0)
        {
            *plist++ = col1;
            *plist++ = col1;
            *plist++ = col1;
        }
        else
        {
            *plist++ = col2;
            *plist++ = col2;
            *plist++ = col2;
        }

        /*
         * Initialize each face triangle with vertex list indices and
         * a material. The first call to RpGeometryTriangleSetMaterial will
         * increment the material's reference count by 2 to indicate
         * ownership by the triangle AND the geometry; subsequent calls
         * only increment it by 1 each time...
         */
        RpGeometryTriangleSetVertexIndices(geometry, tlist, (RwInt16)j, (RwInt16)(j+1), (RwInt16)(j+2));
        RpGeometryTriangleSetMaterial(geometry, tlist++, triangleMaterial);

    }


    /*
     * Need to re-calculate and set the bounding-sphere ourselves
     * before unlocking...
     */

    RpMorphTargetCalcBoundingSphere(morphTarget, &boundingSphere);
    RpMorphTargetSetBoundingSphere(morphTarget, &boundingSphere);

    RpGeometryUnlock(geometry);

    RpMaterialDestroy(triangleMaterial);

    return geometry;
}

RpClump *
PutInClump(RpGeometry* geometry)
{
    RpClump *clump;
    RwFrame *frame;
    RpAtomic *atomic;

    clump = RpClumpCreate();
    frame = RwFrameCreate();
    RpClumpSetFrame(clump, frame);

    atomic = RpAtomicCreate();
    frame = RwFrameCreate();
    RpAtomicSetFrame(atomic, frame);

    /*
     * Associate the geometry with the atomic. This will increment the
     * geometry's reference count by 1...
     */
    RpAtomicSetGeometry(atomic, geometry, 0);

    RpClumpAddAtomic(clump, atomic);

    RwFrameAddChild(RpClumpGetFrame(clump), frame);

    /*
     * As a convenience, we can remove the application's ownership of the 
     * geometry, materials and textures it created by calling the corresponding 
     * destroy functions. This will decrement their reference counts
     * without actually deleting the resources because they now have other
     * owners (the atomic owns the geometry, the geometry and its triangles
     * own the materials, each material owns a texture). Now we can simply use 
     * RpClumpDestroy later when the application has finished with it...
     */
    RpGeometryDestroy(geometry);

    return(clump);
}
/*
 *****************************************************************************
 */
