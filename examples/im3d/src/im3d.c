
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
 * im3d.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate RenderWare's 3D immediate mode.
 *****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"

#include "im3d.h"

RwRGBA SolidWhite  = {255, 255, 255, 255};
RwRGBA SolidBlack  = {  0,   0,   0, 255};
RwRGBA SolidRed    = {200,  64,  64, 255};
RwRGBA SolidGreen  = { 64, 200,  64, 255};
RwRGBA SolidBlue   = { 64,  64, 200, 255};
RwRGBA SolidYellow = {200, 200,  64, 255};
RwRGBA SolidPurple = {200,  64, 200, 255};
RwRGBA SolidCyan   = { 64, 200, 200, 255};

RwInt32 Im3DPrimType = 0;
RwBool Im3DTextured = FALSE;
RwBool Im3DColored = TRUE;

static RwMatrix *Im3DMatrix = NULL;
static RwTexture *Im3DTexture = NULL;



/*
 *****************************************************************************
 */
void
Im3DRender(void)
{
    RwUInt32 transformFlags;
    static RwBool stateSet = FALSE;

    if( !stateSet )
    {
        /* 
         * Render State is persistent - only need to set it once...
         */
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);

        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);

        stateSet = TRUE;
    }
            
    if( Im3DTextured )
    {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 
            (void *)RwTextureGetRaster(Im3DTexture));

        transformFlags = rwIM3D_VERTEXUV | rwIM3D_ALLOPAQUE;
    }
    else
    {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

        transformFlags = rwIM3D_ALLOPAQUE;
    }  
    
    switch( Im3DPrimType )
    {
        case 0:
        {
            LineListRender(Im3DMatrix, transformFlags);
            break;
        }

        case 1:
        {
            IndexedLineListRender(Im3DMatrix, transformFlags); 
            break;
        }

        case 2:
        {
            PolyLineRender(Im3DMatrix, transformFlags);
            break;
        }

        case 3:
        {
            IndexedPolyLineRender(Im3DMatrix, transformFlags);
            break;
        }

        case 4:
        {
            TriListRender(Im3DMatrix, transformFlags);
            break;
        }

        case 5:
        {
            IndexedTriListRender(Im3DMatrix, transformFlags);
            break;
        }

        case 6:
        {     
            TriStripRender(Im3DMatrix, transformFlags);   
            break;
        }

        case 7:
        {    
            IndexedTriStripRender(Im3DMatrix, transformFlags);                 
            break;
        }      
        
        case 8:        
        {        
            TriFanRender(Im3DMatrix, transformFlags);
            break;
        }  

        case 9:        
        {
            IndexedTriFanRender(Im3DMatrix, transformFlags);         
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
Im3DRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix *cameraMatrix;
    RwV3d right, up, pos;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    pos = *RwMatrixGetPos(Im3DMatrix);

    /*
     * First translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(Im3DMatrix, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * ...do the rotations...
     */
    RwMatrixRotate(Im3DMatrix, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwMatrixRotate(Im3DMatrix, &right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * ...and translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(Im3DMatrix, &pos, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 ***************************************************************************
 */
void
Im3DTranslateZ(RwReal zDelta)
{
    RwFrame *cameraFrame;
    RwV3d delta;

    cameraFrame = RwCameraGetFrame(Camera);

    RwV3dScale(&delta, RwMatrixGetAt(RwFrameGetMatrix(cameraFrame)), zDelta); 

    RwMatrixTranslate(Im3DMatrix, &delta, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
RwBool 
Im3DInitialize(void)
{
    RwChar *path;

    /* 
     * Create the matrix for the IM geometry's transformation....
     */
    Im3DMatrix = RwMatrixCreate();

    if( Im3DMatrix == NULL )
    {
        return FALSE;
    }
    else
    {
        /*
         * Initialize the matrix so that it positions the IM geometry
         * in front of the camera...
         */
        RwMatrix *cameraMatrix;
        RwV3d *up, *at, pos;

        cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
        up = RwMatrixGetUp(cameraMatrix);
        at = RwMatrixGetAt(cameraMatrix);

        RwV3dScale(&pos, at, 5.0f);
        RwV3dAdd(&pos, &pos, RwMatrixGetAt(cameraMatrix));

        RwMatrixRotate(Im3DMatrix, up, 30.0f, rwCOMBINEREPLACE);  
        RwMatrixTranslate(Im3DMatrix, &pos, rwCOMBINEPOSTCONCAT);
    }
    
    /* 
     * Load the texture...
     */
    path = RsPathnameCreate(RWSTRING("textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);
     
    Im3DTexture = RwTextureRead(RWSTRING("whiteash"), NULL);
    if( Im3DTexture == NULL )
    {
        return FALSE;
    }

    /*
     * Create the different geometries...
     */
    LineListCreate();
    LineListSetColor(!Im3DColored);
    
    IndexedLineListCreate();
    IndexedLineListSetColor(!Im3DColored);

    PolyLineCreate();
    PolyLineSetColor(!Im3DColored);

    IndexedPolyLineCreate();
    IndexedPolyLineSetColor(!Im3DColored);

    TriListCreate();
    TriListSetColor(!Im3DColored);
    
    IndexedTriListCreate();
    IndexedTriListSetColor(!Im3DColored);
    
    TriStripCreate();
    TriStripSetColor(!Im3DColored);

    IndexedTriStripCreate();
    IndexedTriStripSetColor(!Im3DColored);    
    
    TriFanCreate();
    TriFanSetColor(!Im3DColored);
    
    IndexedTriFanCreate();
    IndexedTriFanSetColor(!Im3DColored);  
    
    return TRUE;
}


/*
 *****************************************************************************
 */
void 
Im3DTerminate(void)
{
    if( Im3DMatrix )
    {
        RwMatrixDestroy(Im3DMatrix);
    }

    if( Im3DTexture )
    {
        RwTextureDestroy(Im3DTexture);
    }

    return;
}

/*
 *****************************************************************************
 */
