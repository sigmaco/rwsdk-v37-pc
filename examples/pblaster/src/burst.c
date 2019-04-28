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
 * burst.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: The PBlaster example demonstrate a blaster gun effects, using both 
 * 3d matrix based particles to generate the laser beams and screen aligned 
 * particles for the charging effect.
 *
 ****************************************************************************/
#if (defined(__MWERKS__) && defined(__PPCGEKKO__))
#include "dtrigf.h"
#endif /* (defined(__MWERKS__) && defined(__PPCGEKKO__)) */

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"
#include "menu.h"

#include "rpptank.h"
/* 
 * Just to get the random macros
 */
#include "rpprtstd.h"

#include "burst.h"

RpAtomic *BurstPTank;

RwInt32 PrtPerBurst;
RwInt32 BurstInRenderList;

RwBool            InRenderList;
RpPTankLockStruct BurstPosLock;

static RwTexture *BurstPTankTexture;
RwInt32 MaxBurst;

static burstObj *BurstObjList;
static RwInt32 ActiveBurst;


/*
 *****************************************************************************
 */
static __inline void
sincos(float angle,
       register float *sine,
       register float *cosine)
{

#if( (defined(WIN32)) || (defined(_XBOX)) )
    __asm
    {
        fld        dword ptr angle
        fsincos
        mov        eax, sine
        mov        edx, cosine
        fstp    dword ptr [edx]
        fstp    dword ptr [eax]
    }
#elif (defined(__MWERKS__) && defined(__PPCGEKKO__))
    *sine = sinf_f(angle);
    *cosine = cosf_f(angle);
#else
    *sine = (RwReal)RwSin(angle);
    *cosine = (RwReal)RwCos(angle);
#endif
}


/*
 *****************************************************************************
 */
RwBool
BurstsInitialize(RwInt32 numMaxBurst, RwInt32 numPrtPerBurst)
{
    RwTexCoords cUV[2] = { { 0.0f, 0.0f } , { 1.0f, 1.0f } };
    RwV2d cSize = { 2.0f, 2.0f};
    RwRGBA cColor = { 64, 64, 255, 255 };

    RwInt32 totalPrt;
    RwChar  *path;
    RpAtomic *ptank;
    RwFrame *frame;
    
    /*
     * Calculate total number of particles
     */
    totalPrt = numMaxBurst * numPrtPerBurst;

    if( 0 == totalPrt )
    {
        return FALSE;
    }

    /*
     * Create burst global PTank :
     * One PTank contains all the burst, reducing 
     * the number of RpAtomic render to one, 
     * whatever the number of burst is.
     */

    /*
     * Load the texture...
     */
    path = RsPathnameCreate(RWSTRING("textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    
    BurstPTankTexture = RwTextureRead(RWSTRING("particle"), RWSTRING("particle"));

    if( BurstPTankTexture )
    {
        RwTextureSetFilterMode(BurstPTankTexture, rwFILTERLINEAR  );
    }

    /*
     * Create the Ptank atomic : Position per particles, 
     * constant color,
     * constant set of texture coordinates.
     */
    ptank = RpPTankAtomicCreate(totalPrt,
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

    if( NULL == ptank )
    {
        RwTextureDestroy(BurstPTankTexture);

        return FALSE;
    }

    /*
     * Create a frame and attach it to the ptank 
     */
    frame = RwFrameCreate();

    RwMatrixSetIdentity(RwFrameGetMatrix(frame));

    RpAtomicSetFrame(ptank,frame);

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
    RpPTankAtomicSetTexture(ptank,BurstPTankTexture);
    
    /*
     * All the particles are using the same texture coordinate set
     */
    RpPTankAtomicSetConstantVtx2TexCoords(ptank, cUV);
    
    /*
     * All the particles are using the same color
     */
    RpPTankAtomicSetConstantColor(ptank, &cColor);

    /*
     * All the particles are using the same size
     */
    RpPTankAtomicSetConstantSize(ptank,&cSize);


    BurstPTank = ptank;
    

    MaxBurst = numMaxBurst;
    PrtPerBurst = numPrtPerBurst;
    BurstInRenderList = 0;

    InRenderList = FALSE;
    
    BurstPosLock.data = NULL;
    BurstPosLock.stride = 0;

    /*
     * Create burst object list
     */
    BurstObjList = (burstObj*)RwMalloc(MaxBurst * sizeof(burstObj),rwID_NAOBJECT);

    if( NULL == BurstObjList )
    {
        return FALSE;
    }

    ActiveBurst = 0;

    return TRUE;
}


/*
 *****************************************************************************
 */
void     
BurstsDestroy(void)
{
    if( BurstPTank )
    {
        RwFrame *frame = RpAtomicGetFrame(BurstPTank);
        
        RpAtomicDestroy(BurstPTank);
        
        RwFrameDestroy(frame);
        
        BurstPTank = NULL;
    }

    if( NULL != BurstObjList )
    {
        RwFree(BurstObjList);
    }
}


/*
 *****************************************************************************
 */
burstObj *
BurstGetNewBurst(void)
{
    burstObj *burst;
    
    /*
     * get a new burst object for the burst object list,
     * the list will loop around if more than ActiveBurst number of burst is 
     * needed. 
     */
    burst = &BurstObjList[ActiveBurst];

    ActiveBurst++;
    if( ActiveBurst == MaxBurst )
    {
        ActiveBurst = 0;
    }
    
    return burst;
}


/*
 *****************************************************************************
 */
extern void     
BurstBeginRenderList(void)
{
    /*
     * Open the burst render list : 
     * - Lock the PTank atomic position
     * - Set up the the InRenderList Flag
     */
    BurstInRenderList = 0;

    RpPTankAtomicLock(BurstPTank, &BurstPosLock, rpPTANKLFLAGPOSITION, rpPTANKLOCKWRITE);

    InRenderList = (BurstPosLock.data != NULL );
    
    return;
}


/*
 *****************************************************************************
 */
void     
BurstAddToRenderList(burstObj **BurstObjList, RwInt32 numBurst)
{
    RwInt32 freeBurst;
    RwInt32 i;

    if( FALSE == InRenderList )
    {
        return;
    }

    freeBurst = (MaxBurst - BurstInRenderList);

    if( freeBurst < numBurst )
    {
        numBurst = freeBurst;
    }

    /*
     * Generate bursts particles, based on the system time,
     * more than one burst can be added through this function
     */
    for(i=0;i<numBurst;i++)
    {
        RwInt32 j;
        RwReal angle1;
        RwReal angle2;

        RwReal sinA2,cosA2;
        RwReal sinA1,cosA1;

        RwV3d  pos;
        RwReal localTime;
        RwUInt32 seed = 1;

        if ( BurstObjList[i]->time >= 0.0f )
        {
            /*
             * The burst animation is parametric : only the burst system time 
             * and radius are needed to generate particle states, allowing for 
             * forward and backward animations, and reducing the memory footpring
             * of the effects.
             */
            for(j=0; j<PrtPerBurst; j++)
            {
                /*
                 * Each particles lies on the burst sphere, specified by 
                 * the burst orientation and it's radius.
                 * two angles defines the positions of the particles on the sphere:
                 * - angle 1 defines particles position around the AT vector of the matrix
                 * - angle 2 defines particles position from the base of the sphere to it's top
                 */

                /*
                 * Angle 1 : based only a random value 
                 */
                PRTSTD_SRAND(seed);
                angle1 = (rwPI/2.0f) + (PRTSTD_2RSRAND2(seed))* rwPI*2.0f;

                /*
                 * Angle 2 : based on the burst global time and a local, per particle,
                 * time multiplier.
                 * As the localtime goes from 0.0f to 1.0f, the particles are moving allong the sphere
                 * from it's base to it's top
                 */
                localTime = BurstObjList[i]->time * PRTSTD_2RSRAND2(seed);

                if( localTime > 1.0f )
                {
                    /* CodeWarrior PC 8.3 compiler bug temporary fixup!! */
#if (defined(__MWERKS__) && defined(WIN32))
                    volatile RwInt32	temp = (RwInt32)(localTime);

                    localTime = localTime - temp;
#else /* (defined(__MWERKS__) && defined(WIN32)) */
                    localTime = localTime-(RwInt32)(localTime);
#endif /* (defined(__MWERKS__) && defined(WIN32)) */
                }

                angle2 = (rwPI/2.0f) + (1.0f - localTime) * rwPI;

                /*
                 * Calculate angle1 and angle 2 sinus and cosinus
                 */
                sincos(angle1, &sinA1, &cosA1);
                sincos(angle2, &sinA2, &cosA2);

                /*
                 * Calculate the particle position in object space
                 */
                pos.x = (BurstObjList[i]->radius.x)*cosA2*cosA1;
                pos.y = (BurstObjList[i]->radius.y)*cosA2*sinA1;
                pos.z = (BurstObjList[i]->radius.z)*sinA2;

                /*
                 * Transform the position in world space
                 */
                RwV3dTransformPoint((RwV3d*)BurstPosLock.data, &pos, &BurstObjList[i]->orientation);

                BurstPosLock.data += BurstPosLock.stride;
            }

            BurstInRenderList++;
        }
    }
}


/*
 *****************************************************************************
 */
extern void     
BurstEndRenderList(void)
{
    /*
     * Close the burst render list : 
     * - Unlock the PTank atomic position
     * - Set up the number of particles in the ptank according to the number 
     * of burst in the render list.
     */
    RpPTankAtomicUnlock(BurstPTank);
    
    RpPTankAtomicSetActiveParticlesCount(BurstPTank,PrtPerBurst * BurstInRenderList);

    InRenderList = FALSE;
}


/*
 *****************************************************************************
 */
extern void
BurstsRender(void)
{
    /*
     * Render the particles : with Z test enabled but no Z write.
     */
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);

    if( FALSE == InRenderList && 0 < BurstInRenderList )
    {
        RpAtomicRender(BurstPTank);
    }
}
