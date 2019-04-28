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
 * pipeline.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example of parallel-projection shadow rendering using
 *          multi-texturing.
 *
*****************************************************************************/

#include <d3d9.h>

#include "rwcore.h"
#include "rpworld.h"

#include "pipeline.h"

static RwTexture    *ShadowRenderTexture = NULL;
static RwReal       ShadowStrength = 0.0f;

/****************************************************************************
 *  ProjectionPipelineBegin
 */
void
ProjectionPipelineBegin(RwTexture *shadowTexture,
                        RwCamera *camera,
                        RwCamera *shadowCamera,
                        RwReal shadowStrength,
                        RwReal shadowZoneRadius)
{
    RwMatrix    *cameraMatrix;
    RwMatrix    fixedCameraMatrix;
    RwMatrix    *shadowMatrix;
    RwMatrix    invShadowMatrix;
    RwReal      radius;
    RwV3d       scl, tr;
    RwMatrix    texMat;

    ShadowRenderTexture = shadowTexture;
    ShadowStrength = shadowStrength;

    cameraMatrix = RwFrameGetLTM(RwCameraGetFrame(camera));
    RwMatrixCopy(&fixedCameraMatrix, cameraMatrix);
    fixedCameraMatrix.right.x = -fixedCameraMatrix.right.x;
    fixedCameraMatrix.right.y = -fixedCameraMatrix.right.y;
    fixedCameraMatrix.right.z = -fixedCameraMatrix.right.z;

    shadowMatrix = RwFrameGetMatrix(RwCameraGetFrame(shadowCamera));
    radius = RwCameraGetViewWindow(shadowCamera)->x;

    RwMatrixInvert(&invShadowMatrix, shadowMatrix);

    scl.x = scl.y = -0.5f / radius;
    scl.z = 1.0f / (shadowZoneRadius + radius);
    RwMatrixScale(&invShadowMatrix, &scl, rwCOMBINEPOSTCONCAT);

    tr.x = tr.y = 0.5f;
    tr.z = 0.0f;
    RwMatrixTranslate(&invShadowMatrix, &tr, rwCOMBINEPOSTCONCAT);

    RwMatrixMultiply(&texMat, &fixedCameraMatrix, &invShadowMatrix);

	((D3DMATRIX *)&texMat)->_14 = 0.0f;
	((D3DMATRIX *)&texMat)->_24 = 0.0f;
	((D3DMATRIX *)&texMat)->_34 = 0.0f;
    ((D3DMATRIX *)&texMat)->_44 = 1.0f;

    RwD3D9SetTransform(D3DTS_TEXTURE1, &texMat);
}

/****************************************************************************
 *  ShadowMultitexturePipeRenderCallback
 */
static void
ShadowMultitexturePipeRenderCallback(RwResEntry *repEntry,
                                     void *object,
                                     RwUInt8 type,
                                     RwUInt32 flags)
{
    RxD3D9ResEntryHeader    *resEntryHeader;
    RxD3D9InstanceData      *instancedData;
    RwInt32                 numMeshes;
    RwBool                  lighting;
    RwBool                  vertexAlphaBlend;

    /*
     * Set the Default Pixel shader
     */
    RwD3D9SetPixelShader(NULL);

    /* Set clipping */
    _rwD3D9EnableClippingIfNeeded(object, type);

    /* Get header */
    resEntryHeader = (RxD3D9ResEntryHeader *)(repEntry + 1);

    /*
     * Data shared between meshes
     */
    if (resEntryHeader->indexBuffer != NULL)
    {
        RwD3D9SetIndices(resEntryHeader->indexBuffer);
    }

    /* Set the stream sources */
    _rwD3D9SetStreams(resEntryHeader->vertexStream,
                      resEntryHeader->useOffsets);

    /*
    * Vertex Declaration
    */
    RwD3D9SetVertexDeclaration(resEntryHeader->vertexDeclaration);

    /* check lighting */
    RwD3D9GetRenderState(D3DRS_LIGHTING, &lighting);

    /* Get vertex alpha Blend state */
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)&vertexAlphaBlend);

    /* Set base texture */
    if (flags & (rxGEOMETRY_TEXTURED | rxGEOMETRY_TEXTURED2))
    {
        RwD3D9SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        RwD3D9SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    else
    {
        RwD3D9SetTexture(NULL, 0);

        RwD3D9SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);
        RwD3D9SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
        RwD3D9SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }

    /* Set projected texture */
    RwD3D9SetTexture(ShadowRenderTexture, 1);

    RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
    if (ShadowStrength > 0.0f)
    {
        RwD3D9SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    }
    else
    {
        RwD3D9SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
    }
    RwD3D9SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

    RwD3D9SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
    RwD3D9SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

    RwD3D9SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    RwD3D9SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);

    if (lighting == FALSE &&
        (flags & rxGEOMETRY_MODULATE) != 0)
    {
        RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        RwD3D9SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_TFACTOR);
        RwD3D9SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT);

        RwD3D9SetTextureStageState(2, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
        RwD3D9SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        RwD3D9SetTextureStageState(2, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    }

    /* Get the instanced data */
    instancedData = (RxD3D9InstanceData *)(resEntryHeader + 1);

    /* Get the number of meshes */
    numMeshes = resEntryHeader->numMeshes;
    while (numMeshes--)
    {
        const RpMaterial    *material;
        const RwRGBA        *matcolor;
        RwUInt32            currentRenderFlags = 0;

        material = instancedData->material;

        matcolor = &(material->color);

        if ((0xFF != matcolor->alpha) ||
            instancedData->vertexAlpha)
        {
            if (vertexAlphaBlend == FALSE)
            {
                vertexAlphaBlend = TRUE;

                RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
            }
        }
        else
        {
            if (vertexAlphaBlend != FALSE)
            {
                vertexAlphaBlend = FALSE;

                RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
            }
        }

        if (lighting)
        {
            RwD3D9SetSurfaceProperties(&(material->surfaceProps),
                                       matcolor,
                                       flags);
        }
        else
        {
            if (flags & rxGEOMETRY_MODULATE)
            {
                RwUInt32 tFactor;

                tFactor =
                ((((RwUInt32)matcolor->alpha)<<24)|(((RwUInt32)matcolor->red)<<16)|(((RwUInt32)matcolor->green)<<8)|((RwUInt32)matcolor->blue));

                RwD3D9SetRenderState(D3DRS_TEXTUREFACTOR, tFactor);
            }
        }

        if (flags & (rxGEOMETRY_TEXTURED | rxGEOMETRY_TEXTURED2))
        {
            RwD3D9SetTexture(material->texture, 0);
        }

        /*
        * Vertex shader
        */
        RwD3D9SetVertexShader(instancedData->vertexShader);

        /*
         * Render
         */
        if (resEntryHeader->indexBuffer != NULL)
        {
            RwD3D9DrawIndexedPrimitive((D3DPRIMITIVETYPE)resEntryHeader->primType,
                                       instancedData->baseIndex,
                                       0, instancedData->numVertices,
                                       instancedData->startIndex, instancedData->numPrimitives);
        }
        else
        {
            RwD3D9DrawPrimitive((D3DPRIMITIVETYPE)resEntryHeader->primType,
                                instancedData->baseIndex,
                                instancedData->numVertices);
        }

        /* Move onto the next instancedData */
        instancedData++;
    }

    RwD3D9SetTexture(NULL, 1);

    RwD3D9SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
    RwD3D9SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

    RwD3D9SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    RwD3D9SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

    if (lighting == FALSE &&
        (flags & rxGEOMETRY_MODULATE) != 0)
    {
        RwD3D9SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);
        RwD3D9SetTextureStageState(2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
    }
}

/****************************************************************************
 *  ProjectionPipelineCreate
 */
RxPipeline *
ProjectionPipelineCreate(void)
{
    const D3DCAPS9 *d3d9Caps;

    d3d9Caps = (const D3DCAPS9 *)RwD3D9GetCaps();

    if (d3d9Caps->MaxSimultaneousTextures > 1)
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
                RxPipelineNode      *node;

                /*
                * Get the instance node definition
                */
                instanceNode = RxNodeDefinitionGetD3D9WorldSectorAllInOne();

                /*
                * Add the node to the pipeline
                */
                lpipe = RxLockedPipeAddFragment(lpipe, NULL, instanceNode, NULL);

                /*
                * Unlock the pipeline
                */
                lpipe = RxLockedPipeUnlock(lpipe);

                /*
                * Set the pipeline specific data
                */
                node = RxPipelineFindNodeByName(pipe, instanceNode->name, NULL, NULL);

                /*
                * Set the render callback
                */
                RxD3D9AllInOneSetRenderCallBack(node, ShadowMultitexturePipeRenderCallback);

                return (pipe);
            }

            RxPipelineDestroy(pipe);
        }
    }

    return (NULL);
}

/****************************************************************************
 *  ProjectionPipelineEnd
 */
void
ProjectionPipelineEnd(void)
{

}

/****************************************************************************
 *  ShadowPipelineCreate
 */
RxPipeline *
ShadowPipelineCreate(void)
{
    return NULL;
}
