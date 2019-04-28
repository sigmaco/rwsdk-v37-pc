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
 * ptank.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: The PTANK2 example shows how to create a PTank.
 * In this second example the particles are long shaped and have no texture.
 * Color is allocated per-particle, 2D rotation is applied per-particle.
 *                         
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"
#include "menu.h"

#include "ptank.h"
#include "rpptank.h"


static RwMatrix *PTankMatrix = NULL;
static RwV3d *NormalsList = NULL;
static RwV3d *PositionsList = NULL;
RwInt32 PTankNumVertices = 0;

RwBool VtxAlphaOn = FALSE;

RwInt32 SrcBlendID;
RwInt32 DestBlendID;

/* General Settings */
#define DONUTNUMSIDES    (50)
#define DONUTNUMRINGS    (50)
#define DONUTRADIUSRING  (8.0f)
#define DONUTRADIUSSIDES (10.0f)

static RwRGBA SpecularColor = { 80, 48, 0, 255};
static RwRGBA DiffuseColor  = { 8,  4, 000, 255};
static RwRGBA AmbientColor  = { 0,   0, 0, 255};

static RwMatrix *PTankLightMatrix = NULL;

static RpAtomic *pTank = NULL;
static RwFrame *frame = NULL;

/*
 *****************************************************************************
 */
static void
PTankRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix *cameraMatrix;
    RwV3d right, up, pos;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    pos = *RwMatrixGetPos(PTankMatrix);

    /*
     * First translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(PTankMatrix, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * ...do the rotations...
     */
    RwMatrixRotate(PTankMatrix, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwMatrixRotate(PTankMatrix, &right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * ...and translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(PTankMatrix, &pos, rwCOMBINEPOSTCONCAT);


    return;
}


/*
 ***************************************************************************
 */
static void
PTankTranslateZ(RwReal zDelta)
{
    RwFrame *cameraFrame;
    RwV3d delta;

    cameraFrame = RwCameraGetFrame(Camera);

    RwV3dScale(&delta, RwMatrixGetAt(RwFrameGetMatrix(cameraFrame)), zDelta);

    RwMatrixTranslate(PTankMatrix, &delta, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
static void
PTankLightRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix *cameraMatrix;
    RwV3d right, up;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    /*
     * Do the rotations...
     */
    RwMatrixRotate(PTankLightMatrix, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwMatrixRotate(PTankLightMatrix, &right, yAngle, rwCOMBINEPOSTCONCAT);


    return;
}


/*
 *****************************************************************************
 */
static void
PTankLighting(void)
{
    RwV3d lightDir;
    RwMatrix *transform = NULL;
    RwInt32 i;
    RwReal lighting = 0.0f;

    RpPTankLockStruct rotLock;
    RpPTankLockStruct posLock;
    RpPTankLockStruct sizeLock;
    RpPTankLockStruct colorLock;

    RwReal c1, c2;

    transform = RwMatrixCreate();
    RwMatrixInvert(transform, PTankMatrix);

    /*
     * Transform the light's direction to the mesh's local space...
     */
    lightDir = *RwMatrixGetAt(PTankLightMatrix);
    RwV3dTransformVector(&lightDir, &lightDir, transform);

    RwMatrixDestroy(transform);

    RpPTankAtomicLock(pTank, &posLock, rpPTANKLFLAGPOSITION, rpPTANKLOCKWRITE);
    RpPTankAtomicLock(pTank, &colorLock, rpPTANKLFLAGCOLOR, rpPTANKLOCKWRITE);
    RpPTankAtomicLock(pTank, &sizeLock, rpPTANKLFLAGSIZE, rpPTANKLOCKWRITE);
    RpPTankAtomicLock(pTank, &rotLock, rpPTANKLFLAG2DROTATE, rpPTANKLOCKWRITE);

    for(i=0; i<PTankNumVertices; i++)
    {
        lighting = -RwV3dDotProduct(&lightDir, &NormalsList[i]);

        /*
         * Set vertex color using the lighting as a blend factor between
         * diffuse color and specular color if lighting is positive or
         * diffuse color and ambient color...
         */
        c1 = RwRealAbs(lighting);
        c2 = 1.0f - c1;

        if( lighting > 0.0f )
        {
            ((RwRGBA*)colorLock.data)->red = 
                                        (RwUInt8)(c1 * SpecularColor.red
                                            + c2 * DiffuseColor.red);
            ((RwRGBA*)colorLock.data)->green = 
                                        (RwUInt8)(c1 * SpecularColor.green
                                        + c2 * DiffuseColor.green);

            ((RwRGBA*)colorLock.data)->blue = 
                                        (RwUInt8)(c1 * SpecularColor.blue
                                        + c2 * DiffuseColor.blue);
        }
        else
        {
            ((RwRGBA*)colorLock.data)->red = 
                                        (RwUInt8)(c1 * AmbientColor.red
                                        + c2 * DiffuseColor.red);
            ((RwRGBA*)colorLock.data)->green = 
                                        (RwUInt8)(c1 * AmbientColor.green
                                        + c2 * DiffuseColor.green);
            ((RwRGBA*)colorLock.data)->blue = 
                                        (RwUInt8)(c1 * AmbientColor.blue
                                        + c2 * DiffuseColor.blue);

        }

        ((RwRGBA*)colorLock.data)->alpha = 16;

        colorLock.data += colorLock.stride;
            
        RwV3dTransformPoint((RwV3d*)posLock.data, &PositionsList[i], PTankMatrix);
        posLock.data += posLock.stride;

        *(RwReal*)rotLock.data = lighting*_RW_pi;
        rotLock.data += rotLock.stride;

        ((RwV2d*)(sizeLock.data))->x = 0.1f;
        ((RwV2d*)(sizeLock.data))->y = 7.5f*(lighting+1.0f);
        sizeLock.data += sizeLock.stride;

    }

    RpPTankAtomicUnlock(pTank);

    return;
}


/*
 *****************************************************************************
 */
void
PTankRender(void)
{
    static RwBool stateSet = FALSE;
    static RwReal translateInc = 0.0f;
    
    PTankRotate(0.5f,0);
    
    translateInc += _RW_pi/200.0f;

    PTankTranslateZ((RwReal)RwSin(translateInc)*0.1f);

	if( stateSet == FALSE )
    {
        /*
         * Particles usually don't need z write enable, though Z Test enable 
         * will be used in most of the case, but as we got only the ptank on 
         * screen there is no real need for it.
         */
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)FALSE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);
    
        stateSet = TRUE;
    }

    /*
     * Update the PTank
     */
	PTankLighting();

    /*
     * Render it
     */
    RpAtomicRender(pTank);

    return;
}


/*
 *****************************************************************************
 */
void
PTankTerminate(void)
{
    if( NormalsList )
    {
        RwFree(NormalsList);

        NormalsList = NULL;
    }

    if( PositionsList )
    {
        RwFree(PositionsList);

        PositionsList = NULL;
    }

    if( pTank )
    {
        RpWorldRemoveAtomic(World,pTank);

        RpAtomicDestroy(pTank);
        RwFrameDestroy(frame);
        pTank = NULL;
    }

    if( PTankMatrix )
    {
        RwMatrixDestroy(PTankMatrix);

        PTankMatrix = NULL;
    }

    if( PTankLightMatrix )
    {
        RwMatrixDestroy(PTankLightMatrix);

        PTankLightMatrix = NULL;
    }

    return;
}


/*
 *****************************************************************************
 */
static RwBool
PTankCreate(void)
{
    int i, j;
    RwReal ringDelta, sideDelta;

    RwV3d        *nrmList;
    RwV3d        *posList;

    RwInt32 numVert;

    RwReal theta, phi, theta1;
    RwReal cosTheta1, sinTheta1;
    RwReal cosPhi, sinPhi, dist;


    numVert = (DONUTNUMSIDES+1) * DONUTNUMRINGS ;

    NormalsList = (RwV3d *)RwMalloc(numVert * sizeof(RwV3d),
                                    rwID_NAOBJECT);
    if( NormalsList == NULL )
    {
        return FALSE;
    }
    nrmList = NormalsList;

    PositionsList = (RwV3d *)RwMalloc(numVert * sizeof(RwV3d),
                                      rwID_NAOBJECT);
    if( PositionsList == NULL )
    {
        return FALSE;
    }
    posList = PositionsList;

    pTank = RpPTankAtomicCreate(numVert,
                            rpPTANKDFLAGPOSITION |
                            rpPTANKDFLAGSIZE |
                            rpPTANKDFLAGCOLOR |
                            rpPTANKDFLAG2DROTATE |
                            rpPTANKDFLAGARRAY,
                            0);

    frame = RwFrameCreate();

    RwMatrixSetIdentity(RwFrameGetMatrix(frame));

    RpAtomicSetFrame(pTank,frame);

    RpPTankAtomicSetActiveParticlesCount(pTank,
        RpPTankAtomicGetMaximumParticlesCount(pTank));
    
    RpPTankAtomicSetVertexAlpha(pTank, TRUE);
    RpPTankAtomicSetBlendModes(pTank,rwBLENDONE,rwBLENDONE);

    ringDelta = 2.0f * rwPI / DONUTNUMRINGS;
    sideDelta = 2.0f * rwPI / DONUTNUMSIDES;

    theta = 0.0f;

    for(i=DONUTNUMRINGS-1; i>=0; i--)
    {
        theta1 = theta + ringDelta;
        cosTheta1 = (RwReal)RwCos(theta1);
        sinTheta1 = (RwReal)RwSin(theta1);

        phi = 0.0f;

        for(j=DONUTNUMSIDES; j>=0; j--)
        {
            phi += sideDelta;
            cosPhi = (RwReal)RwCos(phi);
            sinPhi = (RwReal)RwSin(phi);
            dist = DONUTRADIUSSIDES + DONUTRADIUSRING * cosPhi;

            posList->x = cosTheta1 * dist;
            posList->y = -sinTheta1 * dist;
            posList->z = DONUTRADIUSRING * sinPhi;

            nrmList->x = cosTheta1 * cosPhi;
            nrmList->y = -sinTheta1 * cosPhi;
            nrmList->z = sinPhi;


            posList++;
            nrmList++;

        }

        theta = theta1;
    }

    PTankNumVertices = numVert;

    return TRUE;
}

/*
 *****************************************************************************
 */
RwBool
PTankInitialize(void)
{
    /*
     * Create the matrix for the IM geometry's transformation....
     */
    PTankMatrix = RwMatrixCreate();
    if( PTankMatrix == NULL )
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
        RwV3d  *at, pos;

        cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
        at = RwMatrixGetAt(cameraMatrix);

        RwV3dScale(&pos, at, 40.0f);
        RwV3dAdd(&pos, &pos, RwMatrixGetAt(cameraMatrix));

        RwMatrixTranslate(PTankMatrix, &pos, rwCOMBINEREPLACE);
    }

    /*
     * Create the matrix for the light's transformation....
     */
    PTankLightMatrix = RwMatrixCreate();

    if( PTankLightMatrix == NULL )
    {
        return FALSE;
    }

    RwMatrixSetIdentity(PTankLightMatrix);

    PTankLightRotate(30.0f, 60.0f);

    /*
     * Start with a torus...
     */
    PTankCreate();

    RpWorldAddAtomic(World, pTank);


    return TRUE;
}

/*
 *************************************************************************
 */

