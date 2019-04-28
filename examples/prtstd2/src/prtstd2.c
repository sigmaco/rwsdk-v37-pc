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
 * prtstd2.c
 *
 * Copyright (C) 2002 Criterion Technologies.
 *
 * Original author: Alexandre Hadjdaj.
 * Reviewed by: .
 *
 * Purpose: Illustrate the use custom property in the Standard Particle
 *          Plugin.
 *
 ****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"

#include "prtstd2.h"


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
 * Create a pTank manually for use with the custom particle property.
 *
 * Our custom property animates the particle's color so we will not be using
 * the standard color animation property. However, if we allow the pTank
 * to be created automatically, the color property in the pTank will not be
 * present because the standard callbacks will assume it is not needed.
 *
 * It is not needed by the standard properties but it is needed by our
 * custom property.
 *
 * So we need to create the pTank manually to ensure the color property is
 * present. This will apply to any custom property that will replace the
 * standard callbacks for animating any properties in the pTank.
 */
static RpAtomic *
ParticleGlowCreatePTank( void )
{
    RpAtomic                    *pTank;
    RpPrtStdEmitter             *emt;
    RpPrtStdEmitterStandard     *emtStd;
    RwUInt32                    dataFlags, platFlags;
    RwV2d                       v2d;
    RwFrame                     *frame;
    RwTexCoords                 uv[2];

    emt = PrtData.emitter;
    emtStd = PrtData.EmtStd;

    /* Setup the dataflags for the ptank */
    dataFlags = rpPTANKDFLAGPOSITION | /* Positional particles. */
                rpPTANKDFLAGCOLOR | /* Per particle color */
                rpPTANKDFLAGCNSVTX2TEXCOORDS; /* Constant UV for all particles */

    platFlags = 0;

#ifdef SKY

   /* Additional flag for PlayStation 2. For efficency, the data are arranged
    * as structures of arrays rather than array of structures.
    */
    dataFlags |= rpPTANKDFLAGARRAY;
    platFlags |= rpPTANKSKYFLAGINSTANCEBUFFER;

#endif /* SKY */

    /* Create the pTank atomic */
    pTank = RpPTankAtomicCreate(emtStd->maxPrt,dataFlags, platFlags );

    if (pTank == NULL)
        return((RpAtomic *) NULL);

    /* Set up a dummy frame */
    frame = RwFrameCreate();
    RwMatrixSetIdentity(RwFrameGetMatrix(frame));
    RpAtomicSetFrame(pTank, frame);

    /* Particle centre */
    v2d.x = (RwReal) 0.0;
    v2d.y = (RwReal) 0.0;
    RpPTankAtomicSetConstantCenter(pTank, &v2d);

    /* Particle tex coords */
    uv[0].u = emtStd->prtUV[0].u;
    uv[0].v = emtStd->prtUV[0].v;

    uv[1].u = emtStd->prtUV[1].u;
    uv[1].v = emtStd->prtUV[1].v;

    RpPTankAtomicSetConstantVtx2TexCoords(pTank, uv);

    /* Particle size */
    v2d.x = emtStd->prtSize.x;
    v2d.y = emtStd->prtSize.y;

    RpPTankAtomicSetConstantSize(pTank, &v2d);

    /* Particle texture */
    if (emtStd->texture)
        RpPTankAtomicSetTexture(pTank, emtStd->texture);

    return pTank;
}

/*
 *****************************************************************************
 * Private callback for the custom particle property.
 *
 * This callback is called when a new emitter is created with the custom
 * property. This callback is inserted to be called after the standard
 * create is called. The standard create callback will have setup the
 * known standard properties. We only need to set up the custom property's
 * initial values in this function.
 *
 * We also need to disable the color emit and color update flags in the
 * pTank. Since our property is used to animate the particle's color, we
 * will look after the emission (initial) values and its update during the
 * particle's lifetime. If we do not disable the flags, the standard emit
 * and update callback will attempt to modify the color and more importantly,
 * adjust the color data pointer in the pTank's data array. This will lead
 * to an out of sync of data between the emitter and the pTank.
 */
static RpPrtStdEmitter *
ParticleGlowEmitterCreateCB(RpAtomic * atomic __RWUNUSED__,
                                RpPrtStdEmitter *emt,
                                void * data __RWUNUSED__)
{
    RwInt32                     offset;
    RpPrtStdEmitterPTank        *emtPTank;
    EmitterPrtGlow              *emtPrtGlow;

    /* Get the Emitter Particle Glow property pointer */
    offset  = RpPrtStdPropTabGetPropOffset(
        emt->emtClass->propTab, EMITTERPRTPROPCODEGLOW);

    if (offset >= 0)
    {
        emtPrtGlow = ((EmitterPrtGlow *)
            PARTICLEPROP(emt,offset));

        emtPrtGlow->numGlow = PrtData.numGlow;
        emtPrtGlow->glowTable = PrtData.prtGlowTable;
    }

    /* Get the Emitter PTank property pointer */
    offset = RpPrtStdPropTabGetPropOffset(
        emt->emtClass->propTab, rpPRTSTDPROPERTYCODEEMITTERPTANK);

    if (offset >= 0)
    {
        emtPTank = ((RpPrtStdEmitterPTank *)
            PARTICLEPROP(emt, offset));

        /* Disable the emit and update for pTank colors. We will handling the
         * the emission and update manually.
         */
        emtPTank->updateFlags &= ~rpPTANKDFLAGCOLOR;
        emtPTank->emitFlags &= ~rpPTANKDFLAGCOLOR;
    }

    return emt;
}

/*
 *****************************************************************************
 * Private callback for the custom property.
 *
 * This is the emit callback which is called to emit new particles. This
 * callback is placed AFTER the standard emit callback.
 *
 * The standard emit callback will have created the new particles that were
 * emitted for this time intervals. It also intialised the known standard
 * particle properties to their initial state.
 *
 * This function only needs to intialise the custom property in the newly
 * emitted particles. It also initialised the color property in the pTank.
 */
static RpPrtStdEmitter *
ParticleGlowEmitterEmitCB(RpAtomic * atomic __RWUNUSED__,
                          RpPrtStdEmitter *emt,
                          void * data __RWUNUSED__)

{
    RwInt32                         numPrt, i, prtStride;

    RpPrtStdEmitter                *result;

    RpPrtStdParticleBatch          *prtBatch;

    RwChar                         *pTankColIn = (RwChar *)NULL;
    RwChar                         *pTankColOut = (RwChar *)NULL;

    RwInt32                         pTankColStride = 0;

    RpPrtStdEmitterStandard         *emtStd;
    RpPrtStdEmitterPTank            *emtPTank;
    EmitterPrtGlow                  *emtPrtGlow;

    RwChar                          *prt;
    RpPrtStdParticleStandard        *prtStd;
    ParticleGlow                    *prtGlow;

    result = NULL;

    emtPTank = NULL;

    /* Get the emitter's properties. */
    emtStd = ((RpPrtStdEmitterStandard *)
        PARTICLEPROP(emt, PrtData.offsetEmtStd));

    if (PrtData.offsetEmtPTank >= 0)
        emtPTank = ((RpPrtStdEmitterPTank *)
            PARTICLEPROP(emt, PrtData.offsetEmtPTank));

    if (PrtData.offsetEmtPrtGlow >= 0)
        emtPrtGlow = ((EmitterPrtGlow *)
            PARTICLEPROP(emt, PrtData.offsetEmtPrtGlow));

    /* Get the pointer to the color property. The index position can vary
     * depending on what properties are present in the PTank, but it is
     * always in the same order.
     */
    if (emtPTank)
    {
        /* Position. Ignore. */
        i = 0;

        if (emtPTank->dataFlags & rpPTANKDFLAGPOSITION)
        {
            i++;
        }

        /* Color. */
        pTankColIn = NULL;
        pTankColOut = NULL;
        pTankColStride = 0;
        if (emtPTank->dataFlags & rpPTANKDFLAGCOLOR)
        {
            pTankColIn = (RwChar *) emtPTank->dataInPtrs[i];
            pTankColOut = (RwChar *) emtPTank->dataOutPtrs[i];

            pTankColStride = emtPTank->dataStride[i];

            i++;
        }
    }

    /* Check if we are to emit particles. */
    if (emtStd->currTime >= emtStd->emtEmitTime)
    {
        /* Get the new particles. New particles were created by the
         * standard function and initialised with default data.
         * We only need to initialised our own particular data in the ptank
         * and in the particle.
         */
        prtBatch = emt->activeBatch;

        if (prtBatch != NULL && -1 != prtBatch->newPrt)
        {
            if (prtBatch->newPrt >= 0)
            {
                /* Main process loop. */
                numPrt = prtBatch->numPrt - prtBatch->newPrt;
                prtStride = emt->prtClass->objSize;
                prt = ((RwChar *) prtBatch) +
                    prtBatch->offset + prtBatch->newPrt*prtStride;

                /* Loop through all the new particles in the batch. */
                for (i = 0; i < numPrt; i++)
                {
                    prtStd = (RpPrtStdParticleStandard *) prt;

                    if( prtStd->currTime == 0 )
                    {
                        /* Particle glow property. */
                        if (PrtData.offsetPrtGlow >= 0)
                        {
                            /* Initlialise the property with the default
                             * values.
                             */
                            prtGlow = ((ParticleGlow *)
                                PARTICLEPROP(prt, PrtData.offsetPrtGlow));

                            prtGlow->index = 0;

                            prtGlow->t = PrtData.prtGlowTable[0].t;
                            prtGlow->iDeltaT = PrtData.prtGlowTable[0].iDeltaT;
                            prtGlow->glow0 = PrtData.prtGlowTable[0].glow;
                            prtGlow->glow1 = PrtData.prtGlowTable[1].glow;
                        }

                        /* color */
                        if (emtPTank)
                        {
                            if (PrtData.offsetPrtGlow >= 0)
                            {
                                /* Initialise the color to the initial
                                 * value.
                                 */
                                ((RwRGBA *) pTankColOut)->red = (RwUInt8) 0;
                                ((RwRGBA *) pTankColOut)->green = (RwUInt8) 0;
                                ((RwRGBA *) pTankColOut)->blue = (RwUInt8) 0;
                                ((RwRGBA *) pTankColOut)->alpha = (RwUInt8) 0;

                                /* Next particle color position. */
                                pTankColOut += pTankColStride;
                            }
                        }
                    }

                    prt += prtStride;
                }
            }
        }
    }

    /* Update the PTank's pointers. */
    if (emtPTank)
    {
        i = 0;

        /* Position. Ignore. */
        if (emtPTank->dataFlags & rpPTANKDFLAGPOSITION)
        {
            i++;
        }

        /* Color. Update the data pointers to the new position. */
        if (emtPTank->dataFlags & rpPTANKDFLAGCOLOR)
        {
            emtPTank->dataInPtrs[i] = pTankColIn;
            emtPTank->dataOutPtrs[i] = pTankColOut;
            i++;
        }
    }


    return (emt);
}

/*
 *****************************************************************************
 * Private callback for the custome property.
 *
 * This is the update callback which is called to emit new particles. This
 * callback is placed BEFORE the standard update callback.
 *
 * The update callback is different to the emit callback in that it is called
 * BEFORE the standard update callback. Just as we relie on the standard
 * emit callback to create the new particles, we use the standard callback
 * to remove dead particles.
 *
 * We must do the update before the standard callback otherwise the standard
 * update callback will have removed all the dead particles. This would
 * prevent us from processing dead particles, if we needed to, and also
 * causes data in the pTank to be out of sync. In our case, this would be
 * the color property where we will be updating the wrong color value for
 * the particle.
 */
static RpPrtStdParticleBatch *
ParticleGlowPrtUpdateCB(RpPrtStdEmitter *emt,
                        RpPrtStdParticleBatch *prtBatch,
                        void * data __RWUNUSED__)
{
    RpPrtStdEmitterPTank              * emtPTank;
    RpPrtStdEmitterStandard           * emtStd;
    EmitterPrtGlow                    * emtPrtGlow;

    ParticleGlow                      * prtGlowOut;

    RwChar                            * pTankColIn = (RwChar *)NULL ;
    RwChar                            * pTankColOut = (RwChar *)NULL ;

    RwChar                            * prtIn;
    RwChar                            * prtOut;

    RwInt32                             i;
    RwInt32                             numPrtIn;
    RwInt32                             numPrtOut;
    RwInt32                             pTankColStride = 0;
    RwInt32                             prtStride;

    RwReal                              alphaT;
    RwReal                              glow;

    /* Get the emitter's properties. */
    emtPTank = NULL;

    emtStd = ((RpPrtStdEmitterStandard *)
        PARTICLEPROP(emt, PrtData.offsetEmtStd));

    if (PrtData.offsetEmtPrtGlow >= 0)
        emtPrtGlow = ((EmitterPrtGlow *)
            PARTICLEPROP(emt, PrtData.offsetEmtPrtGlow));

    if (PrtData.offsetEmtPTank >= 0)
        emtPTank = ((RpPrtStdEmitterPTank *)
            PARTICLEPROP(emt, PrtData.offsetEmtPTank));

    /* Get the pointer to the color property. The index position can vary
     * depending on what properties are present in the PTank, but it is
     * always in the same order.
     */
    if (emtPTank)
    {
        /* Position. Ignore. */
        i = 0;

        if (emtPTank->dataFlags & rpPTANKDFLAGPOSITION)
        {
            i++;
        }

        /* Color. */
        pTankColIn = NULL;
        pTankColOut = NULL;
        pTankColStride = 0;
        if (emtPTank->dataFlags & rpPTANKDFLAGCOLOR)
        {
            pTankColIn = (RwChar *) emtPTank->dataInPtrs[i];
            pTankColOut = (RwChar *) emtPTank->dataOutPtrs[i];

            pTankColStride = emtPTank->dataStride[i];
        }
    }

    /* Particles update loop. */
    prtStride = emt->prtClass->objSize;
    prtIn = ((RwChar *)prtBatch) + prtBatch->offset;
    prtOut = prtIn;

    numPrtIn = prtBatch->numPrt;
    numPrtOut = 0;

    while (numPrtIn-- > 0)
    {
        /* Is this particle dead ? */
        if (((RpPrtStdParticleStandard *)prtIn)->currTime <
            ((RpPrtStdParticleStandard *)prtIn)->endTime)
        {
            alphaT =
                ((RpPrtStdParticleStandard *)prtIn)->currTime *
                ((RpPrtStdParticleStandard *)prtIn)->invEndTime;

            /* Clamp the alphatT to 1.0 to avoid artifacts */
            alphaT = (alphaT > (RwReal) 1.0) ?
                    (RwReal) 1.0 : (RwReal) alphaT;

            /* Get the particle glow property from the particle */
            prtGlowOut = ((ParticleGlow *)
                PARTICLEPROP(prtIn, PrtData.offsetPrtGlow));

            /* Update the glow property. */
            if (alphaT >= PrtData.prtGlowTable[prtGlowOut->index + 1].t)
            {
                prtGlowOut->index++;

                prtGlowOut->t =
                    PrtData.prtGlowTable[prtGlowOut->index].t;

                prtGlowOut->iDeltaT =
                    PrtData.prtGlowTable[prtGlowOut->index].iDeltaT;

                prtGlowOut->glow0 = prtGlowOut->glow1;

                prtGlowOut->glow1 =
                    PrtData.prtGlowTable[prtGlowOut->index + 1].glow;
            }

            alphaT = (alphaT - prtGlowOut->t) * prtGlowOut->iDeltaT;

            glow =
                (alphaT * prtGlowOut->glow1) +
                (((RwReal) 1.0 - alphaT) * prtGlowOut->glow0);

            /* Update the pTank */
            if (emtPTank)
            {
                /* color */
                if (PrtData.offsetPrtGlow >= 0)
                {
                    ((RwRGBA *) pTankColOut)->red =
                        RwInt32FromRealMacro(glow * (RwReal) 255.0);

                    ((RwRGBA *) pTankColOut)->green =
                        ((RwRGBA *) pTankColOut)->red;

                    ((RwRGBA *) pTankColOut)->blue =
                        ((RwRGBA *) pTankColOut)->red;

                    ((RwRGBA *) pTankColOut)->alpha =
                        ((RwRGBA *) pTankColOut)->red;
                }

                pTankColOut += pTankColStride;
            }

            prtOut += prtStride;

            numPrtOut++;
        }
        else
        {
            /* Particle is dead so skip. It will be removed by being
             * overwritten later in the standard callback
             */
        }

        /* Next input prt */
        prtIn += prtStride;

        pTankColIn += pTankColStride;
    }

    /* Update the PTank's pointers. */
    if (emtPTank)
    {
        i = 0;

        /* Position. Ignore. */
        if (emtPTank->dataFlags & rpPTANKDFLAGPOSITION)
        {
            i++;
        }

        /* Color. Update the data prts to the new positions. */
        if (emtPTank->dataFlags & rpPTANKDFLAGCOLOR)
        {
            emtPTank->dataInPtrs[i] = pTankColIn;
            emtPTank->dataOutPtrs[i] = pTankColOut;
            i++;
        }
    }

    return (prtBatch);
}

/*
 *****************************************************************************
 * This function creates a new emitter class containing the standard
 * properties plus the custom property.
 *
 * We cannot use the RpPrtStdEClassStdCreate function because it does not
 * handle custom property, so we need to set up the emitter class manually
 * to include it.
 *
 * To do this, we first need to create a property table for the emitter class
 * that contains all the properties we require. Both the standard property
 * and our custom property.
 *
 * Next we set up the callbacks in the emitter class for handling the standard
 * and custom properties. We do not need to set up all the callbacks, only
 * those that are applicable.
 *
 * We will be using a combination of standard callbacks and our custom
 * callbacks. The standard callbacks will be used to manage the standard
 * properties and the custom callback to manage just the custom property.
 *
 * We could have replaced all the standard callbacks with our own set of
 * custom callbacks that would have managed both type of properties.
 */
static RpPrtStdEmitterClass *
EmitterClassCreate( void )
{
    RpPrtStdEmitterClass                *eClass;
    RpPrtStdPropertyTable               *propTab;
    RwInt32                             size, i, j,
                                        prop[10],
                                        propSize[10];
    RpPrtStdEmitterCallBackArray        emtCB[2];

    /* Set up the emitter's property table */
    i = 0;
    size = 0;

    /* Mandatory emitter header property. Must be the first property. */
    prop[0] = rpPRTSTDPROPERTYCODEEMITTER;
    propSize[0] = sizeof(RpPrtStdEmitter);
    i++;
    size += sizeof(RpPrtStdEmitter);

    /* Standard emitter properties in prtstd. */
    prop[i] = rpPRTSTDPROPERTYCODEEMITTERSTANDARD;
    propSize[i] = sizeof(RpPrtStdEmitterStandard);
    i++;
    size += sizeof(RpPrtStdEmitterStandard);

    /* PTank */
    prop[i] = rpPRTSTDPROPERTYCODEEMITTERPTANK;
    propSize[i] =
        sizeof(RpPrtStdEmitterPTank);
    i++;
    size += sizeof(RpPrtStdEmitterPTank);

    /* Private property, particle glow. */
    prop[i] = EMITTERPRTPROPCODEGLOW;
    propSize[i] = sizeof(EmitterPrtGlow);
    i++;
    size += sizeof(EmitterPrtGlow);

    propTab =
        RpPrtStdPropTabCreate(i,
            prop, propSize);

    /* Create and setup the emitter class */
    eClass = RpPrtStdEClassCreate();

    RpPrtStdEClassSetPropTab(eClass, propTab);

    eClass->id = 0;

    /* Destroy the prop tab. The emitter class now owns
     * the property table and will destroy it when it is
     * destroyed.
     */
    RpPrtStdPropTabDestroy(propTab);

    /* Set up the callbacks */
    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < rpPRTSTDEMITTERCALLBACKMAX; i++)
            emtCB[j][i] = NULL;
    }

    /* Setup the standard callbacks */

    emtCB[0][rpPRTSTDEMITTERCALLBACKCREATE] =
        RpPrtStdEmitterStdCreateCB;

    emtCB[0][rpPRTSTDEMITTERCALLBACKEMIT] =
        RpPrtStdEmitterStdEmitCB;

    emtCB[0][rpPRTSTDEMITTERCALLBACKBEGINUPDATE] =
        RpPrtStdEmitterStdBeginUpdateCB;

    emtCB[0][rpPRTSTDEMITTERCALLBACKENDUPDATE] =
        RpPrtStdEmitterStdEndUpdateCB;

    emtCB[0][rpPRTSTDEMITTERCALLBACKBEGINRENDER] =
        RpPrtStdEmitterStdRenderCB;

    emtCB[0][rpPRTSTDEMITTERCALLBACKSTREAMREAD] =
        RpPrtStdEmitterStdStreamReadCB;

    emtCB[0][rpPRTSTDEMITTERCALLBACKSTREAMWRITE] =
        RpPrtStdEmitterStdStreamWriteCB;

    emtCB[0][rpPRTSTDEMITTERCALLBACKSTREAMGETSIZE] =
        RpPrtStdEmitterStdStreamGetSizeCB;

    emtCB[0][rpPRTSTDEMITTERCALLBACKCLONE] =
        RpPrtStdEmitterStdCloneCB;

    /* Setup the additional private callbacks for the private properties. */

    /* Create callback. The private create can only after the default
     * callback. */
    emtCB[1][rpPRTSTDEMITTERCALLBACKCREATE] =
        ParticleGlowEmitterCreateCB;

    /* Emit callback. The private update callback needs to be after the
     * default callback */
    emtCB[1][rpPRTSTDEMITTERCALLBACKEMIT] =
        ParticleGlowEmitterEmitCB;

    RpPrtStdEClassSetCallBack(eClass, 2, emtCB);

    return (eClass);
}

/*
 *****************************************************************************
 * This function creates a new particle class containing the standard property
 * plus the custom particle property.
 *
 * Like the emitter class, we cannot use the RpPrtStdPClassStdCreate function
 * because it does not handle custom property, so we need to set up the
 * particle class manually to include it.
 *
 * The creation procedure for the particle class is identical to the emitter
 * class. We set up the property table with the necessary properties and
 * set up the callbacks.
 *
 * We need to be a bit more careful when setting up the callbacks for
 * the particle class. There are a set of platform specific callbacks for
 * the PlayStation 2 which we can use in place of the generic C functions.
 * These gives better performance than the generic functions.
 */
static RpPrtStdParticleClass *
ParticleClassCreate( void )
{
    RpPrtStdParticleClass               *pClass;
    RpPrtStdPropertyTable               *propTab;
    RwInt32                             size, i, j,
                                        prop[10],
                                        propSize[10];
    RpPrtStdParticleCallBackArray       prtCB[2];

    /* Set up the particle's property table */
    i = 0;
    size = 0;

    /* Standard particle property. */
    prop[i] = rpPRTSTDPROPERTYCODEPARTICLESTANDARD;
    propSize[i] = sizeof(RpPrtStdParticleStandard);
    i++;
    size += sizeof(RpPrtStdParticleStandard);

    /* Velocity */
    prop[i] = rpPRTSTDPROPERTYCODEPARTICLEVELOCITY;
    propSize[i] = sizeof(RwV3d);
    i++;
    size += sizeof(RwV3d);

    /* Private particle glow property */
    prop[i] = PARTICLEPROPCODEGLOW;
    propSize[i] = sizeof(ParticleGlow);
    i++;
    size += sizeof(ParticleGlow);

    propTab =
        RpPrtStdPropTabCreate(i,
            prop, propSize);

    if (propTab == NULL)
        return ((RpPrtStdParticleClass *) NULL);

    /* Create and setup the particle class */
    pClass = RpPrtStdPClassCreate();

    RpPrtStdPClassSetPropTab(pClass, propTab);

    pClass->id = 0;

    /* Destroy the prop tab. The emitter class now owns
     * the property table and will destroy it when it is
     * destroyed.
     */
    RpPrtStdPropTabDestroy(propTab);

    /* Set up the callbacks */
    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < rpPRTSTDPARTICLECALLBACKMAX; i++)
            prtCB[j][i] = NULL;
    }

    /* Set up the standard update callback, depending on the platform. */
#ifdef SKY

    /* Standard particle update callback for PlayStation 2 */
    prtCB[1][rpPRTSTDPARTICLECALLBACKUPDATE] =
        RpPrtStdParticleStdUpdateSkyCB;

#else /* SKY */

    /* Standard particle update callback */
    prtCB[1][rpPRTSTDPARTICLECALLBACKUPDATE] =
        RpPrtStdParticleStdUpdateCB;

#endif /* (SKY) */

    /* Update callback. The private update callbacks needs to be before the
     * default callbacks */

    prtCB[0][rpPRTSTDPARTICLECALLBACKUPDATE] =
        ParticleGlowPrtUpdateCB;

    RpPrtStdPClassSetCallBack(pClass, 2, prtCB);

    return (pClass);
}

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
    PrtData.EmtStd->emtSize.x = 0.5f;
    PrtData.EmtStd->emtSize.y = 0.5f;
    PrtData.EmtStd->emtSize.z = 0.0f;

    /* Set the particle's size */
    PrtData.EmtStd->prtSize.x = 1.0f;
    PrtData.EmtStd->prtSize.y = 1.0f;

    /* Set the particle emission gap : should not be bigger
     * than the batch size set during the creation code
     */
    PrtData.EmtStd->emtPrtEmit = 4;
    PrtData.EmtStd->emtPrtEmitBias = 0;
    PrtData.EmtStd->emtEmitGap = 1.0f;
    PrtData.EmtStd->emtEmitGapBias = 0.0f;

    /* Set the particle's life span */
    PrtData.EmtStd->prtLife = 10.0f;
    PrtData.EmtStd->prtLifeBias = 0.0f;

    /* Set the particles emission speed */
    PrtData.EmtStd->prtInitVel = 1.0f;
    PrtData.EmtStd->prtInitVelBias = 0.4f;

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

    /* No rotation */
    PrtData.EmtStd->prtDelta2DRotate = (RwReal) 0.0;

    PrtEmitterIm3DCreate(PrtData.EmtStd);

    return TRUE;
}


/*
 *****************************************************************************
 */
RwBool
PrtStdInitialize(void)
{
    RwInt32 i;
    RwFrame *frame;
    RwChar *path;
    RpAtomic *pTank;

    /* Create the glow table for our particle */
    PrtData.numGlow = PARTICLEGLOWCOUNT;
    PrtData.prtGlowTable = (ParticleGlowTable * )
        RwMalloc(sizeof(ParticleGlowTable) * (PARTICLEGLOWCOUNT + 1),
                 rwID_NAOBJECT);

    i = 0;
    PrtData.prtGlowTable[i].t = (RwReal) 0.0;
    PrtData.prtGlowTable[i].glow = (RwReal) 0.0;

    i++;
    PrtData.prtGlowTable[i].t = (RwReal) 0.25;
    PrtData.prtGlowTable[i].glow = (RwReal) 1.0;

    i++;
    PrtData.prtGlowTable[i].t = (RwReal) 0.5;
    PrtData.prtGlowTable[i].glow = (RwReal) 0.0;

    i++;
    PrtData.prtGlowTable[i].t = (RwReal) 0.75;
    PrtData.prtGlowTable[i].glow = (RwReal) 1.0;

    i++;
    PrtData.prtGlowTable[i].t = (RwReal) 1.0;
    PrtData.prtGlowTable[i].glow = (RwReal) 0.0;

    for (i = 0; i < PARTICLEGLOWCOUNT; i++)
    {
        PrtData.prtGlowTable[i].iDeltaT =
            (RwReal) 1 / (PrtData.prtGlowTable[i + 1].t -
                          PrtData.prtGlowTable[i].t);
    }

    PrtData.prtGlowTable[i].iDeltaT = (RwReal) 0.0;

    /* Create an emitter class suporting the standard
     * emitter properties
     * and owning and controling a PTank */
    PrtData.eClass = EmitterClassCreate();
    if( NULL == PrtData.eClass )
    {
        return FALSE;
    }

    /* Create a particle class suporting the standard
     * particle properties
     * plus a velocity per particles and the particle glow property. */
    PrtData.pClass = ParticleClassCreate();
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
    PrtData.offsetEmtStd = RpPrtStdPropTabGetPropOffset(
        PrtData.eClass->propTab, rpPRTSTDPROPERTYCODEEMITTERSTANDARD);

    PrtData.EmtStd = (RpPrtStdEmitterStandard *)
        PARTICLEPROP(PrtData.emitter, PrtData.offsetEmtStd);

    /* Get the Emitter PTank property pointer */
    PrtData.offsetEmtPTank = RpPrtStdPropTabGetPropOffset(
        PrtData.eClass->propTab, rpPRTSTDPROPERTYCODEEMITTERPTANK);

    PrtData.EmtPtank = (RpPrtStdEmitterPTank *)
        PARTICLEPROP(PrtData.emitter, PrtData.offsetEmtPTank);

    /* Get the Emitter Particle Glow property pointer */
    PrtData.offsetEmtPrtGlow = RpPrtStdPropTabGetPropOffset(
        PrtData.eClass->propTab, EMITTERPRTPROPCODEGLOW);

    PrtData.emtPrtGlow = (EmitterPrtGlow *)
        PARTICLEPROP(PrtData.emitter, PrtData.offsetEmtPrtGlow);
     /* Get the particle offsets */
    PrtData.offsetPrtStd = RpPrtStdPropTabGetPropOffset(
        PrtData.pClass->propTab, rpPRTSTDPROPERTYCODEPARTICLESTANDARD);

    PrtData.offsetPrtGlow = RpPrtStdPropTabGetPropOffset(
        PrtData.pClass->propTab, PARTICLEPROPCODEGLOW);

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

    /* Setup the pTank. */
    pTank = ParticleGlowCreatePTank();

    RpPrtStdEmitterLinkPTank(PrtData.emitter, pTank);

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

    if( NULL != PrtData.pClass )
    {
        RpPrtStdPClassDestroy(PrtData.pClass);
    }

    if( NULL != PrtData.eClass )
    {
        RpPrtStdEClassDestroy(PrtData.eClass);
    }

    if( NULL != PrtData.prtGlowTable )
    {
        RwFree(PrtData.prtGlowTable);
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


