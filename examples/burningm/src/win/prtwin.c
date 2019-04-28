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

#define GetBoneLTM(hierarchy,boneIndex) \
        (&(hierarchy->pNodeInfo[boneIndex].pFrame->ltm))

static RwV3d positionCache[2500];
static RwV3d normalCache[2500];
static RwInt32 indexCache[2500];

static RwV3d positionCacheOut[2500];
static RwV3d normalCacheOut[2500];

#ifdef    __cplusplus
extern              "C"
{
#endif /* __cplusplus */

/*
 * Exposing CPU skining functions 
 */

#if defined(D3D8_DRVMODEL_H)
extern RwMatrix *
_rwD3D8SkinPrepareMatrix( RpAtomic *atomic, 
                          RpSkin *skin, 
                          RpHAnimHierarchy *hierarchy);

#elif defined(D3D9_DRVMODEL_H)
extern RwMatrix *
_rwD3D9SkinPrepareMatrix( RpAtomic *atomic, 
                          RpSkin *skin, 
                          RpHAnimHierarchy *hierarchy);
#elif defined(OPENGL_DRVMODEL_H)
extern RwMatrix *
_rpSkinOpenGLPrepareAtomicMatrix( RpAtomic *atomic, 
		                          RpSkin *skin, 
		                          RpHAnimHierarchy *hierarchy);
#elif defined(SOFTRAS_DRVMODEL_H)
extern RwMatrix *
_rpSoftRasSkinPrepareAtomicMatrix( RpAtomic *atomic,
                                   RpSkin *skin,
                                   RpHAnimHierarchy *hierarchy );
#else
#error This file is not compatible with a target other than D3D8, D3D9, OpenGL or Softras
#endif


extern void _rpSkinIntelSSEMatrixBlend( RwInt32 numVertices,
                                        const RwMatrixWeights *matrixWeightsMap,
                                        const RwUInt32 *matrixIndexMap,
                                        const RwMatrix *matrixArray,
                                        RwUInt8 *vertices,
                                        const RwV3d *originalVertices,
                                        RwUInt8 *normals,
                                        const RwV3d *originalNormals,
                                        RwUInt32 stride);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

void
EmitNewWin(prtObjInit *dest, prtSystObj *syst, RwInt32 newPrt)
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


    if( newPrt > 2500 )
    {
        /*
         * The cache we're using can't emit more 
         * than 2500 particles in a row
         */
        return;
    }

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

    if (_rwIntelSSEsupported())
    {
        RwMatrix *matrixArray;

        for(i=0;i<newPrt;i++)
        {
            /*
             * Randomly selecte a new vertex
             */ 
            PRTSTD_SRAND(syst->seed);
            vtxIdx = RwInt32FromRealMacro(PRTSTD_2RSRAND2(syst->seed) * numVtx);

            /*
             * Copy the random vertex's position, normal and bones index to the cache
             */ 
            positionCache[i] = verts[vtxIdx];
            normalCache[i] = norms[vtxIdx];
            indexCache[i] = boneIndices[vtxIdx];

        }

        /*
         * We're using the CPU skining code from the rendering pipeline 
         * to accelerate our skinning.
         * This code is not normally exposed, as it wasn't meant to be used
         * that way.
         */ 

        /*
         * Get and setup the matrix array to be used.
         */ 
#if defined(D3D8_DRVMODEL_H)
        matrixArray = _rwD3D8SkinPrepareMatrix(syst->atomic, skin, hierarchy);
#elif defined(D3D9_DRVMODEL_H)
        matrixArray = _rwD3D9SkinPrepareMatrix(syst->atomic, skin, hierarchy);
#elif defined(OPENGL_DRVMODEL_H)
        matrixArray = _rpSkinOpenGLPrepareAtomicMatrix(syst->atomic, skin, hierarchy);
#elif defined(SOFTRAS_DRVMODEL_H)
        matrixArray = _rpSoftRasSkinPrepareAtomicMatrix(syst->atomic, skin, hierarchy);
#endif


        /*
         * Apply skinning using SSE asm
         */ 
        _rpSkinIntelSSEMatrixBlend(newPrt,
                                   weights,
                                   (const RwUInt32 *)indexCache,
                                   matrixArray,
                                   (RwUInt8*)positionCacheOut,
                                   positionCache,
                                   (RwUInt8*)normalCacheOut,
                                   normalCache,
                                   sizeof(RwV3d));


        /*
         * The result of the skinning (in positionCacheOut and normalCacheOut) are in object space,
         * we need to transform them to world space
         */
        RwV3dTransformPoints(positionCache, positionCacheOut, newPrt, RwFrameGetLTM(RpAtomicGetFrame(syst->atomic)));
        RwV3dTransformVectors(normalCache, normalCacheOut, newPrt, RwFrameGetLTM(RpAtomicGetFrame(syst->atomic)));

        /*
         * Assign the result of the skinning (in positionCache and normalCache) to the particles
         * and set up the other particle properties
         */
        for(i=0;i<newPrt;i++)
        {
            RwV3dAssign(&dest->pos,&positionCache[i]);
            
            /*
             * Displace new particle along the normal
             */
	        RwV3dIncrementScaled(&dest->pos, &normalCache[i], syst->disp);   		
        
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
	        dest->spd.x = normalCache[i].x * speed;
	        dest->spd.y = normalCache[i].y * speed;
	        dest->spd.z = normalCache[i].z * speed;	

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
    else
    {
        /*
         * if no SSE support is available on the target machine, we'll 
         * use the good old way.
         */
        for(i=0;i<newPrt;i++)
        {
            /*
             * Randomly selecte a new vertex
             */ 
            PRTSTD_SRAND(syst->seed);
            vtxIdx = RwInt32FromRealMacro(PRTSTD_2RSRAND2(syst->seed) * numVtx);

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
}
