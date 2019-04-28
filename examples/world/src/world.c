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
 * world.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how to construct a RenderWare BSP world
 *          procedurally.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rtimport.h"
#include "rtworld.h"

#include "skeleton.h"

#include "world.h"


/* 
 * Total number of vertices in the world 
 */
#define NOV (180)
 
/* 
 * Total number of triangles in the world 
 */
#define NOT (116)

/* 
 * Number of hexagons 
 */
#define NOH (20)

/* 
 * Number of pentagons 
 */
#define NOP (12)

static RwV3d VertexList[60]  = 
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

static RwUInt16 PentagonList[5 * NOP] = 
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

static RwUInt16 HexagonList[6 * NOH] = 
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



/*
 ***************************************************************************** 
 */
static RtWorldImport *
CreateWorldImport(void)
{
    /*
     * Creates an world import and allocates and initializes the world's 
     * vertices and triangles...
     */
    RtWorldImport *worldImport;
    RtWorldImportTriangle *triangles;
    RtWorldImportVertex *vertices;
    RpMaterial *hexagonMaterial, *pentagonMaterial;
    RwTexture *hexagonTexture, *pentagonTexture;
    RwTexCoords uvPentagon[5], uvHexagon[6];
    RwInt32 pentagonMatIndex, hexagonMatIndex;
    RwSurfaceProperties surfProp;
    RwChar *path;
    RwInt32 i, j;

	worldImport = RtWorldImportCreate();
	if( worldImport == NULL )
    {
        return NULL;
    }

	/* 
     * Allocate the memory to store the world's vertices and triangles... 
     */
	RtWorldImportAddNumVertices(worldImport, NOV);
    RtWorldImportAddNumTriangles(worldImport, NOT);

    /* 
     * Load the textures. These textures are created with a
     * reference count of 1, meaning this application has ownership.
     * Subsequently, when a texture is associated with a material, its 
     * reference count is incremented, indicating ownership also by the 
     * material...
     */
    path = RsPathnameCreate(RWSTRING ("textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    pentagonTexture = RwTextureRead(RWSTRING ("dai"), NULL);
    RwTextureSetFilterMode(pentagonTexture, rwFILTERLINEAR);

    hexagonTexture = RwTextureRead(RWSTRING ("whiteash"), NULL);
    RwTextureSetFilterMode(hexagonTexture, rwFILTERLINEAR);

    /* 
     * Create the materials. These materials are created with a reference
     * count of 1, meaning this application has ownership. Subsequently,
     * when the materials are associated with a worldImport, their reference
     * counts are incremented, indicating ownership also by the worldImport...
     */ 
    pentagonMaterial = RpMaterialCreate();
    RpMaterialSetTexture(pentagonMaterial, pentagonTexture);
    pentagonMatIndex = RtWorldImportAddMaterial(worldImport, pentagonMaterial);

    hexagonMaterial = RpMaterialCreate();
    RpMaterialSetTexture(hexagonMaterial, hexagonTexture);
    hexagonMatIndex = RtWorldImportAddMaterial(worldImport, hexagonMaterial);

    /*
     * Set the surface reflection coefficients...
     */
    surfProp.ambient = 0.3f;
    surfProp.diffuse = 0.7f;
    surfProp.specular = 0.0f;
    RpMaterialSetSurfaceProperties(hexagonMaterial, &surfProp);
    RpMaterialSetSurfaceProperties(pentagonMaterial, &surfProp);

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
            ((RwReal)RwCos (72.0f * i * rwPI / 180.0f) + 1.0f);
        
        uvPentagon[i].v = 0.5f * 
            ((RwReal)RwSin (72.0f * i * rwPI / 180.0f) + 1.0f);
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

    vertices = RtWorldImportGetVertices(worldImport);
    triangles = RtWorldImportGetTriangles(worldImport);

    /* 
     * Define vertices and triangles for pentagons... 
     */
    for(i=0, j=0; i<5*NOP; i+=5, j+=5)
    {
        /*
         * Calculate the face normal...
         */
        
        RwV3d a, b, normal;

        RwV3dSub(&a, &VertexList[PentagonList[i + 2]], 
            &VertexList[PentagonList[i]]);

        RwV3dSub(&b, &VertexList[PentagonList[i + 1]], 
            &VertexList[PentagonList[i]]);

        RwV3dCrossProduct(&normal, &a, &b);
        RwV3dNormalize(&normal, &normal);

        /* 
         * Initialize the vertices with a world-space vertex position, 
         * a normal, texture coordinates and a material...
         */
        vertices->OC = VertexList[PentagonList[i]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvPentagon[0];
        vertices->matIndex = pentagonMatIndex;
        vertices++;
        
        vertices->OC = VertexList[PentagonList[i + 1]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvPentagon[1];
        vertices->matIndex = pentagonMatIndex;
        vertices++;
        
        vertices->OC = VertexList[PentagonList[i + 2]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvPentagon[2];
        vertices->matIndex = pentagonMatIndex;
        vertices++;

        vertices->OC = VertexList[PentagonList[i + 3]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvPentagon[3];
        vertices->matIndex = pentagonMatIndex;
        vertices++;

        vertices->OC = VertexList[PentagonList[i + 4]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvPentagon[4];
        vertices->matIndex = pentagonMatIndex;
        vertices++;

        /* 
         * Initialize the triangles with indices into the vertex list 
         * and a material...
         */
        triangles->vertIndex[0] = j;
        triangles->vertIndex[1] = j + 2;
        triangles->vertIndex[2] = j + 1;
        triangles->matIndex = pentagonMatIndex;
        triangles++;

        triangles->vertIndex[0] = j;
        triangles->vertIndex[1] = j + 3;
        triangles->vertIndex[2] = j + 2;
        triangles->matIndex = pentagonMatIndex;
        triangles++;
       
        triangles->vertIndex[0] = j;
        triangles->vertIndex[1] = j + 4;
        triangles->vertIndex[2] = j + 3;
        triangles->matIndex = pentagonMatIndex;
        triangles++;
    }
    

    /* 
     * Define vertices and triangles for hexagon...
     */
    for(i=0; i<6*NOH; i+=6, j+=6)
    {
        /*
         * Calculate the face normal...
         */
        
        RwV3d a, b, normal;

        RwV3dSub(&a, &VertexList[HexagonList[i + 2]], 
            &VertexList[HexagonList[i]]);

        RwV3dSub(&b, &VertexList[HexagonList[i + 1]], 
            &VertexList[HexagonList[i]]);

        RwV3dCrossProduct(&normal, &a, &b);
        RwV3dNormalize(&normal, &normal);

        /* 
         * Initialize the vertices with world-space vertex position, 
         * a normal, textrue coordinates and a material...
         */
        vertices->OC = VertexList[HexagonList[i]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvHexagon[0];
        vertices->matIndex = hexagonMatIndex;
        vertices++;

        vertices->OC = VertexList[HexagonList[i + 1]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvHexagon[1];
        vertices->matIndex = hexagonMatIndex;
        vertices++;

        vertices->OC = VertexList[HexagonList[i + 2]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvHexagon[2];
        vertices->matIndex = hexagonMatIndex;
        vertices++;

        vertices->OC = VertexList[HexagonList[i + 3]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvHexagon[3];
        vertices->matIndex = hexagonMatIndex;
        vertices++;

        vertices->OC = VertexList[HexagonList[i + 4]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvHexagon[4];
        vertices->matIndex = hexagonMatIndex;
        vertices++;

        vertices->OC = VertexList[HexagonList[i + 5]];
        vertices->normal = normal;
        vertices->texCoords[0] = uvHexagon[5];
        vertices->matIndex = hexagonMatIndex;
        vertices++;

        /* 
         * Initialize the triangles with indices into the vertex list 
         * and a material...
         */
        triangles->vertIndex[0] = j;
        triangles->vertIndex[1] = j + 2;
        triangles->vertIndex[2] = j + 1;
        triangles->matIndex = hexagonMatIndex;
        triangles++;

        triangles->vertIndex[0] = j;
        triangles->vertIndex[1] = j + 3;
        triangles->vertIndex[2] = j + 2;
        triangles->matIndex = hexagonMatIndex;
        triangles++;
       
        triangles->vertIndex[0] = j;
        triangles->vertIndex[1] = j + 4;
        triangles->vertIndex[2] = j + 3;
        triangles->matIndex = hexagonMatIndex;
        triangles++;
       
        triangles->vertIndex[0] = j;
        triangles->vertIndex[1] = j + 5;
        triangles->vertIndex[2] = j + 4;
        triangles->matIndex = hexagonMatIndex;
        triangles++;
    }

    /*
     * We can remove the application's ownership of the materials and textures
     * it created by calling the corresponding destroy functions. This will
     * decrement their reference counts without actually deleting the
     * resources because they now have other owners (the worldImport owns the
     * materials, each material owns a texture). Now we can simply use 
     * RtWorldImportDestroy later when the application has finished with it...
     */
    RwTextureDestroy(pentagonTexture);
    RwTextureDestroy(hexagonTexture);

    RpMaterialDestroy(pentagonMaterial);
    RpMaterialDestroy(hexagonMaterial);

	return worldImport;
}


/*
 ***************************************************************************** 
 */
RpWorld *
CreateWorld(void)
{
    RtWorldImport *worldImport;
    static RtWorldImportParameters params;
    RpWorld *world;

    worldImport = CreateWorldImport();
    if( worldImport == NULL )
    {
        return NULL;
    }

    RtWorldImportParametersInit(&params);

    params.flags = rpWORLDTEXTURED | rpWORLDNORMALS | rpWORLDLIGHT;
    params.conditionGeometry = FALSE;
    params.calcNormals = FALSE;

    /*
     * Create the BSP world from the worldImport. 
     * During this process the materials have their reference counts incremented
     * to indicate ownership also by the BSP world...
     */
    world = RtWorldImportCreateWorld(worldImport, &params);

#if (defined(SAVEBSP))
    if( world )
    {
        static RwChar world_bsp[] = RWSTRING("world.bsp");
        RwChar *path  = RsPathnameCreate(world_bsp);
        RwStream *stream  = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, path);

        RsPathnameDestroy(path);
	        
        if( stream )
        {
            RpWorldStreamWrite(world, stream);

            RwStreamClose(stream, NULL);
        }
    }
#endif /* (defined(SAVEBSP)) */

    /*
     * Destroy the worldImport.
     * This process will also decrement the reference counts on the 
     * materials without actually deleting these resources because they 
     * now have other owners (the BSP world). Now we can simply use 
     * RpWorldDestroy later when the application has finished with it...
     */
    RtWorldImportDestroy(worldImport);

    return world;
}

/*
 ***************************************************************************** 
 */
