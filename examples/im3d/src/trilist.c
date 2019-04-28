
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
 * trilist.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate RenderWare's 3D immediate mode.
 *****************************************************************************/

#include "rwcore.h"

#include "im3d.h"


/*
 * rwPRIMTYPETRILIST geometry data - a cube.
 * (X, Y, Z, U, V).
 */
static RwReal TriListData[36][5] = 
{    
    /* front */
    { 1.000f,  1.000f,  1.000f,  1.000f,  1.000f},
    {-1.000f, -1.000f,  1.000f,  0.000f,  0.000f},
    { 1.000f, -1.000f,  1.000f,  1.000f,  0.000f},    
    { 1.000f,  1.000f,  1.000f,  1.000f,  1.000f},
    {-1.000f,  1.000f,  1.000f,  0.000f,  1.000f},
    {-1.000f, -1.000f,  1.000f,  0.000f,  0.000f},    
    /* back */
    {-1.000f, -1.000f, -1.000f,  1.000f,  1.000f},
    {-1.000f,  1.000f, -1.000f,  1.000f,  0.000f},
    { 1.000f,  1.000f, -1.000f,  0.000f,  0.000f},    
    {-1.000f, -1.000f, -1.000f,  1.000f,  1.000f},
    { 1.000f,  1.000f, -1.000f,  0.000f,  0.000f},
    { 1.000f, -1.000f, -1.000f,  0.000f,  1.000f},    
    /* top */ 
    { 1.000f,  1.000f,  1.000f,  1.000f,  1.000f},
    { 1.000f,  1.000f, -1.000f,  1.000f,  0.000f},
    {-1.000f,  1.000f, -1.000f,  0.000f,  0.000f},    
    { 1.000f,  1.000f,  1.000f,  1.000f,  1.000f},
    {-1.000f,  1.000f, -1.000f,  0.000f,  0.000f},
    {-1.000f,  1.000f,  1.000f,  0.000f,  1.000f},
    /* bottom */
    {-1.000f, -1.000f, -1.000f,  1.000f,  1.000f},
    { 1.000f, -1.000f,  1.000f,  0.000f,  0.000f},
    {-1.000f, -1.000f,  1.000f,  1.000f,  0.000f},    
    {-1.000f, -1.000f, -1.000f,  1.000f,  1.000f},
    { 1.000f, -1.000f, -1.000f,  0.000f,  1.000f},
    { 1.000f, -1.000f,  1.000f,  0.000f,  0.000f},
    /* left */
    {-1.000f, -1.000f, -1.000f,  1.000f,  1.000f},
    {-1.000f,  1.000f,  1.000f,  0.000f,  0.000f},
    {-1.000f,  1.000f, -1.000f,  1.000f,  0.000f},    
    {-1.000f, -1.000f, -1.000f,  1.000f,  1.000f},
    {-1.000f, -1.000f,  1.000f,  0.000f,  1.000f},
    {-1.000f,  1.000f,  1.000f,  0.000f,  0.000f},
    /* right */
    { 1.000f,  1.000f,  1.000f,  1.000f,  1.000f},
    { 1.000f, -1.000f,  1.000f,  1.000f,  0.000f},
    { 1.000f, -1.000f, -1.000f,  0.000f,  0.000f},    
    { 1.000f,  1.000f,  1.000f,  1.000f,  1.000f},
    { 1.000f, -1.000f, -1.000f,  0.000f,  0.000f},
    { 1.000f,  1.000f, -1.000f,  0.000f,  1.000f}
};


/*
 * Indexed rwPRIMTYPETRILIST geometry data - a cube.
 * (X, Y, Z, U, V).
 */
static RwReal IndexedTriListData[8][5] = 
{    
    { 1.000f,  1.000f,  1.000f,  1.000f,  0.000f},
    {-1.000f,  1.000f,  1.000f,  1.000f,  1.000f},
    {-1.000f, -1.000f,  1.000f,  0.500f,  1.000f},    
    { 1.000f, -1.000f,  1.000f,  0.500f,  0.000f},    
    
    { 1.000f,  1.000f, -1.000f,  0.500f,  0.000f},
    {-1.000f,  1.000f, -1.000f,  0.500f,  1.000f},
    {-1.000f, -1.000f, -1.000f,  0.000f,  1.000f},    
    { 1.000f, -1.000f, -1.000f,  0.000f,  0.000f}    
};


static RwImVertexIndex IndexedTriListIndices[36] = 
{ 
    /* front */
    0, 1, 3,  1, 2, 3,
    /* back */
    7, 5, 4,  5, 7, 6, 
    /* left */
    6, 2, 1,  1, 5, 6,
    /* right */
    0, 3, 4,  4, 3, 7, 
    /* top */
    1, 0, 4,  1, 4, 5,
    /* bottom */
    2, 6, 3,  6, 7, 3
};


static RwIm3DVertex TriList[36];
static RwIm3DVertex IndexedTriList[8];



/*
 *****************************************************************************
 */
void
TriListCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup rwPRIMTYPETRILIST geometry, based on vertex data...
     */
    for(i=0; i<36; i++)
    {
        RwIm3DVertexSetPos(&TriList[i], 
            TriListData[i][0], TriListData[i][1], TriListData[i][2]);

        RwIm3DVertexSetU(&TriList[i], TriListData[i][3]);    
        RwIm3DVertexSetV(&TriList[i], TriListData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
TriListSetColor(RwBool white)
{
    RwInt32 i;

    RwRGBA SolidColor1 = SolidRed;
    RwRGBA SolidColor2 = SolidBlue;
    RwRGBA SolidColor3 = SolidGreen;
    RwRGBA SolidColor4 = SolidYellow;
    RwRGBA SolidColor5 = SolidCyan;
    RwRGBA SolidColor6 = SolidPurple;

    if( white )
    {
        SolidColor1 = SolidWhite;
        SolidColor2 = SolidWhite;
        SolidColor3 = SolidWhite;
        SolidColor4 = SolidWhite;
        SolidColor5 = SolidWhite;
        SolidColor6 = SolidWhite;        
    }    
    /* 
     * Every face has a different color...
     */
    for(i = 0; i < 6; i++)       
    {
        RwIm3DVertexSetRGBA(&TriList[i], SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }

    for(i = 0; i < 6; i++)       
    {
        RwIm3DVertexSetRGBA(&TriList[6+i], SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);
    }

    for(i = 0; i < 6; i++)       
    {
        RwIm3DVertexSetRGBA(&TriList[12+i], SolidColor3.red, SolidColor3.green,
            SolidColor3.blue, SolidColor3.alpha);
    }

    for(i = 0; i < 6; i++)       
    {
        RwIm3DVertexSetRGBA(&TriList[18+i], SolidColor4.red, SolidColor4.green,
            SolidColor4.blue, SolidColor4.alpha);
    }

    for(i = 0; i < 6; i++)       
    {
        RwIm3DVertexSetRGBA(&TriList[24+i], SolidColor5.red, SolidColor5.green,
            SolidColor5.blue, SolidColor5.alpha);
    }

    for(i = 0; i < 6; i++)       
    {
        RwIm3DVertexSetRGBA(&TriList[30+i], SolidColor6.red, SolidColor6.green,
            SolidColor6.blue, SolidColor6.alpha);
    }  
    
    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriListCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup Indexed rwPRIMTYPETRILIST geometry, based on vertex data...
     */
    for(i=0; i<8; i++)
    {
        RwIm3DVertexSetPos(&IndexedTriList[i], IndexedTriListData[i][0], 
            IndexedTriListData[i][1], IndexedTriListData[i][2]);

        RwIm3DVertexSetU(&IndexedTriList[i], IndexedTriListData[i][3]);    
        RwIm3DVertexSetV(&IndexedTriList[i], IndexedTriListData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriListSetColor(RwBool white)
{
    RwRGBA SolidColor1 = SolidRed;
    RwRGBA SolidColor2 = SolidYellow;
    RwRGBA SolidColor3 = SolidBlack;
    RwRGBA SolidColor4 = SolidPurple;
    RwRGBA SolidColor5 = SolidGreen;
    RwRGBA SolidColor6 = SolidCyan;
    RwRGBA SolidColor7 = SolidBlue;
    RwRGBA SolidColor8 = SolidWhite;

    if( white )
    {
        SolidColor1 = SolidWhite;
        SolidColor2 = SolidWhite;
        SolidColor3 = SolidWhite;
        SolidColor4 = SolidWhite;
        SolidColor5 = SolidWhite;
        SolidColor6 = SolidWhite;        
        SolidColor7 = SolidWhite;
        SolidColor8 = SolidWhite;        
    }
    /* 
     * Every vertex has a different color...
     */
    RwIm3DVertexSetRGBA(&IndexedTriList[0], SolidColor1.red, 
        SolidColor1.green, SolidColor1.blue, SolidColor1.alpha);

    RwIm3DVertexSetRGBA(&IndexedTriList[1], SolidColor2.red, 
        SolidColor2.green, SolidColor2.blue, SolidColor2.alpha);

    RwIm3DVertexSetRGBA(&IndexedTriList[2], SolidColor3.red, 
        SolidColor3.green, SolidColor3.blue, SolidColor3.alpha);

    RwIm3DVertexSetRGBA(&IndexedTriList[3], SolidColor4.red, 
        SolidColor4.green, SolidColor4.blue, SolidColor4.alpha);

    RwIm3DVertexSetRGBA(&IndexedTriList[4], SolidColor5.red, 
        SolidColor5.green, SolidColor5.blue, SolidColor5.alpha);

    RwIm3DVertexSetRGBA(&IndexedTriList[5], SolidColor6.red, 
        SolidColor6.green, SolidColor6.blue, SolidColor6.alpha);

    RwIm3DVertexSetRGBA(&IndexedTriList[6], SolidColor7.red, 
        SolidColor7.green, SolidColor7.blue, SolidColor7.alpha);

    RwIm3DVertexSetRGBA(&IndexedTriList[7], SolidColor8.red, 
        SolidColor8.green, SolidColor8.blue, SolidColor8.alpha);

    return;
}


/*
 *****************************************************************************
 */
void
TriListRender(RwMatrix *transform, RwUInt32 transformFlags)
{
    if( RwIm3DTransform(TriList, 36, transform, transformFlags) )
    {                         
        RwIm3DRenderPrimitive(rwPRIMTYPETRILIST);   
        
        RwIm3DEnd();
    }   

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriListRender(RwMatrix *transform, RwUInt32 transformFlags)
{    
    if( RwIm3DTransform(IndexedTriList, 8, transform, transformFlags) )
    {                         
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, 
            IndexedTriListIndices, 36);

        RwIm3DEnd();
    }   

    return;
}

/*
 *****************************************************************************
 */
