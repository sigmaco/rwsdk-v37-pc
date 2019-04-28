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
 * Copyright (c) 2001 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *                                                                         
 * blend.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: To illustrate alpha blending between two IM2D 
 *          rendered geometries.
 ****************************************************************************/

#include "rwcore.h"

#include "blend.h"

#define NUMSRCVERTS (6)
#define NUMDESTVERTS (6)

#define SCALE (0.7f)

#define BLENDSRCALPHA (4)
#define BLENDINVSRCALPHA (5)

static RwIm2DVertex SrcGeometry[NUMSRCVERTS];
static RwIm2DVertex DestGeometry[NUMDESTVERTS];

static RwV2d ScreenSize;
static RwReal Scale;

static RwBlendFunction BlendFunctions[NUMBLENDFUNCTIONS] =
{
    rwBLENDZERO,
    rwBLENDONE,
    rwBLENDSRCCOLOR,
    rwBLENDINVSRCCOLOR,
    rwBLENDSRCALPHA,
    rwBLENDINVSRCALPHA,
    rwBLENDDESTALPHA,
    rwBLENDINVDESTALPHA,
    rwBLENDDESTCOLOR,
    rwBLENDINVDESTCOLOR,
    rwBLENDSRCALPHASAT
};

RwBool BlendMode[NUMBLENDFUNCTIONS][NUMBLENDFUNCTIONS];

RwInt32 SrcBlendID;
RwInt32 DestBlendID;

RwRGBAReal SrcColor  = {0.00f, 0.75f, 0.75f, 0.5f};
RwRGBAReal DestColor = {0.75f, 0.75f, 0.00f, 1.0f};



/*
 *****************************************************************************
 */
void
QueryBlendFunctionInfo(void)
{
    RwUInt8 i, j;

    for(i=0; i<NUMBLENDFUNCTIONS; i++)
    {
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);

        if( RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)BlendFunctions[i]) )
        {
            for(j=0; j<NUMBLENDFUNCTIONS; j++)
            {
                if( RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)BlendFunctions[j]) )
                {
                    BlendMode[i][j] = TRUE;
                }
                else
                {
                    BlendMode[i][j] = FALSE;
                }
            }
        }
        else
        {
            for(j=0; j<NUMBLENDFUNCTIONS; j++)
            {
                BlendMode[i][j] = FALSE;
            }
        }
    }

    /*
     * Set the initial blend modes to SRCALPHA and INVSRCALPHA,
     * if available, otherwise use the first available conbination...
     */
    if( BlendMode[BLENDSRCALPHA][BLENDINVSRCALPHA] )
    {
        SrcBlendID = BLENDSRCALPHA;
        DestBlendID = BLENDINVSRCALPHA;
    }
    else
    {
        for(i=0; i<NUMBLENDFUNCTIONS; i++)
        {
            for(j=0; j<NUMBLENDFUNCTIONS; j++)
            {
                if( BlendMode[i][j] )
                {
                    SrcBlendID = i;
                    DestBlendID = j;

                    break;
                }
            }
        }
    }

    return;
}


/*
 *****************************************************************************
 */
static void
Im2DGeometryCreate(RwCamera *camera)
{
    RwV2d srcData[NUMSRCVERTS] =
    {
        { 0.6f,  0.6f},
        {-0.6f,  0.6f},
        {-0.6f, -0.6f},

        { 0.6f,  0.6f},
        {-0.6f, -0.6f},
        { 0.6f, -0.6f}
    };

    RwV2d destData[NUMDESTVERTS] =
    {
        { 1.3f,  0.0f},
        { 0.0f,  1.3f},
        {-1.3f,  0.0f},

        { 1.3f,  0.0f},
        {-1.3f,  0.0f},
        { 0.0f, -1.3f}
    };

    RwInt32 i;
    RwReal zPos, recipZ;
    RwRGBA color;

    /*
     * Place the 'source' geometry on the near clip-plane...
     */
    zPos = RwIm2DGetNearScreenZ();
    recipZ = 1.0f / RwCameraGetNearClipPlane(camera);

    color.red   = (RwUInt8)(255.0f * SrcColor.red);
    color.green = (RwUInt8)(255.0f * SrcColor.green);
    color.blue  = (RwUInt8)(255.0f * SrcColor.blue);
    color.alpha = (RwUInt8)(255.0f * SrcColor.alpha);

    for(i=0; i<NUMSRCVERTS; i++)
    {
        RwReal xPos, yPos;

        xPos = ScreenSize.x * 0.5f + srcData[i].x * Scale;
        yPos = ScreenSize.y - ScreenSize.y * 0.5f - srcData[i].y * Scale;

        RwIm2DVertexSetScreenX(&SrcGeometry[i], xPos);
        RwIm2DVertexSetScreenY(&SrcGeometry[i], yPos);
        RwIm2DVertexSetScreenZ(&SrcGeometry[i], zPos);

        RwIm2DVertexSetRecipCameraZ(&SrcGeometry[i], recipZ);

        RwIm2DVertexSetIntRGBA(&SrcGeometry[i], 
            color.red, color.green, color.blue, color.alpha);
    }

    /*
     * Place the 'destination' geometry on the far clip-plane...
     */
    zPos = RwIm2DGetFarScreenZ();
    recipZ = 1.0f / RwCameraGetFarClipPlane(camera);

    color.red   = (RwUInt8)(255.0f * DestColor.red);
    color.green = (RwUInt8)(255.0f * DestColor.green);
    color.blue  = (RwUInt8)(255.0f * DestColor.blue);
    color.alpha = (RwUInt8)(255.0f * DestColor.alpha);

    for(i=0; i<NUMDESTVERTS; i++)
    {
        RwReal xPos, yPos;

        xPos = ScreenSize.x * 0.5f + destData[i].x * Scale;
        yPos = ScreenSize.y - ScreenSize.y * 0.5f - destData[i].y * Scale;

        RwIm2DVertexSetScreenX(&DestGeometry[i], xPos);
        RwIm2DVertexSetScreenY(&DestGeometry[i], yPos);
        RwIm2DVertexSetScreenZ(&DestGeometry[i], zPos);

        RwIm2DVertexSetRecipCameraZ(&DestGeometry[i], recipZ);

        RwIm2DVertexSetIntRGBA(&DestGeometry[i], 
            color.red, color.green, color.blue, color.alpha);
    }

    return;
}


/*
 *****************************************************************************
 */
void 
UpdateSrcGeometryColor(RwRGBAReal *scrColor)
{
    RwInt32 i;
    RwRGBA color;

    color.red   = (RwUInt8)(255.0f * scrColor->red);
    color.green = (RwUInt8)(255.0f * scrColor->green);
    color.blue  = (RwUInt8)(255.0f * scrColor->blue);
    color.alpha = (RwUInt8)(255.0f * scrColor->alpha);

    for(i=0; i<NUMSRCVERTS; i++)
    {
        RwIm2DVertexSetIntRGBA(&SrcGeometry[i], 
            color.red, color.green, color.blue, color.alpha);
    }

    return;
}


/*
 *****************************************************************************
 */
void 
UpdateDestGeometryColor(RwRGBAReal *destColor)
{
    RwInt32 i;
    RwRGBA color;

    color.red   = (RwUInt8)(255.0f * destColor->red);
    color.green = (RwUInt8)(255.0f * destColor->green);
    color.blue  = (RwUInt8)(255.0f * destColor->blue);
    color.alpha = (RwUInt8)(255.0f * destColor->alpha);

    for(i=0; i<NUMDESTVERTS; i++)
    {
        RwIm2DVertexSetIntRGBA(&DestGeometry[i], 
            color.red, color.green, color.blue, color.alpha);
    }

    return;
}


/*
 *****************************************************************************
 */
RwBool 
Im2DInitialize(RwCamera *camera)
{
    ScreenSize.x = (RwReal)RwRasterGetWidth(RwCameraGetRaster(camera));
    ScreenSize.y = (RwReal)RwRasterGetHeight(RwCameraGetRaster(camera));

    Scale = 0.5f * ScreenSize.y * SCALE;

    /*
     * Create geometry...
     */
    Im2DGeometryCreate(camera);

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
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);

        stateSet = TRUE;
    }

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);

    /*
     * Draw the first geometry WITHOUT blending...
     */
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);

    RwIm2DRenderPrimitive(rwPRIMTYPETRILIST, DestGeometry, NUMDESTVERTS);

    /*
     * Draw the second geometry WITH blending...
     */
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)BlendFunctions[SrcBlendID]);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)BlendFunctions[DestBlendID]);

    RwIm2DRenderPrimitive(rwPRIMTYPETRILIST, SrcGeometry, NUMSRCVERTS);

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
        Scale = 0.5f * ScreenSize.y * SCALE;
    }
    else
    {
        Scale = 0.5f * ScreenSize.x * SCALE;
    }

    /*
     * Re-create geometry...
     */
    Im2DGeometryCreate(camera);

    return;
}

/*
 *****************************************************************************
 */
