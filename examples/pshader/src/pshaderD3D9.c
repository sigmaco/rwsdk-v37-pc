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

#include <d3d9.h>
#include <d3dx9.h>

#include "rwcore.h"
#include "rpworld.h"
#include "skeleton.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "pshader.h"
#include "blurvshaderdefs.h"
#include "blurvshaderD3D9.h"
#include "blurpshaderD3D9.h"

/* Globals */

/* Locals */

static void *BlurVertexShader = NULL;
static void *BlurPixelShader = NULL;

/*
Render a quad with this vertex
*/
typedef struct RenderedVertexFormat RenderedVertexFormat;
struct RenderedVertexFormat
{
    RwReal x;
    RwReal y;
    RwReal z;
    RwReal u;
    RwReal v;
};

static RenderedVertexFormat RenderedVertex[4];

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


static const D3DXVECTOR3 vEyePt    = { 0.0f, 0.0f, -5.0f };
static const D3DXVECTOR3 vLookatPt = { 0.0f, 0.0f, 0.0f };
static const D3DXVECTOR3 vUp       = { 0.0f, 1.0f, 0.0f };

static const D3DXVECTOR4 pixelshaderLerp = {0.5f, 0.5f, 0.5f, 0.5f};

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
    RwBool  status;
    RwInt32  i;
    const D3DCAPS9      *d3dCaps;

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
    d3dCaps = (const D3DCAPS9 *)RwD3D9GetCaps();
    if ( (d3dCaps->VertexShaderVersion & 0xffff) >= 0x0101)
    {
        status = 
            RwD3D9CreateVertexShader((RwUInt32 *)dwBlurvshaderD3D9VertexShader,
                                     &BlurVertexShader);
    
        if (FAILED(status))
        {
            return FALSE;
        }
    }
    else
    {
        BlurVertexShader = NULL;
    }

    /*
     * Create the pixel shader
     */
    if ( (d3dCaps->PixelShaderVersion & 0xffff) >= 0x0101)
    {
        RwD3D9CreatePixelShader((RwUInt32 *)dwBlurpshaderD3D9PixelShader, 
                                &BlurPixelShader);
    }
    else
    {
        BlurPixelShader = NULL;
    }

    /*
     *   Fill the vertex array.
     */
    for (i = 0; i < 4; ++i)
    {
        RenderedVertex[i].x = ((i==0 || i==2) ? -1.0f : 1.0f);
        RenderedVertex[i].y = ((i<2)          ? -1.0f : 1.0f);
        RenderedVertex[i].z = 0.0f;

        RenderedVertex[i].u = ((i==0 || i==2) ? 0.0f : 1.0f);
        RenderedVertex[i].v = ((i<2)          ? 1.0f : 0.0f);
    }

    return TRUE;
}

#if defined(__MWERKS__)
static
void BoxFilterSetup()
{ 
    const RwReal kPerTexelWidth  = 1.0f/(RwReal)TEXSIZE;
    const RwReal kPerTexelHeight = 1.0f/(RwReal)TEXSIZE;
    const RwReal eps             = 10.0e-4f;

    /*
    From nVidia's filter blit demo, samples 16 pixels by sampling in the center of 4 pixels
    and letting the bilinear filtering average them together.
    The pixel shader averages the result.
    */
    const RwReal offsetX0 = -.5f * kPerTexelWidth + eps;
    const RwReal offsetX1 = -.5f * kPerTexelWidth + eps; 
    const RwReal offsetX2 = 1.5f * kPerTexelWidth - eps;
    const RwReal offsetX3 = 1.5f * kPerTexelWidth - eps;
    
    const RwReal offsetY0 = -.5f * kPerTexelHeight + eps;
    const RwReal offsetY1 = 1.5f * kPerTexelHeight - eps; 
    const RwReal offsetY2 = 1.5f * kPerTexelHeight - eps;
    const RwReal offsetY3 = -.5f * kPerTexelHeight+ eps;

	/* unroll loop */
	RwReal	stageOffset[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	
	
	stageOffset[0] = offsetX0;
	stageOffset[1] = offsetY0;
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET,
        stageOffset, VSCONST_REG_T0_SIZE );
        
	stageOffset[0] = offsetX1;
	stageOffset[1] = offsetY1;
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + VSCONST_REG_T0_SIZE,
        stageOffset, VSCONST_REG_T0_SIZE );
        
	stageOffset[0] = offsetX2;
	stageOffset[1] = offsetY2;
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + 2 * VSCONST_REG_T0_SIZE,
        stageOffset, VSCONST_REG_T0_SIZE );

	stageOffset[0] = offsetX3;
	stageOffset[1] = offsetY3;
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + 3 * VSCONST_REG_T0_SIZE,
        stageOffset, VSCONST_REG_T0_SIZE );
}
#else /* defined(__MWERKS__) */
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
        RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + i * VSCONST_REG_T0_SIZE,
            stageOffset, VSCONST_REG_T0_SIZE );
    }
}
#endif /* defined(__MWERKS__) */


#if defined(__MWERKS__)
static
void VBoxFilterSetup()
{ 
    const RwReal kPerTexelWidth  = 1.0f/(RwReal)TEXSIZE;
    const RwReal kPerTexelHeight = 1.0f/(RwReal)TEXSIZE;
    const RwReal eps             = 10.0e-4f;

    /*
    Adding this fudge factor keeps the texture from crawling across the screen on repeated passes
    */
    const RwReal offsetX0 = eps;
    const RwReal offsetX1 = eps;
    const RwReal offsetX2 = eps;
    const RwReal offsetX3 = eps;

    /*
    Sample 8 pixels in a column, sample points are fudged to land in between 2 pixels
    which will be averaged by bilinear filtering, the pixel shader does the rest
    */
    const RwReal offsetY0 = -.5f * kPerTexelHeight + eps;
    const RwReal offsetY1 = 1.5f * kPerTexelHeight - eps;
    const RwReal offsetY2 = 3.5f * kPerTexelHeight - eps;
    const RwReal offsetY3 = -2.5f * kPerTexelHeight+ eps;

	/* unroll the loop */
	RwReal	stageOffset[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	
	
	stageOffset[0] = offsetX0;
	stageOffset[1] = offsetY0;
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET,
        stageOffset, VSCONST_REG_T0_SIZE );

	stageOffset[0] = offsetX1;
	stageOffset[1] = offsetY1;
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + VSCONST_REG_T0_SIZE,
        stageOffset, VSCONST_REG_T0_SIZE );

	stageOffset[0] = offsetX2;
	stageOffset[1] = offsetY2;
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + 2 * VSCONST_REG_T0_SIZE,
        stageOffset, VSCONST_REG_T0_SIZE );

	stageOffset[0] = offsetX3;
	stageOffset[1] = offsetY3;
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + 3 * VSCONST_REG_T0_SIZE,
        stageOffset, VSCONST_REG_T0_SIZE );
}
#else /* defined(__MWERKS__) */
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
        RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + i * VSCONST_REG_T0_SIZE,
            stageOffset, VSCONST_REG_T0_SIZE );
    }
}
#endif /* defined(__MWERKS__) */

#if defined(__MWERKS__)
static
void HBoxFilterSetup()
{ 
    const RwReal kPerTexelWidth  = 1.0f/(RwReal)TEXSIZE;
    const RwReal kPerTexelHeight = 1.0f/(RwReal)TEXSIZE;
    const RwReal eps             = 10.0e-4f;

    /*
    Sample 8 pixels in a row, sample points are fudged to land in between 2 pixels
    which will be averaged by bilinear filtering, the pixel shader does the rest
    */
    const RwReal offsetX0 = -.5f * kPerTexelWidth + eps;
    const RwReal offsetX1 = -2.5f * kPerTexelWidth + eps;
    const RwReal offsetX2 = 1.5f * kPerTexelWidth - eps;
    const RwReal offsetX3 = 3.5f * kPerTexelWidth - eps;
    
    /*
    Adding this fudge factor keeps the texture from crawling down the screen on repeated passes
    */
    const RwReal offsetY0 = eps;
    const RwReal offsetY1 = eps;
    const RwReal offsetY2 = eps;
    const RwReal offsetY3 = eps;

	/* unroll loop */
	RwReal	stageOffset[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	stageOffset[0] = offsetX0;
	stageOffset[1] = offsetY0;	
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET,
        stageOffset, VSCONST_REG_T0_SIZE );
	
	stageOffset[0] = offsetX1;
	stageOffset[1] = offsetY1;	
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + VSCONST_REG_T0_SIZE,
        stageOffset, VSCONST_REG_T0_SIZE );

	stageOffset[0] = offsetX2;
	stageOffset[1] = offsetY2;	
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + 2 * VSCONST_REG_T0_SIZE,
        stageOffset, VSCONST_REG_T0_SIZE );

	stageOffset[0] = offsetX3;
	stageOffset[1] = offsetY3;	
    RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + 3 * VSCONST_REG_T0_SIZE,
        stageOffset, VSCONST_REG_T0_SIZE );
}
#else /* defined(__MWERKS__) */
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
        RwD3D9SetVertexShaderConstant( VSCONST_REG_T0_OFFSET + i * VSCONST_REG_T0_SIZE,
            stageOffset, VSCONST_REG_T0_SIZE );
    }
}
#endif /* defined(__MWERKS__) */

static int blurDir = 0;

static void Blur(int srcTexture)
{
    /*
     * Render quad with pixel shader operating on input texture.
     */
    RwInt32 i;
    RwUInt32  cullMode;

    D3DXMATRIX matWorld;
    D3DXMATRIX matView;
    D3DXMATRIX matProj;
    D3DXMATRIX matViewProj;
    D3DXMATRIX matWorldViewProj;

    /* Set World, View, Projection, and combination matrices. */
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUp);
    D3DXMatrixOrthoLH(&matProj, 4.0f, 4.0f, 0.2f, 20.0f);
    D3DXMatrixMultiply(&matViewProj, &matView, &matProj);

    /* draw a single quad to texture: the quad covers the whole "screen" exactly */
    D3DXMatrixScaling(&matWorld, 2.0f, 2.0f, 1.0f);
    D3DXMatrixMultiply(&matWorldViewProj, &matWorld, &matViewProj);
    D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);

    RwD3D9SetVertexShaderConstant(VSCONST_REG_TRANSFORM_OFFSET,
                                  &matWorldViewProj,
                                  VSCONST_REG_TRANSFORM_SIZE);

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

    RwD3D9SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);

    RwD3D9SetVertexShader(BlurVertexShader);

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) FALSE);

    RwD3D9GetRenderState(D3DRS_CULLMODE, &cullMode);

    RwD3D9SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    /* stick the rendered texture into texture stages 0-3 */
    for (i = 0; i < 4; ++i)
    {
        RwD3D9SetTexture(CameraTexture[srcTexture], i);

        RwD3D9SetSamplerState(i, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP);
        RwD3D9SetSamplerState(i, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP);

        RwD3D9SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        RwD3D9SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        RwD3D9SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    }

    /* turn on the bluring pixel shader */
    RwD3D9SetPixelShader(BlurPixelShader);      

    RwD3D9SetPixelShaderConstant(0, &pixelshaderLerp, 1);

    /* render the quad, invoking the vertex & pixel shaders */
    RwD3D9DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, RenderedVertex, sizeof(RenderedVertexFormat));

    /* Restore renderstates */
    for (i = 0; i < 4; ++i)
    {
        RwD3D9SetTexture(NULL, i);
    }

    RwD3D9SetRenderState(D3DRS_CULLMODE, cullMode);

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) TRUE);

    /* turn off pixel shading */
    RwD3D9SetPixelShader(NULL);    
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

    if (BlurVertexShader != NULL)
    {
        RwD3D9DeleteVertexShader(BlurVertexShader);
        BlurVertexShader = NULL;
    }

    if (BlurPixelShader != NULL)
    {
        RwD3D9DeletePixelShader(BlurPixelShader);
        BlurPixelShader = NULL;
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
