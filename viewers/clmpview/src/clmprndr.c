
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
 * clmprndr.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Hierarchie, WireFrame, Normals and Bounding Boxes
 *          rendering call backs
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rprandom.h"
#include "rphanim.h"

#include "skeleton.h"

#include "main.h"

#include "clmpview.h"
#include "clmppick.h"
#include "clmphanm.h"
#include "clmprndr.h"

/*
 * Number of immediate mode vertices to transform and render at any one time...
 */
#define DISPATCHSIZE (1000)

typedef struct _TriStripData
{
    RwMatrix *matrix;
    RwV3d *vertices;
    RwBool lengths;
} TriStripData;

static RwRGBA Red = { 255, 0, 0, 255 };
static RwRGBA Cyan = { 0, 255, 255, 255 };
static RwRGBA Green = { 0, 255, 0, 255 };
static RwRGBA Blue = { 0, 0, 255, 255 };
static RwRGBA Yellow = { 255, 255, 0, 255 };

static RwImVertexIndex AtomicBBoxIndices[24] = {
    0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7,
    7, 6, 6, 4, 0, 4, 1, 5, 2, 6, 3, 7
};

static RwRGBA Color;

/*
 *****************************************************************************
 */
static void
SkeletonRender(RwMatrix *LTM, RpHAnimHierarchy *hier)
{

    if (hier)
    {
        RwMatrix *matrices;
        RpHAnimNodeInfo *finfo;
        RwMatrix *mstack[128],*parent;
        RwInt32 tos,i;
        RwIm3DVertex *im3DVertices; 
        RwIm3DVertex *imVertex;

        matrices = RpHAnimHierarchyGetMatrixArray(hier);
        finfo = hier->pNodeInfo;
        tos = 0;
        parent = mstack[0] = LTM;

        im3DVertices = (RwIm3DVertex *)RwMalloc(sizeof(RwIm3DVertex) * 128 * 8, rwID_NAOBJECT);
        imVertex = im3DVertices;
        for (i=0; i<hier->numNodes; i++)
        {
            RwV3d tmp;

            RwIm3DVertexSetPos(imVertex, parent->pos.x,parent->pos.y,parent->pos.z);
            RwIm3DVertexSetRGBA(imVertex, 255,255,0,255);
            imVertex++;

            RwIm3DVertexSetPos(imVertex, matrices[i].pos.x,matrices[i].pos.y,matrices[i].pos.z);
            RwIm3DVertexSetRGBA(imVertex, 255,255,0,255);
            imVertex++;

            /* drawn bone axes */
            RwIm3DVertexSetPos(imVertex, matrices[i].pos.x,matrices[i].pos.y,matrices[i].pos.z);
            RwIm3DVertexSetRGBA(imVertex, 255,0,0,255);
            imVertex++;
            RwV3dAdd(&tmp, &matrices[i].pos, &matrices[i].right);
            RwIm3DVertexSetPos(imVertex, tmp.x,tmp.y,tmp.z);
            RwIm3DVertexSetRGBA(imVertex, 255,0,0,255);
            imVertex++;

            RwIm3DVertexSetPos(imVertex, matrices[i].pos.x,matrices[i].pos.y,matrices[i].pos.z);
            RwIm3DVertexSetRGBA(imVertex, 0,255,0,255);
            imVertex++;
            RwV3dAdd(&tmp, &matrices[i].pos, &matrices[i].up);
            RwIm3DVertexSetPos(imVertex, tmp.x,tmp.y,tmp.z);
            RwIm3DVertexSetRGBA(imVertex, 0,255,0,255);
            imVertex++;

            RwIm3DVertexSetPos(imVertex, matrices[i].pos.x,matrices[i].pos.y,matrices[i].pos.z);
            RwIm3DVertexSetRGBA(imVertex, 0,0,255,255);
            imVertex++;
            RwV3dAdd(&tmp, &matrices[i].pos, &matrices[i].at);
            RwIm3DVertexSetPos(imVertex, tmp.x,tmp.y,tmp.z);
            RwIm3DVertexSetRGBA(imVertex, 0,0,255,255);
            imVertex++;

            if (finfo[i].flags & rpHANIMPUSHPARENTMATRIX)
            {
                mstack[++tos] = &matrices[i];
            }

            if (finfo[i].flags & rpHANIMPOPPARENTMATRIX)
            {
                parent = mstack[tos--];
            }
            else
            {
                parent = &matrices[i];
            }
        }

        if( RwIm3DTransform(im3DVertices, imVertex - im3DVertices, (RwMatrix *)NULL, 0) )
        {
            RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);
            RwIm3DEnd();
        }

        RwFree(im3DVertices);
    }
}


static RwFrame *
HierarchyRender(RwFrame *frame, 
                void * data __RWUNUSED__)
{
    RwMatrix *mat[2];
    RwIm3DVertex    imVertex[2];

    mat[0] = RwFrameGetLTM(RwFrameGetParent(frame));
    mat[1] = RwFrameGetLTM(frame);

    RwIm3DVertexSetPos(&imVertex[0],
                       (RwMatrixGetPos(mat[0]))->x,
                       (RwMatrixGetPos(mat[0]))->y,
                       (RwMatrixGetPos(mat[0]))->z);

    RwIm3DVertexSetRGBA(&imVertex[0], Color.red, Color.green, Color.blue, Color.alpha);

    RwIm3DVertexSetPos(&imVertex[1],
                       (RwMatrixGetPos (mat[1]))->x,
                       (RwMatrixGetPos (mat[1]))->y,
                       (RwMatrixGetPos (mat[1]))->z);

    RwIm3DVertexSetRGBA(&imVertex[1], Color.red, Color.green, Color.blue, Color.alpha);

    if( RwIm3DTransform(imVertex, 2, (RwMatrix *)NULL, 0) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);
        RwIm3DEnd();
    }

    SkeletonRender(RwFrameGetLTM(frame), RpHAnimGetHierarchy(frame));

    return RwFrameForAllChildren(frame, HierarchyRender, NULL);

}

/*
 *****************************************************************************
 */
RpAtomic *
AtomicRenderSkeleton(RpAtomic *atomic, 
                     void * data __RWUNUSED__)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RwFrame *frame = RwFrameGetRoot(RpAtomicGetFrame(atomic));

        if( frame != NULL)
        {
            if( AtomicSelected == atomic )
            {
                Color = Cyan;
            }
            else
            {
                Color = Red;
            }

            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);

            RwFrameForAllChildren(frame, HierarchyRender, NULL);
        }
    }

    return atomic;
}


/*
 *****************************************************************************
 */
RpAtomic *
AtomicRenderWireMesh(RpAtomic *atomic, 
                     void * data __RWUNUSED__)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RwReal interpPos = 1.0f, invInterpPos = 0.0f;
        RpMorphTarget *morphTarget;
        RwInt32 nkf;
        RpTriangle *triangle;
        RwV3d *vertPosStart = (RwV3d *)NULL;
        RwV3d *vertPosEnd = (RwV3d *)NULL;
        RwMatrix *LTM;
        RwInt32 numTri, numImmVert, i;
        RwIm3DVertex *imVertex;

        nkf = RpGeometryGetNumMorphTargets(geometry);
        numTri = RpGeometryGetNumTriangles(geometry);
        imVertex = (RwIm3DVertex *)RwMalloc(numTri * 6 * sizeof(RwIm3DVertex),
                                            rwID_NAOBJECT);

        if( nkf > 1 )
        {
            RpInterpolator *interp;
            RwInt32 startMorphTarget, endMorphTarget;

            interp = RpAtomicGetInterpolator(atomic);

            interpPos = RpInterpolatorGetValue(interp) / RpInterpolatorGetScale(interp);

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

        if( AtomicSelected == atomic )
        {
            Color = Red;
        }
        else
        {
            Color = Cyan;
        }

        triangle = RpGeometryGetTriangles(geometry);

        for( i = 0; i < numTri; i++ )
        {
            RwUInt16 vert0, vert1, vert2;
            RwV3d vertPos[3];

            RpGeometryTriangleGetVertexIndices(geometry, triangle, &vert0, &vert1, &vert2);

            if( nkf > 1 )
            {
                RwV3d tempVec1, tempVec2;

                RwV3dScale(&tempVec1, &vertPosStart[vert0], invInterpPos);
                RwV3dScale(&tempVec2, &vertPosEnd[vert0], interpPos);
                RwV3dAdd(&vertPos[0], &tempVec1, &tempVec2);

                RwV3dScale(&tempVec1, &vertPosStart[vert1], invInterpPos);
                RwV3dScale(&tempVec2, &vertPosEnd[vert1], interpPos);
                RwV3dAdd(&vertPos[1], &tempVec1, &tempVec2);

                RwV3dScale(&tempVec1, &vertPosStart[vert2], invInterpPos);
                RwV3dScale(&tempVec2, &vertPosEnd[vert2], interpPos);
                RwV3dAdd(&vertPos[2], &tempVec1, &tempVec2);

            }
            else
            {
                vertPos[0] = vertPosStart[vert0];
                vertPos[1] = vertPosStart[vert1];
                vertPos[2] = vertPosStart[vert2];
            }

            RwIm3DVertexSetPos(&imVertex[i*6+0], vertPos[0].x, vertPos[0].y, vertPos[0].z);
            RwIm3DVertexSetRGBA (&imVertex[i*6+0], Color.red, Color.green, Color.blue, Color.alpha);
            RwIm3DVertexSetPos(&imVertex[i*6+1], vertPos[1].x, vertPos[1].y, vertPos[1].z);
            RwIm3DVertexSetRGBA(&imVertex[i*6+1], Color.red, Color.green, Color.blue, Color.alpha);


            RwIm3DVertexSetPos(&imVertex[i*6+2], vertPos[1].x, vertPos[1].y, vertPos[1].z);
            RwIm3DVertexSetRGBA(&imVertex[i*6+2], Color.red, Color.green, Color.blue, Color.alpha);
            RwIm3DVertexSetPos(&imVertex[i*6+3], vertPos[2].x, vertPos[2].y, vertPos[2].z);
            RwIm3DVertexSetRGBA(&imVertex[i*6+3], Color.red, Color.green, Color.blue, Color.alpha);

            RwIm3DVertexSetPos(&imVertex[i*6+4], vertPos[2].x, vertPos[2].y, vertPos[2].z);
            RwIm3DVertexSetRGBA(&imVertex[i*6+4], Color.red, Color.green, Color.blue, Color.alpha);
            RwIm3DVertexSetPos(&imVertex[i*6+5], vertPos[0].x, vertPos[0].y, vertPos[0].z);
            RwIm3DVertexSetRGBA(&imVertex[i*6+5], Color.red, Color.green, Color.blue, Color.alpha);

            triangle++;
        }

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

        i = 0;
        numImmVert = numTri * 6;
        LTM = RwFrameGetLTM(RpAtomicGetFrame(atomic));
        while (numImmVert > DISPATCHSIZE)
        {
            if( RwIm3DTransform(&imVertex[i], DISPATCHSIZE, LTM, 0) )
            {
                RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

                RwIm3DEnd();
            }

            numImmVert -= DISPATCHSIZE;
            i += DISPATCHSIZE;
        }

        if( RwIm3DTransform(&imVertex[i], numImmVert, LTM, 0) )
        {
            RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

            RwIm3DEnd();
        }

        RwFree(imVertex);
    }

    return atomic;
}


/*
 *****************************************************************************
 */
RpAtomic *
AtomicRenderVertexNormals(RpAtomic *atomic, 
                          void * data __RWUNUSED__)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry && (RpGeometryGetFlags(geometry) & rpGEOMETRYNORMALS) )
    {
        RpMorphTarget *morphTarget;
        RwInt32 numVert, nkf, numImmVert;
        RwV3d *vertPosStart = (RwV3d *)NULL;
        RwV3d *vertPosEnd = (RwV3d *)NULL;
        RwV3d *vertNormalStart = (RwV3d *)NULL;
        RwV3d *vertNormalEnd = (RwV3d *)NULL;
        RwReal interpPos = 1.0f, invInterpPos = 0.0f;
        RwMatrix *LTM;
        RwInt32 i;
        RwIm3DVertex *imVertex;

        nkf = RpGeometryGetNumMorphTargets(geometry);
        numVert = RpGeometryGetNumVertices(geometry);

        imVertex = (RwIm3DVertex *)RwMalloc(numVert * 2 * sizeof(RwIm3DVertex),
                                            rwID_NAOBJECT);

        if( nkf > 1 )
        {
            RpInterpolator *interp;
            RwInt32 startMorphTarget, endMorphTarget;

            interp = RpAtomicGetInterpolator(atomic);
            interpPos = RpInterpolatorGetValue (interp) / RpInterpolatorGetScale (interp);

            invInterpPos = 1.0f - interpPos;

            startMorphTarget = RpInterpolatorGetStartMorphTarget(interp);
            endMorphTarget = RpInterpolatorGetEndMorphTarget(interp);

            morphTarget = RpGeometryGetMorphTarget(geometry, startMorphTarget);
            vertPosStart = RpMorphTargetGetVertices(morphTarget);
            vertNormalStart = RpMorphTargetGetVertexNormals(morphTarget);

            morphTarget = RpGeometryGetMorphTarget(geometry, endMorphTarget);
            vertPosEnd = RpMorphTargetGetVertices(morphTarget);
            vertNormalEnd = RpMorphTargetGetVertexNormals(morphTarget);
        }
        else
        {
            morphTarget = RpGeometryGetMorphTarget(geometry, 0);
            vertPosStart = RpMorphTargetGetVertices(morphTarget);
            vertNormalStart = RpMorphTargetGetVertexNormals(morphTarget);
        }

        if( AtomicSelected == atomic )
        {
            Color = Blue;
        }
        else
        {
            Color = Green;
        }

        for( i = 0; i < numVert; i++ )
        {
            RwV3d vertPos, vertNormal;

            if( nkf > 1 )
            {
                RwV3d tempVec1, tempVec2;

                RwV3dScale(&tempVec1, &vertPosStart[i], invInterpPos);
                RwV3dScale(&tempVec2, &vertPosEnd[i], interpPos);
                RwV3dAdd(&vertPos, &tempVec1, &tempVec2);

                RwV3dScale(&tempVec1, &vertNormalStart[i], invInterpPos);
                RwV3dScale(&tempVec2, &vertNormalEnd[i], interpPos);
                RwV3dAdd(&vertNormal, &tempVec1, &tempVec2);
            }
            else
            {
                vertPos = vertPosStart[i];
                vertNormal = vertNormalStart[i];
            }

            RwV3dScale(&vertNormal, &vertNormal, NormalsScaleFactor);

            RwIm3DVertexSetPos(&imVertex[i*2+0], vertPos.x, vertPos.y, vertPos.z);
            RwIm3DVertexSetRGBA(&imVertex[i*2+0], Color.red, Color.green, Color.blue, Color.alpha);

            RwV3dAdd(&vertPos, &vertPos, &vertNormal);
            RwIm3DVertexSetPos(&imVertex[i*2+1], vertPos.x, vertPos.y, vertPos.z);
            RwIm3DVertexSetRGBA (&imVertex[i*2+1], Color.red, Color.green, Color.blue, Color.alpha);
        }

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

        i = 0;
        numImmVert = numVert * 2;
        LTM = RwFrameGetLTM(RpAtomicGetFrame(atomic));
        while (numImmVert > DISPATCHSIZE)
        {
            if( RwIm3DTransform(&imVertex[i], DISPATCHSIZE, LTM, 0) )
            {
                RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

                RwIm3DEnd();
            }

            numImmVert -= DISPATCHSIZE;
            i += DISPATCHSIZE;
        }

        if( RwIm3DTransform(&imVertex[i], numImmVert, LTM, 0) )
        {
            RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

            RwIm3DEnd();
        }

        RwFree(imVertex);
    }

    return atomic;
}

/*
 *****************************************************************************
 */
static RpMesh *
MeshRenderTriStrip(RpMesh * mesh, 
                   RpMeshHeader * meshHeader __RWUNUSED__, 
                   void * data)
{
    RwMatrix        *matrix       = ((TriStripData *)data)->matrix;
    RwV3d           *vertices     = ((TriStripData *)data)->vertices;
    RwBool           lengths      = ((TriStripData *)data)->lengths;
    RwImVertexIndex *currentIndex = mesh->indices;
    RwUInt32         vertCounter  = 0;
    
    RwIm3DVertex    *TriStrip;
    RwUInt32         i, j;

    TriStrip = (RwIm3DVertex *)RwMalloc(mesh->numIndices * sizeof(RwIm3DVertex),
                                        rwID_NAOBJECT);
    
    Color.red   = 0;
    Color.green = 0;
    Color.blue  = 0;
    Color.alpha = 255;

    if(!lengths)
    {
        RpRandomSeed(mesh->numIndices);
    }

    /* Process the index array, test for duplicate vertices */
    for(i = 0; i < mesh->numIndices; i++)
    {
        RwIm3DVertexSetPos( &TriStrip[i], 
                            vertices[*currentIndex].x,
                            vertices[*currentIndex].y,
                            vertices[*currentIndex].z );

        /* Check indices for join-strip degenerates */
        if((i > 0) && ((*currentIndex) == (*(currentIndex-1))))
        {
            /* This is degenerate - start a new strip if we've got a triangle */
            if(vertCounter >= 3)
            {
                if(lengths)
                {
                    if((vertCounter-3)<(RwUInt32)NumTriStripAngles)
                    {
                        Color.red = 
                            (RwUInt8)(255 - (255*(vertCounter-3))/NumTriStripAngles);
                    }
                    else
                    {
                        Color.red = 0;
                    }
                }
                else
                {
                    Color.red   = (RwUInt8)(RpRandom() % 200) + 50;
                    Color.green = (RwUInt8)(RpRandom() % 200) + 50;
                    Color.blue  = (RwUInt8)(RpRandom() % 200) + 50;
                }

                /* Colour my verts */
                for(j = 0; j < vertCounter; j++)
                {
                    RwIm3DVertexSetRGBA( &TriStrip[i - (j+1)],
                                         Color.red,
                                         Color.green,
                                         Color.blue,
                                         Color.alpha );
                }

                /* Zero counter */
                vertCounter = 0;
            }
        }
        else
        {
            /* count vertices. Need 3 to make a triangle. */
            vertCounter++;
        }

        /* Point to the next source vertex */
        currentIndex++;
    }

    /* final geometry */
    currentIndex--;
    if(vertCounter >= 3)
    {
        if(lengths)
        {
            if((vertCounter-3)<(RwUInt32)NumTriStripAngles)
            {
                Color.red = 
                    (RwUInt8)(255 - (255*(vertCounter-3))/NumTriStripAngles);
            }
            else
            {
                Color.red = 0;
            }
        }
        else
        {
            Color.red   = (RwUInt8)(RpRandom() % 200) + 50;
            Color.green = (RwUInt8)(RpRandom() % 200) + 50;
            Color.blue  = (RwUInt8)(RpRandom() % 200) + 50;
        }

        /* Colour my verts */
        for(j = 0; j < vertCounter; j++)
        {
            RwIm3DVertexSetRGBA( &TriStrip[i - (j+1)],
                                 Color.red,
                                 Color.green,
                                 Color.blue,
                                 Color.alpha );
        }
    }

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);

    if( RwIm3DTransform(TriStrip, mesh->numIndices, matrix, 0) )
    {                         
        RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP); 
        RwIm3DEnd();
    }   

    RwFree(TriStrip);

    return mesh;
}


/*
 *****************************************************************************
 */
static RpMesh *
MeshRenderMeshes(RpMesh * mesh, 
                 RpMeshHeader * meshHeader __RWUNUSED__, 
                 void * data)
{
    RwMatrix        *matrix       = ((TriStripData *)data)->matrix;
    RwV3d           *vertices     = ((TriStripData *)data)->vertices;
    RwImVertexIndex *currentIndex = mesh->indices;
    
    RwIm3DVertex    *TriStrip;
    RwUInt32         i;

    TriStrip = (RwIm3DVertex *)RwMalloc(mesh->numIndices * sizeof(RwIm3DVertex),
                                        rwID_NAOBJECT);
    

    RpRandomSeed(mesh->numIndices);
    Color.red   = (RwUInt8)(RpRandom() % 200) + 50;
    Color.green = (RwUInt8)(RpRandom() % 200) + 50;
    Color.blue  = (RwUInt8)(RpRandom() % 200) + 50;
    Color.alpha = 255;

    /* Process the index array, test for duplicate vertices */
    for(i = 0; i < mesh->numIndices; i++, currentIndex++)
    {
        RwIm3DVertexSetPos( &TriStrip[i], 
                            vertices[*currentIndex].x,
                            vertices[*currentIndex].y,
                            vertices[*currentIndex].z );

        RwIm3DVertexSetRGBA( &TriStrip[i],
                             Color.red,
                             Color.green,
                             Color.blue,
                             Color.alpha );
    }

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);

    if( RwIm3DTransform(TriStrip, mesh->numIndices, matrix, 0) )
    {                         
        RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP); 
        RwIm3DEnd();
    }   

    RwFree(TriStrip);

    return mesh;
}

/*
 *****************************************************************************
 */
RpAtomic *
AtomicRenderTriStrip(RpAtomic * atomic, void * data)
{
    RpGeometry *geometry = RpAtomicGetGeometry(atomic);
    TriStripData info;

    info.matrix = RwFrameGetLTM(RpAtomicGetFrame(atomic));
    info.vertices = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry, 0));
    info.lengths = (RwBool)data;

    /* Is this tristripped? */
    if(RpGeometryGetFlags(geometry) & rpGEOMETRYTRISTRIP)
    {
        /* Enumerate all meshes */
        RpGeometryForAllMeshes(geometry, MeshRenderTriStrip, &info);
    }
    else
    {
        /* Just render it. */
        RpAtomicRender(atomic);
    }

    return atomic;
}


/*
 *****************************************************************************
 */
RpAtomic *
AtomicRenderMeshes(RpAtomic * atomic, 
                   void * data __RWUNUSED__)
{
    RpGeometry *geometry = RpAtomicGetGeometry(atomic);
    TriStripData info;

    info.matrix = RwFrameGetLTM(RpAtomicGetFrame(atomic));
    info.vertices = RpMorphTargetGetVertices(RpGeometryGetMorphTarget(geometry, 0));
    info.lengths = FALSE;

    /* Is this tristripped? */
    if(RpGeometryGetFlags(geometry) & rpGEOMETRYTRISTRIP)
    {
        /* Enumerate all meshes */
        RpGeometryForAllMeshes(geometry, MeshRenderMeshes, &info);
    }
    else
    {
        /* Just render it. */
        RpAtomicRender(atomic);
    }

    return atomic;
}


/*
 *****************************************************************************
 */
void
AtomicRenderBoundingBox(RpAtomic * atomic, RwBBox * bbox)
{
    RwInt32 i;
    RwMatrix *LTM;
    RwIm3DVertex imVertex[8];

    Color = Yellow;

    for( i = 0; i < 8; i++ )
    {
        RwIm3DVertexSetPos(&imVertex[i],
            i & 1 ? bbox->sup.x : bbox->inf.x,
            i & 2 ? bbox->sup.y : bbox->inf.y,
            i & 4 ? bbox->sup.z : bbox->inf.z);

        RwIm3DVertexSetRGBA(&imVertex[i],
            Color.red, Color.green, Color.blue, Color.alpha);
    }

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

    LTM = RwFrameGetLTM(RpAtomicGetFrame(atomic));
    if( RwIm3DTransform(imVertex, 8, LTM, 0) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, AtomicBBoxIndices, 24);

        RwIm3DEnd();
    }

    return;

}


/*
 *****************************************************************************
 */

