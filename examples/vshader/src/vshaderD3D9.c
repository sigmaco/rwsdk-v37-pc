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
#include "rwcore.h"

#include <d3d9.h>
#include <d3dx9.h>

#include "rpworld.h"

#include "rtcharse.h"

#include "skeleton.h"

#include "rainbowdefs.h"
#include "rainbowD3D9.h"

#include "vshader.h"

/*--- Global ---*/

/*--- Local Variables ---*/

static RxPipeline   *AtomicVShaderPipe;
static RxD3D9AllInOneRenderCallBack DefaultRenderCallback = NULL;

static void     *RainbowVertexShader = NULL;
static RpClump  *Clump = NULL;
static RpLight  *Light = NULL;
static RwFrame  *LightFrame = NULL;

extern RwBool   VertexShaderOn;

extern RpWorld  *World;

static RwTexture *Texture = NULL;

/*
 ***************************************************************************
 */
static void
VertexShaderSetConstantRegisters(RwMatrix *ltm)
{
#if defined(_MSC_VER) || defined(__ICL)
    __declspec(align(16)) D3DMATRIX   composedMatrix;
#else
    D3DMATRIX   composedMatrix;
#endif
    D3DMATRIX   worldTransposedMatrix;
    D3DMATRIX   worldInverseMat;
    RwV3d       *vecPos;

    RwCamera    *camera;
    RwMatrix    *camLTM;

    /*
     * Set active world matrix
     */
    _rwD3D9VSSetActiveWorldMatrix(ltm);

    _rwD3D9VSGetComposedTransformMatrix((void *)&composedMatrix);

    /*
     * Set the constant registers c0-c3 with the transformation matrix
     */
    RwD3D9SetVertexShaderConstant(VSCONST_REG_TRANSFORM_OFFSET,
                                  (void *)&composedMatrix,
                                  VSCONST_REG_TRANSFORM_SIZE);

    /* 
     * Transposed World matrix
     */
    worldTransposedMatrix.m[0][0] = ltm->right.x;
    worldTransposedMatrix.m[0][1] = ltm->up.x;
    worldTransposedMatrix.m[0][2] = ltm->at.x;
    worldTransposedMatrix.m[0][3] = ltm->pos.x;

    worldTransposedMatrix.m[1][0] = ltm->right.y;
    worldTransposedMatrix.m[1][1] = ltm->up.y;
    worldTransposedMatrix.m[1][2] = ltm->at.y;
    worldTransposedMatrix.m[1][3] = ltm->pos.y;

    worldTransposedMatrix.m[2][0] = ltm->right.z;
    worldTransposedMatrix.m[2][1] = ltm->up.z;
    worldTransposedMatrix.m[2][2] = ltm->at.z;
    worldTransposedMatrix.m[2][3] = ltm->pos.z;

    worldTransposedMatrix.m[3][0] = 0.0f;
    worldTransposedMatrix.m[3][1] = 0.0f;
    worldTransposedMatrix.m[3][2] = 0.0f;
    worldTransposedMatrix.m[3][3] = 1.0f;

    RwD3D9SetVertexShaderConstant(VSCONST_REG_WORLD_TRANSFORM_OFFSET,
                                  (void *)&worldTransposedMatrix,
                                  VSCONST_REG_WORLD_TRANSFORM_SIZE);

    _rwD3D9VSGetInverseWorldMatrix((void *)&worldInverseMat);

    RwD3D9SetVertexShaderConstant(VSCONST_REG_WORLD_INVERSE_OFFSET,
                                  (void *)&worldInverseMat,
                                  VSCONST_REG_WORLD_INVERSE_SIZE);

    /* Get Eye vector */
    camera = RwCameraGetCurrentCamera();
    camLTM = RwFrameGetLTM(RwCameraGetFrame(camera));

    vecPos = RwMatrixGetPos(camLTM);
    RwD3D9SetVertexShaderConstant(VSCONST_REG_EYEPOS_OFFSET,
                                  (void *)vecPos,
                                  VSCONST_REG_EYEPOS_SIZE);
}

/*
 ***************************************************************************
 */
static void
VShaderRenderCallBack(RwResEntry *repEntry, void *object, RwUInt8 type, RwUInt32 flags)
{
    RxD3D9ResEntryHeader    *resEntryHeader;
    RxD3D9InstanceData      *instancedData;
    RwInt32                 numMeshes;
    DWORD                   oldColorArg1, oldColorOp, oldColorArg2,
                            oldAlphaEnable, oldAlphaTestEnable;

    if (VertexShaderOn == FALSE ||
        RainbowVertexShader == NULL)
    {
        DefaultRenderCallback(repEntry, object, type, flags);

        return;
    }

    /*
     * Set up vertex shader with required constants
     */
    VertexShaderSetConstantRegisters(RwFrameGetLTM(RpAtomicGetFrame((RpAtomic *)object)));

    RwD3D9SetVertexShader(RainbowVertexShader);

    /*
     * Set up texture stages
     */
    RwD3D9SetPixelShader(NULL);

    /* stick the rainbow color texture into texture stages 0 & 1 */
    RwD3D9SetTexture(Texture, 0 );
    RwD3D9SetTexture(Texture, 1 );

    /*
     * Note previous renderstates on texture stage 0 so we can restore them later.
     * RenderWare caches renderstates for best performance and expects the hardware
     * to be in the same state as we found it.
     */
    RwD3D9GetTextureStageState(0, D3DTSS_COLORARG1, &oldColorArg1);
    RwD3D9GetTextureStageState(0, D3DTSS_COLOROP,   &oldColorOp);
    RwD3D9GetTextureStageState(0, D3DTSS_COLORARG2, &oldColorArg2);

    /* generate 2 sets of uvs in the vertex shader
    for the same texture and blend them in a funky way */
    RwD3D9SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE2X);
    RwD3D9SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE);

    RwD3D9SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

    RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_ADDSIGNED );
    RwD3D9SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT );
    RwD3D9SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE);

    RwD3D9SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    RwD3D9SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

    /* turn off alpha blending */
    RwD3D9GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaEnable);
    RwD3D9GetRenderState(D3DRS_ALPHATESTENABLE, &oldAlphaTestEnable);

    RwD3D9SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    RwD3D9SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

    /*
     * Get the instanced data
     */
    resEntryHeader = (RxD3D9ResEntryHeader *)(repEntry + 1);

    /* Set shared index buffer */
    RwD3D9SetIndices(resEntryHeader->indexBuffer);

    /* Set the stream sources */
    _rwD3D9SetStreams(resEntryHeader->vertexStream,
                      resEntryHeader->useOffsets);

    /* Set vertex declaration */
    RwD3D9SetVertexDeclaration(resEntryHeader->vertexDeclaration);

    /* Render individual mesh */
    instancedData = (RxD3D9InstanceData *)(resEntryHeader + 1);
    numMeshes = resEntryHeader->numMeshes;
    while (numMeshes--)
    {
        /*
         * Draw the indexed primitive
         */
        RwD3D9DrawIndexedPrimitive((D3DPRIMITIVETYPE)resEntryHeader->primType,
                                    instancedData->baseIndex,
                                    0, instancedData->numVertices,
                                    instancedData->startIndex,
                                    instancedData->numPrimitives);

        instancedData++;
    }

    /*
     * Restore renderstates
     */
    RwD3D9SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaEnable);
    RwD3D9SetRenderState(D3DRS_ALPHATESTENABLE, oldAlphaTestEnable);

    RwD3D9SetTextureStageState(0, D3DTSS_COLORARG1, oldColorArg1);
    RwD3D9SetTextureStageState(0, D3DTSS_COLOROP,   oldColorOp);
    RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, oldColorArg2);

    RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
    RwD3D9SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

    RwD3D9SetTexture(NULL, 1 );
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

    RwTextureSetMipmapping(TRUE);
    RwTextureSetAutoMipmapping(TRUE);

    path = RsPathnameCreate(RWSTRING("models/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);
    Texture = RwTextureRead("colors", NULL);
    if (!Texture)
    {
        return FALSE;
    }

    /* the texture doesn't tile so hide any funny artifacts */
    RwTextureSetAddressing(Texture, rwTEXTUREADDRESSMIRROR);

    /* Trilinear looks nicer */
    RwTextureSetFilterMode(Texture, rwFILTERLINEARMIPLINEAR);

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
static RxPipeline *
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
            instanceNode = RxNodeDefinitionGetD3D9AtomicAllInOne();

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
    HRESULT             hr;
    const D3DCAPS9      *d3dCaps;

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
    instanceNode = RxNodeDefinitionGetD3D9AtomicAllInOne();

    /*
     * Set the pipeline specific data
     */
    node = RxPipelineFindNodeByName(AtomicVShaderPipe, instanceNode->name, NULL, NULL);

    /*
     * Cache the default render callback
     */
    DefaultRenderCallback = RxD3D9AllInOneGetRenderCallBack(node);

    /*
     * Set the VShader render callback
     */
    RxD3D9AllInOneSetRenderCallBack(node, VShaderRenderCallBack);

    /*
     * Create the vertex shader
     */
    d3dCaps = (const D3DCAPS9 *)RwD3D9GetCaps();
    if ( (d3dCaps->VertexShaderVersion & 0xffff) >= 0x0101)
    {
        hr = RwD3D9CreateVertexShader((RwUInt32 *)dwRainbowD3D9VertexShader,
                                      &RainbowVertexShader);
    }
    else
    {
        hr = S_OK;
    }

    if (FAILED(hr))
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

    if (RainbowVertexShader != NULL)
    {
        RwD3D9DeleteVertexShader(RainbowVertexShader);
        RainbowVertexShader = NULL;
    }

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
