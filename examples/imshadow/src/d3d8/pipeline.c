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

#include <d3d8.h>

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

    RwD3D8SetTransform(D3DTS_TEXTURE1, &texMat);
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
    RpWorldSector           *worldSector;
    RwCamera                *cam;
    RxD3D8ResEntryHeader    *resEntryHeader;
    RxD3D8InstanceData      *instancedData;
    RwInt32                 numMeshes;
    RwBool                  lighting;
    RwBool                  vertexAlphaBlend;
    void                    *lastVertexBuffer;

    /* Enable clipping */
    worldSector = (RpWorldSector *)object;

    cam = RwCameraGetCurrentCamera();

    if (RwD3D8CameraIsBBoxFullyInsideFrustum(cam, RpWorldSectorGetTightBBox(worldSector)))
    {
        RwD3D8SetRenderState(D3DRS_CLIPPING, FALSE);
    }
    else
    {
        RwD3D8SetRenderState(D3DRS_CLIPPING, TRUE);
    }

    /* Get lighting state */
    RwD3D8GetRenderState(D3DRS_LIGHTING, &lighting);
    if (lighting)
    {
        if (flags & rxGEOMETRY_PRELIT)
        {
            /* Emmisive color from the vertex colors */
            RwD3D8SetRenderState(D3DRS_COLORVERTEX, TRUE);
            RwD3D8SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1);
        }
        else
        {
            /* Emmisive color from material, set to black in the submit node */
            RwD3D8SetRenderState(D3DRS_COLORVERTEX, FALSE);
            RwD3D8SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
        }
    }

    /*
     * Set the Default Pixel shader
     */
    RwD3D8SetPixelShader(0);

    /* Get vertex alpha Blend state */
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)&vertexAlphaBlend);

    /* Set projected texture */
    RwD3D8SetTexture(ShadowRenderTexture, 1);

    RwD3D8SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
    if (ShadowStrength > 0.0f)
    {
        RwD3D8SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    }
    else
    {
        RwD3D8SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
    }
    RwD3D8SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

    RwD3D8SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
    RwD3D8SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

    RwD3D8SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    RwD3D8SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);

    /* Set Last vertex buffer to force the call */
    lastVertexBuffer = (void *)0xffffffff;

    /* Get header */
    resEntryHeader = (RxD3D8ResEntryHeader *)(repEntry + 1);

    /* Get the instanced data */
    instancedData = (RxD3D8InstanceData *)(resEntryHeader + 1);

    /*
    * Vertex shader
    */
    RwD3D8SetVertexShader(instancedData->vertexShader);

    /* Get the number of meshes */
    numMeshes = resEntryHeader->numMeshes;
    while (numMeshes--)
    {
        const RpMaterial    *material;
        const RwRGBA        *matcolor;
        RwUInt32            currentRenderFlags = 0;

        material = instancedData->material;

        if (flags & (rxGEOMETRY_TEXTURED | rxGEOMETRY_TEXTURED2))
        {
            RwD3D8SetTexture(material->texture, 0);
        }
        else
        {
            RwD3D8SetTexture(NULL, 0);
        }

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
            if (instancedData->vertexAlpha)
            {
                RwD3D8SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
            }
            else
            {
                RwD3D8SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
            }

            RwD3D8SetSurfaceProperties(matcolor,
                                       &material->surfaceProps,
                                       (flags & rxGEOMETRY_MODULATE));
        }

        if (lastVertexBuffer != instancedData->vertexBuffer)
        {
            RwD3D8SetStreamSource(0, instancedData->vertexBuffer, instancedData->stride);

            lastVertexBuffer = instancedData->vertexBuffer;
        }

        /* Set the Index buffer */
        if (instancedData->indexBuffer != NULL)
        {
            RwD3D8SetIndices(instancedData->indexBuffer, instancedData->baseIndex);

            /* Draw the indexed primitive */
            RwD3D8DrawIndexedPrimitive((D3DPRIMITIVETYPE)instancedData->primType,
                                       0, instancedData->numVertices,
                                       0, instancedData->numIndices);
        }
        else
        {
            RwD3D8DrawPrimitive((D3DPRIMITIVETYPE)instancedData->primType,
                                instancedData->baseIndex,
                                instancedData->numVertices);
        }

        /* Move onto the next instancedData */
        instancedData++;
    }

    RwD3D8SetTexture(NULL, 1);

    RwD3D8SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
    RwD3D8SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

    RwD3D8SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    RwD3D8SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
}

/****************************************************************************
 *  ProjectionPipelineCreate
 */
RxPipeline *
ProjectionPipelineCreate(void)
{
    const D3DCAPS8 *d3d8Caps;

    d3d8Caps = (const D3DCAPS8 *)RwD3D8GetCaps();

    if (d3d8Caps->MaxSimultaneousTextures > 1)
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
                instanceNode = RxNodeDefinitionGetD3D8WorldSectorAllInOne();

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
                RxD3D8AllInOneSetRenderCallBack(node, ShadowMultitexturePipeRenderCallback);

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
