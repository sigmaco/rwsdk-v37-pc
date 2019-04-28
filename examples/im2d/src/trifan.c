
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
 * Purpose: To demonstrate RenderWare's 2D immediate mode.
 * *****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"

#include "im2d.h"


/*
 * rwPRIMTYPETRIFAN geometry data - disk shape.
 * (X, Y, U, V).
 */
static RwReal TriFanData[17][4] =
{
    { 0.000f,  1.000f,  0.500f,  1.000f},
    {-0.383f,  0.924f,  0.308f,  0.962f},    
    {-0.707f,  0.707f,  0.146f,  0.854f},    
    {-0.924f,  0.383f,  0.038f,  0.692f},        
    {-1.000f,  0.000f,  0.000f,  0.500f},
    {-0.924f, -0.383f,  0.038f,  0.308f},        
    {-0.707f, -0.707f,  0.146f,  0.146f},    
    {-0.383f, -0.924f,  0.308f,  0.038f},        
    { 0.000f, -1.000f,  0.500f,  0.000f},
    { 0.383f, -0.924f,  0.692f,  0.038f},        
    { 0.707f, -0.707f,  0.854f,  0.146f},    
    { 0.924f, -0.383f,  0.962f,  0.308f},        
    { 1.000f,  0.000f,  1.000f,  0.500f},
    { 0.924f,  0.383f,  0.962f,  0.692f},        
    { 0.707f,  0.707f,  0.854f,  0.854f},    
    { 0.383f,  0.924f,  0.692f,  0.962f},        
    { 0.000f,  1.000f,  0.500f,  1.000f},
}; 


/*
 * Indexed rwPRIMTYPETRIFAN geometry data - disk shape.
 * (X, Y, U, V).
 */
static RwReal IndexedTriFanData[16][4] =
{
    { 0.000f,  1.000f,  0.500f,  1.000f},
    {-0.383f,  0.924f,  0.308f,  0.962f},    
    {-0.707f,  0.707f,  0.146f,  0.854f},    
    {-0.924f,  0.383f,  0.038f,  0.692f},        
    {-1.000f,  0.000f,  0.000f,  0.500f},
    {-0.924f, -0.383f,  0.038f,  0.308f},        
    {-0.707f, -0.707f,  0.146f,  0.146f},    
    {-0.383f, -0.924f,  0.308f,  0.038f},        
    { 0.000f, -1.000f,  0.500f,  0.000f},
    { 0.383f, -0.924f,  0.692f,  0.038f},        
    { 0.707f, -0.707f,  0.854f,  0.146f},    
    { 0.924f, -0.383f,  0.962f,  0.308f},        
    { 1.000f,  0.000f,  1.000f,  0.500f},
    { 0.924f,  0.383f,  0.962f,  0.692f},        
    { 0.707f,  0.707f,  0.854f,  0.854f},    
    { 0.383f,  0.924f,  0.692f,  0.962f},        
}; 


static RwImVertexIndex IndexedTriFanIndices[17] = 
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0
};


static RwIm2DVertex TriFan[17];
static RwIm2DVertex IndexedTriFan[16];



/*
 *****************************************************************************
 */
void
TriFanCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ = 1.0f / RwCameraGetNearClipPlane(camera);

    /* 
     * Setup rwPRIMTYPETRIFAN geometry, based on vertex data...
     */
    for(i=0; i<17; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&TriFan[i], 
            (ScreenSize.x/2.0f) + (TriFanData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&TriFan[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (TriFanData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&TriFan[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&TriFan[i], recipCameraZ);
        
        RwIm2DVertexSetU(&TriFan[i], TriFanData[i][2], recipCameraZ);
        RwIm2DVertexSetV(&TriFan[i], TriFanData[i][3], recipCameraZ);
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
    
    if( white )
    {
        SolidColor1 = SolidWhite;
    }

    for(i=0; i<17; i++)       
    {
        RwIm2DVertexSetIntRGBA(&TriFan[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }    

    return;
}


/*
 *****************************************************************************
 */
void
TriFanRender(void)
{
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, TriFan, 17);

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriFanCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ = 1.0f / RwCameraGetNearClipPlane(camera);

    /* 
     * Setup indexed rwPRIMTYPETRIFAN geometry, based on vertex data...
     */
    for(i=0; i<16; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&IndexedTriFan[i], 
            (ScreenSize.x/2.0f) + (IndexedTriFanData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&IndexedTriFan[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (IndexedTriFanData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&IndexedTriFan[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&IndexedTriFan[i], recipCameraZ);
        
        RwIm2DVertexSetU(&IndexedTriFan[i], 
            IndexedTriFanData[i][2], recipCameraZ);
        
        RwIm2DVertexSetV(&IndexedTriFan[i], 
            IndexedTriFanData[i][3], recipCameraZ);
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
    
    if( white )
    {
        SolidColor1 = SolidWhite;
    }

    for(i=0; i<16; i++)       
    {
        RwIm2DVertexSetIntRGBA(&IndexedTriFan[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }    

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriFanRender(void)
{    
    RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRIFAN, 
        IndexedTriFan, 16, IndexedTriFanIndices, 17);

    return;
}

/*
 *****************************************************************************
 */
