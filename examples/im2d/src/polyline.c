
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
 * Purpose: To demonstrate RenderWare's 2D immediate mode.
 * *****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"

#include "im2d.h"


/*
 * rwPRIMTYPEPOLYLINE geometry data - a coil.
 * (X, Y, U, V).
 */
static RwReal PolyLineData[16][4] =
{
    { 0.000f,  1.000f,  0.500f,  1.000f},   
    { 0.672f,  0.672f,  0.836f,  0.836f},
    { 0.900f,  0.000f,  0.950f,  0.500f},
    { 0.601f, -0.601f,  0.800f,  0.200f},
    { 0.000f, -0.800f,  0.500f,  0.100f},
    {-0.530f, -0.530f,  0.245f,  0.245f},
    {-0.700f,  0.000f,  0.150f,  0.500f},    
    {-0.460f,  0.460f,  0.270f,  0.770f},
    { 0.000f,  0.600f,  0.500f,  0.800f},
    { 0.389f,  0.389f,  0.695f,  0.695f},
    { 0.500f,  0.000f,  0.750f,  0.500f},
    { 0.318f, -0.318f,  0.659f,  0.341f},
    { 0.000f, -0.400f,  0.500f,  0.300f},
    {-0.247f, -0.247f,  0.376f,  0.376f},
    {-0.300f,  0.000f,  0.350f,  0.500f},    
    {-0.177f,  0.177f,  0.411f,  0.589f}  
}; 


/*
 * Indexed rwPRIMTYPEPOLYLINE geometry data - triangle of triangles.
 * (X, Y, U, V).
 */
static RwReal IndexedPolyLineData[21][4] =
{
    { 0.000f,  1.000f,  0.500f,  1.000f},   
    
    {-0.200f,  0.600f,  0.400f,  0.800f},    
    { 0.200f,  0.600f,  0.600f,  0.800f},
        
    {-0.400f,  0.200f,  0.300f,  0.600f},
    { 0.000f,  0.200f,  0.500f,  0.600f},
    { 0.400f,  0.200f,  0.700f,  0.600f},    
    
    {-0.600f, -0.200f,  0.200f,  0.400f},
    {-0.200f, -0.200f,  0.400f,  0.400f},    
    { 0.200f, -0.200f,  0.600f,  0.400f},        
    { 0.600f, -0.200f,  0.800f,  0.400f},    

    {-0.800f, -0.600f,  0.100f,  0.200f},
    {-0.400f, -0.600f,  0.300f,  0.200f},
    { 0.000f, -0.600f,  0.500f,  0.200f},
    { 0.400f, -0.600f,  0.700f,  0.200f},    
    { 0.800f, -0.600f,  0.900f,  0.200f},        

    {-1.000f, -1.000f,  0.000f,  0.000f},
    {-0.600f, -1.000f,  0.200f,  0.000f},
    {-0.200f, -1.000f,  0.400f,  0.000f},
    { 0.200f, -1.000f,  0.600f,  0.000f},            
    { 0.600f, -1.000f,  0.800f,  0.000f},
    { 1.000f, -1.000f,  1.000f,  0.000f}
};


static RwImVertexIndex IndexedPolyLineIndices[46] = 
{
     0,  2,  5,  4,  8,  5,  9,  8, 13,  9,
    14, 13, 19, 14, 20, 19, 18, 13, 12,  8,
     7, 12, 18, 17, 12, 11, 17, 16, 15, 10,
    16, 11, 10,  6, 11,  7,  6,  3,  7,  4,
     3, 1, 4, 2, 1, 0
};


static RwIm2DVertex PolyLine[16];
static RwIm2DVertex IndexedPolyLine[21];


/*
 *****************************************************************************
 */
void
PolyLineCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ = 1.0f / RwCameraGetNearClipPlane(camera);

    /* 
     * Setup rwPRIMTYPEPOLYLINE geometry, based on vertex data...
     */
    for(i=0; i<16; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&PolyLine[i], 
            (ScreenSize.x/2.0f) + (PolyLineData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&PolyLine[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (PolyLineData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&PolyLine[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&PolyLine[i], recipCameraZ);
        
        RwIm2DVertexSetU(&PolyLine[i], PolyLineData[i][2], recipCameraZ);
        RwIm2DVertexSetV(&PolyLine[i], PolyLineData[i][3], recipCameraZ);
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
    RwRGBA SolidColor1 = SolidBlue;
    
    if( white )
    {
        SolidColor1 = SolidWhite;
    }

    for(i=0; i<16; i++)       
    {
        RwIm2DVertexSetIntRGBA(&PolyLine[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }   
    
    return;
}


/*
 *****************************************************************************
 */
void
PolyLineRender(void)
{
    RwIm2DRenderPrimitive(rwPRIMTYPEPOLYLINE, PolyLine, 16);

    return;
}


/*
 *****************************************************************************
 */
void
IndexedPolyLineCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ = 1.0f / RwCameraGetFarClipPlane(camera);

    /* 
     * Setup indexed rwPRIMTYPEPOLYLINE geometry, based on vertex data...
     */
    for(i=0; i<21; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&IndexedPolyLine[i], 
            (ScreenSize.x/2.0f) + (IndexedPolyLineData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&IndexedPolyLine[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (IndexedPolyLineData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&IndexedPolyLine[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&PolyLine[i], recipCameraZ);
        
        RwIm2DVertexSetU(&IndexedPolyLine[i], 
            IndexedPolyLineData[i][2], recipCameraZ);
        
        RwIm2DVertexSetV(&IndexedPolyLine[i], 
            IndexedPolyLineData[i][3], recipCameraZ);
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedPolyLineSetColor(RwBool white)
{
    RwInt32 i;
    RwRGBA SolidColor1 = SolidBlue;
    
    if( white )
    {
        SolidColor1 = SolidWhite;
    }

    for(i=0; i<21; i++)       
    {
        RwIm2DVertexSetIntRGBA(&IndexedPolyLine[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }    

    return;
}


/*
 *****************************************************************************
 */
void
IndexedPolyLineRender(void)
{
    RwIm2DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE, 
        IndexedPolyLine, 21, IndexedPolyLineIndices, 46);

    return;
}

/*
 *****************************************************************************
 */
