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
 * vshader.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose : RenderWare 3.0 example.
 *
 ****************************************************************************/

#include <xtl.h>
#include <d3d8.h>
#include <d3dx8core.h>

#include "rwcore.h"
#include "rpworld.h"

#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"

#include "rainbowdefs.h"
#include "rainbow.h"

/*--- Global ---*/

/*--- Local Variables ---*/

static RxPipeline   *AtomicVShaderPipe;
static RxXboxAllInOneRenderCallBack DefaultRenderCallback = NULL;

static DWORD
RainbowVertexShaderDeclaration[] =
{
    D3DVSD_STREAM(0),
    D3DVSD_REG(VSD_REG_POS,       D3DVSDT_FLOAT3),  /* POSITION  - register v0 */
    D3DVSD_REG(VSD_REG_NORMAL,    D3DVSDT_NORMPACKED3),     /* NORMAL    - register v1 */
    D3DVSD_END()
};

static DWORD    RainbowVertexShader;
static RpClump  *Clump = NULL;
static RpLight  *Light = NULL;
static RwFrame  *LightFrame = NULL;

extern RwBool       VertexShaderOn;

extern RpWorld      *World;

static RwTexture *texture = NULL;

/*
 ***************************************************************************
 */
void
VertexShaderSetConstantRegisters(RwMatrix *ltm)
{
    RwCamera    *camera;
    RwMatrix    *camLTM;
    RwMatrix    invLtm;
    RwMatrix    invCamMtx;
    D3DMATRIX   viewMatrix;
    D3DMATRIX   worldMatrix;
    D3DMATRIX   projMatrix;
    D3DMATRIX   destMatrix;
    D3DMATRIX   tmpMatrix;
    D3DMATRIX   worldITMat;

    /*
     * View matrix - (camera matrix)
     */
    camera = RwCameraGetCurrentCamera();
    camLTM = RwFrameGetLTM(RwCameraGetFrame(camera));

    RwMatrixSetIdentity(&invCamMtx);
    RwMatrixInvert(&invCamMtx, camLTM);

    viewMatrix.m[0][0] = -invCamMtx.right.x;
    viewMatrix.m[0][1] = -invCamMtx.up.x;
    viewMatrix.m[0][2] = -invCamMtx.at.x;
    viewMatrix.m[0][3] = -invCamMtx.pos.x;

    viewMatrix.m[1][0] = invCamMtx.right.y;
    viewMatrix.m[1][1] = invCamMtx.up.y;
    viewMatrix.m[1][2] = invCamMtx.at.y;
    viewMatrix.m[1][3] = invCamMtx.pos.y;


    viewMatrix.m[2][0] = invCamMtx.right.z;
    viewMatrix.m[2][1] = invCamMtx.up.z;
    viewMatrix.m[2][2] = invCamMtx.at.z;
    viewMatrix.m[2][3] = invCamMtx.pos.z;

    viewMatrix.m[3][0] = 0.0f;
    viewMatrix.m[3][1] = 0.0f;
    viewMatrix.m[3][2] = 0.0f;
    viewMatrix.m[3][3] = 1.0f;

    /* 
     * World matrix
     */
    worldMatrix.m[0][0] = ltm->right.x;
    worldMatrix.m[0][1] = ltm->up.x;
    worldMatrix.m[0][2] = ltm->at.x;
    worldMatrix.m[0][3] = ltm->pos.x;

    worldMatrix.m[1][0] = ltm->right.y;
    worldMatrix.m[1][1] = ltm->up.y;
    worldMatrix.m[1][2] = ltm->at.y;
    worldMatrix.m[1][3] = ltm->pos.y;

    worldMatrix.m[2][0] = ltm->right.z;
    worldMatrix.m[2][1] = ltm->up.z;
    worldMatrix.m[2][2] = ltm->at.z;
    worldMatrix.m[2][3] = ltm->pos.z;

    worldMatrix.m[3][0] = 0.0f;
    worldMatrix.m[3][1] = 0.0f;
    worldMatrix.m[3][2] = 0.0f;
    worldMatrix.m[3][3] = 1.0f;

    /*
     * Projection matrix
     */
    projMatrix.m[0][0] = camera->recipViewWindow.x;
    projMatrix.m[0][1] = 0.0f;
    projMatrix.m[0][2] = 0.0f;
    projMatrix.m[0][3] = 0.0f;

    projMatrix.m[1][0] = 0.0f;
    projMatrix.m[1][1] = camera->recipViewWindow.y;
    projMatrix.m[1][2] = 0.0f;
    projMatrix.m[1][3] = 0.0f;

    projMatrix.m[2][0] = 0.0f;
    projMatrix.m[2][1] = 0.0f;
    projMatrix.m[2][2] = camera->farPlane / (camera->farPlane - camera->nearPlane);
    projMatrix.m[2][3] = -projMatrix.m[2][2] * camera->nearPlane;

    projMatrix.m[3][0] = 0.0f;
    projMatrix.m[3][1] = 0.0f;
    projMatrix.m[3][2] = 1.0f;
    projMatrix.m[3][3] = 0.0f;

    D3DXMatrixMultiply(&tmpMatrix, &viewMatrix, &worldMatrix);
    D3DXMatrixMultiply(&destMatrix, &projMatrix, &tmpMatrix);

    /*
     * Set the constant registers c0-c3 with the transformation matrix
     */
    D3DDevice_SetVertexShaderConstant(VSCONST_REG_TRANSFORM_OFFSET,
                                      (void *)&destMatrix,
                                      VSCONST_REG_TRANSFORM_SIZE);

    D3DDevice_SetVertexShaderConstant(VSCONST_REG_WORLD_TRANSFORM_OFFSET,
                                      (void *)&worldMatrix,
                                      VSCONST_REG_WORLD_TRANSFORM_SIZE);

    RwMatrixInvert(&invLtm, ltm);

    worldITMat.m[0][0] = invLtm.right.x;
    worldITMat.m[0][1] = invLtm.right.y;
    worldITMat.m[0][2] = invLtm.right.z;
    worldITMat.m[0][3] = 0.0f;

    worldITMat.m[1][0] = invLtm.up.x;
    worldITMat.m[1][1] = invLtm.up.y;
    worldITMat.m[1][2] = invLtm.up.z;
    worldITMat.m[1][3] = 0.0f;

    worldITMat.m[2][0] = invLtm.at.x;
    worldITMat.m[2][1] = invLtm.at.y;
    worldITMat.m[2][2] = invLtm.at.z;
    worldITMat.m[2][3] = 0.0f;

    worldITMat.m[3][0] = invLtm.pos.x;
    worldITMat.m[3][1] = invLtm.pos.y;
    worldITMat.m[3][2] = invLtm.pos.z;
    worldITMat.m[3][3] = 1.0f;

    D3DDevice_SetVertexShaderConstant(VSCONST_REG_WORLD_INVERSE_OFFSET,
                                      (void *)&worldITMat,
                                      VSCONST_REG_WORLD_INVERSE_SIZE);
}

/*
 ***************************************************************************
 */
void
VShaderRenderCallBack(RxXboxResEntryHeader *resEntryHeader, void *object, RwUInt8 type, RwUInt32 flags)
{
    RxXboxInstanceData      *instancedMesh;
    DWORD                   oldAddressU, oldAddressV,
                            oldMinFilter, oldMagFilter, oldMipFilter,
                            oldColorArg1, oldColorOp, oldColorArg2,
                            oldAlphaEnable, oldAlphaTest, oldSrcBlend, oldDestBlend;

    if (!VertexShaderOn)
    {
        DefaultRenderCallback(resEntryHeader, object, type, flags);

        return;
    }

    /*
     * Set up vertex shader with required constants
     */
    VertexShaderSetConstantRegisters(RwFrameGetLTM(RpAtomicGetFrame((RpAtomic *)object)));
    RwXboxSetCurrentVertexShader(RainbowVertexShader);

    /*
     * Note previous renderstates on texture stage 0 so we can restore them later.
     * RenderWare caches renderstates for best performance and expects the hardware
     * to be in the same state as we found it.
     */

    RwXboxGetCachedTextureStageState(0, D3DTSS_ADDRESSU, &oldAddressU );
    RwXboxGetCachedTextureStageState(0, D3DTSS_ADDRESSV, &oldAddressV );
    RwXboxGetCachedTextureStageState(0, D3DTSS_MINFILTER, &oldMinFilter);
    RwXboxGetCachedTextureStageState(0, D3DTSS_MAGFILTER, &oldMagFilter);
	RwXboxGetCachedTextureStageState(0, D3DTSS_MIPFILTER, &oldMipFilter);
	RwXboxGetCachedTextureStageState(0, D3DTSS_COLORARG1, &oldColorArg1);
	RwXboxGetCachedTextureStageState(0, D3DTSS_COLOROP, &oldColorOp);
	RwXboxGetCachedTextureStageState(0, D3DTSS_COLORARG2, &oldColorArg2);
    RwXboxGetCachedRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaEnable);
    RwXboxGetCachedRenderState(D3DRS_ALPHATESTENABLE, &oldAlphaTest);
    RwXboxGetCachedRenderState(D3DRS_SRCBLEND, &oldSrcBlend );
    RwXboxGetCachedRenderState(D3DRS_DESTBLEND, &oldDestBlend );

    /*
     * Set up texture stages
     */

    /* stick the rainbow color texture into texture stages 0 & 1 */
    RwXboxRenderStateSetTexture(texture, 0);
    RwXboxRenderStateSetTexture(texture, 1);

    /* the texture doesn't tile so hide any funny artifacts */
    RwXboxSetCachedTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_MIRROR);
    RwXboxSetCachedTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_MIRROR);

    RwXboxSetCachedTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_MIRROR);
    RwXboxSetCachedTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_MIRROR);

    /* Bilinear looks nicer */
    RwXboxSetCachedTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    RwXboxSetCachedTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	RwXboxSetCachedTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

    RwXboxSetCachedTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    RwXboxSetCachedTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	RwXboxSetCachedTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

    /* generate 2 sets of uvs in the vertex shader
    for the same texture and blend them in a funky way */
	RwXboxSetCachedTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	RwXboxSetCachedTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
	RwXboxSetCachedTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE |D3DTA_ALPHAREPLICATE);

	RwXboxSetCachedTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT );
	RwXboxSetCachedTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED );
	RwXboxSetCachedTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE |D3DTA_ALPHAREPLICATE);

	RwXboxSetCachedTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	RwXboxSetCachedTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	
	/* turn off other texture stages */
	RwXboxSetCachedTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);

    /* turn off alpha blending */
    RwXboxSetCachedRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    RwXboxSetCachedRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    RwXboxSetCachedRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO );

    /*
     * Set the stream source - shared by all meshes in geometry
     */
    D3DDevice_SetStreamSource(0,
                              (D3DVertexBuffer *)resEntryHeader->vertexBuffer,
                              resEntryHeader->stride);


    /* for each mesh in geometry */
    for (instancedMesh = resEntryHeader->begin;
         instancedMesh != resEntryHeader->end;
         ++instancedMesh)
    {
        /*
         * Draw the indexed primitive - note primitive type is same for all meshes
         * in geometry
         */
        RwXboxDrawIndexedVertices((D3DPRIMITIVETYPE)resEntryHeader->primType,
                                      instancedMesh->numIndices,
                                      instancedMesh->indexBuffer);
    }

    /*
     * Restore renderstates
     */
    RwXboxRenderStateSetTexture(NULL, 0);
    RwXboxRenderStateSetTexture(NULL, 1);
    RwXboxSetCachedTextureStageState(0, D3DTSS_ADDRESSU, oldAddressU);
    RwXboxSetCachedTextureStageState(0, D3DTSS_ADDRESSV, oldAddressV);
    RwXboxSetCachedTextureStageState(0, D3DTSS_MINFILTER, oldMinFilter);
    RwXboxSetCachedTextureStageState(0, D3DTSS_MAGFILTER, oldMagFilter);
	RwXboxSetCachedTextureStageState(0, D3DTSS_MIPFILTER, oldMipFilter);
	RwXboxSetCachedTextureStageState(0, D3DTSS_COLORARG1, oldColorArg1);
	RwXboxSetCachedTextureStageState(0, D3DTSS_COLOROP, oldColorOp);
	RwXboxSetCachedTextureStageState(0, D3DTSS_COLORARG2, oldColorArg2);
	RwXboxSetCachedTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    RwXboxSetCachedRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaEnable);
    RwXboxSetCachedRenderState(D3DRS_SRCBLEND, oldSrcBlend );
    RwXboxSetCachedRenderState(D3DRS_DESTBLEND, oldDestBlend );
}

/*
 ***************************************************************************
 */
static RpAtomic *
AtomicSetVShaderPipeline(RpAtomic *atomic, void *data)
{
    RpAtomicSetPipeline(atomic, AtomicVShaderPipe);

    return (atomic);
}

/*
 ***************************************************************************
 */
static void
ClumpSetVShaderPipeline(RpClump *clump)
{
    RpClumpForAllAtomics(clump, AtomicSetVShaderPipeline, NULL);
}

/*
 *****************************************************************************
 */
static RwBool
SetUpDFF(void)
{
    RwChar *path;
    RwStream *stream;

    path = RsPathnameCreate(RWSTRING("models/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);
    texture = RwTextureRead("colors", NULL);
    if (!texture)
    {
        return FALSE;
    }

    path = RsPathnameCreate(RWSTRING("models/teapot/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING("models/teapot.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            Clump = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);

        if( Clump )
        {
            RwV3d pos = { 0.0f, 0.0f, 0.0f };

            pos.z += (RwReal)60.0f;

            RwFrameTranslate(RpClumpGetFrame(Clump), &pos, rwCOMBINEREPLACE);

            RpWorldAddClump(World, Clump);
        }

        Light = RpLightCreate(rpLIGHTDIRECTIONAL);
        LightFrame = RwFrameCreate();

        RpLightSetFrame(Light, LightFrame);
        RpWorldAddLight(World, Light);

        return TRUE;
    }

    return FALSE;
}

/*
 ***************************************************************************
 */
RxPipeline *
CreateAtomicPipeline(void)
{
    RxPipeline  *pipe;

    pipe = RxPipelineCreate();
    if (pipe)
    {
        RxLockedPipe    *lpipe;

        lpipe = RxPipelineLock(pipe);
        if (NULL != lpipe)
        {
            RxNodeDefinition    *instanceNode;

            /*
             * Get the instance node definition
             */
            instanceNode = RxNodeDefinitionGetXboxAtomicAllInOne();

            /*
             * Add the node to the pipeline
             */
            lpipe = RxLockedPipeAddFragment(lpipe, NULL, instanceNode, NULL);

            /*
             * Unlock the pipeline
             */
            lpipe = RxLockedPipeUnlock(lpipe);

            return pipe;
        }

        RxPipelineDestroy(pipe);
    }

    return NULL;
}

/*
 ***************************************************************************
 */
RwBool
VShaderOpen(void)
{
    RxNodeDefinition    *instanceNode;
    RxPipelineNode      *node;

    if (!SetUpDFF())
    {
        return (FALSE);
    }

    /*
     * Create a new atomic pipeline
     */
    AtomicVShaderPipe = CreateAtomicPipeline();

    /*
     * Get the instance node definition
     */
    instanceNode = RxNodeDefinitionGetXboxAtomicAllInOne();

    /*
     * Set the pipeline specific data
     */
    node = RxPipelineFindNodeByName(AtomicVShaderPipe, instanceNode->name, NULL, NULL);

    /*
     * Cache the default render callback
     */
    DefaultRenderCallback = RxXboxAllInOneGetRenderCallBack(node);

    /*
     * Set the VShader render callback
     */
    RxXboxAllInOneSetRenderCallBack(node, VShaderRenderCallBack);

    /*
     * Create the vertex shader
     */
    if (D3D_OK != D3DDevice_CreateVertexShader(RainbowVertexShaderDeclaration,
                                               dwRainbowVertexShader,
                                               &RainbowVertexShader,
                                               0))
    {
        return FALSE;
    }

    /* Set the VShader pipeline for the atomic */
    ClumpSetVShaderPipeline(Clump);

    return TRUE;
}

/*
 ***************************************************************************
 */
void
VShaderClose(void)
{
    if (Light)
    {
        RpWorldRemoveLight(World, Light);
        RpLightSetFrame(Light, NULL);
        RwFrameDestroy(LightFrame);
        RpLightDestroy(Light);
    }

    if (Clump)
    {
        RpWorldRemoveClump(World, Clump);
        RpClumpDestroy(Clump);
    }

    D3DDevice_DeleteVertexShader(RainbowVertexShader);

    RxPipelineDestroy(AtomicVShaderPipe);
}

/*
 ***************************************************************************
 */
void
VShaderUpdate(RwReal delta)
{
    RwV3d       v = {0.0, 1.0, 0.0};
    RwMatrix    *matrix;
    RwV3d       *up, *right;

    matrix = RwFrameGetLTM(LightFrame);
    up = RwMatrixGetUp(matrix);
    right = RwMatrixGetRight(matrix);

    RwFrameRotate(LightFrame, up, delta * (RwReal)60.0f, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(LightFrame, right, delta * (RwReal)60.0f, rwCOMBINEPOSTCONCAT);
}

/*
 *****************************************************************************
 */

extern RwCamera *Camera;

void
CameraTranslate(RwReal xDelta, RwReal zDelta)
{
    RwFrame *frame;
    RwV3d at, right;

    if (Camera)
    {
        frame = RwCameraGetFrame(Camera);
        at = *RwMatrixGetAt(RwFrameGetMatrix(frame));

        RwV3dScale(&at, &at, 0.1f * zDelta);
        RwFrameTranslate(frame, &at, rwCOMBINEPRECONCAT);

        right = *RwMatrixGetRight(RwFrameGetMatrix(frame));

        RwV3dScale(&right, &right, 0.1f * xDelta);
        RwFrameTranslate(frame, &right, rwCOMBINEPRECONCAT);
    }

    return;
}


/*
 *****************************************************************************
 */

static RwReal RotateX = 0.0f;
static RwReal RotateY = 0.0f;

static RpClump *
RotateClump(RpClump *clump, RwReal deltaTime)
{
    RwFrame *clumpFrame;
    RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
    RwV3d Yaxis = {0.0f, 1.0f, 0.0f};

    clumpFrame = RpClumpGetFrame(clump);

    /*
     * Do the rotation.
     * Use a time step to give a constant rate of movement 
     * independent of FPS...
     */  
    RwFrameRotate(clumpFrame, &Yaxis, 
        RotateY * deltaTime , rwCOMBINEPRECONCAT);

    RwFrameRotate(clumpFrame, &Xaxis, 
        RotateX * deltaTime, rwCOMBINEPRECONCAT);

    return clump;
}


/*
 *****************************************************************************
 */
void
SetRotation(RwReal deltaX, RwReal deltaY)
{
    RwReal maxRot = 360.0f;
    RwReal timeStep = 0.5f;

    /* 
     * Accumulate rotation up to some maximum, for spinning...
     */
    RotateX = (RotateX > maxRot) ? maxRot : 
        (RotateX < -maxRot) ? -maxRot : RotateX - deltaY;
    
    RotateY = (RotateY > maxRot) ? maxRot : 
        (RotateY < -maxRot) ? -maxRot : RotateY + deltaX;

    /* 
     * Rotate at a fixed rate...
     */
    RotateClump(Clump, timeStep);

    /* 
     * Reset rotation values...
     */
    RotateX = 0.0f;
    RotateY = 0.0f;
    
    return;
}
