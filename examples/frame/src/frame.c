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
 * frame.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the frame hierarchy of a series of atomics 
 * in a clump.
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"

#include "main.h"
#include "frame.h"

/*
 * Stores the level in the hierarchy the frame is on...
 */
static RwInt32 LevelTable[ATOMICNUM];

RpAtomic *SelectedAtomic = NULL;

/*
 * Stores the next and previous atomics that are on the same level...
 */
NextAndPrevious NextAndPreviousAtomic[ATOMICNUM];

static void FindLevels(RwFrame *frame);



/*
 *****************************************************************************
 */
void
HighlightAtomic(void)
{
    const RwReal extra = 1.0f;

    static RwImVertexIndex index[24] = {
        0, 1,  1, 2,  2, 3,  3, 0,  4, 5,  5, 6,
        6, 7,  7, 4,  0, 4,  3, 7,  1, 5,  2, 6
    };

    RwIm3DVertex boxVertices[8];
    RwUInt8 red, green, blue, alpha;

    RpGeometry *geometry;
    RpMorphTarget *morphTargert;
    RwV3d *vertices;
    RwInt32 numVerts;
    RwBBox bBox;
    RwMatrix *ltm;

    /*
     * Get the atomic's vertices to calculate its bounding box...
     */
    geometry = RpAtomicGetGeometry(SelectedAtomic);
    morphTargert  = RpGeometryGetMorphTarget(geometry, 0);
    vertices  = RpMorphTargetGetVertices(morphTargert);
    numVerts = RpGeometryGetNumVertices(geometry);

    RwBBoxCalculate(&bBox, vertices, numVerts);
    
    /*
     * Make the bounding box bigger, so there are no 
     * problems with z-buffering...
     */
    bBox.inf.x -= extra;
    bBox.inf.y -= extra;
    bBox.inf.z -= extra;

    bBox.sup.x += extra;
    bBox.sup.y += extra;
    bBox.sup.z += extra;

    red = green = blue = 255;
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

    ltm = RwFrameGetLTM(RpAtomicGetFrame(SelectedAtomic));

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
void
RenderFrameHierarchy(void) 
{
    RwFrame *parentFrame;
    RwInt32 i;

    for(i = 0; i < ATOMICNUM; i++)
    {
        /*
         * Draw the link between each frame origin and its parent's
         * if any...
         */
        parentFrame = RwFrameGetParent(RpAtomicGetFrame(Atomics[i]));
        if( parentFrame )
        {
            RwFrame *childFrame;
            RwIm3DVertex vertex[2];
            RwUInt8 red, green, blue, alpha;
            RwV3d parentPos, childPos;

            childFrame = RpAtomicGetFrame(Atomics[i]);

            parentPos = *RwMatrixGetPos(RwFrameGetLTM(parentFrame));
            childPos = *RwMatrixGetPos(RwFrameGetLTM(childFrame));

            red = 255;
            green = 230; 
            blue = 0;
            alpha = 255;

            RwIm3DVertexSetRGBA(&vertex[0], red, green, blue, alpha);
            RwIm3DVertexSetPos(&vertex[0], parentPos.x, parentPos.y, parentPos.z);

            RwIm3DVertexSetRGBA(&vertex[1], red, green, blue, alpha);
            RwIm3DVertexSetPos(&vertex[1], childPos.x, childPos.y, childPos.z);

            if( RwIm3DTransform(vertex, 2, NULL, rwIM3D_ALLOPAQUE) )
            {
                RwIm3DRenderLine(0, 1);

                RwIm3DEnd();
            }
        }
    }

    return;
}


/*
 *****************************************************************************
 */
void 
FrameRotate(RwReal xAngle, RwReal yAngle)
{
    RwFrame *frame;
    RwV3d *right, *up;
    RwV3d pos;

    frame = RpAtomicGetFrame(SelectedAtomic);   

    right = RwMatrixGetRight(RwFrameGetMatrix(RwCameraGetFrame(Camera)));
    up = RwMatrixGetUp(RwFrameGetMatrix(RwCameraGetFrame(Camera)));

    pos = *RwMatrixGetPos(RwFrameGetMatrix(frame));

    /*
     * Translate to parent's frame origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * Rotate...
     */
    RwFrameRotate(frame, up, xAngle, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(frame, right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * Translate back to position...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
static void
PositionFrames(void)
{
    /*
     * The distance between two connected frames...
     */
    const RwReal dist = -20.0f;

    RwFrame *parentFrame, *childFrame;
    RwV3d pos;

    /* 
     * Position the children of Atomics[0]...
     */
    childFrame = RpAtomicGetFrame(Atomics[1]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    pos.x -= dist;
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    childFrame = RpAtomicGetFrame(Atomics[2]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    childFrame = RpAtomicGetFrame(Atomics[3]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    pos.x += dist;
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    /* 
     * Position the children of Atomics[1]...
     */
    childFrame = RpAtomicGetFrame(Atomics[4]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    pos.x -= dist;
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    childFrame = RpAtomicGetFrame(Atomics[5]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    /* 
     * Position the children of Atomics[3]...
     */
    childFrame = RpAtomicGetFrame(Atomics[6]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    pos.x -= dist;
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    childFrame = RpAtomicGetFrame(Atomics[7]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    childFrame = RpAtomicGetFrame(Atomics[8]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    pos.x += dist;
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    /* 
     * Position the children of Atomics[7]...
     */
    childFrame = RpAtomicGetFrame(Atomics[9]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    /* 
     * Position the children of Atomics[8]...
     */
    childFrame = RpAtomicGetFrame(Atomics[10]);
    parentFrame = RwFrameGetParent(childFrame);
    pos = *RwMatrixGetUp(RwFrameGetMatrix(parentFrame));
    RwV3dScale(&pos, &pos, dist);
    RwFrameTranslate(childFrame, &pos, rwCOMBINEREPLACE);

    return;
}


/*
 *****************************************************************************
 */
RwUInt32
GetAtomicIndex(RpAtomic *atomic)
{
    RwInt32 i;

    for(i = 0; i < ATOMICNUM; i++)
    {
        if( Atomics[i] == atomic )
        {
            return i;
        }
    }

    return 0;
}


/*
 *****************************************************************************
 */
static void
BuildTable(void)
{
    static RwInt32 index = 0;
    static RwBool done[ATOMICNUM];

    RwInt32 i, j;
    RwInt32 siblings[ATOMICNUM];
    RwInt32 level;
    RwInt32 previous, next;

    /*
     * Find a frame's siblings / cousins frames...
     */
    level = LevelTable[index];
    
    if( !done[level] )
    {
        j = 0;
        for(i = 0; i < ATOMICNUM; i++)
        {
            if( LevelTable[i] == level )
            {
                siblings[j++] = i;
            }
        }
    
        /*
         * Set previous and next frames for the frames on the same level...
         */
        for(i = 0; i < j; i++)
        {
            if( i == 0 )
            {
                previous = siblings[j - 1];
            }
            else
            {
                previous = siblings[i - 1];
            }
        
            if( i == (j - 1) )
            {
                next = siblings[0];
            }
            else
            {
                next = siblings[i + 1];
            }

            NextAndPreviousAtomic[siblings[i]].previous = Atomics[previous];
            NextAndPreviousAtomic[siblings[i]].next = Atomics[next];
        }

        done[level] = TRUE;
    }

    index++;
    if( index < ATOMICNUM )
    {
        BuildTable();
    }

    return;
}


/*
 *****************************************************************************
 */
RwObject * 
GetFirstAtomic(RwObject *object, void *data)
{ 
    if( RwObjectGetType(object) == rpATOMIC )
    {
        *(void **)data = (void *)object;

        return NULL;
    }

    return object;
}


static RwFrame *
FrameCallBack(RwFrame *frame, 
              void *data __RWUNUSED__)
{
    FindLevels(frame);

    return frame;
}


static void
FindLevels(RwFrame *frame)
{
    RwInt32 level;
    RwFrame *parent;
    RwObject *object;

    /*
     * For each frame find the level it belongs to in the hierarchy
     * e.g. the root is at level == 0, the root's children are at level == 1,
     * the root's grandchildren are at level == 2, etc.
     */
    level = 0;

    parent = RwFrameGetParent(frame);
    while( parent )
    {   
        level++;
        parent = RwFrameGetParent(parent);
    }
    
    object = NULL;
    RwFrameForAllObjects(frame, GetFirstAtomic, (void *)&object);

    if( object )
    {
        LevelTable[GetAtomicIndex((RpAtomic *)object)] = level;
    }

    RwFrameForAllChildren(frame, FrameCallBack, NULL);

    return;
}


/*
 *****************************************************************************
 */
void
LinkFrameHierarchy(void)
{
    RwFrame *parent;

    /*
     * Establish the root's immediate children...
     */
    parent = RpAtomicGetFrame(Atomics[0]);
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[1]));
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[2]));
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[3]));
   
    /*
     * Establish the immediate children of Atomics[1]...
     */
    parent = RpAtomicGetFrame(Atomics[1]);
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[4]));
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[5]));

    /*
     * Establish the immediate children of Atomics[3]...
     */
    parent = RpAtomicGetFrame(Atomics[3]);
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[6]));
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[7]));
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[8]));

    /*
     * Establish the immediate children of Atomics[7]...
     */
    parent = RpAtomicGetFrame(Atomics[7]);
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[9]));

    /*
     * Establish the immediate children of Atomics[8]...
     */
    parent = RpAtomicGetFrame(Atomics[8]);
    RwFrameAddChild(parent, RpAtomicGetFrame(Atomics[10]));

    /*
     * Establish the relative positions of the frames...
     */
    PositionFrames();

    /*
     * Set the root as the first selected atomic...
     */
    SelectedAtomic = Atomics[0];

    /*
     * Preprocess table to navigate between sibling / cousin frames.
     * Does not require prior knowlegde of hierarchy, only the root frame
     * (but still limited to number of atomics because of fixed length
     * table)...
     */
    FindLevels(RpAtomicGetFrame(Atomics[0]));
    BuildTable();

    return;
}


/*
 *****************************************************************************
 */
RwBool 
ResetFrameHierarchyCallback(RwBool testEnable)
{
    RwV3d pos;

    if( testEnable )
    {
        return TRUE;
    }

    pos.x = POS_X;
    pos.y = POS_Y;
    pos.z = POS_Z;
    RwFrameTranslate(RpClumpGetFrame(Clump), &pos, rwCOMBINEREPLACE);

    PositionFrames();

    return TRUE;
}

/*
 *****************************************************************************
 */
