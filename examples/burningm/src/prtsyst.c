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
 * scene.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example of a particle system, using a ptank for the rendering, 
 *          using a skinned character as an emitter.
 *
*****************************************************************************/
#include <rwcore.h>

#include <rpworld.h>
#include <rpskin.h>
#include <rpptank.h>

#include <rpprtstd.h> /* random macros ! */

#include <skeleton.h>

#include "prtsyst.h"


#define DONT_SKINx


/*
 *****************************************************************************
 */
prtSystObj *
PrtSystemCreate(RwInt32 numPrt, 
                RwReal life, 
                RwReal frameRate, 
                RpAtomic *atomic, 
                RwTexture *texture)
{
    prtSystObj *syst = NULL;	
	RwUInt32 size;
	RwUInt32 numPrtInNewBuffer;

    RpPTankLockStruct shotPositionLock;
    RwV3d *posOut;
    RpPTankLockStruct shotColorLock;
    RwRGBA *colOut;
    
    /*
     * Calculating size of particle object
     */
    size = sizeof(prtSystObj);

	/*
     * Adding particles buffer 
     */
    size += numPrt*sizeof(prtObj);
    
	/*
     * Adding new particles buffer :
     * As we're using a fixed framerate for the particle system, 
     * we can guaranty a fixed number of particle emitted per frame, 
     * allowing us to emit all the new particles in one go
     */
    numPrtInNewBuffer = RwInt32FromRealMacro(  
                            (((RwReal)numPrt / life) * frameRate)
                            );

    size += numPrtInNewBuffer*sizeof(prtObjInit);


    syst = (prtSystObj *)RwMalloc(size,rwID_NAOBJECT);


	if(syst != NULL)
	{	
        RwTexCoords cUV[2] = { { 0.0f, 0.0f } , { 1.0f, 1.0f } };
        RwV2d cSize = { 6.0f, 6.0f };
		const RwV3d zero = {0.0f, 0.0f, 0.0f};		
        RpAtomic *ptank;
    	RwInt32 i;  

        /*
         * Setting up particle buffer pointer
         */
		syst->prtList = (prtObj *)((RwUInt32)syst + sizeof(prtSystObj));

        /*
         * Setting up new particle buffer pointer
         */
		syst->prtListNext = (prtObjInit *)(
                            (RwUInt32)(syst->prtList) + numPrt * sizeof(prtObj) 
                            );

        syst->friction = 0.0f;
        syst->gravity = zero;
        syst->wind = zero;

		syst->color.red = 128.0f;
		syst->color.green = 0.0f;
		syst->color.blue = 128.0f;
		syst->color.alpha = 128.0f;

        syst->colorDelta = 0.0f;
        syst->colorInitial = 0.0f;

        syst->systemTime = 0.0f;

        syst->seed = 1;
        syst->currentVtx = 0;

        syst->active = FALSE;

        syst->atomic = NULL;       	

        syst->speed = 0.0f;


        syst->rndSpeed = 0.0f;
        syst->disp = 0.0f;

	    syst->atomic = atomic;

        syst->actPrtNum = 0;

        syst->maxPrtNum = numPrt;
        syst->prtLife = life;

	    syst->frameRate = frameRate;
	    syst->systemTime  = 0.0f;
	    syst->frameLocker = 0.0f;

        syst->emitionRate = numPrtInNewBuffer;

        /*
         * Creating a ptank, with Position, and color per particles
         */
        ptank = RpPTankAtomicCreate(numPrt,
                            rpPTANKDFLAGPOSITION |
                            rpPTANKDFLAGCNSVTX2TEXCOORDS |
                            rpPTANKDFLAGCOLOR,
#if defined(D3D8_DRVMODEL_H)
                            /*
                             * Try to use point sprites if they are 
                             * available on the current hardware.
                             */
                            rpPTANKD3D8FLAGSUSEPOINTSPRITES);
#elif defined(D3D9_DRVMODEL_H)
                            /*
                             * Try to use point sprites if they are 
                             * available on the current hardware.
                             */
                            rpPTANKD3D9FLAGSUSEPOINTSPRITES);
#elif defined(SKY2_DRVMODEL_H)
                            /*
                             * We're using multibuffering on PS2 so
                             * that we can update one buffer while 
                             * rendering the other one
                             */
                            rpPTANKSKYFLAGINSTANCEBUFFER);
#else
                            0);
#endif

        /*
         * Create a frame and attach it to the ptank 
         */
        RpAtomicSetFrame(ptank,RpAtomicGetFrame(atomic));

        /*
         * deactivate all the particle in the PTank 
         */
        RpPTankAtomicSetActiveParticlesCount(ptank,0);
    
        /*
         * the PTank is using Vertex Alpha Blending
         */
        RpPTankAtomicSetVertexAlpha(ptank, TRUE);

        /*
         * Set a basic blending mode for the PTank
         */
        RpPTankAtomicSetBlendModes(ptank,rwBLENDONE,rwBLENDONE);
    
        /*
         * Set the texture
         */
        RpPTankAtomicSetTexture(ptank,texture);
    
        /*
         * All the particles are using the same texture coordinate set
         */
        RpPTankAtomicSetConstantVtx2TexCoords(ptank, cUV);

        /*
         * All the particles are using the same size
         */
        RpPTankAtomicSetConstantSize(ptank, &cSize);

        RpPTankAtomicLock(ptank, &shotPositionLock, 
                                rpPTANKLFLAGPOSITION, 
                                rpPTANKLOCKWRITE);


        RpPTankAtomicLock(ptank, &shotColorLock, 
                                rpPTANKLFLAGCOLOR, 
                                rpPTANKLOCKWRITE);

        if( (NULL == shotColorLock.data) ||
            (NULL == shotPositionLock.data) )
        {
            RpPTankAtomicDestroy(ptank);
            RwFree(syst);

            return NULL;
        }

        /*
         * Clear all the particle's data
         */
        for(i = 0; i < syst->maxPrtNum ;i++) 
	    {
		    syst->prtList[i].state = 0;
		    syst->prtList[i].spd = zero;
		    syst->prtList[i].age = 1.0f;

            posOut = (RwV3d*)shotPositionLock.data;
            shotPositionLock.data += shotPositionLock.stride;

            colOut = (RwRGBA*)shotColorLock.data;
            shotColorLock.data += shotColorLock.stride;

            *posOut = zero;
            colOut->red = 
            colOut->green = 
            colOut->blue = 
            colOut->alpha = 0;

	    }

        RpPTankAtomicUnlock(ptank);

        syst->ptank = ptank;

	}	

	return syst;
}


/*
 *****************************************************************************
 */
void
PrtSystemDestroy(prtSystObj *syst)
{
    if( NULL != syst->ptank )
    {
        RpPTankAtomicDestroy(syst->ptank);
    }

    RwFree(syst);
}


/*
 *****************************************************************************
 */
void
PrtSystemSetParticleFadeRange(prtSystObj *syst, RwReal initf, RwReal finalf)
{
    /*
     * To simplify runtime operations, we just store the delta and initial 
     * color.
     * a particle color is calculated as following:
     * color = color*(colorInitial + age * colorDelta)
     */
	syst->colorDelta = (finalf - initf) / syst->prtLife;
    syst->colorInitial = initf; 
}


/*
 *****************************************************************************
 */
#define GetBoneLTM(hierarchy,boneIndex) \
        (&(hierarchy->pNodeInfo[boneIndex].pFrame->ltm))

#if !((defined(D3D8_DRVMODEL_H) || defined(D3D9_DRVMODEL_H) || defined(OPENGL_DRVMODEL_H)))
static void
EmitNew(prtObjInit *dest, prtSystObj *syst, RwInt32 newPrt)
{
    RwInt32 i;

    RpAtomic      *atm = syst->atomic; 
	RpGeometry    *geom = RpAtomicGetGeometry(atm); 
    RwReal        numVtx = (RwReal)RpGeometryGetNumVertices(geom);
    RpMorphTarget *morphTarget;
    RwV3d         *verts;
    RwV3d         *norms;
    RwUInt32      numVerts;
    RpSkin        *skin = RpSkinAtomicGetSkin(atm);
    RpHAnimHierarchy *hierarchy;

    const RwUInt32 *boneIndices;
    const RwMatrixWeights *weights;
    const RwMatrix *skinToBones;
               
	RwReal speed;   
    
	RwInt32 vtxIdx = 0;
	RwV3d Vnorm;

    /*
     * Emit a series of new particles
     */


    /*
     * each atomic must only have one morph target, which should be the
     * case for all skinned atomics
     */
    if( RpGeometryGetNumMorphTargets(geom) > 1 )
    {
        RsErrorMessage(RWSTRING("ERROR: Atomic with more than 1 morph target found!"));
    }
    
    morphTarget = RpGeometryGetMorphTarget(geom, 0);
    numVerts = RpGeometryGetNumVertices(geom);
    verts = RpMorphTargetGetVertices(morphTarget);
    norms = RpMorphTargetGetVertexNormals(morphTarget);

    hierarchy = RpSkinAtomicGetHAnimHierarchy(atm);

    boneIndices = RpSkinGetVertexBoneIndices(skin);
    weights = RpSkinGetVertexBoneWeights(skin);
    skinToBones = RpSkinGetSkinToBoneMatrices(skin);
                    

    for(i=0;i<newPrt;i++)
    {
        /*
         * Randomly selecte a new vertex
         */ 
        PRTSTD_SRAND(syst->seed);
        vtxIdx = RwInt32FromRealMacro(PRTSTD_2RSRAND2(syst->seed) * numVtx);

#ifndef DONT_SKIN
        if(NULL != skin)
        {
            RwUInt32 bonesIdx0,bonesIdx1,bonesIdx2,bonesIdx3;

            RwMatrix *boneLTM0, *boneLTM1, *boneLTM2, *boneLTM3;
            RwMatrix tmpMtx0, tmpMtx1, tmpMtx2, tmpMtx3;
            const RwMatrix *matrix0, *matrix1, *matrix2, *matrix3;
            RwV3d sVert0, sVert1, sVert2, sVert3;
            RwV3d skinedPos = {0.0f,0.0f,0.0f};
            RwV3d skinedNorm = {0.0f,0.0f,0.0f};

            /*
             * Get the index of the bones needed to perform vertex skinning
             */ 
            bonesIdx0 = (boneIndices[vtxIdx] & 0x000000ff);
            bonesIdx1 = (boneIndices[vtxIdx] & 0x0000ff00) >> 8;
            bonesIdx2 = (boneIndices[vtxIdx] & 0x00ff0000) >> 16;
            bonesIdx3 = (boneIndices[vtxIdx] & 0xff000000) >> 24;

            /*
             * Apply skinning using only the needed number of bones
             */ 
            if( bonesIdx3 ) goto weight3;
            if( bonesIdx2 ) goto weight2;
            if( bonesIdx1 ) goto weight1;
            
            goto weight0;

            weight3:
            /*
             * Apply bone 3 skinning if applicable
             */

            /*
             * Get the skin to bone matrix for bone 3
             */
            matrix3 = &skinToBones[bonesIdx3];

            /*
             * Get bone 3 ltm
             */
            boneLTM3 = GetBoneLTM(hierarchy,bonesIdx3);

            /*
             * TmpMtx3 is now the full skinning matrix for bone 3
             */
            RwMatrixMultiply(&tmpMtx3, matrix3, boneLTM3);

            /*
             * Apply skining on the vertex
             */
            RwV3dTransformPoint(&sVert3, &verts[vtxIdx], &tmpMtx3);

            /*
             * Apply bone 3 weigth
             */
            RwV3dScale(&sVert3, &sVert3, weights[vtxIdx].w3);

            /*
             * Add up bone 3 position
             */
            RwV3dAdd(&skinedPos,&skinedPos,&sVert3);

            /*
             * Apply skining on the normal
             */
            RwV3dTransformVector(&sVert3, &norms[vtxIdx], &tmpMtx3);

            /*
             * Apply bone 3 weigth
             */
            RwV3dScale(&sVert3, &sVert3, weights[vtxIdx].w3);

            /*
             * Add up bone 3 normal
             */
            RwV3dAdd(&skinedNorm,&skinedNorm,&sVert3);

            weight2:
            /*
             * Apply bone 2 skinning if applicable
             */
            matrix2 = &skinToBones[bonesIdx2];
            boneLTM2 = GetBoneLTM(hierarchy,bonesIdx2);
            RwMatrixMultiply(&tmpMtx2, matrix2, boneLTM2);

            RwV3dTransformPoint(&sVert2, &verts[vtxIdx], &tmpMtx2);
            RwV3dScale(&sVert2, &sVert2, weights[vtxIdx].w2);
            RwV3dAdd(&skinedPos,&skinedPos,&sVert2);

            RwV3dTransformVector(&sVert2, &norms[vtxIdx], &tmpMtx2);
            RwV3dScale(&sVert2, &sVert2, weights[vtxIdx].w2);
            RwV3dAdd(&skinedNorm,&skinedNorm,&sVert2);

            weight1:
            /*
             * Apply bone 1 skinning if applicable
             */

            matrix1 = &skinToBones[bonesIdx1];
            boneLTM1 = GetBoneLTM(hierarchy,bonesIdx1);
            RwMatrixMultiply(&tmpMtx1, matrix1, boneLTM1);

            RwV3dTransformPoint(&sVert1, &verts[vtxIdx], &tmpMtx1);
            RwV3dScale(&sVert1, &sVert1, weights[vtxIdx].w1);
            RwV3dAdd(&skinedPos,&skinedPos,&sVert1);

            RwV3dTransformVector(&sVert1, &norms[vtxIdx], &tmpMtx1);
            RwV3dScale(&sVert1, &sVert1, weights[vtxIdx].w1);
            RwV3dAdd(&skinedNorm,&skinedNorm,&sVert1);
            
            weight0:
            /*
             * Apply bone 0 skinning if applicable
             */

            matrix0 = &skinToBones[bonesIdx0];
            boneLTM0 = GetBoneLTM(hierarchy,bonesIdx0);
            RwMatrixMultiply(&tmpMtx0, matrix0, boneLTM0);

            RwV3dTransformPoint(&sVert0, &verts[vtxIdx], &tmpMtx0);
            RwV3dScale(&sVert0, &sVert0, weights[vtxIdx].w0);
            RwV3dAdd(&skinedPos,&skinedPos,&sVert0);

            RwV3dTransformVector(&sVert0, &norms[vtxIdx], &tmpMtx0);
            RwV3dScale(&sVert0, &sVert0, weights[vtxIdx].w0);
            RwV3dAdd(&skinedNorm,&skinedNorm,&sVert0);

            /*
             * Store skinned vertex position and normal in world space
             */
            RwV3dAssign(&dest->pos,&skinedPos);
            RwV3dAssign(&Vnorm,&skinedNorm);
        }
        else
#endif /* DONT_SKIN */
        {
	        RwMatrix *ltm = RwFrameGetLTM(RpAtomicGetFrame(atm));

            /*
             * Store non skinned vertex position and normal in world space
             */
	        RwV3dTransformPoint(&dest->pos, &verts[vtxIdx], ltm);
            RwV3dTransformVector(&Vnorm, &norms[vtxIdx], ltm);
        }

        /*
         * Displace new particle along the normal
         */
	    RwV3dIncrementScaled(&dest->pos, &Vnorm, syst->disp);   		
        
        /*
         * Calculate speed
         */
        PRTSTD_SRAND(syst->seed);

        /*
         * Speed = baseSpeed (constant for all particles) + random speed (per particle
         */
        speed = syst->speed + syst->rndSpeed * PRTSTD_RSRAND(syst->seed);

        /*
         * Particles initial direction follow the normal
         */
	    dest->spd.x = Vnorm.x * speed;
	    dest->spd.y = Vnorm.y * speed;
	    dest->spd.z = Vnorm.z * speed;	

        /*
         * Particles age is 0.0f (new born)
         */
	    dest->age   = 0.0f;

        /*
         * Particles state is 1 (active)
         */
	    dest->state = 1;

        dest++;
    }

}
#endif /* !((defined(D3D8_DRVMODEL_H) || defined(D3D9_DRVMODEL_H) || defined(OPENGL_DRVMODEL_H))) */


/*
 *****************************************************************************
 */
void 
PrtSystemUpdate(prtSystObj *syst,RwReal deltaT)
{
    /*
     * The particle system is using a locked frame rate
     * If the deltaT is not fix, we just buildup the delta time
     * until the framelocker reach the desired framerate value
     */
    syst->frameLocker+=deltaT;
    
    if( syst->frameLocker >= syst->frameRate )
    {
        RwBool addPrt = TRUE;
        RwInt32 i;
	    prtObj *currentPrt;
	    RwV3d  force;
        RwInt32 numNewPrt;
        RwRGBAReal initColor;
        RwRGBAReal deltaColor;
        RwRGBA initColorRGBA;

        RpPTankLockStruct shotPositionInLock;
        RwV3d *posIn;
        RpPTankLockStruct shotPositionLock;
        RwV3d *posOut;
        RpPTankLockStruct shotColorLock;
        RwRGBA *colOut;

        /*
         * Clear the framelocker and specify the deltaT 
         * for the particle system
         */
        deltaT = syst->frameRate;
        syst->frameLocker = 0.0f;

	    syst->systemTime   += deltaT; 	
		    
        /*
         * Force is expressed in unit per second, scale down to match deltaT
         */
	    RwV3dScale(&force, &syst->gravity, deltaT);	

        /*
         * Add wind reduced by friction
         */
	    RwV3dIncrementScaled(&force, &syst->wind, syst->friction*deltaT);		

        currentPrt = syst->prtList;
        numNewPrt  = 0;

        if( TRUE == syst->active )
        {
            /*
             * If the system is active (i.e : emits particles)
             * then prepare a set of fresh particle for emitions
             */
#if (defined(D3D8_DRVMODEL_H) || defined(D3D9_DRVMODEL_H) || defined(OPENGL_DRVMODEL_H) )
            EmitNewWin(syst->prtListNext,syst,syst->emitionRate);
#else
            EmitNew(syst->prtListNext,syst,syst->emitionRate);
#endif
        }

        /*
         * Color interpolation precalculations
         */
        initColor.red = syst->color.red * syst->colorInitial;
        initColor.green = syst->color.green * syst->colorInitial;
        initColor.blue = syst->color.blue * syst->colorInitial;
        initColor.alpha = syst->color.alpha * syst->colorInitial;

        
        deltaColor.red = syst->color.red * syst->colorDelta;
        deltaColor.green = syst->color.green * syst->colorDelta;
        deltaColor.blue = syst->color.blue * syst->colorDelta;
        deltaColor.alpha = syst->color.alpha * syst->colorDelta;


        initColorRGBA.red = (RwChar)RwFastRealToUInt32(initColor.red);
        initColorRGBA.green = (RwChar)RwFastRealToUInt32(initColor.green);
        initColorRGBA.blue = (RwChar)RwFastRealToUInt32(initColor.blue);
        initColorRGBA.alpha = (RwChar)RwFastRealToUInt32(initColor.alpha);

#ifdef SKY        
        /*
         * As we're using multibuffering on PS2 we
         * need to swap the frames so that we can update 
         * the next buffer
         */
        RpPTankAtomicSkySwapFrames(syst->ptank);
#endif /* SKY */

        /*
         * Lock the ptank positions and colors
         */
        RpPTankAtomicLock(syst->ptank, &shotPositionLock, 
                                rpPTANKLFLAGPOSITION, 
                                rpPTANKLOCKWRITE);

        shotPositionInLock = shotPositionLock;

#ifdef SKY        
        /*
         * As we're using multibuffering on PS2, the previous 
         * positions are in the buffer being rendered.
         * we change the position in pointer to get the correct values
         * There is no need to do so for the color as 
         * there value doesn't depends on the previous frame
         */
        shotPositionInLock.data = 
                RpPTankAtomicSkyGetPreviousBuffer(syst->ptank, rpPTANKLFLAGPOSITION);
#endif /* SKY */

        RpPTankAtomicLock(syst->ptank, &shotColorLock, 
                                rpPTANKLFLAGCOLOR, 
                                rpPTANKLOCKWRITE);

        /*
         * Update particles
         */
	    for(i = 0; i < syst->maxPrtNum; i++) 
	    {
            /*
             * Get the current particle position and color pointers
             */
            posOut = (RwV3d*)shotPositionLock.data;
            shotPositionLock.data += shotPositionLock.stride;

            posIn = (RwV3d*)shotPositionInLock.data;
            shotPositionInLock.data += shotPositionInLock.stride;

            colOut = (RwRGBA*)shotColorLock.data;
            shotColorLock.data += shotColorLock.stride;
    
		    if(currentPrt->state)
		    {
			    currentPrt->age += deltaT;
			    if(currentPrt->age >= syst->prtLife ) 
			    {
                    /*
                     * Particles is dead
                     */
				    currentPrt->state = 0;
                    currentPrt->age = syst->prtLife;
				    syst->actPrtNum--;
			    }
			    else
			    {
                    /*
                     * Integrate particle speed
                     */
				    RwV3dIncrementScaled(&currentPrt->spd, &currentPrt->spd, -syst->friction*deltaT);	
				    RwV3dAdd(&currentPrt->spd, &currentPrt->spd, &force);

                    /*
                     * Update particle position accordingly
                     */
                    {
                        RwV3d temp;

                        RwV3dScale(&temp,&currentPrt->spd, deltaT);
                        RwV3dAdd(posOut,posIn,&temp);
                    }


                    /*
                     * No need for a new particle
                     */
                    addPrt = FALSE;
			    }   

                /*
                 * Color interpolation
                 */
                colOut->red = (RwChar)RwFastRealToUInt32(initColor.red + deltaColor.red * currentPrt->age);
                colOut->green = (RwChar)RwFastRealToUInt32(initColor.green + deltaColor.green * currentPrt->age);
                colOut->blue = (RwChar)RwFastRealToUInt32(initColor.blue + deltaColor.blue * currentPrt->age);
                colOut->alpha = (RwChar)RwFastRealToUInt32(initColor.alpha + deltaColor.alpha * currentPrt->age);
            
		    }     

            if( addPrt == TRUE && numNewPrt < syst->emitionRate && syst->active == TRUE) 
			{
                /*
                 * Get a new particle
                 */
                *currentPrt = *(prtObj*)(&syst->prtListNext[numNewPrt]);
                currentPrt->age = syst->prtListNext[numNewPrt].age;
                currentPrt->spd = syst->prtListNext[numNewPrt].spd;
                currentPrt->state = syst->prtListNext[numNewPrt].state;

                /*
                 * Assign the initial position
                 */
                RwV3dAssign(posOut,&syst->prtListNext[numNewPrt].pos);

                /*
                 * Assign the initial color
                 */
                *colOut = initColorRGBA;
            
                numNewPrt++;
                syst->actPrtNum++;
			}

            addPrt = TRUE;


            currentPrt++;
	    } 

    }

    /*
     * Unlock ptank
     */
    RpPTankAtomicUnlock(syst->ptank);

    /*
     * Set number of particle to be rendered by the ptank
     */
    RpPTankAtomicSetActiveParticlesCount(syst->ptank,syst->actPrtNum);

}


/*
 *****************************************************************************
 */
void
PrtSystemRender(prtSystObj *syst)
{
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void *)FALSE);

    RpAtomicRender(syst->ptank);

	RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void *)TRUE);
  	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void *)TRUE);
}

