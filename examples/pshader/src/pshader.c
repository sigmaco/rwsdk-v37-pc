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
 * pshader.c
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
#include "skeleton.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "pshader.h"
#include "blurvshader.h"
#include "blurvshaderdefs.h"
#include "blurpshader.h"

static DWORD
BlurVertexShaderDeclaration[] =
{
    D3DVSD_STREAM(0),
    D3DVSD_REG(VSD_REG_POS,       D3DVSDT_FLOAT3),  /* POSITION  - register v0 */
    D3DVSD_REG(VSD_REG_TEXCOORDS, D3DVSDT_FLOAT2),  /* TEXCOORDS - register v2 */
    D3DVSD_END()
};

static DWORD BlurVertexShader, BlurPixelShader;

/*
Render a quad with this vertex format
*/
typedef struct RenderedVertex
{
    RwReal      x, y, z;        /* The un-transformed position for the vertex */
    RwReal      u;              /* Texture coordinate */
    RwReal      v;              /* Texture coordinate */
} RenderedVertex;

LPDIRECT3DVERTEXBUFFER8 RenderedVertexBuffer = NULL;

static RpClump  *Clump = NULL;
static RpLight  *Light = NULL;
static RwFrame  *LightFrame = NULL;
extern RpWorld  *World;

/*
Create a camera for rendering into textures with
*/
#define NUM_CAMERA_TEXTURES 2 /* Need at least two, having more doesn't seem to help */
static RwCamera *TexCamera[NUM_CAMERA_TEXTURES];
static RwTexture *CameraTexture[NUM_CAMERA_TEXTURES];
#define TEXSIZE (256)

RwInt32 BlurPasses = 0, BlurType = 0;

static RwRGBA clearColor = {0, 0, 0, 255};

/*
 *****************************************************************************
 */
static RwCamera *
CreateTextureCamera(RpWorld *world, RwBool wantZRaster)
{
    RwRaster *raster;

    raster = RwRasterCreate(TEXSIZE, TEXSIZE, 0, rwRASTERTYPECAMERATEXTURE);

    if( raster )
    {
        RwRaster *zRaster = 0;

        if (wantZRaster)
        {
            zRaster = RwRasterCreate(TEXSIZE, TEXSIZE, 0, rwRASTERTYPEZBUFFER);
        }

        if( zRaster || ! wantZRaster)
        {
            RwFrame *frame;

            frame = RwFrameCreate();

            if( frame )
            {
                RwCamera *camera = RwCameraCreate();

                if( camera)
                {
                    RwV2d vw;

                    RwCameraSetRaster(camera, raster);

                    if (zRaster)
                    {
                        RwCameraSetZRaster(camera, zRaster);
                    }

                    RwCameraSetFrame(camera, frame);

                    RwCameraSetNearClipPlane(camera, 0.1f);
                    RwCameraSetFarClipPlane(camera, 250.0f);

                    vw.x = 0.6f; vw.y = 0.3375f;
                    RwCameraSetViewWindow(camera, &vw);

                    RpWorldAddCamera(world, camera);
                    
#ifdef RWLOGO
                    /*
                     * Do not render the logo for texture rendering...
                     */
                    RpLogoSetState(camera, FALSE);
#endif
                    return camera;
                }

                RwFrameDestroy(frame);
            }

            RwRasterDestroy(zRaster);
        }

        RwRasterDestroy(raster);
    }

    return 0;
}

/*
 *****************************************************************************
 */

static RwBool
SetUpDFF(void)
{
    RwChar *path;
    RwStream *stream;

    path = RsPathnameCreate(RWSTRING("models/test/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING("models/test.dff"));
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

            pos.z += (RwReal)30.0f;

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

RwBool
PShaderOpen(void)
{
    RwInt32 i;
    for (i=0; i<NUM_CAMERA_TEXTURES; i++)
    {
        TexCamera[i] = CreateTextureCamera(World, i == 0);
        if (!TexCamera[i])
        {
            return FALSE;
        }

        CameraTexture[i] = RwTextureCreate(RwCameraGetRaster(TexCamera[i]));
        RwCameraClear(TexCamera[i], &clearColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);
    }

    if (!SetUpDFF())
    {
        return FALSE;
    }
    if (!TexCamera[0])
    {
        return FALSE;
    }

    /*
     * Create the vertex shader
     */
    if (D3D_OK != D3DDevice_CreateVertexShader(BlurVertexShaderDeclaration,
                                               dwBlurvshaderVertexShader,
                                               &BlurVertexShader,
                                               0))
    {
        return FALSE;
    }

    /*
     * Get a vertex buffer for rendered vertices of quad
     */
    if (D3DDevice_CreateVertexBuffer( 4 * sizeof(RenderedVertex),
      D3DUSAGE_WRITEONLY,
      0,
      D3DPOOL_MANAGED, &RenderedVertexBuffer ) != D3D_OK)
    {
        /* Ooops */
        return FALSE;
    }
    else
    {
        /* Initialize the texture coordinates now because they're constant */
        RwInt32             i;

        /*
        Fill the vertex buffer. To do this, we need to Lock() the VB to
        gain access to the vertices.
        */

        RenderedVertex* pVertices;
        D3DVertexBuffer_Lock( RenderedVertexBuffer,
            0, 0, (RwUInt8**)&pVertices, 0 );

        for (i = 0; i < 4; ++i)
        {
            pVertices->x = (i==0 || i==3) ? -1.0f : 1.0f;
            pVertices->y = (i<2)          ? -1.0f : 1.0f;
            pVertices->z = 0.0f;
		    pVertices->u = (i==0 || i==3) ? 0.0f : 1.0f;
            pVertices->v = (i<2)          ? 1.0f : 0.0f;
		    pVertices++;
        }

        D3DVertexBuffer_Unlock( RenderedVertexBuffer );
    }

    D3DDevice_CreatePixelShader((D3DPIXELSHADERDEF* )dwBlurpshaderPixelShader,
        &BlurPixelShader );

    return TRUE;
}

static
void BoxFilterSetup()
{ 
    RwInt32 i;

    RwReal kPerTexelWidth  = 1.0f/(RwReal)TEXSIZE;
    RwReal kPerTexelHeight = 1.0f/(RwReal)TEXSIZE;
    RwReal eps             = 10.0e-4f;

    /*
    From nVidia's filter blit demo, samples 16 pixels by sampling in the center of 4 pixels
    and letting the bilinear filtering average them together.
    The pixel shader averages the result.
    */
    RwReal offsetX[4] = { -.5f * kPerTexelWidth + eps,  
                          -.5f * kPerTexelWidth + eps, 
                          1.5f * kPerTexelWidth - eps, 
                          1.5f * kPerTexelWidth - eps };
    RwReal offsetY[4] = { -.5f * kPerTexelHeight+ eps, 
                          1.5f * kPerTexelHeight- eps, 
                          1.5f * kPerTexelHeight- eps, 
                          -.5f * kPerTexelHeight+ eps };

    for (i = 0; i < 4; ++i)
    {
        RwReal stageOffset[4] = { offsetX[i], offsetY[i], 0.0f, 0.0f };
        D3DDevice_SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + i * VSCONST_REG_T0_SIZE,
            stageOffset, VSCONST_REG_T0_SIZE );
    }
}

static
void VBoxFilterSetup()
{ 
    RwInt32 i;

    RwReal kPerTexelWidth  = 1.0f/(RwReal)TEXSIZE;
    RwReal kPerTexelHeight = 1.0f/(RwReal)TEXSIZE;
    RwReal eps             = 10.0e-4f;

    /*
    Adding this fudge factor keeps the texture from crawling across the screen on repeated passes
    */
    RwReal offsetX[4] = { eps, 
                          eps, 
                          eps, 
                          eps };
    /*
    Sample 8 pixels in a column, sample points are fudged to land in between 2 pixels
    which will be averaged by bilinear filtering, the pixel shader does the rest
    */
    RwReal offsetY[4] = { -.5f * kPerTexelHeight+ eps, 
                          1.5f * kPerTexelHeight- eps, 
                          3.5f * kPerTexelHeight- eps, 
                          -2.5f * kPerTexelHeight+ eps };


    for (i = 0; i < 4; ++i)
    {
        RwReal stageOffset[4] = { offsetX[i], offsetY[i], 0.0f, 0.0f };
        D3DDevice_SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + i * VSCONST_REG_T0_SIZE,
            stageOffset, VSCONST_REG_T0_SIZE );
    }
}

static
void HBoxFilterSetup()
{ 
    RwInt32 i;

    RwReal kPerTexelWidth  = 1.0f/(RwReal)TEXSIZE;
    RwReal kPerTexelHeight = 1.0f/(RwReal)TEXSIZE;
    RwReal eps             = 10.0e-4f;

    /*
    Sample 8 pixels in a row, sample points are fudged to land in between 2 pixels
    which will be averaged by bilinear filtering, the pixel shader does the rest
    */
    RwReal offsetX[4] = { -.5f * kPerTexelWidth + eps,  
                          -2.5f * kPerTexelWidth + eps, 
                          1.5f * kPerTexelWidth - eps, 
                          3.5f * kPerTexelWidth - eps };
    /*
    Adding this fudge factor keeps the texture from crawling down the screen on repeated passes
    */
    RwReal offsetY[4] = { eps, 
                          eps, 
                          eps, 
                          eps };

    for (i = 0; i < 4; ++i)
    {
        RwReal stageOffset[4] = { offsetX[i], offsetY[i], 0.0f, 0.0f };
        D3DDevice_SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + i * VSCONST_REG_T0_SIZE,
            stageOffset, VSCONST_REG_T0_SIZE );
    }
}

static int blurDir = 0;

void Blur( int srcTexture )
{
    /*
     * Render quad with pixel shader operating on input texture.
     */
    RwInt32 i;

	D3DXMATRIX matWorld;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;
	D3DXMATRIX matViewProj;
	D3DXMATRIX matWorldViewProj;

    D3DXVECTOR3 vEyePt    = { 0.0f, 0.0f, -5.0f };
    D3DXVECTOR3 vLookatPt = { 0.0f, 0.0f, 0.0f };
    D3DXVECTOR3 vUp       = { 0.0f, 1.0f, 0.0f };

	/* Set World, View, Projection, and combination matrices. */
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUp);
	D3DXMatrixOrthoLH(&matProj, 4.0f, 4.0f, 0.2f, 20.0f);
    D3DXMatrixMultiply(&matViewProj, &matView, &matProj);

    /* draw a single quad to texture: the quad covers the whole "screen" exactly */
	D3DXMatrixScaling(&matWorld, 2.0f, 2.0f, 1.0f);
	D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
    D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);
    
    D3DDevice_SetVertexShaderConstant(VSCONST_REG_TRANSFORM_OFFSET,
        &matWorldViewProj, VSCONST_REG_TRANSFORM_SIZE);

    /* Stuff in the right constants to do a box filter */
    if (BlurType == 0)
    {
        /* just the 4x4 box filter */
        BoxFilterSetup();
    }
    else
    {
        /*
        Alternate blurring 8 pixels together in horizontal and vertical directions.
        Gives a more extreme blur in less passes (faster!)
        */
        if (blurDir & 1)
        {
            HBoxFilterSetup();
        }
        else
        {
            VBoxFilterSetup();
        }

        blurDir++;
    }

    D3DDevice_SetStreamSource( 0, RenderedVertexBuffer, sizeof(RenderedVertex) );

    RwXboxSetCurrentVertexShader( BlurVertexShader );

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) FALSE);

    /* stick the rendered texture into texture stages 0-3 */
    for (i = 0; i < 4; ++i)
    {
        RwXboxRenderStateSetTexture( CameraTexture[srcTexture], i );

	    RwXboxSetCachedTextureStageState(i, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	    RwXboxSetCachedTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	    RwXboxSetCachedTextureStageState(i, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

        RwXboxSetCachedTextureStageState(i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	    RwXboxSetCachedTextureStageState(i, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	    RwXboxSetCachedTextureStageState(i, D3DTSS_MIPFILTER, D3DTEXF_NONE);
	    RwXboxSetCachedTextureStageState(i, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP);
	    RwXboxSetCachedTextureStageState(i, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP);
    }
    RwXboxSetCachedTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);

    /* turn on the bluring pixel shader */
    RwXboxSetCurrentPixelShader(BlurPixelShader);	

    /* render the quad, invoking the vertex & pixel shaders */
    RwXboxDrawVertices( D3DPT_QUADLIST, 0, 4);

    /* Restore renderstates */
    RwXboxSetCachedTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
    for (i = 0; i < 4; ++i)
    {
        RwXboxRenderStateSetTexture( NULL, i );
    }

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) TRUE);

    /* turn off pixel shading */
    RwXboxSetCurrentPixelShader(0);	
}

static int mostBlurred;

void
PShaderUpdate(RwReal delta)
{
    /*
     * Render scene into the camera texture raster...
     */
    RwInt32 srcTexture, dstTexture, i;

    RwCameraClear(TexCamera[0], &clearColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(TexCamera[0]) )
    {
        RpWorldRender(World);

        RwCameraEndUpdate(TexCamera[0]);
    }

    blurDir = 0;

    /*
     * Blur texture repeatedly
     */
    srcTexture = 0;
    dstTexture = 1;

    for (i=0; i < BlurPasses; i++)
    {
         if( RwCameraBeginUpdate(TexCamera[dstTexture]) )
         {
            Blur( srcTexture );

            RwCameraEndUpdate(TexCamera[dstTexture]);

            mostBlurred = dstTexture;

            srcTexture++;
            if (srcTexture >= NUM_CAMERA_TEXTURES)
            {
                srcTexture = 0;
            }

            dstTexture++;
            if (dstTexture >= NUM_CAMERA_TEXTURES)
            {
                dstTexture = 0;
            }
         }
    }
}

void PShaderRender()
{
    /* Render a quad to the back buffer for display, giving it one last blur pass */
    Blur( mostBlurred );
}

void
PShaderClose(void)
{
    RwInt32 i;

    D3DDevice_DeleteVertexShader(BlurVertexShader);

    D3DDevice_DeletePixelShader(BlurPixelShader);

    if (RenderedVertexBuffer)
    {
        IDirect3DVertexBuffer8_Release(RenderedVertexBuffer);
        RenderedVertexBuffer = NULL;
    }

    RpWorldRemoveLight(World, Light);
    RpLightSetFrame(Light, NULL);
    RwFrameDestroy(LightFrame);
    RpLightDestroy(Light);

    RpWorldRemoveClump(World, Clump);
    RpClumpDestroy(Clump);

    for (i=0; i<NUM_CAMERA_TEXTURES; i++)
    {
        RwRaster *raster;
        RwRaster *zRaster;
        RwFrame  *frame;

        RpWorldRemoveCamera(World, TexCamera[i]);

        raster = RwCameraGetRaster(TexCamera[i]);
        if (raster)
        {
            RwCameraSetRaster(TexCamera[i], NULL);
            //RwRasterDestroy(raster);
        }

        zRaster = RwCameraGetZRaster(TexCamera[i]);
        if (zRaster)
        {
            RwCameraSetZRaster(TexCamera[i], NULL);
            RwRasterDestroy(zRaster);
        }

        RwTextureDestroy(CameraTexture[i]);
        
        frame = RwCameraGetFrame(TexCamera[i]);
        if (frame)
        {
            RwCameraSetFrame(TexCamera[i], NULL);
            RwFrameDestroy(frame);
        }

        RwCameraDestroy(TexCamera[i]);
    }
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

void CameraTranslate(RwReal xDelta, RwReal zDelta)
{
    RwFrame *frame;
    RwV3d at, right;

    if (TexCamera[0])
    {
        frame = RwCameraGetFrame(TexCamera[0]);
        at = *RwMatrixGetAt(RwFrameGetMatrix(frame));

        RwV3dScale(&at, &at, 0.1f * zDelta);
        RwFrameTranslate(frame, &at, rwCOMBINEPRECONCAT);

        right = *RwMatrixGetRight(RwFrameGetMatrix(frame));

        RwV3dScale(&right, &right, 0.1f * xDelta);
        RwFrameTranslate(frame, &right, rwCOMBINEPRECONCAT);
    }

    return;
}
