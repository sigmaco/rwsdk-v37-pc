
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
 * Purpose: To demonstrate RenderWare's 2D immediate mode.
 * *****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"

#include "im2d.h"


/*
 * rwPRIMTYPETRILIST geometry data - square (six triangles).
 * (X, Y, U, V).
 */
static RwReal TriListData[18][4] =
{
    { 0.000f,  1.000f,  0.500f,  1.000f},   
    {-0.500f,  0.500f,  0.250f,  0.750f},   
    { 0.500f,  0.500f,  0.750f,  0.750f},       
    
    {-0.500f,  0.500f,  0.250f,  0.750f},   
    { 0.500f, -0.500f,  0.750f,  0.250f},   
    { 0.500f,  0.500f,  0.750f,  0.750f},           
    
    { 0.500f,  0.500f,  0.750f,  0.750f},           
    { 0.500f, -0.500f,  0.750f,  0.250f},           
    { 1.000f,  0.000f,  1.000f,  0.500f},           
       
    { 0.500f, -0.500f,  0.750f,  0.250f},   
    {-0.500f, -0.500f,  0.250f,  0.250f},   
    { 0.000f, -1.000f,  0.500f,  0.000f},   
    
    { 0.500f, -0.500f,  0.750f,  1.250f},   
    {-0.500f,  0.500f,  0.250f,  1.750f},   
    {-0.500f, -0.500f,  0.250f,  1.250f},   
    
    {-0.500f, -0.500f,  0.250f,  0.250f},   
    {-0.500f,  0.500f,  0.250f,  0.750f},   
    {-1.000f,  0.000f,  0.000f,  0.500f}     
}; 


/*
 * Indexed rwPRIMTYPETRILIST geometry data - triangle of triangles.
 * (X, Y, U, V).
 */
static RwReal IndexedTriListData[21][4] =
{
    { 0.000f,  1.000f,  0.500f,  1.000f},   
    
    {-0.200f,  0.600f,  0.400f,  0.800f},    
    { 0.200f,  0.600f,  0.600f,  0.800f},
        
    {-0.400f,  0.200f,  0.300f,  0.600f},
    { 0.000f,  0.200f,  0.500f,  0.600f},
    { 0.400f,  0.200f,  0.300f,  0.600f},    
    
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


static RwImVertexIndex IndexedTriListIndices[45] = 
{
    0, 1, 2,
    1, 3, 4, 2, 4, 5, 
    3, 6, 7, 4, 7, 8, 5, 8, 9, 
    6, 10, 11, 7, 11, 12, 8, 12, 13, 9, 13, 14,
    10, 15, 16, 11, 16, 17, 12, 17, 18, 13, 18, 19, 14, 19, 20
};


static RwIm2DVertex TriList[18];
static RwIm2DVertex IndexedTriList[21];



/*
 *****************************************************************************
 */
void
TriListCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ = 1.0f / RwCameraGetNearClipPlane(camera);

    /* 
     * Setup rwPRIMTYPETRILIST geometry, based on vertex data...
     */
    for(i=0; i<18; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&TriList[i], 
            (ScreenSize.x/2.0f) + (TriListData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&TriList[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (TriListData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&TriList[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&TriList[i], recipCameraZ);
        
        RwIm2DVertexSetU(&TriList[i], TriListData[i][2], recipCameraZ);
        RwIm2DVertexSetV(&TriList[i], TriListData[i][3], recipCameraZ);
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

    RwRGBA SolidColor1 = SolidBlue;
    RwRGBA SolidColor2 = SolidRed;
    RwRGBA SolidColor3 = SolidGreen;
    RwRGBA SolidColor4 = SolidYellow;
    RwRGBA SolidColor5 = SolidPurple;
    RwRGBA SolidColor6 = SolidCyan;    
    
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
     * Every triangle is a different color...
     */
    for(i=0; i<3; i++)       
    {
        RwIm2DVertexSetIntRGBA(&TriList[i], SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }
    
    for(i=3; i<6; i++)       
    {
        RwIm2DVertexSetIntRGBA(&TriList[i], SolidColor2.red, SolidColor2.green,
            SolidColor2.blue, SolidColor2.alpha);
    }
    
    for(i=6; i<9; i++)       
    {
        RwIm2DVertexSetIntRGBA(&TriList[i], SolidColor3.red, SolidColor3.green,
            SolidColor3.blue, SolidColor3.alpha);
    }
    
    for(i=9; i<12; i++)       
    {
        RwIm2DVertexSetIntRGBA(&TriList[i], SolidColor4.red, SolidColor4.green,
            SolidColor4.blue, SolidColor4.alpha);
    }
    
    for(i=12; i<15; i++)       
    {
        RwIm2DVertexSetIntRGBA(&TriList[i], SolidColor5.red, SolidColor5.green,
            SolidColor5.blue, SolidColor5.alpha);
    } 
    
    for(i=15; i<18; i++)       
    {
        RwIm2DVertexSetIntRGBA(&TriList[i], SolidColor6.red, SolidColor6.green,
            SolidColor6.blue, SolidColor6.alpha);
    }    

    return;
}


/*
 *****************************************************************************
 */
void
TriListRender(void)
{
    RwIm2DRenderPrimitive(rwPRIMTYPETRILIST, TriList, 18);

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriListCreate(RwCamera *camera)
{
    RwInt32 i;   
    RwReal recipCameraZ = 1.0f / RwCameraGetNearClipPlane(camera);

    /* 
     * Setup indexed rwPRIMTYPETRILIST geometry, based on vertex data.
     */
    for(i=0; i<21; i++)
    {
        /*
         * Scale, position, and convert co-ordinate system...
         */       
        RwIm2DVertexSetScreenX(&IndexedTriList[i], 
            (ScreenSize.x/2.0f) + (IndexedTriListData[i][0] * Scale));
        
        RwIm2DVertexSetScreenY(&IndexedTriList[i], 
            ScreenSize.y - ((ScreenSize.y/2.0f) + (IndexedTriListData[i][1] * Scale)));
        
        RwIm2DVertexSetScreenZ(&IndexedTriList[i], RwIm2DGetNearScreenZ());
        
        RwIm2DVertexSetRecipCameraZ(&IndexedTriList[i], recipCameraZ);
        
        RwIm2DVertexSetU(&IndexedTriList[i], 
            IndexedTriListData[i][2], recipCameraZ);
        
        RwIm2DVertexSetV(&IndexedTriList[i], 
            IndexedTriListData[i][3], recipCameraZ);
    }

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriListSetColor(RwBool white)
{
    RwInt32 i;
    RwRGBA SolidColor1 = SolidBlue;
    
    if( white )
    {
        SolidColor1 = SolidWhite;
    }

    for(i=0; i<21; i++)       
    {
        RwIm2DVertexSetIntRGBA(&IndexedTriList[i], 
            SolidColor1.red, SolidColor1.green,
            SolidColor1.blue, SolidColor1.alpha);
    }    

    return;
}


/*
 *****************************************************************************
 */
void
IndexedTriListRender(void)
{    
    RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, 
        IndexedTriList, 21, IndexedTriListIndices, 45);

    return;
}


/*
 *****************************************************************************
 */
