
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
 * Purpose: To demonstrate RenderWare's 2D immediate mode.
 * *****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"

#include "im2d.h"


/*
 * rwPRIMTYPELINELIST geometry data - star shape.
 * (X, Y, U, V).
 */
static RwReal LineListData[32][4] = 
{
    { 0.000f,  0.000f,  0.000f,  0.000f},
    { 0.000f,  1.000f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f},
    { 0.383f,  0.924f,  1.000f,  1.000f},
    
    { 0.000f,  0.000f,  0.000f,  0.000f},
    { 0.707f,  0.707f,  1.000f,  1.000f},
    
    { 0.000f,  0.000f,  0.000f,  0.000f},
    { 0.924f,  0.383f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f},
    { 1.000f,  0.000f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f},
    { 0.924f, -0.383f,  1.000f,  1.000f},
    
    { 0.000f,  0.000f,  0.000f,  0.000f},
    { 0.707f, -0.707f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f},
    { 0.383f, -0.924f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f},
    { 0.000f, -1.000f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f},
    {-0.383f, -0.924f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f},
    {-0.707f, -0.707f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f},
    {-0.924f, -0.383f,  1.000f,  1.000f},
    
    { 0.000f,  0.000f,  0.000f,  0.000f},
    {-1.000f,  0.000f,  1.000f,  1.000f},    

    { 0.000f,  0.000f,  0.000f,  0.000f},
    {-0.924f,  0.383f,  1.000f,  1.000f},
    
    { 0.000f,  0.000f,  0.000f,  0.000f},
    {-0.707f,  0.707f,  1.000f,  1.000f},

    { 0.000f,  0.000f,  0.000f,  0.000f},
    {-0.383f,  0.924f,  1.000f,  1.000f}
};


/*
 * Indexed rwPRIMTYPELINELIST geometry data - grid shape.
 * (X, Y, U, V).
 */
static RwReal IndexedLineListData[16][4] = 
{
    {-1.000f,  1.000f,  0.000f,  1.000f},
    {-0.500f,  1.000f,  0.250f,  1.000f},
    { 0.000f,  1.000f,  0.500f,  1.000f},    
    { 0.500f,  1.000f,  0.750f,  1.000f},        
    { 1.000f,  1.000f,  1.000f,  1.000f},            
    
    {-1.000f,  0.500f,  0.000f,  0.750f},            
    {-1.000f,  0.000f,  0.000f,  0.500f},            
    {-1.000f, -0.500f,  0.000f,  0.250f},                

    { 1.000f,  0.500f,  1.000f,  0.750f},            
    { 1.000f,  0.000f,  1.000f,  0.500f},            
    { 1.000f, -0.500f,  1.000f,  0.250f},                

    {-1.000f, -1.000f,  0.000f,  0.000f},
    {-0.500f, -1.000f,  0.250f,  0.000f},
    { 0.000f, -1.000f,  0.500f,  0.000f},    
    { 0.500f, -1.000f,  0.750f,  0.000f},        
    { 1.000f, -1.000f,  1.000f,  0.000f}            
};


static RwImVertexIndex IndexedLineListIndices[20] = 
{
    0, 11, 1, 12, 2, 13, 3, 14, 4, 15, 
    0, 4, 5, 8, 6, 9, 7, 10, 11, 15
};


static RwIm2DVertex LineList[32];
static RwIm2DVertex IndexedLineList[16];



/*
 *****************************************************************************
 */
void
LineListCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ = 1.0f / RwCameraGetNearClipPlane(camera);

    /* 
     * Setup rwPRIMTYPELINELIST geometry, based on vertex data...
     */
    for(i=0; i<32; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&LineList[i], 
            (ScreenSize.x/2.0f) + (LineListData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&LineList[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (LineListData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&LineList[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&LineList[i], recipCameraZ);
        
        RwIm2DVertexSetU(&LineList[i], LineListData[i][2], recipCameraZ);
        RwIm2DVertexSetV(&LineList[i], LineListData[i][3], recipCameraZ);
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
    for(i=0; i<32; i+=2)       
    {
        RwIm2DVertexSetIntRGBA(&LineList[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
        
        RwIm2DVertexSetIntRGBA(&LineList[i+1], 
            SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);            
    }

    return;
}


/*
 *****************************************************************************
 */
void
LineListRender(void)
{   
    RwIm2DRenderPrimitive(rwPRIMTYPELINELIST, LineList, 32);

    return;
}


/*
 *****************************************************************************
 */
void
IndexedLineListCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ =  1.0f / RwCameraGetNearClipPlane(camera);

    /* 
     * Setup indexed rwPRIMTYPELINELIST geometry, based on vertex data...
     */
    for(i=0; i<16; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&IndexedLineList[i], 
            (ScreenSize.x/2.0f) + (IndexedLineListData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&IndexedLineList[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (IndexedLineListData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&IndexedLineList[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&IndexedLineList[i], recipCameraZ);
        
        RwIm2DVertexSetU(&IndexedLineList[i], 
            IndexedLineListData[i][2], recipCameraZ);

        RwIm2DVertexSetV(&IndexedLineList[i], 
            IndexedLineListData[i][3], recipCameraZ);
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
    
    if( white )
    {
        SolidColor1 = SolidWhite;
    }

    for(i=0; i<16; i++)       
    {
        RwIm2DVertexSetIntRGBA(&IndexedLineList[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }    

    return;
}


/*
 *****************************************************************************
 */
void
IndexedLineListRender(void)
{
    RwIm2DRenderIndexedPrimitive(rwPRIMTYPELINELIST, 
        IndexedLineList, 16, IndexedLineListIndices, 20);

    return;
}

/*
 *****************************************************************************
 */
