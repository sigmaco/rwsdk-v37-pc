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
 * Copyright (c) 2003 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * gcond.c
 *
 * Copyright (C) 2003 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how to use geometry conditioning for textured scenes
 * with discontiguous UV coordinates.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rtimport.h"
#include "rtworld.h"
#include "rtgcond.h"

#include "gcond.h"

#include "skeleton.h"






 /*
 * Number of triangles that make up the surface...
 */
#define NumTri ((NumEdges-1)*(NumEdges-1)*2)

/*
 * Total number of vertices...
 */
#define NumVert (NumTri*3)

typedef struct
{
    RwV3d vertex;
    RwTexCoords uvs;
}vInfo;

typedef struct
{
    RwUInt16 index;
    RwInt32 uOffset;
    RwInt32 vOffset;
}iInfo;

static vInfo VertexList[NumEdges*NumEdges];
static iInfo IndexList[NumVert];

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
    RpMaterial *polygonMaterial;
    RwTexture *polygonTexture;
    RwInt32 polygonMatIndex;
    RwSurfaceProperties surfProp;
    RwChar *path;
    RwInt32 j;
    RwInt32 rows, cols, tricount=0;
	RwReal scale = 100.0f;
    RwV3d normal = {0.0f, 1.0f, 0.0f};
    RwInt32 matIndex;
    RwInt32 nudge = 1;

    worldImport = RtWorldImportCreate();
	if( worldImport == NULL )
    {
        return NULL;
    }

	/* 
     * Allocate the memory to store the world's vertices and triangles... 
     */
	RtWorldImportAddNumVertices(worldImport, NumVert);
    RtWorldImportAddNumTriangles(worldImport, NumTri);

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

    polygonTexture = RwTextureRead(RWSTRING ("t2"), NULL);
    RwTextureSetFilterMode(polygonTexture, rwFILTERLINEAR);



    /* 
     * Create the materials. These materials are created with a reference
     * count of 1, meaning this application has ownership. Subsequently,
     * when the materials are associated with a worldImport, their reference
     * counts are incremented, indicating ownership also by the worldImport...
     */ 
    polygonMaterial = RpMaterialCreate();
    RpMaterialSetTexture(polygonMaterial, polygonTexture);
    polygonMatIndex = RtWorldImportAddMaterial(worldImport, polygonMaterial);
    

    /*
     * Set the surface reflection coefficients...
     */
    surfProp.ambient = 0.5f;
    surfProp.diffuse = 0.8f;
    surfProp.specular = 0.0f;
    RpMaterialSetSurfaceProperties(polygonMaterial, &surfProp);

    vertices = RtWorldImportGetVertices(worldImport);
    triangles = RtWorldImportGetTriangles(worldImport);


    matIndex = polygonMatIndex;

    for (rows=0; rows<NumEdges-1; rows++)
    {
        for (cols=0; cols<NumEdges-1; cols++)
        {
            RwInt32 uOffset = (cols >= NumEdges / 2 ? nudge : 0);
            RwInt32 vOffset = (rows >= NumEdges / 2 ? nudge : 0);

            IndexList[tricount].index = (RwUInt16)(cols + rows*NumEdges);
            IndexList[tricount].uOffset = uOffset;
            IndexList[tricount].vOffset = vOffset;
            tricount++;
            IndexList[tricount].index = (RwUInt16)(1 + cols + rows*NumEdges);
            IndexList[tricount].uOffset = uOffset;
            IndexList[tricount].vOffset = vOffset;
            tricount++;
            IndexList[tricount].index = (RwUInt16)(1 + NumEdges + cols + rows*NumEdges);
            IndexList[tricount].uOffset = uOffset;
            IndexList[tricount].vOffset = vOffset;
            tricount++;
            IndexList[tricount].index = (RwUInt16)(cols + rows*NumEdges);
            IndexList[tricount].uOffset = uOffset;
            IndexList[tricount].vOffset = vOffset;
            tricount++;
            IndexList[tricount].index = (RwUInt16)(1 + NumEdges + cols + rows*NumEdges);
            IndexList[tricount].uOffset = uOffset;
            IndexList[tricount].vOffset = vOffset;
            tricount++;
            IndexList[tricount].index = (RwUInt16)(NumEdges + cols + rows*NumEdges);
            IndexList[tricount].uOffset = uOffset;
            IndexList[tricount].vOffset = vOffset;
            tricount++;
        }
    }
    for (rows=0; rows<NumEdges; rows++)
    {
        for (cols=0; cols<NumEdges; cols++)
        {
            VertexList[cols + rows*NumEdges].vertex.x = ((RwReal)cols)*scale/(NumEdges-1) - scale/2.0f;
            VertexList[cols + rows*NumEdges].vertex.z = ((RwReal)rows)*scale/(NumEdges-1) - scale/2.0f;
            VertexList[cols + rows*NumEdges].vertex.y = 0.0f;
            VertexList[cols + rows*NumEdges].uvs.u = (RwReal)cols;
            VertexList[cols + rows*NumEdges].uvs.v = (RwReal)rows;
        }
    }

    for (j = 0; j < NumVert; j += 3)
    {
        /*
         * Initialize each vertex of the face with a position, a normal
         * and texture coordinates...
         */
        vertices->OC = VertexList[IndexList[j].index].vertex;
        vertices->normal = normal;
        vertices->texCoords[0].u = VertexList[IndexList[j].index].uvs.u + IndexList[j].uOffset;
        vertices->texCoords[0].v = VertexList[IndexList[j].index].uvs.v + IndexList[j].vOffset;
        vertices->matIndex = matIndex;
        vertices++;
        vertices->OC = VertexList[IndexList[j+1].index].vertex;
        vertices->normal = normal;
        vertices->texCoords[0].u = VertexList[IndexList[j+1].index].uvs.u + IndexList[j+1].uOffset;
        vertices->texCoords[0].v = VertexList[IndexList[j+1].index].uvs.v + IndexList[j+1].vOffset;
        vertices->matIndex = matIndex;
        vertices++;
        vertices->OC = VertexList[IndexList[j+2].index].vertex;
        vertices->normal = normal;
        vertices->texCoords[0].u = VertexList[IndexList[j+2].index].uvs.u + IndexList[j+2].uOffset;
        vertices->texCoords[0].v = VertexList[IndexList[j+2].index].uvs.v + IndexList[j+2].vOffset;
        vertices->matIndex = matIndex;
        vertices++;

    
        triangles->vertIndex[0] = j;
        triangles->vertIndex[1] = j + 2;
        triangles->vertIndex[2] = j + 1;
        triangles->matIndex = matIndex;
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
    RwTextureDestroy(polygonTexture);
    RpMaterialDestroy(polygonMaterial);

	return worldImport;
}

static void
GeometryConditioningPipeline(RtGCondGeometryList *geometryList)
{
    RtGCondFixAndFilterGeometryPipeline(geometryList);
    //Do something here, say
    RtGCondDecimateAndWeldGeometryPipeline(geometryList);
    //RtGCondSpecializeGeometryPipeline(geometryList);
}

/*
 ***************************************************************************** 
 */

RpWorld *
CreateWorld(RwBool geometryConditioning, RwBool wrap, RwInt32 uvLimit)
{
    RtWorldImport *worldImport;
    RpWorld *world;
    static RtGCondParameters gcParams;
    static RtWorldImportParameters params;
    
    /* Set up the geometry conditioning parameters... */
    RtGCondParametersInit(&gcParams);
    if (wrap)
    {
        gcParams.textureMode[0] = rwTEXTUREADDRESSWRAP;
    }
    else
    {
        gcParams.textureMode[0] = rwTEXTUREADDRESSNATEXTUREADDRESS;
    }
    gcParams.uvLimitMax = (RwReal)uvLimit;
    gcParams.uvLimitMin = 0;
    gcParams.decimationPasses = 5;
    /* We do not have prelights... */
    gcParams.flags = rtGCONDNORMALS | rtGCONDTEXTURES;

    RtGCondParametersSet(&gcParams);

    worldImport = CreateWorldImport();
    if( worldImport == NULL )
    {
        return NULL;
    }

    RtWorldImportParametersInit(&params);
    RtGCondSetGeometryConditioningPipeline(GeometryConditioningPipeline);


    params.flags = rpWORLDTEXTURED | rpWORLDNORMALS | rpWORLDLIGHT;
    params.numTexCoordSets = 1;
    params.conditionGeometry = geometryConditioning;

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
