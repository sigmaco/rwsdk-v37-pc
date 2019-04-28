
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
 * geometry.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how RenderWare clumps can be generated dynamically.
 * *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"


/*
 * Number of pentagons...
 */
#define NOP (12)

/*
 * Number of hexagons...
 */
#define NOH (20)

/*
 * Total number of vertices (non-shared between faces)...
 */
#define NOV (180)

/*
 * Total number of triangles the faces are divided up into...
 */
#define NOT (116)

static RwV3d BuckyBallVertexList[60] = 
{
    {   0.00f,  145.60f,   30.00f },
    {   0.00f,  145.60f,  -30.00f },
    {  48.50f,  127.00f,   60.00f },
    {  97.00f,  108.50f,   30.00f },
    {  30.00f,   97.00f,  108.50f },
    {  60.00f,   48.50f,  127.00f },
    { -30.00f,   97.00f,  108.50f },
    { -60.00f,   48.50f,  127.00f },
    { -48.50f,  127.00f,   60.00f },
    { -97.00f,  108.50f,   30.00f },

    {  30.00f,  -97.00f, -108.50f },
    {  60.00f,  -48.50f, -127.00f },
    { -30.00f,  -97.00f, -108.50f },
    { -60.00f,  -48.50f, -127.00f },
    { -48.50f, -127.00f,  -60.00f },
    { -97.00f, -108.50f,  -30.00f },
    {   0.00f, -145.60f,  -30.00f },
    {   0.00f, -145.60f,   30.00f },
    {  48.50f, -127.00f,  -60.00f },
    {  97.00f, -108.50f,  -30.00f },

    {  48.50f,  127.00f,  -60.00f },
    {  97.00f,  108.50f,  -30.00f },
    { 127.00f,   50.00f,   48.50f },
    { 108.50f,   30.00f,   97.00f },
    {  30.00f,    0.00f,  145.60f },
    { -30.00f,    0.00f,  145.60f },
    {-108.50f,   30.00f,   97.00f },
    {-127.00f,   60.00f,   48.50f },
    { -97.00f,  108.50f,  -30.00f },
    { -48.50f,  127.00f,  -60.00f },

    {  30.00f,    0.00f, -145.60f },
    { -30.00f,    0.00f, -145.60f },
    {-108.50f,  -30.00f,  -97.00f },
    {-127.00f,  -60.00f,  -48.50f },
    { -97.00f, -108.50f,   30.00f },
    { -48.50f, -127.00f,   60.00f },
    {  48.50f, -127.00f,   60.00f },
    {  97.00f, -108.50f,   30.00f },
    { 127.00f,  -60.00f,  -48.50f },
    { 108.50f,  -30.00f,  -97.00f },

    { -30.00f,   97.00f, -108.50f },
    { -60.00f,   48.50f, -127.00f },
    {-108.50f,   30.00f,  -97.00f },
    {-127.00f,   60.00f,  -48.50f },
    {-145.60f,   30.00f,    0.00f },
    {-145.60f,  -30.00f,    0.00f },
    {-127.00f,  -60.00f,   48.50f },
    {-108.50f,  -30.00f,   97.00f },
    { -60.00f,  -48.50f,  127.00f },
    { -30.00f,  -97.00f,  108.50f },

    {  30.00f,  -97.00f,  108.50f },
    {  60.00f,  -48.50f,  127.00f },
    { 108.50f,  -30.00f,   97.00f },
    { 127.00f,  -60.00f,   48.50f },
    { 145.60f,  -30.00f,    0.00f },
    { 145.60f,   30.00f,    0.00f },
    { 127.00f,   60.00f,  -48.50f },
    { 108.50f,   30.00f,  -97.00f },
    {  60.00f,   48.50f, -127.00f },
    {  30.00f,   97.00f, -108.50f }
};

static RwUInt16 BuckyBallPentagonList[5*NOP] = 
{
     0,  2,  4,  6,  8,
    10, 12, 14, 16, 18,
     1, 29, 40, 59, 20,
    57, 58, 30, 11, 39,
     3, 21, 56, 55, 22,
    53, 54, 38, 19, 37,
     5, 23, 52, 51, 24,
    49, 50, 36, 17, 35,
     7, 25, 48, 47, 26,
    15, 33, 45, 46, 34,
     9, 27, 44, 43, 28,
    13, 31, 41, 42, 32
};

static RwUInt16 BuckyBallHexagonList[6*NOH] = 
{
     0,  1, 20, 21,  3,  2,
     2,  3, 22, 23,  5,  4,
     4,  5, 24, 25,  7,  6,
     6,  7, 26, 27,  9,  8,
     8,  9, 28, 29,  1,  0,
    10, 11, 30, 31, 13, 12,
    12, 13, 32, 33, 15, 14,
    14, 15, 34, 35, 17, 16,
    16, 17, 36, 37, 19, 18,
    18, 19, 38, 39, 11, 10,
    59, 58, 57, 56, 21, 20,
    54, 55, 56, 57, 39, 38,
    22, 55, 54, 53, 52, 23,
    50, 51, 52, 53, 37, 36,
    51, 50, 49, 48, 25, 24,
    46, 47, 48, 49, 35, 34,
    47, 46, 45, 44, 27, 26,
    42, 43, 44, 45, 33, 32,
    43, 42, 41, 40, 29, 28,
    40, 41, 31, 30, 58, 59
};

RpClump *CreateBuckyBall(RwBool normalize);



/*
 *****************************************************************************
 */
RpClump * 
CreateBuckyBall(RwBool normalize)
{
    RpClump *clump;
    RwFrame *frame;
    RpAtomic *atomic;
    RpGeometry *geometry;
    RpMorphTarget *morphTarget;
    RwV3d *vlist, *nlist;
    RpTriangle *tlist;
    RpMaterial *pentagonMaterial, *hexagonMaterial;
    RwTexture *pentagonTexture, *hexagonTexture;
    RwSurfaceProperties surfProp;
    RwTexCoords *texCoord;
    RwTexCoords uvPentagon[5], uvHexagon[6];
    RwInt32 i, j;
    RwChar *path;

    /*
     * Create the bucky-ball textures. These textures are created with a
     * reference count of 1, meaning this application has ownership.
     * Subsequently, when a texture is associated with a material, its 
     * reference count is incremented, indicating ownership also by the 
     * material...
     */
    path = RsPathnameCreate(RWSTRING("textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    pentagonTexture = RwTextureRead(RWSTRING("dai"), NULL);
    RwTextureSetFilterMode(pentagonTexture, rwFILTERLINEAR);

    hexagonTexture = RwTextureRead(RWSTRING("whiteash"), NULL);
    RwTextureSetFilterMode(hexagonTexture, rwFILTERLINEAR);

    /*
     * ...and materials. These materials are created with a reference
     * count of 1, meaning this application has ownership. Subsequently,
     * when the materials are associated with a geometry and its 
     * triangles, their reference counts are incremented, indicating 
     * ownership also by the geometry and each triangle...
     */
    pentagonMaterial = RpMaterialCreate();
    RpMaterialSetTexture(pentagonMaterial, pentagonTexture);

    hexagonMaterial = RpMaterialCreate();
    RpMaterialSetTexture(hexagonMaterial, hexagonTexture);

    /*
     * Pre-calculate some texture coordinates for the pentagons...
     */
    for(i=0; i<5; i++)
    {
        /*
         * u = 0.5 * [cos(72i*pi/180)+1]
         * v = 0.5 * [sin(72i*pi/180)+1]
         * Angle at center of pentagon between adjacent vertices = 72 degrees.
         */
        uvPentagon[i].u = 0.5f * 
            ((RwReal)RwCos(72.0f * i * rwPI / 180.0f) + 1.0f);
        
        uvPentagon[i].v = 0.5f *
            ((RwReal)RwSin(72.0f * i * rwPI / 180.0f) + 1.0f);
    }

    /*
     * ...and for the hexagons...
     */
    for(i=0; i<6; i++)
    {
        /*
         * u = 0.5 * [cos(60i*pi/180)+1]
         * v = 0.5 * [sin(60i*pi/180)+1]
         * Angle at center of hexagon between adjacent vertices = 60 degrees.
         */
        uvHexagon[i].u = 0.5f *
            ((RwReal)RwCos(60.0f * i * rwPI / 180.0f) + 1.0f);
        
        uvHexagon[i].v = 0.5f *
            ((RwReal)RwSin(60.0f * i * rwPI / 180.0f) + 1.0f);
    }

    /*
     * Create a geometry that will be lit, have vertex normals, 
     * and have texture coordinates. This geometry is LOCKED. It also
     * has a reference count of 1, meaning this application has ownership.
     * Subsequently, when the geometry is associated with atomics
     * the reference count is incremented for each atomic...
     */
    geometry = RpGeometryCreate(NOV, NOT, 
        rpGEOMETRYLIGHT | rpGEOMETRYNORMALS | rpGEOMETRYTEXTURED);

    if( geometry == NULL )
    {
        return NULL;
    }

    /*
     * Set the surface reflection coefficients...
     */
    surfProp.ambient = 0.3f;
    surfProp.diffuse = 0.7f;
    surfProp.specular = 0.0f;
    RpMaterialSetSurfaceProperties(pentagonMaterial, &surfProp);
    RpMaterialSetSurfaceProperties(hexagonMaterial, &surfProp);

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
    texCoord = RpGeometryGetVertexTexCoords(geometry, rwTEXTURECOORDINATEINDEX0);

    /*
     * THREE triangles per pentagon...
     */
    for(i=0, j=0; i<5*NOP; i+=5, j+=5)
    {
        /*
         * Calculate the face normal...
         */
        RwV3d a, b, normal;

        RwV3dSub(&a, &BuckyBallVertexList[BuckyBallPentagonList[i + 2]], 
            &BuckyBallVertexList[BuckyBallPentagonList[i]]);

        RwV3dSub(&b, &BuckyBallVertexList[BuckyBallPentagonList[i + 1]], 
            &BuckyBallVertexList[BuckyBallPentagonList[i]]);

        RwV3dCrossProduct(&normal, &a, &b);
        RwV3dNormalize(&normal, &normal);

        /*
         * Initialize each vertex of the face with a position, a normal
         * and texture coordinates...
         */
        *vlist++ = BuckyBallVertexList[BuckyBallPentagonList[i]];
        *nlist++ = normal;
        *texCoord++ = uvPentagon[0];

        *vlist++ = BuckyBallVertexList[BuckyBallPentagonList[i + 1]];
        *nlist++ = normal;
        *texCoord++ = uvPentagon[1];

        *vlist++ = BuckyBallVertexList[BuckyBallPentagonList[i + 2]];
        *nlist++ = normal;
        *texCoord++ = uvPentagon[2];

        *vlist++ = BuckyBallVertexList[BuckyBallPentagonList[i + 3]];
        *nlist++ = normal;
        *texCoord++ = uvPentagon[3];

        *vlist++ = BuckyBallVertexList[BuckyBallPentagonList[i + 4]];
        *nlist++ = normal;
        *texCoord++ = uvPentagon[4];

        /*
         * Initialize each face triangle with vertex list indices and
         * a material. The first call to RpGeometryTriangleSetMaterial will
         * increment the material's reference count by 2 to indicate
         * ownership by the triangle AND the geometry; subsequent calls
         * only increment it by 1 each time...
         */
        RpGeometryTriangleSetVertexIndices(geometry, tlist, 
            (RwUInt16)j, (RwUInt16)(j + 2), (RwUInt16)(j + 1));
        RpGeometryTriangleSetMaterial(geometry, tlist++, pentagonMaterial);

        RpGeometryTriangleSetVertexIndices(geometry, tlist, (RwUInt16)j, 
            (RwUInt16)(j + 3), (RwUInt16)(j + 2));
        RpGeometryTriangleSetMaterial(geometry, tlist++, pentagonMaterial);

        RpGeometryTriangleSetVertexIndices(geometry, tlist, (RwUInt16)j, 
            (RwUInt16)(j + 4), (RwUInt16)(j + 3));
        RpGeometryTriangleSetMaterial(geometry, tlist++, pentagonMaterial);
        
    }

    /*
     * FOUR triangles per hexagon....
     */
    for(i=0; i<6*NOH; i+=6, j+=6)
    {
        /*
         * Calculate the face normal...
         */
        RwV3d a, b, normal;

        RwV3dSub(&a, &BuckyBallVertexList[BuckyBallHexagonList[i+2]], 
            &BuckyBallVertexList[BuckyBallHexagonList[i]]);

        RwV3dSub(&b, &BuckyBallVertexList[BuckyBallHexagonList[i+1]], 
            &BuckyBallVertexList[BuckyBallHexagonList[i]]);

        RwV3dCrossProduct(&normal, &a, &b);
        RwV3dNormalize(&normal, &normal);

        /*
         * Initialize each vertex of the face with a position, a normal
         * and texture coordinates...
         */
        *vlist++ = BuckyBallVertexList[BuckyBallHexagonList[i]];
        *nlist++ = normal;
        *texCoord++ = uvHexagon[0];

        *vlist++ = BuckyBallVertexList[BuckyBallHexagonList[i + 1]];
        *nlist++ = normal;
        *texCoord++ = uvHexagon[1];

        *vlist++ = BuckyBallVertexList[BuckyBallHexagonList[i + 2]];
        *nlist++ = normal;
        *texCoord++ = uvHexagon[2];

        *vlist++ = BuckyBallVertexList[BuckyBallHexagonList[i + 3]];
        *nlist++ = normal;
        *texCoord++ = uvHexagon[3];

        *vlist++ = BuckyBallVertexList[BuckyBallHexagonList[i + 4]];
        *nlist++ = normal;
        *texCoord++ = uvHexagon[4];

        *vlist++ = BuckyBallVertexList[BuckyBallHexagonList[i + 5]];
        *nlist++ = normal;
        *texCoord++ = uvHexagon[5];

        /*
         * Initialize each face triangle with vertex list indices and
         * a material. The first call to RpGeometryTriangleSetMaterial will
         * increment the material's reference count by 2 to indicate
         * ownership by the triangle AND the geometry; subsequent calls
         * only increment it by 1 each time...
         */
        RpGeometryTriangleSetVertexIndices(geometry, tlist,
            (RwUInt16)j, (RwUInt16)(j + 2), (RwUInt16)(j + 1));
        RpGeometryTriangleSetMaterial(geometry, tlist++, hexagonMaterial);

        RpGeometryTriangleSetVertexIndices(geometry, tlist, (RwUInt16)j, 
            (RwUInt16)(j + 3), (RwUInt16)(j + 2));
        RpGeometryTriangleSetMaterial(geometry, tlist++, hexagonMaterial);

        RpGeometryTriangleSetVertexIndices(geometry, tlist, (RwUInt16)j, 
            (RwUInt16)(j + 4), (RwUInt16)(j + 3));
        RpGeometryTriangleSetMaterial(geometry, tlist++, hexagonMaterial);

        RpGeometryTriangleSetVertexIndices(geometry, tlist, (RwUInt16)j, 
            (RwUInt16)(j + 5), (RwUInt16)(j + 4));
        RpGeometryTriangleSetMaterial(geometry, tlist++, hexagonMaterial);
    }

    if( normalize )
    {
        /* 
         * Center and scale to unit size...
         */
        RwSphere boundingSphere;
        RwMatrix *matrix;
        RwV3d temp;

        RpMorphTargetCalcBoundingSphere(morphTarget, &boundingSphere);
        matrix = RwMatrixCreate();

        RwV3dScale(&temp, &boundingSphere.center, -1.0f);
        RwMatrixTranslate(matrix, &temp, rwCOMBINEREPLACE);

        temp.x = temp.y = temp.z = 1.0f / boundingSphere.radius;
        RwMatrixScale(matrix, &temp, rwCOMBINEPOSTCONCAT);

        /*
         * This will re-calculate and set the new bounding-sphere 
         * and also UNLOCK the geometry...
         */
        RpGeometryTransform(geometry, matrix);
        
        RwMatrixDestroy(matrix);
    }
    else
    {
        /*
         * Need to re-calculate and set the bounding-sphere ourselves
         * before unlocking...
         */
        RwSphere boundingSphere;

        RpMorphTargetCalcBoundingSphere(morphTarget, &boundingSphere);
        RpMorphTargetSetBoundingSphere(morphTarget, &boundingSphere);

        RpGeometryUnlock(geometry);
    }

    /*
     * That's it...stick it in a single-atomic clump and return...
     */
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

    RpMaterialDestroy(pentagonMaterial);
    RpMaterialDestroy(hexagonMaterial);

    RwTextureDestroy(pentagonTexture);
    RwTextureDestroy(hexagonTexture);

    return clump;
}

/*
 *****************************************************************************
 */
