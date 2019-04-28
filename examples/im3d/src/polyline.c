
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
 * polyline.c
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
 * rwPRIMTYPEPOLYLINE geometry data - a coil/spring.
 * (X, Y, Z, U, V).
 */
static RwReal PolyLineData[21][5] = 
{
    { 0.000f,  1.000f,  1.000f,  0.500f,  1.000f},
    { 0.707f,  0.707f,  0.900f,  0.854f,  0.854f},
    { 1.000f,  0.000f,  0.800f,  1.000f,  0.500f},
    { 0.707f, -0.707f,  0.700f,  0.854f,  0.146f},
    { 0.000f, -1.000f,  0.600f,  0.500f,  0.000f},
    {-0.707f, -0.707f,  0.500f,  0.146f,  0.146f},
    {-1.000f, -0.000f,  0.400f,  0.000f,  0.500f},
    {-0.707f,  0.707f,  0.300f,  0.146f,  0.854f},
    
    { 0.000f,  1.000f,  0.200f,  0.500f,  1.000f},
    { 0.707f,  0.707f,  0.100f,  0.854f,  0.854f},
    { 1.000f,  0.000f,  0.000f,  1.000f,  0.500f},
    { 0.707f, -0.707f, -0.100f,  0.854f,  0.146f},
    { 0.000f, -1.000f, -0.200f,  0.500f,  0.000f},
    {-0.707f, -0.707f, -0.300f,  0.146f,  0.146f},
    {-1.000f, -0.000f, -0.400f,  0.000f,  0.500f},
    {-0.707f,  0.707f, -0.500f,  0.146f,  0.854f},
    
    { 0.000f,  1.000f, -0.600f,  0.500f,  1.000f},
    { 0.707f,  0.707f, -0.700f,  0.854f,  0.854f},
    { 1.000f,  0.000f, -0.800f,  1.000f,  0.500f},
    { 0.707f, -0.707f, -0.900f,  0.854f,  0.146f},
    { 0.000f, -1.000f, -1.000f,  0.500f,  0.000f}
};


/*
 * Indexed rwPRIMTYPEPOLYLINE geometry data - wireframe cube.
 * (X, Y, Z, U, V).
 */
static RwReal IndexedPolyLineData[8][5] = 
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


static RwImVertexIndex IndexedPolyLineIndices[25] =
{
    0, 1, 2, 3, 0, 2, 6, 5, 1, 3, 7, 4, 0, 5, 4, 6, 1, 4, 3, 6, 7, 5, 2, 7, 0
};


static RwIm3DVertex PolyLine[21];
static RwIm3DVertex IndexedPolyLine[8];


/*
 *****************************************************************************
 */
void
PolyLineCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup rwPRIMTYPEPOLYLINE geometry, based on vertex data...
     */
    for(i=0; i<21; i++)
    {
        RwIm3DVertexSetPos(&PolyLine[i], 
            PolyLineData[i][0], PolyLineData[i][1], PolyLineData[i][2]);

        RwIm3DVertexSetU(&PolyLine[i], PolyLineData[i][3]);    
        RwIm3DVertexSetV(&PolyLine[i], PolyLineData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
PolyLineSetColor(RwBool white)
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
     * Each coil has a different color...
     */
    for(i=0; i<7; i++)       
    {
        RwIm3DVertexSetRGBA(&PolyLine[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }

    for(i=7; i<14; i++)       
    {
        RwIm3DVertexSetRGBA(&PolyLine[i], 
            SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);
    }

    for(i=14; i<21; i++)       
    {
        RwIm3DVertexSetRGBA(&PolyLine[i], 
            SolidColor3.red, SolidColor3.green,
            SolidColor3.blue, SolidColor3.alpha);
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedPolyLineCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup indexed rwPRIMTYPEPOLYLINE geometry, based on vertex data...
     */
    for(i=0; i<8; i++)
    {
        RwIm3DVertexSetPos(&IndexedPolyLine[i], IndexedPolyLineData[i][0], 
            IndexedPolyLineData[i][1], IndexedPolyLineData[i][2]);

        RwIm3DVertexSetU(&IndexedPolyLine[i], IndexedPolyLineData[i][3]);    
        RwIm3DVertexSetV(&IndexedPolyLine[i], IndexedPolyLineData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedPolyLineSetColor(RwBool white)
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
     * Every vertex has a different color, (color cube)...
     */
    RwIm3DVertexSetRGBA(&IndexedPolyLine[0], SolidColor1.red, 
        SolidColor1.green, SolidColor1.blue, SolidColor1.alpha);

    RwIm3DVertexSetRGBA(&IndexedPolyLine[1], SolidColor2.red, 
        SolidColor2.green, SolidColor2.blue, SolidColor2.alpha);

    RwIm3DVertexSetRGBA(&IndexedPolyLine[2], SolidColor3.red, 
        SolidColor3.green, SolidColor3.blue, SolidColor3.alpha);

    RwIm3DVertexSetRGBA(&IndexedPolyLine[3], SolidColor4.red, 
        SolidColor4.green, SolidColor4.blue, SolidColor4.alpha);

    RwIm3DVertexSetRGBA(&IndexedPolyLine[4], SolidColor5.red, 
        SolidColor5.green, SolidColor5.blue, SolidColor5.alpha);

    RwIm3DVertexSetRGBA(&IndexedPolyLine[5], SolidColor6.red, 
        SolidColor6.green, SolidColor6.blue, SolidColor6.alpha);

    RwIm3DVertexSetRGBA(&IndexedPolyLine[6], SolidColor7.red, 
        SolidColor7.green, SolidColor7.blue, SolidColor7.alpha);

    RwIm3DVertexSetRGBA(&IndexedPolyLine[7], SolidColor8.red, 
        SolidColor8.green, SolidColor8.blue, SolidColor8.alpha);

    return;
}


/*
 *****************************************************************************
 */
void
PolyLineRender(RwMatrix *transform, RwUInt32 transformFlags)
{
    if( RwIm3DTransform(PolyLine, 21, transform, transformFlags) )
    {                         
        RwIm3DRenderPrimitive(rwPRIMTYPEPOLYLINE);  
        
        RwIm3DEnd();
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedPolyLineRender(RwMatrix *transform, RwUInt32 transformFlags)
{
    if( RwIm3DTransform(IndexedPolyLine, 8, transform, transformFlags) )
    {                         
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE, 
            IndexedPolyLineIndices, 25);

        RwIm3DEnd();
    }   

    return;
}

/*
 *****************************************************************************
 */
