
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
 * prtstd.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjdaj.
 * Reviewed by: .
 *
 * Purpose: Illustrate the use of the Standard Particle Plugin.
 *
 ****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"

#include "prtstd.h"


PrtStdData PrtData;

RwBool renderEmitter = TRUE;

static RwIm3DVertex             EmitterVertex[20];
static RwImVertexIndex          EmitterVertexIndex[48] =
                                {  0,  3,  1,
                                   1,  3,  2,
                                   4,  5,  7,
                                   7,  5,  6,
                                   0,  4,  3,
                                   3,  4,  7,
                                   1,  2,  5,
                                   5,  2,  6,
                                   3,  7,  2,
                                   2,  7,  6,
                                   0,  1,  4,
                                   4,  1,  5,
                                   8,  9, 10,
                                  11, 12, 13,
                                  14, 15, 16,
                                  17, 18, 19 };

static const RwRGBA              EmitterColor = {64, 64, 64, 64};    /* WHITE */

static const RwRGBA              EmitterDirColor = {128, 0, 0, 128};    /* WHITE */

static RwBlendFunction BlendFunctions[11] =
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

RwInt32 SrcBlendID = 4;
RwInt32 DestBlendID = 1;

RwBool VtxAlphaOn = TRUE;



/*
 *****************************************************************************
 */
RwBool
PrtStdSetPtankAlphaBlending( RwBool justCheck )
{
    if( justCheck )
    {
        return TRUE;
    }

    RpPTankAtomicSetVertexAlpha(PrtData.EmtPtank->pTank, VtxAlphaOn);

    RpPTankAtomicSetBlendModes(PrtData.EmtPtank->pTank,
                                BlendFunctions[SrcBlendID],
                                BlendFunctions[DestBlendID]);

    return TRUE;
}


/*
 *****************************************************************************
 */
void
PrtEmitterIm3DCreate( RpPrtStdEmitterStandard *emtStd )
{
    RwInt32     i;
    RwReal      sx;

    /* Initialize the vertex data for the cube representing the emitter */

    sx = 0.0f;
    sx = (emtStd->emtSize.x > sx) ? emtStd->emtSize.x : (RwReal) sx;
    sx = (emtStd->emtSize.y > sx) ? emtStd->emtSize.y : (RwReal) sx;
    sx = (emtStd->emtSize.z > sx) ? emtStd->emtSize.z : (RwReal) sx;

    i = 0;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(-emtStd->emtSize.x),
                        (RwReal)( emtStd->emtSize.y),
                        (RwReal)( emtStd->emtSize.z));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterColor.red,
                        EmitterColor.green,
                        EmitterColor.blue,
                        EmitterColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( emtStd->emtSize.x),
                        (RwReal)( emtStd->emtSize.y),
                        (RwReal)( emtStd->emtSize.z));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterColor.red,
                        EmitterColor.green,
                        EmitterColor.blue,
                        EmitterColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( emtStd->emtSize.x),
                        (RwReal)(-emtStd->emtSize.y),
                        (RwReal)( emtStd->emtSize.z));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterColor.red,
                        EmitterColor.green,
                        EmitterColor.blue,
                        EmitterColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(-emtStd->emtSize.x),
                        (RwReal)(-emtStd->emtSize.y),
                        (RwReal)( emtStd->emtSize.z));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterColor.red,
                        EmitterColor.green,
                        EmitterColor.blue,
                        EmitterColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(-emtStd->emtSize.x),
                        (RwReal)( emtStd->emtSize.y),
                        (RwReal)(-emtStd->emtSize.z));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterColor.red,
                        EmitterColor.green,
                        EmitterColor.blue,
                        EmitterColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( emtStd->emtSize.x),
                        (RwReal)( emtStd->emtSize.y),
                        (RwReal)(-emtStd->emtSize.z));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterColor.red,
                        EmitterColor.green,
                        EmitterColor.blue,
                        EmitterColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( emtStd->emtSize.x),
                        (RwReal)(-emtStd->emtSize.y),
                        (RwReal)(-emtStd->emtSize.z));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterColor.red,
                        EmitterColor.green,
                        EmitterColor.blue,
                        EmitterColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(-emtStd->emtSize.x),
                        (RwReal)(-emtStd->emtSize.y),
                        (RwReal)(-emtStd->emtSize.z));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterColor.red,
                        EmitterColor.green,
                        EmitterColor.blue,
                        EmitterColor.alpha);
    i++;


    sx = 2.0f;
    sx = (emtStd->emtSize.x > sx) ? emtStd->emtSize.x : (RwReal) sx;
    sx = (emtStd->emtSize.y > sx) ? emtStd->emtSize.y : (RwReal) sx;
    sx = (emtStd->emtSize.z > sx) ? emtStd->emtSize.z : (RwReal) sx;
    /* A pointer to show the direction of the emitter */
    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(0.25f * sx),
                        (RwReal)(0.0f),
                        (RwReal)(0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);

    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(-0.25f * sx),
                        (RwReal)( 0.0f),
                        (RwReal)( 0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;


    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( 0.0f),
                        (RwReal)( 0.0f),
                        (RwReal)( 0.5f * sx));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;

    /* A pointer to show the direction of the emitter  */
    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(0.0f),
                        (RwReal)(0.25f * sx),
                        (RwReal)(0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;


    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( 0.0f),
                        (RwReal)(-0.25f * sx),
                        (RwReal)( 0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( 0.0f),
                        (RwReal)( 0.0f),
                        (RwReal)( 0.5f * sx));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;

    /* A pointer base */
    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(0.0f),
                        (RwReal)(0.25f * sx),
                        (RwReal)(0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;


    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( 0.0f),
                        (RwReal)(-0.25f * sx),
                        (RwReal)( 0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( 0.25f * sx),
                        (RwReal)( 0.0f),
                        (RwReal)( 0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(0.0f),
                        (RwReal)(0.25f * sx),
                        (RwReal)(0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)(-0.25f * sx),
                        (RwReal)( 0.0f),
                        (RwReal)( 0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;

    RwIm3DVertexSetPos (&EmitterVertex[i],
                        (RwReal)( 0.0f),
                        (RwReal)(-0.25f * sx),
                        (RwReal)( 0.0f));
    RwIm3DVertexSetRGBA(&EmitterVertex[i],
                        EmitterDirColor.red,
                        EmitterDirColor.green,
                        EmitterDirColor.blue,
                        EmitterDirColor.alpha);
    i++;
}

/*
 *****************************************************************************
 */
void
PrtStdRender(void)
{
    static RwBool stateSet = FALSE;

    if( !stateSet )
    {
        /*
         * Render state is persistent - only need to set it once...
         */
        RwRenderStateSet(rwRENDERSTATECULLMODE,(void *)rwCULLMODECULLNONE);

        RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void*)rwBLENDONE);

        if( NULL != PrtData.EmtPtank->pTank )
        {
            /*
             * Setup PTank's Alpha Blending whenever the PTank is created
             */
            PrtStdSetPtankAlphaBlending(FALSE);

            stateSet = TRUE;
        }

    }

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
    RpAtomicRender(PrtData.atmEmitter);

    if( TRUE == renderEmitter )
    {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER ,(void*)NULL);


        if (RwIm3DTransform(EmitterVertex, 14, RwFrameGetLTM(RpAtomicGetFrame(PrtData.atmEmitter)), 0))
        {
            RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, EmitterVertexIndex, 42);

            RwIm3DEnd();
        }
    }

    return;
}


/*
 *****************************************************************************
 */
RwBool
PrtStdSetDefault(RwBool justcheck)
{
    if( justcheck )
    {
        return TRUE;
    }

    /* Set the emitter's maximum particles */
    PrtData.EmtStd->maxPrt = 10000;

    /* Set the emitter's emission area */
    PrtData.EmtStd->emtSize.x = 0.0f;
    PrtData.EmtStd->emtSize.y = 0.0f;
    PrtData.EmtStd->emtSize.z = 0.0f;

    /* Set the particle's size */
    PrtData.EmtStd->prtSize.x = 1.0f;
    PrtData.EmtStd->prtSize.y = 1.0f;

    /* Set the particle emission gap : should not be bigger
     *  than the batch size setted during the creation code
     */
    PrtData.EmtStd->emtPrtEmit = 20;
    PrtData.EmtStd->emtPrtEmitBias = 0;
    PrtData.EmtStd->emtEmitGap = 0.0f;
    PrtData.EmtStd->emtEmitGapBias = 0.0f;

    /* Set the particle's life span */
    PrtData.EmtStd->prtLife = 1.0f;
    PrtData.EmtStd->prtLifeBias = 0.0f;

    /* Set the particles emission speed */
    PrtData.EmtStd->prtInitVel = 1.0f;
    PrtData.EmtStd->prtInitVelBias = 0.00f;

    /* Set the particles emission Direction */
    PrtData.EmtStd->prtInitDir.x = 0.0f;
    PrtData.EmtStd->prtInitDir.y = 0.0f;
    PrtData.EmtStd->prtInitDir.z = 1.0f;

    PrtData.EmtStd->prtInitDirBias.x = 0.0f;
    PrtData.EmtStd->prtInitDirBias.y = 0.0f;
    PrtData.EmtStd->prtInitDirBias.z = 0.0f;

    /* Set the force emission Direction */
    PrtData.EmtStd->force.x = 0.0f;
    PrtData.EmtStd->force.y = 0.0f;
    PrtData.EmtStd->force.z = 0.0f;

    /* Set the default Color */
    PrtData.EmtStd->prtColor.red = 255;
    PrtData.EmtStd->prtColor.green = 255;
    PrtData.EmtStd->prtColor.blue = 255;
    PrtData.EmtStd->prtColor.alpha = 128;

    /* Set the default Texture coordinate */
    PrtData.EmtStd->prtUV[0].u = 0.0f;
    PrtData.EmtStd->prtUV[0].v = 0.0f;

    PrtData.EmtStd->prtUV[1].u = 1.0f;
    PrtData.EmtStd->prtUV[1].v = 1.0f;

    /* Set the texture */
    PrtData.EmtStd->texture = NULL;

#ifdef COLORANIM
    PrtData.emtPrtCol->prtStartCol.red = 255.0f;
    PrtData.emtPrtCol->prtStartCol.green = 255.0f;
    PrtData.emtPrtCol->prtStartCol.blue = 255.0f;
    PrtData.emtPrtCol->prtStartCol.alpha = 128.0f;

    PrtData.emtPrtCol->prtEndCol.red = 255.0f;
    PrtData.emtPrtCol->prtEndCol.green = 255.0f;
    PrtData.emtPrtCol->prtEndCol.blue = 255.0f;
    PrtData.emtPrtCol->prtEndCol.alpha = 128.0f;
#endif

#ifdef SIZEANIM
    PrtData.emtPrtSize->prtStartSize.x = 0.0f;
    PrtData.emtPrtSize->prtStartSize.y = 0.0f;
    PrtData.emtPrtSize->prtStartSizeBias.x = 0.0f;
    PrtData.emtPrtSize->prtStartSizeBias.y = 0.0f;
    PrtData.emtPrtSize->prtEndSize.x = 1.0f;
    PrtData.emtPrtSize->prtEndSize.y = 1.0f;
    PrtData.emtPrtSize->prtEndSizeBias.x = 0.0f;
    PrtData.emtPrtSize->prtEndSizeBias.y = 0.0f;
#endif

    PrtEmitterIm3DCreate(PrtData.EmtStd);

    return TRUE;
}


/*
 *****************************************************************************
 */
RwBool
PrtStdInitialize(void)
{
    RwInt32 offset;
    RwFrame *frame;
    RwChar *path;
    /* Create an emitter class suporting the standard
       emitter properties
       plus Size animations
       plus Colors animations
       and owning and controling a PTank */
    PrtData.eClass = RpPrtStdEClassStdCreate(
        (rpPRTSTDEMITTERDATAFLAGSTANDARD |
#ifdef COLORANIM
        rpPRTSTDEMITTERDATAFLAGPRTCOLOR |
#endif
#ifdef SIZEANIM
        rpPRTSTDEMITTERDATAFLAGPRTSIZE |
#endif
        rpPRTSTDEMITTERDATAFLAGPTANK));

    if( NULL == PrtData.eClass )
    {
        return FALSE;
    }

    /* Create a particle class suporting the standard
       particle properties
       plus Size animations
       plus Colors animations
       plus a velocity per particles */
    PrtData.pClass = RpPrtStdPClassStdCreate(
        (rpPRTSTDPARTICLEDATAFLAGSTANDARD |
#ifdef COLORANIM
        rpPRTSTDPARTICLEDATAFLAGCOLOR |
#endif
#ifdef SIZEANIM
        rpPRTSTDPARTICLEDATAFLAGSIZE |
#endif
        rpPRTSTDPARTICLEDATAFLAGVELOCITY));

    if(NULL == PrtData.eClass )
    {
        return FALSE;
    }


    /* Create the Emitter's Atomic */
    PrtData.atmEmitter = RpPrtStdAtomicCreate(PrtData.eClass, NULL);
    if( NULL == PrtData.atmEmitter )
    {
        return FALSE;
    }

    /* Get the emitter Pointer */
    PrtData.emitter = RpPrtStdAtomicGetEmitter(PrtData.atmEmitter);

    /* Set particle class and particle batch size */
    RpPrtStdEmitterSetPClass(PrtData.emitter, PrtData.pClass, BATCHSIZE);

    /* Get the Emitter Standard properties pointer */
    offset = RpPrtStdPropTabGetPropOffset(
        PrtData.eClass->propTab, rpPRTSTDPROPERTYCODEEMITTERSTANDARD);

    PrtData.EmtStd = (RpPrtStdEmitterStandard *) (((RwChar *)PrtData.emitter) + offset);

#ifdef COLORANIM
    offset = RpPrtStdPropTabGetPropOffset(
        PrtData.eClass->propTab, rpPRTSTDPROPERTYCODEEMITTERPRTCOLOR);
    PrtData.emtPrtCol = (RpPrtStdEmitterPrtColor *) (((RwChar *)PrtData.emitter) + offset);
#endif

#ifdef SIZEANIM
    offset = RpPrtStdPropTabGetPropOffset(
        PrtData.eClass->propTab, rpPRTSTDPROPERTYCODEEMITTERPRTSIZE);
    PrtData.emtPrtSize = (RpPrtStdEmitterPrtSize *) (((RwChar *)PrtData.emitter) + offset);
#endif

    offset = RpPrtStdPropTabGetPropOffset(
        PrtData.eClass->propTab, rpPRTSTDPROPERTYCODEEMITTERPTANK);

    PrtData.EmtPtank = (RpPrtStdEmitterPTank *) (((RwChar *)PrtData.emitter) + offset);


    /* Setup The Default Properties */
    PrtStdSetDefault(FALSE);


    /* Setup a frame for the emitter */
    frame = RwFrameCreate();
    RwMatrixSetIdentity(RwFrameGetMatrix(frame));
    RpAtomicSetFrame(PrtData.atmEmitter, frame);

    /* Set the particle's texture*/
        /*
     * Load the texture...
     */
    path = RsPathnameCreate(RWSTRING("textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    PrtData.PrtTexture = RwTextureRead(RWSTRING("particle"), RWSTRING("particle"));

    PrtData.EmtStd->texture = PrtData.PrtTexture;

    return TRUE;
}


/*
 *****************************************************************************
 */
void
PrtStdTerminate(void)
{

    if( NULL != PrtData.atmEmitter )
    {
        RwFrame *frame = RpAtomicGetFrame(PrtData.atmEmitter);

        if( NULL != frame)
        {
             RpAtomicSetFrame(PrtData.atmEmitter, NULL);
             RwFrameDestroy(frame);
        }

        RpAtomicDestroy(PrtData.atmEmitter);
		PrtData.atmEmitter = NULL;
    }

    if( NULL != PrtData.eClass )
    {
        RpPrtStdEClassDestroy(PrtData.eClass);
    }

    if( NULL != PrtData.pClass )
    {
        RpPrtStdPClassDestroy(PrtData.pClass);
    }


    return;
}


/*
 *****************************************************************************
 */
void
PrtStdCameraRotate(RwReal xAngle, RwReal yAngle)
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


/*
 *****************************************************************************
 */
void
PrtStdEmitterRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix    *mtx, *camMtx;
    RwV3d       right, up, pos;

    camMtx = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(camMtx);
    up = *RwMatrixGetUp(camMtx);

    mtx = RwFrameGetMatrix(RpAtomicGetFrame(PrtData.atmEmitter));

    pos = *RwMatrixGetPos(mtx);

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
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(mtx, &pos, rwCOMBINEPOSTCONCAT);
    RwFrameUpdateObjects(RpAtomicGetFrame(PrtData.atmEmitter));

    return;
}



/*
 *****************************************************************************
 */
void
PrtStdEmitterWorldRotate(RwReal xAngle, RwReal yAngle, RwReal zAngle)
{
    RwMatrix    *mtx;
    RwV3d       pos;
    static RwV3d Axis[3] = {{ 1.0f, 0.0f, 0.0f },
                            { 0.0f, 1.0f, 0.0f },
                            { 0.0f, 0.0f, 1.0f }};
    static RwV3d result[3];

    mtx = RwFrameGetMatrix(RpAtomicGetFrame(PrtData.atmEmitter));

    pos = *RwMatrixGetPos(mtx);

    /*
     * First translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(mtx, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * ...do the rotations...
     */
    RwV3dTransformVectors(result, Axis, 3, mtx);

    RwMatrixRotate(mtx, &result[0], xAngle, rwCOMBINEPOSTCONCAT);
    RwMatrixRotate(mtx, &result[1], yAngle, rwCOMBINEPOSTCONCAT);

    RwMatrixRotate(mtx, &result[2], zAngle, rwCOMBINEPOSTCONCAT);

    /*
     * ...and translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(mtx, &pos, rwCOMBINEPOSTCONCAT);
    RwFrameUpdateObjects(RpAtomicGetFrame(PrtData.atmEmitter));

    return;
}
