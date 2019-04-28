
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
 * im2d.c
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

RwBool Im2DTextured = FALSE;
RwBool Im2DColored = TRUE;

RwInt32 Im2DPrimType = 0;

RwV2d  ScreenSize;
RwReal Scale;

RwRGBA SolidWhite  = {255, 255, 255, 255};
RwRGBA SolidBlack  = {  0,   0,   0, 255};
RwRGBA SolidRed    = {200,  64,  64, 255};
RwRGBA SolidGreen  = { 64, 200,  64, 255};
RwRGBA SolidBlue   = { 64,  64, 200, 255};
RwRGBA SolidYellow = {200, 200,  64, 255};
RwRGBA SolidPurple = {200,  64, 200, 255};
RwRGBA SolidCyan   = { 64, 200, 200, 255};

static RwTexture *Im2DTexture = NULL;


/*
 *****************************************************************************
 */
RwBool 
Im2DInitialize(RwCamera *camera)
{
    RwChar *path;

    ScreenSize.x = (RwReal)RwRasterGetWidth(RwCameraGetRaster(camera));
    ScreenSize.y = (RwReal)RwRasterGetHeight(RwCameraGetRaster(camera));
        
    Scale = ScreenSize.y / 3.0f;

    /* 
     * Load texture...
     */
    path = RsPathnameCreate(RWSTRING("textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);
     
    Im2DTexture = RwTextureRead(RWSTRING("whiteash"), NULL);
    if( Im2DTexture == NULL )
    {
        return FALSE;
    }

    /*
     * Create geometry...
     */
    LineListCreate(camera);
    LineListSetColor(!Im2DColored);   
     
    IndexedLineListCreate(camera);
    IndexedLineListSetColor(!Im2DColored);
    
    PolyLineCreate(camera);
    PolyLineSetColor(!Im2DColored);    
    
    IndexedPolyLineCreate(camera);
    IndexedPolyLineSetColor(!Im2DColored);  
          
    TriListCreate(camera);
    TriListSetColor(!Im2DColored);
    
    IndexedTriListCreate(camera);
    IndexedTriListSetColor(!Im2DColored);
    
    TriStripCreate(camera);
    TriStripSetColor(!Im2DColored);    
    
    IndexedTriStripCreate(camera);
    IndexedTriStripSetColor(!Im2DColored);  
          
    TriFanCreate(camera);
    TriFanSetColor(!Im2DColored);
    
    IndexedTriFanCreate(camera);
    IndexedTriFanSetColor(!Im2DColored); 
           
    return TRUE;
}


/*
 *****************************************************************************
 */
void
Im2DRender(void)
{
    static RwBool stateSet = FALSE;

    if( !stateSet )
    {
        /* 
         * Render state is persistent - only need to set it once...
         */
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *) rwSHADEMODEGOURAUD);

        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) FALSE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) FALSE);

        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);

        stateSet = TRUE;
    }

    if( Im2DTextured )
    {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 
            (void *)RwTextureGetRaster(Im2DTexture));        
    }
    else
    {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);    
    }        

    switch( Im2DPrimType )
    {
        case 0:
        {
            LineListRender(); 
            break;
        }

        case 1:
        {
            IndexedLineListRender();
            break;
        }

        case 2:
        {
            PolyLineRender(); 
            break;
        }

        case 3:
        {
            IndexedPolyLineRender();
            break;
        }

        case 4:
        {
            TriListRender();
            break;
        }    
        
        case 5:
        { 
            IndexedTriListRender();                     
            break;
        }

        case 6:
        {
            TriStripRender();
            break;
        }

        case 7:
        {
            IndexedTriStripRender();             
            break;
        }

        case 8:
        {
            TriFanRender();
            break;
        }

        case 9:
        {
            IndexedTriFanRender();
            break;
        }

        default:
        {
            break;
        }    
    }

    return;
}

/*
 *****************************************************************************
 */
void
Im2DSize(RwCamera *camera, RwInt32 width, RwInt32 height)
{
    ScreenSize.x = (RwReal)width;
    ScreenSize.y = (RwReal)height;

    if( ScreenSize.x > ScreenSize.y )
    {        
        Scale = ScreenSize.y / 3.0f;
    }
    else
    {
        Scale = ScreenSize.x / 3.0f;    
    }        

    /*
     * Re-create geometry...
     */
    LineListCreate(camera);
     
    IndexedLineListCreate(camera);
    
    PolyLineCreate(camera);
    
    IndexedPolyLineCreate(camera);
          
    TriListCreate(camera);
    
    IndexedTriListCreate(camera);
    
    TriStripCreate(camera);
    
    IndexedTriStripCreate(camera);
          
    TriFanCreate(camera);
    
    IndexedTriFanCreate(camera);
}


/*
 *****************************************************************************
 */
void 
Im2DTerminate(void)
{
    if( Im2DTexture )
    {
        RwTextureDestroy(Im2DTexture);
    }

    return;
}

/*
 *****************************************************************************
 */
