
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
 * tristrip.c
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
 * rwPRIMTYPETRISTRIP geometry data - two-sided cylinder.
 * (X, Y, Z, U, V).
 */
static RwReal TriStripData[36][5] = 
{
    { 0.000f,  1.000f, -1.000f,  0.000f,  0.000f},    
    { 0.000f,  1.000f,  1.000f,  0.000f,  1.000f},
   
    { 0.707f,  0.707f, -1.000f,  0.125f,  0.000f},
    { 0.707f,  0.707f,  1.000f,  0.125f,  1.000f},    
    
    { 1.000f,  0.000f, -1.000f,  0.250f,  0.000f},
    { 1.000f,  0.000f,  1.000f,  0.250f,  1.000f},

    { 0.707f, -0.707f, -1.000f,  0.375f,  0.000f},
    { 0.707f, -0.707f,  1.000f,  0.375f,  1.000f},
        
    { 0.000f, -1.000f, -1.000f,  0.500f,  0.000f},
    { 0.000f, -1.000f,  1.000f,  0.500f,  1.000f},
        
    {-0.707f, -0.707f, -1.000f,  0.625f,  0.000f},
    {-0.707f, -0.707f,  1.000f,  0.625f,  1.000f},
        
    {-1.000f, -0.000f, -1.000f,  0.750f,  0.000f},
    {-1.000f, -0.000f,  1.000f,  0.750f,  1.000f},
        
    {-0.707f,  0.707f, -1.000f,  0.875f,  0.000f},
    {-0.707f,  0.707f,  1.000f,  0.875f,  1.000f},

    { 0.000f,  1.000f, -1.000f,  1.000f,  0.000f},    
    { 0.000f,  1.000f,  1.000f,  1.000f,  1.000f},
    
    { 0.000f,  1.000f,  1.000f,  0.000f,  0.000f},
    { 0.000f,  1.000f, -1.000f,  0.000f,  1.000f},        
    
    { 0.707f,  0.707f,  1.000f,  0.125f,  0.000f},    
    { 0.707f,  0.707f, -1.000f,  0.125f,  1.000f},    
        
    { 1.000f,  0.000f,  1.000f,  0.250f,  0.000f},
    { 1.000f,  0.000f, -1.000f,  0.250f,  1.000f},
        
    { 0.707f, -0.707f,  1.000f,  0.375f,  0.000f},
    { 0.707f, -0.707f, -1.000f,  0.375f,  1.000f},
        
    { 0.000f, -1.000f,  1.000f,  0.500f,  0.000f},
    { 0.000f, -1.000f, -1.000f,  0.500f,  1.000f},
        
    {-0.707f, -0.707f,  1.000f,  0.625f,  0.000f},
    {-0.707f, -0.707f, -1.000f,  0.625f,  1.000f},
        
    {-1.000f, -0.000f,  1.000f,  0.750f,  0.000f},
    {-1.000f, -0.000f, -1.000f,  0.750f,  1.000f},
        
    {-0.707f,  0.707f,  1.000f,  0.875f,  0.000f},
    {-0.707f,  0.707f, -1.000f,  0.875f,  1.000f},
        
    { 0.000f,  1.000f,  1.000f,  1.000f,  0.000f},    
    { 0.000f,  1.000f, -1.000f,  1.000f,  1.000f}      
};


/*
 * Indexed rwPRIMTYPETRISTRIP geometry data - two-sided cylinder.
 * (X, Y, Z, U, V).
 */
static RwReal IndexedTriStripData[16][5] = 
{
    { 0.000f,  1.000f,  1.000f,  0.000f,  0.000f},
    { 0.707f,  0.707f,  1.000f,  0.250f,  0.000f},
    { 1.000f,  0.000f,  1.000f,  0.500f,  0.000f},
    { 0.707f, -0.707f,  1.000f,  0.750f,  0.000f},
    { 0.000f, -1.000f,  1.000f,  1.000f,  0.000f},
    {-0.707f, -0.707f,  1.000f,  0.750f,  0.000f},
    {-1.000f, -0.000f,  1.000f,  0.500f,  0.000f},
    {-0.707f,  0.707f,  1.000f,  0.250f,  0.000f},
    
    { 0.000f,  1.000f, -1.000f,  0.000f,  1.000f},
    { 0.707f,  0.707f, -1.000f,  0.250f,  1.000f},
    { 1.000f,  0.000f, -1.000f,  0.500f,  1.000f},
    { 0.707f, -0.707f, -1.000f,  0.750f,  1.000f},
    { 0.000f, -1.000f, -1.000f,  1.000f,  1.000f},
    {-0.707f, -0.707f, -1.000f,  0.750f,  1.000f},
    {-1.000f, -0.000f, -1.000f,  0.500f,  1.000f},
    {-0.707f,  0.707f, -1.000f,  0.250f,  1.000f},
};


static RwImVertexIndex IndexedTriStripIndices[36] = 
{ 
    /*
     * Inside... 
     */ 
    0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15, 0, 8,
    
    /*
     * Outside...
     */ 
    8, 0, 9, 1, 10, 2, 11, 3, 12, 4, 13, 5, 14, 6, 15, 7, 8, 0
};


static RwIm3DVertex TriStrip[36];
static RwIm3DVertex IndexedTriStrip[16];



/*
 *****************************************************************************
 */
void
TriStripCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup rwPRIMTYPETRISTRIP geometry, based on vertex data...
     */
    for(i=0; i<36; i++)
    {
        RwIm3DVertexSetPos(&TriStrip[i], 
            TriStripData[i][0], TriStripData[i][1], TriStripData[i][2]);

        RwIm3DVertexSetU(&TriStrip[i], TriStripData[i][3]);    
        RwIm3DVertexSetV(&TriStrip[i], TriStripData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
TriStripSetColor(RwBool white)
{
    RwInt32 i;

    RwRGBA SolidColor1 = SolidRed;
    RwRGBA SolidColor2 = SolidYellow;

    if( white )
    {
        SolidColor1 = SolidWhite;
        SolidColor2 = SolidWhite;
    }

    /* 
     * Top of cylinder is different color to base of cylinder...
     */
    for(i=0; i<36; i+=2)       
    {
        RwIm3DVertexSetRGBA(&TriStrip[i], SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);

        RwIm3DVertexSetRGBA(&TriStrip[i+1], SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);
    }  
    
    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriStripCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup rwPRIMTYPETRISTRIP geometry, based on vertex data...
     */
    for(i=0; i<16; i++)
    {
        RwIm3DVertexSetPos(&IndexedTriStrip[i], IndexedTriStripData[i][0], 
            IndexedTriStripData[i][1], IndexedTriStripData[i][2]);

        RwIm3DVertexSetU(&IndexedTriStrip[i], IndexedTriStripData[i][3]);    
        RwIm3DVertexSetV(&IndexedTriStrip[i], IndexedTriStripData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriStripSetColor(RwBool white)
{
    RwInt32 i;

    RwRGBA SolidColor1 = SolidBlue;
    RwRGBA SolidColor2 = SolidGreen;

    if( white )
    {
        SolidColor1 = SolidWhite;
        SolidColor2 = SolidWhite;
    }
    /* 
     * Top of cylinder is different color to base of cylinder...
     */
    for(i=0; i<8; i++)       
    {
        RwIm3DVertexSetRGBA(&IndexedTriStrip[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }      
    
    for(i=8; i<16; i++)       
    {
        RwIm3DVertexSetRGBA(&IndexedTriStrip[i], 
            SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);
    }  
    
    return;
}


/*
 *****************************************************************************
 */
void
TriStripRender(RwMatrix *transform, RwUInt32 transformFlags)
{    
    if( RwIm3DTransform(TriStrip, 36, transform, transformFlags) )
    {                         
        RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP); 
        
        RwIm3DEnd();
    }   

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriStripRender(RwMatrix *transform, RwUInt32 transformFlags)
{    
    if( RwIm3DTransform(IndexedTriStrip, 16, transform, transformFlags) )
    {                         
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRISTRIP, 
            IndexedTriStripIndices, 36);

        RwIm3DEnd();
    }    
    
    return;
}

/*
 *****************************************************************************
 */
