
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
 * Copyright (c) 2003 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * PickUp->c
 *
 * Copyright (C) 2003 Criterion Technologies.
 *
 * Original author: RenderWare Team.
 *
 * Purpose: demonstrate a health pickup effects, using one ptank atomic per 
 * pickup.
 * 
 ****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"

#include "pickup.h"

#define ANIMATION_NUM_FRAME         (60)
#define ANIMATION_FRAME_RATE        (1.0f/60.0f)


/*
 *****************************************************************************
 */
static void
PickUpPtankInit(RpAtomic *ptank, RwReal radius, RwInt32 numPrt)
{
    RpPTankLockStruct shotPositionLock;
    RwV3d *posOut;
    RwInt32 i;
    RwReal angle = 0.0f;
    RwReal deltaA = (4.0f*_RW_pi / (RwReal)numPrt);
    RwMatrix *matrix = RwFrameGetLTM(RpAtomicGetFrame(ptank));
    
    /*
     * Lock the PTank position for writing 
     */
    RpPTankAtomicLock(ptank, &shotPositionLock, 
                            rpPTANKLFLAGPOSITION, 
                            rpPTANKLOCKWRITE);

    for(i=0;i<numPrt/2;i++)
    {
        /*
         * Add a particle to the first circle
         */
        posOut = (RwV3d*)shotPositionLock.data;
        shotPositionLock.data += shotPositionLock.stride;
        
        posOut->x = (RwReal)RwSin(angle) * radius;
        posOut->y = (RwReal)RwCos(angle) * radius;
        posOut->z = 0.0f;
     
        RwV3dTransformPoint(posOut,
        posOut,matrix);

        /*
         * Add a particle to the second circle
         */
        posOut = (RwV3d*)shotPositionLock.data;
        shotPositionLock.data += shotPositionLock.stride;
        
        posOut->x = 0.0f;
        posOut->y = (RwReal)RwSin(angle) * radius;
        posOut->z = (RwReal)RwCos(angle) * radius;

        RwV3dTransformPoint(posOut,posOut,matrix);

        angle += deltaA;
    }

    /*
     * Unlock the ptank 
     */
    RpPTankAtomicUnlock(ptank);

    /*
     * Set the number of active particles in the Ptank.
     */
    RpPTankAtomicSetActiveParticlesCount(ptank,numPrt);

}



/*
 *****************************************************************************
 */
PickUpObj *
PickUpCreate(RwFrame *frame, RwTexture *prtTexture, RwReal radius, RwInt32 numPrt, RwRGBA *color1, RwRGBA *color2)
{
    RwTexCoords cUV[2] = { { 0.0f, 0.0f } , { 1.0f, 1.0f } };
    RwV2d cSize = { 8.0f, 8.0f};
    RwRGBA cColor = { 0, 0, 0, 255 };
    RwRGBAReal lowColor,highColor;

    PickUpObj *pickup = NULL;

    pickup = RwMalloc(sizeof(PickUpObj),rwID_NAOBJECT);
    if( NULL == pickup )
    {
        return NULL;
    }

     /*
     * Create the Ptank atomic : Position per particles, 
     * constant color,
     * constant set of texture coordinates.
     */
    pickup->pTank = RpPTankAtomicCreate(numPrt,
                            rpPTANKDFLAGPOSITION |
                            rpPTANKDFLAGCNSVTX2TEXCOORDS |
                            rpPTANKDFLAGARRAY,
#if defined(D3D8_DRVMODEL_H)
                            rpPTANKD3D8FLAGSUSEPOINTSPRITES);
#elif defined(D3D9_DRVMODEL_H)
                            rpPTANKD3D9FLAGSUSEPOINTSPRITES);
#else
                            0);
#endif

    if( NULL == pickup->pTank )
    {
        return FALSE;
    }

    /*
     * Create a frame and attach it to the ptank 
     */
    RpAtomicSetFrame(pickup->pTank,frame);

    /*
     * deactivate all the particle in the PTank 
     */
    RpPTankAtomicSetActiveParticlesCount(pickup->pTank,0);
    
    /*
     * the PTank is using Vertex Alpha Blending
     */
    RpPTankAtomicSetVertexAlpha(pickup->pTank, TRUE);

    /*
     * Set a basic blending mode for the PTank
     */
    RpPTankAtomicSetBlendModes(pickup->pTank,rwBLENDONE,rwBLENDONE);
    
    /*
     * Set the texture
     */
    RpPTankAtomicSetTexture(pickup->pTank,prtTexture);
    
    /*
     * All the particles are using the same texture coordinate set
     */
    RpPTankAtomicSetConstantVtx2TexCoords(pickup->pTank, cUV);
    
    /*
     * All the particles are using the same color
     */
    RpPTankAtomicSetConstantColor(pickup->pTank, &cColor);

    /*
     * All the particles are using the same size
     */
    RpPTankAtomicSetConstantSize(pickup->pTank,&cSize);

    /*
     * Initialize the particles' position
     */
    PickUpPtankInit(pickup->pTank,radius,numPrt );

    /*
     * Setup the delta color, for color animation
     */
    RwRGBARealFromRwRGBA(&lowColor,color1);
    RwRGBARealFromRwRGBA(&highColor,color2);

    RwRGBARealFromRwRGBA(&pickup->color,color1);
    
    RwRGBARealSub(&pickup->deltaColor,&highColor,&lowColor);
    RwRGBARealScale(&pickup->deltaColor,&pickup->deltaColor,(1.0f / (RwReal)ANIMATION_NUM_FRAME));
    
    /*
     * Reset the effects
     */
    pickup->direction = TRUE;
    pickup->time = 0.0f;
    pickup->step = 0;

    return pickup;
}


/*
 *****************************************************************************
 */
void
PickUpRender(PickUpObj *pickup)
{
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);

    RpAtomicRender(pickup->pTank);

    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

    return;
}


/*
 *****************************************************************************
 */
void
PickUpUpdate(PickUpObj *pickup, RwReal deltaT)
{
    pickup->time += deltaT;

    /*
     * if one "pickup frame" as passed.
     */
    if(pickup->time > ANIMATION_FRAME_RATE)
    {
        RwRGBA color;
        pickup->time = 0;

        /*
         * Update color and animation's step
         */
        if( pickup->direction )
        {
            RwRGBARealAdd(&pickup->color,&pickup->color,&pickup->deltaColor);
            pickup->step++;
        }
        else
        {
            RwRGBARealSub(&pickup->color,&pickup->color,&pickup->deltaColor);
            pickup->step--;
        }

        RwRGBAFromRwRGBAReal(&color,&pickup->color);


        if( (pickup->step == ANIMATION_NUM_FRAME - 1) ||
            (pickup->step == 0 ) )
        {
            pickup->direction = !pickup->direction;
        }

        /*
         * Set ptank constant color
         */
        RpPTankAtomicSetConstantColor(pickup->pTank, &color);
    }

    return;
}


/*
 *****************************************************************************
 */
void
PickUpDestroy(PickUpObj *pickup)
{
    if( pickup->pTank )
    {
        RpAtomicDestroy(pickup->pTank);
    }

    RwFree(pickup);

    return;
}


/*
 *****************************************************************************
 */
void
PickUpCameraRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix    *mtx;
    RwV3d       right, up, pos, at;
    RwReal      dist;

    mtx = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(mtx);
    up = *RwMatrixGetUp(mtx);
    pos = *RwMatrixGetPos(mtx);

    /* Find the distance */
    dist = RwV3dLength(&pos);

    /*
     * First translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(mtx, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * ...do the rotations...
     */
    RwMatrixRotate(mtx, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwMatrixRotate(mtx, &right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * ...and translate back...
     */
    at = *RwMatrixGetAt(mtx);
    RwV3dNormalize(&at, &at);

    RwV3dScale(&pos, &at, -dist);
    RwMatrixTranslate(mtx, &pos, rwCOMBINEPOSTCONCAT);
    RwFrameUpdateObjects(RwCameraGetFrame(Camera));

    return;
}
