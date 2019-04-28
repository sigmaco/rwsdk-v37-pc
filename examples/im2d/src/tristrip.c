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
 * Purpose: To demonstrate RenderWare's 2D immediate mode.
 * *****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"

#include "im2d.h"


/*
 * rwPRIMTYPETRISTRIP geometry data - ring shape.
 * (X, Y, U, V).
 */
static RwReal TriStripData[18][4] =
{
    { 0.000f,  1.000f,  0.500f,  1.000f},
    { 0.000f,  0.500f,  0.500f,  0.750f},    

    { 0.707f,  0.707f,  0.853f,  0.853f},    
    { 0.354f,  0.354f,  0.677f,  0.677f},

    { 1.000f,  0.000f,  1.000f,  0.500f},
    { 0.500f,  0.000f,  0.750f,  0.500f},
    
    { 0.707f, -0.707f,  0.853f,  0.147f},    
    { 0.354f, -0.354f,  0.677f,  0.323f},

    { 0.000f, -1.000f,  0.500f,  0.000f},
    { 0.000f, -0.500f,  0.500f,  0.250f},
    
    {-0.707f, -0.707f,  0.147f,  0.147f},    
    {-0.354f, -0.354f,  0.323f,  0.323f},

    {-1.000f,  0.000f,  0.000f,  0.500f},
    {-0.500f,  0.000f,  0.250f,  0.500f},
    
    {-0.707f,  0.707f,  0.147f,  0.853f},    
    {-0.354f,  0.354f,  0.323f,  0.677f},

    { 0.000f,  1.000f,  0.500f,  1.000f},
    { 0.000f,  0.500f,  0.500f,  0.750f}   
};


/*
 * Indexed rwPRIMTYPETRISTRIP geometry data - ring shape.
 * (X, Y, U, V).
 */
static RwReal IndexedTriStripData[16][4] =
{
    { 0.000f,  1.000f,  0.500f,  1.000f},
    { 0.707f,  0.707f,  0.853f,  0.853f},    
    { 1.000f,  0.000f,  1.000f,  0.500f},
    { 0.707f, -0.707f,  0.853f,  0.147f},    
    { 0.000f, -1.000f,  0.500f,  0.000f},
    {-0.707f, -0.707f,  0.147f,  0.147f},    
    {-1.000f,  0.000f,  0.000f,  0.500f},    
    {-0.707f,  0.707f,  0.147f,  0.853f},    
    
    { 0.000f,  0.500f,  0.500f,  0.750f},    
    { 0.354f,  0.354f,  0.677f,  0.677f},    
    { 0.500f,  0.000f,  0.750f,  0.500f},    
    { 0.354f, -0.354f,  0.677f,  0.323f},
    { 0.000f, -0.500f,  0.500f,  0.250f},        
    {-0.354f, -0.354f,  0.323f,  0.323f},    
    {-0.500f,  0.000f,  0.250f,  0.500f},    
    {-0.354f,  0.354f,  0.323f,  0.677f}    
}; 


static RwImVertexIndex IndexedTriStripIndices[18] = 
{
    0, 8,  1, 9,  2, 10, 3, 11,
    4, 12, 5, 13, 6, 14, 7, 15, 0, 8
};


static RwIm2DVertex TriStrip[18];
static RwIm2DVertex IndexedTriStrip[16];


/*
 *****************************************************************************
 */
void
TriStripCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ = 1.0f / RwCameraGetNearClipPlane(camera);

    /* 
     * Setup rwPRIMTYPETRISTRIP geometry, based on vertex data...
     */
    for(i=0; i<18; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&TriStrip[i],
            (ScreenSize.x/2.0f) + (TriStripData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&TriStrip[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (TriStripData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&TriStrip[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&TriStrip[i], recipCameraZ);
        
        RwIm2DVertexSetU(&TriStrip[i], TriStripData[i][2], recipCameraZ);
        RwIm2DVertexSetV(&TriStrip[i], TriStripData[i][3], recipCameraZ);
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
    RwRGBA SolidColor1 = SolidPurple;
    
    if( white )
    {
        SolidColor1 = SolidWhite;
    }

    for(i=0; i<18; i++)       
    {
        RwIm2DVertexSetIntRGBA(&TriStrip[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }    

    return;
}


/*
 *****************************************************************************
 */
void
TriStripRender(void)
{           
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, TriStrip, 18);

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriStripCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ = 1.0f / RwCameraGetNearClipPlane(camera);

    /* 
     * Setup indexed rwPRIMTYPETRISTRIP geometry, based on vertex data...
     */
    for(i=0; i<16; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&IndexedTriStrip[i],
            (ScreenSize.x/2.0f) + (IndexedTriStripData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&IndexedTriStrip[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (IndexedTriStripData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&IndexedTriStrip[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&IndexedTriStrip[i], recipCameraZ);
        
        RwIm2DVertexSetU(&IndexedTriStrip[i], 
            IndexedTriStripData[i][2], recipCameraZ);
        
        RwIm2DVertexSetV(&IndexedTriStrip[i], 
            IndexedTriStripData[i][3], recipCameraZ);
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
    RwRGBA SolidColor1 = SolidCyan;
    
    if( white )
    {
        SolidColor1 = SolidWhite;
    }

    for(i=0; i<16; i++)       
    {
        RwIm2DVertexSetIntRGBA(&IndexedTriStrip[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }    

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriStripRender(void)
{    
    RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRISTRIP, 
        IndexedTriStrip, 16, IndexedTriStripIndices, 18);

    return;
}


/*
 *****************************************************************************
 */
