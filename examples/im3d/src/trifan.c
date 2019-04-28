
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
 * trifan.c
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
 * rwPRIMTYPETRIFAN geometry data - a two-sided cone.
 * (X, Y, Z, U, V).
 */
static RwReal TriFanData[34][5] = 
{
    { 0.000f,  0.000f, -1.000f,  0.500f,  0.500f},

    { 0.000f,  1.000f,  0.000f,  0.500f,  1.000f},
    { 0.383f,  0.924f,  0.000f,  0.691f,  0.962f},
    { 0.707f,  0.707f,  0.000f,  0.854f,  0.854f},
    { 0.924f,  0.383f,  0.000f,  0.962f,  0.691f},
    { 1.000f,  0.000f,  0.000f,  1.000f,  0.500f},
    { 0.924f, -0.383f,  0.000f,  0.962f,  0.309f},
    { 0.707f, -0.707f,  0.000f,  0.854f,  0.146f},
    { 0.383f, -0.924f,  0.000f,  0.691f,  0.038f},
    { 0.000f, -1.000f,  0.000f,  0.500f,  0.000f},
    {-0.383f, -0.924f,  0.000f,  0.309f,  0.038f},
    {-0.707f, -0.707f,  0.000f,  0.146f,  0.146f},
    {-0.924f, -0.383f,  0.000f,  0.038f,  0.309f},
    {-1.000f, -0.000f,  0.000f,  0.000f,  0.500f},
    {-0.924f,  0.383f,  0.000f,  0.038f,  0.691f},
    {-0.707f,  0.707f,  0.000f,  0.146f,  0.854f},
    {-0.383f,  0.924f,  0.000f,  0.309f,  0.962f},
    
    { 0.000f,  1.000f,  0.000f,  0.500f,  1.000f},
    
    {-0.383f,  0.924f,  0.000f,  0.309f,  0.962f},
    {-0.707f,  0.707f,  0.000f,  0.146f,  0.854f},
    {-0.924f,  0.383f,  0.000f,  0.038f,  0.691f},
    {-1.000f, -0.000f,  0.000f,  0.000f,  0.500f},
    {-0.924f, -0.383f,  0.000f,  0.038f,  0.309f},
    {-0.707f, -0.707f,  0.000f,  0.146f,  0.146f},
    {-0.383f, -0.924f,  0.000f,  0.309f,  0.038f},
    { 0.000f, -1.000f,  0.000f,  0.500f,  0.000f},
    { 0.383f, -0.924f,  0.000f,  0.691f,  0.038f},
    { 0.707f, -0.707f,  0.000f,  0.854f,  0.146f},
    { 0.924f, -0.383f,  0.000f,  0.962f,  0.309f},
    { 1.000f,  0.000f,  0.000f,  1.000f,  0.500f},
    { 0.924f,  0.383f,  0.000f,  0.962f,  0.691f},
    { 0.707f,  0.707f,  0.000f,  0.854f,  0.854f},    
    { 0.383f,  0.924f,  0.000f,  0.691f,  0.962f},
    { 0.000f,  1.000f,  0.000f,  0.500f,  1.000f}
};


/*
 * Indexed rwPRIMTYPETRIFAN geometry data - a two-sided cone.
 * (X, Y, Z, U, V).
 */
static RwReal IndexedTriFanData[17][5] = 
{
    /* top */
    { 0.000f,  0.000f, -1.000f,  0.500f,  0.500f},
    /* circle */
    { 0.000f,  1.000f,  0.000f,  0.500f,  1.000f},
    { 0.383f,  0.924f,  0.000f,  0.691f,  0.962f},
    { 0.707f,  0.707f,  0.000f,  0.854f,  0.854f},
    { 0.924f,  0.383f,  0.000f,  0.962f,  0.691f},
    { 1.000f,  0.000f,  0.000f,  1.000f,  0.500f},
    { 0.924f, -0.383f,  0.000f,  0.962f,  0.309f},
    { 0.707f, -0.707f,  0.000f,  0.854f,  0.146f},
    { 0.383f, -0.924f,  0.000f,  0.691f,  0.038f},
    { 0.000f, -1.000f,  0.000f,  0.500f,  0.000f},
    {-0.383f, -0.924f,  0.000f,  0.309f,  0.038f},
    {-0.707f, -0.707f,  0.000f,  0.146f,  0.146f},
    {-0.924f, -0.383f,  0.000f,  0.038f,  0.309f},
    {-1.000f, -0.000f,  0.000f,  0.000f,  0.500f},
    {-0.924f,  0.383f,  0.000f,  0.038f,  0.691f},
    {-0.707f,  0.707f,  0.000f,  0.146f,  0.854f},
    {-0.383f,  0.924f,  0.000f,  0.309f,  0.962f}
};


static RwImVertexIndex IndexedTriFanIndices[34] = 
{ 
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 1,
    16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,
};


static RwIm3DVertex TriFan[34];
static RwIm3DVertex IndexedTriFan[17];



/*
 *****************************************************************************
 */
void
TriFanCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup rwPRIMTYPETRIFAN geometry, based on vertex data...
     */
    for(i=0; i<34; i++)
    {
        RwIm3DVertexSetPos(&TriFan[i], 
            TriFanData[i][0], TriFanData[i][1], TriFanData[i][2]);

        RwIm3DVertexSetU(&TriFan[i], TriFanData[i][3]);    
        RwIm3DVertexSetV(&TriFan[i], TriFanData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
TriFanSetColor(RwBool white)
{
    RwInt32 i;

    RwRGBA SolidColor1 = SolidYellow;
    RwRGBA SolidColor2 = SolidBlue;
    RwRGBA SolidColor3 = SolidRed;

    if( white )
    {
        SolidColor1 = SolidWhite;
        SolidColor2 = SolidWhite;
        SolidColor3 = SolidWhite;
    }

    /* 
     * Tip of cone is different color to base...
     */
    RwIm3DVertexSetRGBA(&TriFan[0], SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);

    for(i=1; i<34; i++)       
    {
        RwIm3DVertexSetRGBA(&TriFan[i], SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);
    }  
    
    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriFanCreate(void)
{
    RwInt32 i;   

    /* 
     * Setup indexed rwPRIMTYPETRIFAN geometry, based on vertex data...
     */
    for(i=0; i<17; i++)
    {
        RwIm3DVertexSetPos(&IndexedTriFan[i], IndexedTriFanData[i][0], 
            IndexedTriFanData[i][1], IndexedTriFanData[i][2]);

        RwIm3DVertexSetU(&IndexedTriFan[i], IndexedTriFanData[i][3]);    
        RwIm3DVertexSetV(&IndexedTriFan[i], IndexedTriFanData[i][4]);  
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriFanSetColor(RwBool white)
{
    RwInt32 i;

    RwRGBA SolidColor1 = SolidGreen;
    RwRGBA SolidColor2 = SolidBlack;
    RwRGBA SolidColor3 = SolidPurple;

    if( white )
    {
        SolidColor1 = SolidWhite;
        SolidColor2 = SolidWhite;
        SolidColor3 = SolidWhite;
    }

    /* 
     * Tip of cone is different color to base...
     */
    RwIm3DVertexSetRGBA(&IndexedTriFan[0], 
        SolidColor1.red, SolidColor1.green,
        SolidColor1.blue, SolidColor1.alpha);

    for(i=1; i<17; i++)       
    {
        RwIm3DVertexSetRGBA(&IndexedTriFan[i], 
            SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);
    }  
    
    return;
}


/*
 *****************************************************************************
 */
void
TriFanRender(RwMatrix *transform, RwUInt32 transformFlags)
{
    if( RwIm3DTransform(TriFan, 34, transform, transformFlags) )
    {                         
        RwIm3DRenderPrimitive(rwPRIMTYPETRIFAN);  
        
        RwIm3DEnd();
    }   

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriFanRender(RwMatrix *transform, RwUInt32 transformFlags)
{    
    if( RwIm3DTransform(IndexedTriFan, 17, transform, transformFlags) )
    {                                 
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRIFAN, 
            IndexedTriFanIndices, 34);        

        RwIm3DEnd();
    }   

    return;
}

/*
 *****************************************************************************
 */
