
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
 * linelist.c
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
 * rwPRIMTYPELINELIST geometry data - star shape.
 * (X, Y, Z, U, V).
 */
static RwReal LineListData[28][5] = 
{
    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},
    { 0.000f,  1.000f,  0.000f,  1.000f,  1.000f},
    
    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
    { 0.000f, -1.000f,  0.000f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
    { 0.000f,  0.000f,  1.000f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
    { 0.000f,  0.000f, -1.000f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},
    { 1.000f,  0.000f,  0.000f,  1.000f,  1.000f},    

    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
    {-1.000f,  0.000f,  0.000f,  1.000f,  1.000f},    

    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},
    { 0.577f,  0.577f,  0.577f,  1.000f,  1.000f},
    
    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},
    { 0.577f, -0.577f,  0.577f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},
    {-0.577f,  0.577f, -0.577f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
    {-0.577f, -0.577f, -0.577f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
    { 0.577f, -0.577f, -0.577f,  1.000f,  1.000f},       
    
    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
    { 0.577f,  0.577f, -0.577f,  1.000f,  1.000f},       

    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
    {-0.577f, -0.577f,  0.577f,  1.000f,  1.000f},       
    
    { 0.000f,  0.000f,  0.000f,  0.000f,  0.000f},    
    {-0.577f,  0.577f,  0.577f,  1.000f,  1.000f},       
};


/*
 * Indexed rwPRIMTYPELINELIST geometry data - wireframe sphere.
 * No texture coordinates).
 * (X, Y, Z, U, V).
 */
static RwReal IndexedLineListData[18][5] = 
{
    { 0.000f,  1.000f,  0.000f,  0.000f,  0.000f},

    { 0.577f,  0.577f,  0.577f,  0.000f,  0.000f},
    { 0.577f,  0.577f, -0.577f,  0.000f,  0.000f},
    {-0.577f,  0.577f, -0.577f,  0.000f,  0.000f},
    {-0.577f,  0.577f,  0.577f,  0.000f,  0.000f},
            
    { 0.000f,  0.000f,  1.000f,  0.000f,  0.000f},
    { 0.707f,  0.000f,  0.707f,  0.000f,  0.000f},
    { 1.000f,  0.000f,  0.000f,  0.000f,  0.000f},
    { 0.707f,  0.000f, -0.707f,  0.000f,  0.000f},
    { 0.000f,  0.000f, -1.000f,  0.000f,  0.000f},
    {-0.707f,  0.000f, -0.707f,  0.000f,  0.000f},
    {-1.000f,  0.000f,  0.000f,  0.000f,  0.000f},
    {-0.707f,  0.000f,  0.707f,  0.000f,  0.000f},

    { 0.577f, -0.577f,  0.577f,  0.000f,  0.000f},
    { 0.577f, -0.577f, -0.577f,  0.000f,  0.000f},
    {-0.577f, -0.577f, -0.577f,  0.000f,  0.000f},
    {-0.577f, -0.577f,  0.577f,  0.000f,  0.000f},

    { 0.000f, -1.000f,  0.000f,  0.000f,  0.000f},    
};


static RwImVertexIndex IndexedLineListIndices[96] = 
{
    0,1,   0,2,   0,3,   0,4,   1,2,   2,3,   3,4,   4,1,
    
    1,5,   1,6,   1,7,   2,7,   2,8,   2,9,   3,9,   3,10,  3,11,  4,11,  4,12,  4,5,
    5,6,   6,7,   7,8,   8,9,   9,10,  10,11, 11,12, 12,5,
    13,5,  13,6,  13,7,  14,7,  14,8,  14,9,  15,9,  15,10, 15,11, 16,11, 16,12, 16,5,
    
    17,13, 17,14, 17,15, 17,16, 13,14,  14,15, 15,16, 16,13    
};


static RwIm3DVertex LineList[28];
static RwIm3DVertex IndexedLineList[18];


/*
 *****************************************************************************
 */
void
LineListCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup rwPRIMTYPELINELIST geometry, based on vertex data...
     */
    for(i=0; i<28; i++)
    {
        RwIm3DVertexSetPos(&LineList[i], 
            LineListData[i][0], LineListData[i][1], LineListData[i][2]);

        RwIm3DVertexSetU(&LineList[i], LineListData[i][3]);    
        RwIm3DVertexSetV(&LineList[i], LineListData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
LineListSetColor(RwBool white)
{
    RwInt32 i;

    RwRGBA SolidColor1 = SolidRed;
    RwRGBA SolidColor2 = SolidWhite;

    if( white )
    {
        SolidColor1 = SolidWhite;
        SolidColor2 = SolidWhite;
    }

    /*
     * Every line changes from Red to White...
     */
    for(i = 0; i < 28; i+=2)       
    {
        RwIm3DVertexSetRGBA(&LineList[i], SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);

        RwIm3DVertexSetRGBA(&LineList[i+1], SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);            
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedLineListCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup rwPRIMTYPELINELIST geometry, based on vertex data...
     */
    for(i=0; i<18; i++)
    {
        RwIm3DVertexSetPos(&IndexedLineList[i], IndexedLineListData[i][0], 
            IndexedLineListData[i][1], IndexedLineListData[i][2]);

        RwIm3DVertexSetU(&IndexedLineList[i], IndexedLineListData[i][3]);    
        RwIm3DVertexSetV(&IndexedLineList[i], IndexedLineListData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedLineListSetColor(RwBool white)
{
    RwInt32 i;

    RwRGBA SolidColor1 = SolidRed;
    RwRGBA SolidColor2 = SolidGreen;    
    RwRGBA SolidColor3 = SolidBlue;        

    if( white )
    {
        SolidColor1 = SolidWhite;
        SolidColor2 = SolidWhite;        
        SolidColor3 = SolidWhite;                
    }

    /*
     * Each layer in the sphere has a different color...
     */
    RwIm3DVertexSetRGBA(&IndexedLineList[0], 
        SolidColor1.red, SolidColor1.green,
        SolidColor1.blue, SolidColor1.alpha);

    for(i=1; i<5; i++)       
    {
        RwIm3DVertexSetRGBA(&IndexedLineList[i], 
            SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);
    }

    for(i=5; i<13; i++)       
    {
        RwIm3DVertexSetRGBA(&IndexedLineList[i], 
            SolidColor3.red, SolidColor3.green,
            SolidColor3.blue, SolidColor3.alpha);
    }
    
    for(i=13; i<17; i++)       
    {
        RwIm3DVertexSetRGBA(&IndexedLineList[i], 
            SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);
    }

    RwIm3DVertexSetRGBA(&IndexedLineList[17], 
        SolidColor1.red, SolidColor1.green,
        SolidColor1.blue, SolidColor1.alpha);

    return;
}


/*
 *****************************************************************************
 */
void
LineListRender(RwMatrix *transform, RwUInt32 transformFlags)
{
    if( RwIm3DTransform(LineList, 28, transform, transformFlags) )
    {                         
        RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);        
        
        RwIm3DEnd();
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedLineListRender(RwMatrix *transform, RwUInt32 transformFlags)
{
    if( RwIm3DTransform(IndexedLineList, 18, transform, transformFlags) )
    {                         
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST,
            IndexedLineListIndices, 96);

        RwIm3DEnd();
    }   

    return;
}

/*
 *****************************************************************************
 */
