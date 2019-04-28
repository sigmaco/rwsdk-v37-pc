
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
 * lodatom.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the use of the level-of-detail plugin.
 *
*****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rplodatm.h"
#include "skeleton.h"

#include "main.h"
#include "lodatom.h"

/*
 * The number of LOD files to read in...
 */
#define LODNUM (7)

static RpAtomicCallBackRender DefaultAtomicRenderCallback;

/*
 * For each LOD index, the corresponding range is the minimum distance
 * from the camera that that LOD geometry can be rendered at.
 * The values given here have been estimated to give good LOD management
 * only for the camera (with default view-window and frame-buffer 
 * resolution) and clump used in this example...
 */
static RwReal LODRanges[LODNUM] = 
{
    2.0f, 5.0f, 10.0f, 30.0f, 50.0f, 90.0f, 110.0f
};

RpAtomic *LODAtomic = NULL;

/*
 * Stores vertices for drawing the LOD as a wire-frame...
 */
RwIm3DVertex *Im3DVertexBuffer = NULL;

/*
 * Number of immediate mode vertices to transform and render 
 * at any one time...
 */
#define DISPATCHSIZE (1000)

RwBool WireFrameOn = FALSE;

/*
 * The position of the camera in the local-space of the clump,
 * used for back-face culling in wire-frame rendering...
 */
RwV3d LocalSpaceCameraPosition;



/*
 *****************************************************************************
 */
static RwIm3DVertex *
AllocateIm3DVertexBuffer(RpAtomic *atomic)
{
    RwIm3DVertex *buffer = NULL;
    RwInt32 numTri;

    numTri = RpGeometryGetNumTriangles(RpAtomicGetGeometry(atomic));
    
    /*
     * A triangle's wire-frame is made up of 3 lines, so this requires 6 
     * vertices per triangle...
     */
    numTri *= 6;

    buffer = (RwIm3DVertex *)RwMalloc(numTri * sizeof(RwIm3DVertex),
                                      rwID_NAOBJECT);
    
    return buffer;
}


/*
 *****************************************************************************
 */
void
FreeIm3DVertexBuffer(void)
{
    if( Im3DVertexBuffer )
    {
        RwFree(Im3DVertexBuffer);
    }

    return;
}


/*
 *****************************************************************************
 */
static RwBool 
BackfaceCulledTriangle(RpAtomic *atomic __RWUNUSED__,
                       RwV3d *vert0, 
                       RwV3d *vert1, 
                       RwV3d *vert2)
{
    RwV3d a, b, n, v;

    RwV3dSub(&a, vert1, vert0);
    RwV3dSub(&b, vert2, vert0);

    RwV3dCrossProduct(&n, &a, &b);

    RwV3dSub(&v, vert0, &LocalSpaceCameraPosition);

    if( RwV3dDotProduct(&n, &v) >= 0.0f )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*
 *****************************************************************************
 */
static void 
RenderWireFrame(RpAtomic *atomic)
{
    RpGeometry *geometry;
    RpMorphTarget *morphTarget;
    RpTriangle *triangle;
    RwV3d *vertices;
    RwMatrix *LTM;
    RwIm3DVertex *imVertex;
    RwInt32 numTri, numImmVert, i, lodIndex;
    RwRGBA color = {0, 200, 200, 255};
    
    lodIndex = RpLODAtomicGetCurrentLOD(atomic);

    geometry = RpLODAtomicGetGeometry(atomic, lodIndex);
    morphTarget = RpGeometryGetMorphTarget(geometry, 0);
    vertices = RpMorphTargetGetVertices(morphTarget);

    imVertex = Im3DVertexBuffer;

    numTri = RpGeometryGetNumTriangles(geometry);
    triangle = RpGeometryGetTriangles(geometry);

    numImmVert = 0;

    for(i=0; i<numTri; i++)
    {
        RwUInt16 vert0, vert1, vert2;
        RwV3d vertPos[3];

        RpGeometryTriangleGetVertexIndices(geometry, triangle, 
            &vert0, &vert1, &vert2);

        vertPos[0] = vertices[vert0];
        vertPos[1] = vertices[vert1];
        vertPos[2] = vertices[vert2];

        if( !BackfaceCulledTriangle(atomic, &vertPos[0], &vertPos[1], &vertPos[2]) )
        {
            RwIm3DVertexSetPos(imVertex, vertPos[0].x, vertPos[0].y, vertPos[0].z);
            RwIm3DVertexSetRGBA(imVertex, 
                color.red, color.green, color.blue, color.alpha);
            imVertex++;

            RwIm3DVertexSetPos(imVertex, vertPos[1].x, vertPos[1].y, vertPos[1].z);
            RwIm3DVertexSetRGBA(imVertex, 
                color.red, color.green, color.blue, color.alpha);
            imVertex++;

            RwIm3DVertexSetPos(imVertex, vertPos[1].x, vertPos[1].y, vertPos[1].z);
            RwIm3DVertexSetRGBA(imVertex, 
                color.red, color.green, color.blue, color.alpha);
            imVertex++;

            RwIm3DVertexSetPos(imVertex, vertPos[2].x, vertPos[2].y, vertPos[2].z);
            RwIm3DVertexSetRGBA(imVertex, 
                color.red, color.green, color.blue, color.alpha);
            imVertex++;

            RwIm3DVertexSetPos(imVertex, vertPos[2].x, vertPos[2].y, vertPos[2].z);
            RwIm3DVertexSetRGBA(imVertex, 
                color.red, color.green, color.blue, color.alpha);
            imVertex++;

            RwIm3DVertexSetPos(imVertex, vertPos[0].x, vertPos[0].y, vertPos[0].z);
            RwIm3DVertexSetRGBA(imVertex, 
                color.red, color.green, color.blue, color.alpha);
            imVertex++;

            numImmVert += 6;
        }

        triangle++;
    }

    LTM = RwFrameGetLTM(RpAtomicGetFrame(atomic));

    for(i=0; i<numImmVert; i+=DISPATCHSIZE)
    {
        RwInt32 size;
        
        size = ((numImmVert - i) > DISPATCHSIZE) ? DISPATCHSIZE : (numImmVert - i);

        if( RwIm3DTransform(&Im3DVertexBuffer[i], size, LTM, rwIM3D_ALLOPAQUE) )
        {
            RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

            RwIm3DEnd();
        }
    }

    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicRenderCallBack(RpAtomic *atomic)
{
    if( WireFrameOn )
    {
        /*
         * Render the atomic in wire-frame...
         */

        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);

        RenderWireFrame(atomic);

        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);
    }
    else
    {
        /*
         * Render the atomic using its default render callback...
         */
        DefaultAtomicRenderCallback(atomic);
    }

    return atomic;
}


/*
 *****************************************************************************
 */
static RpClump * 
LoadDFF(RwChar *file)
{
    RwStream *stream;
    RwChar *path;
    RpClump *clump;

    clump = NULL;

    path = RsPathnameCreate(file);
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) ) 
        {
            clump = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    return clump;
}


/*
 *****************************************************************************
 */
static RpAtomic *
GetAtomic(RpAtomic *atomic, void *data)
{
    *(RpAtomic **)data = atomic;
    
    return NULL;
}


/*
 *****************************************************************************
 */
static RwInt32
LODAtomicCallback(RpAtomic *atomic)
{
    RwV3d atomicPos, cameraPos, cameraAt;
    RwV3d temp;
    RwReal distance;
    RwInt32 lodIndex;

    /*
     * Calculate the perpendicular distance of the atomic
     * from the camera...
     */
    atomicPos = RwFrameGetLTM(RpAtomicGetFrame(atomic))->pos;
    cameraPos = RwFrameGetLTM(RwCameraGetFrame(Camera))->pos;
    cameraAt = RwFrameGetLTM(RwCameraGetFrame(Camera))->at;

    RwV3dSub(&temp, &atomicPos, &cameraPos);

    distance = RwV3dDotProduct(&temp, &cameraAt);

    /*
     * Determine the LOD index given the minimum distance each one
     * has been specified to have...
     */
    for(lodIndex=0; lodIndex<LODNUM; lodIndex++)
    {
        if( distance < LODRanges[lodIndex] )
        {
            return lodIndex;
        }
    }

    /*
     * The atomic must be further than the last range, so
     * use the last defined LOD...
     */
    return LODNUM-1;
}


/*
 *****************************************************************************
 */
RpClump *
CreateLODClump(RpWorld *world)
{
    RpClump *clump[LODNUM];
    RpAtomic *atomic;
    RwInt32 i;
    RwV3d pos;

    /*
     * Load the LODs from clump files...
     */
    for(i=0; i<LODNUM; i++)
    {
        RwChar dffName[256];

        RsSprintf(dffName, RWSTRING("models/lod%d.dff"), i);

        clump[i] = LoadDFF(dffName);

        if( clump[i] == NULL )
        {
            RwInt32 j;

            for(j=0; j<i; j++)
            {
                RpClumpDestroy(clump[j]);
            }

            return NULL;
        }
    }

    /*
     * The atomic's render callback needs to be set before specifying the 
     * LOD geometries (we set our own render callback to choose between 
     * normal and wire-frame rendering of the atomic). We will be using 
     * clump[0] and it's atomic for the base-level geometry...
     */
    RpClumpForAllAtomics(clump[0], GetAtomic, (void *)&LODAtomic);
    DefaultAtomicRenderCallback = RpAtomicGetRenderCallBack(LODAtomic);
    RpAtomicSetRenderCallBack(LODAtomic, AtomicRenderCallBack);

    /*
     * Specify the LOD geometry for the base-level...
     */
    RpLODAtomicSetGeometry(LODAtomic, 0, RpAtomicGetGeometry(LODAtomic));

    /*
     * Specify the LOD geometry for the other levels.
     * For completeness, set any remaining levels with NULL geometry...
     */
    for(i=1; i<LODNUM; i++)
    {
        RpClumpForAllAtomics(clump[i], GetAtomic, (void *)&atomic);

        RpLODAtomicSetGeometry(LODAtomic, i, RpAtomicGetGeometry(atomic));
    }

    for(i=LODNUM; i<RPLODATOMICMAXLOD; i++)
    {
        RpLODAtomicSetGeometry(LODAtomic, i, NULL);
    }

    /*
     * The LOD callback is executed every time the atomic is rendered.
     * It will only choose a LOD index in the range 0 --> LODNUM-1...
     */
    RpLODAtomicSetLODCallBack(LODAtomic, LODAtomicCallback);

    /*
     * Enable LOD rendering... 
     */
    RpLODAtomicHookRender(LODAtomic);
     
    /*
     * Destroy the created clumps, except the first one...
     */
    for(i=1; i<LODNUM; i++)
    {
        RpClumpDestroy(clump[i]);
    }

    /*
     * Allocate the memory to store IM3D vertices for rendering wire-frame,
     * Use the base LOD geometry as this has the most...
     */
    Im3DVertexBuffer = AllocateIm3DVertexBuffer(LODAtomic);

    if( Im3DVertexBuffer )
    {
        /*
         * Place the LOD clump in front of the camera (assumed to be 
         * at the center of the world)...
         */
        pos.x = pos.y = 0.0f;
        pos.z = ClumpDistance;
        RwFrameTranslate(RpClumpGetFrame(clump[0]), &pos, rwCOMBINEREPLACE);

        RpWorldAddClump(world, clump[0]);

        return clump[0];
    }
    else
    {
        RpClumpDestroy(clump[0]);

        return NULL;
    }
}

/*
 *****************************************************************************
 */
