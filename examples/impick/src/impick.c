
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
 * impick.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrate the picking and dragging of 3D immediate vertices.
 *
*****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"

#include "impick.h"

#define GRIDSIZE (7)

static RwRGBA PickedColor = {255, 0, 0, 255};
static RwRGBA NonPickedColor = {255, 255, 0, 255};

static RwInt32 NumVertices = 0;
static RwInt32 NumIndices = 0;

static RwIm3DVertex *Im3DVertices = NULL;
static RwImVertexIndex *Im3DLineIndices = NULL;

static RwMatrix *Im3DMatrix = NULL;

static RwIm3DVertex *PickedIm3DVertex = NULL;
static RwReal PickedCameraVertexZ;



/*
 *****************************************************************************
 */
static void
Im3DClearData(void)
{
    NumVertices = NumIndices = 0;

    if( Im3DVertices )
    {
        RwFree(Im3DVertices);

        Im3DVertices = NULL;
    }

    if( Im3DLineIndices )
    {
        RwFree(Im3DLineIndices);

        Im3DLineIndices = NULL;
    }

    PickedIm3DVertex = NULL;
    PickedCameraVertexZ = 0;

    return;
}


/*
 *****************************************************************************
 */
RwBool 
Im3DCreateCube(void)
{
    RwIm3DVertex *vert = NULL;
    RwImVertexIndex *indices = NULL;
    RwReal cubeSize = 5.0f;
    int i;

    Im3DClearData();

    NumVertices = 8;
    NumIndices = 24;

    Im3DVertices = 
        (RwIm3DVertex *)RwMalloc(NumVertices * sizeof(RwIm3DVertex),
                                 rwID_NAOBJECT);
    
    if( Im3DVertices == NULL )
    {
        return FALSE;
    }

    Im3DLineIndices = 
        (RwImVertexIndex *)RwMalloc(NumIndices * sizeof(RwImVertexIndex),
                                    rwID_NAOBJECT);

    if( Im3DLineIndices == NULL )
    {
        Im3DClearData();

        return FALSE;
    }
    
    vert = Im3DVertices;
    indices = Im3DLineIndices;

    for(i=0; i<8; i++)
    {
        RwIm3DVertexSetPos(vert,
            i & 1 ? cubeSize : -cubeSize,
            i & 2 ? cubeSize : -cubeSize,
            i & 4 ? cubeSize : -cubeSize
        );

        RwIm3DVertexSetRGBA(vert,
            NonPickedColor.red, NonPickedColor.green, 
            NonPickedColor.blue, NonPickedColor.alpha);

        vert++;
    }

    for(i=0; i<8; i+=4)
    {
        *indices++ = (RwImVertexIndex)i;

        *indices++ = (RwImVertexIndex)(i+1);

        *indices++ = (RwImVertexIndex)(i+1);

        *indices++ = (RwImVertexIndex)(i+3);

        *indices++ = (RwImVertexIndex)(i+3);

        *indices++ = (RwImVertexIndex)(i+2);

        *indices++ = (RwImVertexIndex)(i+2);

        *indices++ = (RwImVertexIndex)i;
    }

    for(i=0; i<4; i++)
    {
        *indices++ = (RwImVertexIndex)i;

        *indices++ = (RwImVertexIndex)(i+4);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
RwBool
Im3DCreateGrid(void)
{
    RwIm3DVertex *vert = NULL;
    RwImVertexIndex *indices = NULL;
    RwReal gridInterval = 2.5f;
    int i, j;

    Im3DClearData();

    NumVertices = GRIDSIZE * GRIDSIZE;
    NumIndices = (GRIDSIZE-1) * GRIDSIZE * 4;

    Im3DVertices = 
        (RwIm3DVertex *)RwMalloc(NumVertices * sizeof(RwIm3DVertex),
                                 rwID_NAOBJECT);
    
    if( Im3DVertices == NULL )
    {
        return FALSE;
    }

    Im3DLineIndices = 
        (RwImVertexIndex *)RwMalloc(NumIndices * sizeof(RwImVertexIndex),
                                    rwID_NAOBJECT);
    
    if( Im3DLineIndices == NULL )
    {
        Im3DClearData();

        return FALSE;
    }
    
    vert = Im3DVertices;
    indices = Im3DLineIndices;

    for(i=0; i<GRIDSIZE; i++)
    {
        RwReal x, y;

        for(j=0; j<GRIDSIZE-1; j++)
        {
            x = (RwReal)(i - (GRIDSIZE>>1)) * gridInterval;
            y = (RwReal)(j - (GRIDSIZE>>1)) * gridInterval;

            RwIm3DVertexSetPos(vert, x, y, 0);

            RwIm3DVertexSetRGBA(vert,
                NonPickedColor.red, NonPickedColor.green, 
                NonPickedColor.blue, NonPickedColor.alpha);

            vert++;

            *indices++ = (RwImVertexIndex)(GRIDSIZE*i+j);

            *indices++ = (RwImVertexIndex)(GRIDSIZE*i+j+1);

            *indices++ = (RwImVertexIndex)(i+GRIDSIZE*j);

            *indices++ = (RwImVertexIndex)(i+GRIDSIZE*(j+1));
        }

        j = GRIDSIZE-1;

        x = (RwReal)(i - (GRIDSIZE>>1)) * gridInterval;
        y = (RwReal)(j - (GRIDSIZE>>1)) * gridInterval;

        RwIm3DVertexSetPos(vert, x, y, 0);

        RwIm3DVertexSetRGBA(vert,
            NonPickedColor.red, NonPickedColor.green, 
            NonPickedColor.blue, NonPickedColor.alpha);

        vert++;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
void 
Im3DResetPickedVertexColor(void)
{
    if (PickedIm3DVertex)
    {
        RwIm3DVertexSetRGBA(PickedIm3DVertex, 
            NonPickedColor.red, NonPickedColor.green, 
            NonPickedColor.blue, NonPickedColor.alpha);
    }

    return;
}


/*
 *****************************************************************************
 */
RwBool 
Im3DPickVertices(RwV2d *screenPos)
{
    RwMatrix *matrix;
    RwIm3DVertex *sourceVert;
    RwInt32 i, width, height;
    RwInt16 xOffset, yOffset;
    RwRaster *camRas;
    RwReal nearClip, farClip;
    RwV2d testWindowTL, testWindowBR;

    camRas = RwCameraGetRaster(Camera);
    width = RwRasterGetWidth(camRas);
    height = RwRasterGetHeight(camRas);
    nearClip = RwCameraGetNearClipPlane(Camera);
    farClip = RwCameraGetFarClipPlane(Camera);
    RwRasterGetOffset(camRas, &xOffset, &yOffset);

    /* 
     * Window to test against...
     */
    testWindowTL.x = screenPos->x - (RwReal)xOffset - 4.0f;
    testWindowTL.y = screenPos->y - (RwReal)yOffset - 4.0f;

    testWindowBR.x = screenPos->x - (RwReal)xOffset + 4.0f;
    testWindowBR.y = screenPos->y - (RwReal)yOffset + 4.0f;

    /* 
     * Build matrix so we can project...
     */
    matrix = RwMatrixCreate();
    RwMatrixMultiply(matrix, Im3DMatrix, RwCameraGetViewMatrix(Camera));

    sourceVert = Im3DVertices;

    /* 
     * Loop through all vertices finding the one that we picked...
     */
    for(i=0; i<NumVertices; i++)
    {
        RwV3d pos, *point;
        RwV2d projPos;

        /* 
         * Transform to camera clip-space and test against z-clip planes...
         */
        point = RwIm3DVertexGetPos(sourceVert);
        RwV3dTransformPoint(&pos, point, matrix);
        
        if( (pos.z > nearClip) && (pos.z < farClip) )
        {
            /* 
             * We can project this into screen space and test it...
             */
            projPos.x = pos.x / pos.z * width;
            projPos.y = pos.y / pos.z * height;

            /* 
             * Compare it against the test window...
             */
            if( (projPos.x > testWindowTL.x) && (projPos.y > testWindowTL.y) &&
                (projPos.x < testWindowBR.x) && (projPos.y < testWindowBR.y) )
            {
                /*
                 * Save some data about the picked vertex...
                 */
                PickedCameraVertexZ = pos.z;
                PickedIm3DVertex = &Im3DVertices[i];

                /*
                 * Highlight the picked vertex...
                 */
                RwIm3DVertexSetRGBA(PickedIm3DVertex, 
                    PickedColor.red, PickedColor.green, 
                    PickedColor.blue, PickedColor.alpha);

                RwMatrixDestroy(matrix);

                return TRUE;
            }
        }

        sourceVert++;
    }
    
    RwMatrixDestroy(matrix);
    
    return FALSE;
}


/*
 *****************************************************************************
 */
void 
Im3DSetVertexXY(RwV2d *screenPos)
{
    RwRaster *camRas;
    RwMatrix *tempMat1, *tempMat2;
    RwV3d pos;
    RwInt32 width, height;
    RwInt16 xOffset, yOffset;

    if( PickedIm3DVertex != NULL)
    {
        camRas = RwCameraGetRaster(Camera);
        width = RwRasterGetWidth(camRas);
        height = RwRasterGetHeight(camRas);
        RwRasterGetOffset(camRas, &xOffset, &yOffset);

        tempMat1 = RwMatrixCreate();
        tempMat2 = RwMatrixCreate();

        /*
         * Convert the screen position to a camera clip-space position. 
         * Dragging takes place in a plane through the vertex, 
         * parallel to the view plane...
         */
        pos.x = (screenPos->x - (RwReal)xOffset) * PickedCameraVertexZ / width;
        pos.y = (screenPos->y - (RwReal)yOffset) * PickedCameraVertexZ / height;
        pos.z = PickedCameraVertexZ;

        /*
         * Transform the camera clip-space position to object space...
         */
        RwMatrixMultiply(tempMat1, Im3DMatrix, RwCameraGetViewMatrix(Camera));
        RwMatrixInvert(tempMat2, tempMat1);
        RwV3dTransformPoint(&pos, &pos, tempMat2);

        /*
         * Update the IM geometry...
         */
        RwIm3DVertexSetPos(PickedIm3DVertex, pos.x, pos.y, pos.z);

        RwMatrixDestroy(tempMat2);
        RwMatrixDestroy(tempMat1);
    }

    return;
}


/*
 *****************************************************************************
 */
void
Im3DRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix *cameraMatrix;
    RwV3d right, up, pos;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    pos = *RwMatrixGetPos(Im3DMatrix);

    /*
     * First translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(Im3DMatrix, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * ...do the rotations...
     */
    RwMatrixRotate(Im3DMatrix, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwMatrixRotate(Im3DMatrix, &right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * ...and translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(Im3DMatrix, &pos, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 ***************************************************************************
 */
void
Im3DTranslateZ(RwReal zDelta)
{
    RwFrame *cameraFrame;
    RwV3d delta;

    cameraFrame = RwCameraGetFrame(Camera);

    RwV3dScale(&delta, RwMatrixGetAt(RwFrameGetMatrix(cameraFrame)), zDelta); 

    RwMatrixTranslate(Im3DMatrix, &delta, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
void 
Im3DRender(void)
{
    static RwBool stateSet = FALSE;

    if( !stateSet )
    {
        /* 
         * Render state is persistent - only need to set it once...
         */
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);

        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

        stateSet = TRUE;
    }

    if( RwIm3DTransform(Im3DVertices, NumVertices, Im3DMatrix, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, 
            Im3DLineIndices, NumIndices);

        RwIm3DEnd();
    }

    return;
}


/*
 *****************************************************************************
 */
RwBool 
Im3DInitialize(void)
{
    /* 
     * Create the matrix for the IM geometry's transformation....
     */
    Im3DMatrix = RwMatrixCreate();

    if( Im3DMatrix == NULL )
    {
        return FALSE;
    }
    else
    {
        /*
         * Initialize the matrix so that it positions the IM geometry
         * in front of the camera...
         */
        RwMatrix *cameraMatrix;
        RwV3d  *at, pos;

        cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
        at = RwMatrixGetAt(cameraMatrix);

        RwV3dScale(&pos, at, 30.0f);
        RwV3dAdd(&pos, &pos, RwMatrixGetAt(cameraMatrix));
        
        RwMatrixTranslate(Im3DMatrix, &pos, rwCOMBINEREPLACE);
    }

    /*
     * Start with a wire-frame cube...
     */
    if( !Im3DCreateCube() )
    {
        return FALSE;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
void 
Im3DTerminate(void)
{
    Im3DClearData();

    if( Im3DMatrix )
    {
        RwMatrixDestroy(Im3DMatrix);
    }

    return;
}

/*
 *****************************************************************************
 */

