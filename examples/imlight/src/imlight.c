
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
 * imlight.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrate the lighting of 3D immediate vertices.
 *
*****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"

#include "imlight.h"

#define SCALELIGHT (0.4f)
#define LIGHTRADIUS (10.0f)
#define LIGHTRADIUS2 (100.0f)

static RwIm3DVertex *Im3DMeshVertices;
static RwMatrix *Im3DMeshMatrix = NULL;
static RwInt32 Im3DMeshNumVertices = 0;

static RwRGBA SpecularColor = {255, 255, 255, 255};
static RwRGBA DiffuseColor  = {200,   0,   0, 255};
static RwRGBA AmbientColor  = { 70,   0,   0, 255};

static RwIm3DVertex *Im3DPointLightVertices = NULL;
static RwMatrix *Im3DLightMatrix = NULL;
static RwInt32 Im3DPointLightNumVertices = 0;

RwInt32 LightMode = LIGHTDIRECTIONAL;
RwInt32 ShadeMode = SHADEGOURAUD;



/*
 *****************************************************************************
 */
void
Im3DMeshRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix *cameraMatrix;
    RwV3d right, up, pos;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    pos = *RwMatrixGetPos(Im3DMeshMatrix);

    /*
     * First translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(Im3DMeshMatrix, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * ...do the rotations...
     */
    RwMatrixRotate(Im3DMeshMatrix, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwMatrixRotate(Im3DMeshMatrix, &right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * ...and translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwMatrixTranslate(Im3DMeshMatrix, &pos, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 ***************************************************************************
 */
void
Im3DMeshTranslateZ(RwReal zDelta)
{
    RwFrame *cameraFrame;
    RwV3d delta;

    cameraFrame = RwCameraGetFrame(Camera);

    RwV3dScale(&delta, RwMatrixGetAt(RwFrameGetMatrix(cameraFrame)), zDelta);

    RwMatrixTranslate(Im3DMeshMatrix, &delta, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
void
Im3DLightRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix *cameraMatrix;
    RwV3d right, up;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    /*
     * Do the rotations...
     */
    RwMatrixRotate(Im3DLightMatrix, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwMatrixRotate(Im3DLightMatrix, &right, yAngle, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 ***************************************************************************
 */
void
Im3DLightTranslateZ(RwReal zDelta)
{
    RwFrame *cameraFrame;
    RwV3d delta;

    cameraFrame = RwCameraGetFrame(Camera);

    RwV3dScale(&delta, RwMatrixGetAt(RwFrameGetMatrix(cameraFrame)), zDelta);
    RwMatrixTranslate(Im3DLightMatrix, &delta, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 ***************************************************************************
 */
void
Im3DLightTranslateXY(RwReal xDelta, RwReal yDelta)
{
    RwFrame *cameraFrame;
    RwV3d delta;

    cameraFrame = RwCameraGetFrame(Camera);

    RwV3dScale(&delta, RwMatrixGetRight(RwFrameGetMatrix(cameraFrame)), xDelta);
    RwMatrixTranslate(Im3DLightMatrix, &delta, rwCOMBINEPOSTCONCAT);

    RwV3dScale(&delta, RwMatrixGetUp(RwFrameGetMatrix(cameraFrame)), yDelta);
    RwMatrixTranslate(Im3DLightMatrix, &delta, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
static void
Im3DMeshLighting(void)
{
    RwV3d lightDir, lightPos;
    RwMatrix *transform = NULL;
    RwInt32 i;

    if( LightMode != LIGHTAMBIENT )
    {
        transform = RwMatrixCreate();
        RwMatrixInvert(transform, Im3DMeshMatrix);

        if( LightMode == LIGHTDIRECTIONAL )
        {
            /*
             * Transform the light's direction to the mesh's local space...
             */
            lightDir = *RwMatrixGetAt(Im3DLightMatrix);
            RwV3dTransformVector(&lightDir, &lightDir, transform);
        }
        else
        {
            /*
             * Transform the light's position to the mesh's local space...
             */
            lightPos = *RwMatrixGetPos(Im3DLightMatrix);
            RwV3dTransformPoint(&lightPos, &lightPos, transform);
        }

        RwMatrixDestroy(transform);
    }

    for(i=0; i<Im3DMeshNumVertices; i++)
    {
        RwReal lighting = 0.0f, c1, c2;

        switch( LightMode )
        {
            case LIGHTDIRECTIONAL:
            {
                /*
                 * The lighting factor is the dot product of the vertex
                 * normals and the light direction vector if the normal
                 * is facing the light (dot prod = -1.0f)...
                 */
                RwV3d *normal;

                normal = RwIm3DVertexGetNormal(&Im3DMeshVertices[i]);
                lighting = -RwV3dDotProduct(&lightDir, normal);

                break;
            }

            case LIGHTPOINT:
            {
                /*
                 * The lighting factor is the distance
                 * between the light and the vertex
                 * scaled from [0.0f...Distance] to [-1.0f...1.0f]
                 */
                RwV3d *vertexPos;
                RwV3d range;
                RwReal distance;

                vertexPos = RwIm3DVertexGetPos(&Im3DMeshVertices[i]);
                RwV3dSub(&range, vertexPos, &lightPos);

                distance = RwV3dLength(&range);

                if( distance > LIGHTRADIUS )
                {
                    distance = LIGHTRADIUS;
                }

                lighting = 1.0f - 2.0f * distance / LIGHTRADIUS;

                break;
            }

            case LIGHTAMBIENT:
            {
                lighting = -1.0f;

                break;
            }
        }

        /*
         * Set vertex color using the lighting as a blend factor beetween
         * diffuse color and specular color if lighting is positive or
         * diffuse color and ambient color...
         */
        c1 = RwRealAbs(lighting);
        c2 = 1.0f - c1;

        if( lighting > 0.0f )
        {
            RwIm3DVertexSetRGBA(&Im3DMeshVertices[i],
                (RwUInt8)(c1 * SpecularColor.red   + c2 * DiffuseColor.red),
                (RwUInt8)(c1 * SpecularColor.green + c2 * DiffuseColor.green),
                (RwUInt8)(c1 * SpecularColor.blue  + c2 * DiffuseColor.blue),
                255);
        }
        else
        {
            RwIm3DVertexSetRGBA(&Im3DMeshVertices[i],
                (RwUInt8)(c1 * AmbientColor.red   + c2 * DiffuseColor.red),
                (RwUInt8)(c1 * AmbientColor.green + c2 * DiffuseColor.green),
                (RwUInt8)(c1 * AmbientColor.blue  + c2 * DiffuseColor.blue),
                255);
        }
    }

    return;
}


/*
 *****************************************************************************
 */
void
Im3DRender(void)
{
    static RwBool stateSet = FALSE;

    if( !stateSet )
    {
        /*
         * Render state is persistent - only need to set it once...
         */
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDZERO);

        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

        stateSet = TRUE;
    }

    /*
     * Calculate the current vertex colors due to lighting...
     */
    Im3DMeshLighting();

    /*
     * Set the shading model...
     */
    switch( ShadeMode )
    {
        case SHADEGOURAUD:
        {
            RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

            break;
        }

        case SHADEFLAT:
        {
            RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);

            break;
        }
    }

    /*
     * Render the mesh...
     */
    if( RwIm3DTransform(Im3DMeshVertices,
            Im3DMeshNumVertices, Im3DMeshMatrix, 0) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP);

        RwIm3DEnd();
    }

    /*
     * Render the point Light...
     */
    if( LightMode == LIGHTPOINT)
    {
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

        if( RwIm3DTransform(Im3DPointLightVertices,
                Im3DPointLightNumVertices, Im3DLightMatrix, 0) )
        {
            RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

            RwIm3DEnd();
        }
    }

    return;
}


/*
 *****************************************************************************
 */
void
Im3DTerminate(void)
{
    if( Im3DMeshVertices )
    {
        RwFree(Im3DMeshVertices);

        Im3DMeshVertices = NULL;
    }

    if( Im3DMeshMatrix )
    {
        RwMatrixDestroy(Im3DMeshMatrix);

        Im3DMeshMatrix = NULL;
    }

    if( Im3DMeshVertices )
    {
        RwFree(Im3DMeshVertices);

        Im3DMeshVertices = NULL;
    }

    if( Im3DLightMatrix )
    {
        RwMatrixDestroy(Im3DLightMatrix);

        Im3DLightMatrix = NULL;
    }

    if( Im3DPointLightVertices )
    {
        RwFree(Im3DPointLightVertices);

        Im3DPointLightVertices = NULL;
    }

    return;
}


/*
 *****************************************************************************
 */
void
Im3DLightReset(void)
{
    switch( LightMode )
    {
        case LIGHTDIRECTIONAL:
        {
            RwMatrixSetIdentity(Im3DLightMatrix);

            Im3DLightRotate(30.0f, 60.0f);

            break;
        }

        case LIGHTPOINT:
        {
            RwV3d lightPos = {0.0f, 0.0f, 40.0f};

            RwMatrixTranslate(Im3DLightMatrix, &lightPos, rwCOMBINEREPLACE);

            break;
        }

        case LIGHTAMBIENT:
        default:
        {
            RwMatrixSetIdentity(Im3DLightMatrix);

            break;
        }
    }


    return;
}


/*
 *****************************************************************************
 */
static RwBool
Im3DMeshCreate(void)
{
    int i, j;
    RwReal r = 4.0f, R = 7.0f;
    RwInt32 numSides = 40, numRings = 24;
    RwReal ringDelta, sideDelta;

    RwIm3DVertex *im3DVertices;
    RwInt32 numVert;

    RwReal theta, phi, theta1;
    RwReal cosTheta, sinTheta;
    RwReal cosTheta1, sinTheta1;
    RwReal cosPhi, sinPhi, dist;

    numVert = (numSides+1) * numRings * 2;

    Im3DMeshVertices = (RwIm3DVertex *)RwMalloc(numVert * sizeof(RwIm3DVertex),
                                                rwID_NAOBJECT);
    if( Im3DMeshVertices == NULL )
    {
        return FALSE;
    }

    im3DVertices = Im3DMeshVertices;

    ringDelta = 2.0f * rwPI / numRings;
    sideDelta = 2.0f * rwPI / numSides;

    theta = 0.0f;
    cosTheta = 1.0f;
    sinTheta = 0.0f;

    for(i=numRings-1; i>=0; i--)
    {
        theta1 = theta + ringDelta;
        cosTheta1 = (RwReal)RwCos(theta1);
        sinTheta1 = (RwReal)RwSin(theta1);

        phi = 0.0f;

        for(j=numSides; j>=0; j--)
        {
            phi += sideDelta;
            cosPhi = (RwReal)RwCos(phi);
            sinPhi = (RwReal)RwSin(phi);
            dist = R + r * cosPhi;

            RwIm3DVertexSetPos(im3DVertices,
                cosTheta1 * dist, -sinTheta1 * dist, r * sinPhi);
            RwIm3DVertexSetNormal(im3DVertices,
                cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi);
            im3DVertices++;

            RwIm3DVertexSetPos(im3DVertices,
                cosTheta * dist, -sinTheta * dist, r * sinPhi);
            RwIm3DVertexSetNormal(im3DVertices,
                cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi);
            im3DVertices++;



        }

        theta = theta1;
        cosTheta = cosTheta1;
        sinTheta = sinTheta1;
    }

    Im3DMeshNumVertices = numVert;

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
Im3DPointLightCreate(void)
{
    RwIm3DVertex *vert = NULL;
    int i;

    static RwReal LineListData[28][3] =
    {
        { 0.000f,  0.000f,  0.000f},
        { 0.000f,  1.000f,  0.000f},

        { 0.000f,  0.000f,  0.000f},
        { 0.000f, -1.000f,  0.000f},

        { 0.000f,  0.000f,  0.000f},
        { 0.000f,  0.000f,  1.000f},

        { 0.000f,  0.000f,  0.000f},
        { 0.000f,  0.000f, -1.000f},

        { 0.000f,  0.000f,  0.000f},
        { 1.000f,  0.000f,  0.000f},

        { 0.000f,  0.000f,  0.000f},
        {-1.000f,  0.000f,  0.000f},

        { 0.000f,  0.000f,  0.000f},
        { 0.577f,  0.577f,  0.577f},

        { 0.000f,  0.000f,  0.000f},
        { 0.577f, -0.577f,  0.577f},

        { 0.000f,  0.000f,  0.000f},
        {-0.577f,  0.577f, -0.577f},

        { 0.000f,  0.000f,  0.000f},
        {-0.577f, -0.577f, -0.577f},

        { 0.000f,  0.000f,  0.000f},
        { 0.577f, -0.577f, -0.577f},

        { 0.000f,  0.000f,  0.000f},
        { 0.577f,  0.577f, -0.577f},

        { 0.000f,  0.000f,  0.000f},
        {-0.577f, -0.577f,  0.577f},

        { 0.000f,  0.000f,  0.000f},
        {-0.577f,  0.577f,  0.577f},
    };

    Im3DPointLightNumVertices = 28;

    Im3DPointLightVertices =
        (RwIm3DVertex *)RwMalloc(Im3DPointLightNumVertices * sizeof(RwIm3DVertex),
                                 rwID_NAOBJECT);

    if( Im3DPointLightVertices == NULL )
    {
        return FALSE;
    }

    vert = Im3DPointLightVertices;

    for(i=0; i<28; i++)
    {
        RwIm3DVertexSetPos(vert,
            LineListData[i][0] * SCALELIGHT,
            LineListData[i][1] * SCALELIGHT,
            LineListData[i][2] * SCALELIGHT
        );

        if( i & 0x01 )
        {
            RwIm3DVertexSetRGBA(vert, 255, 255, 255, 0);
        }
        else
        {
            RwIm3DVertexSetRGBA(vert, 255, 255, 255, 255);
        }

        vert++;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
RwBool
Im3DInitialize(void)
{
    /*
     * Create the matrix for the IM geometry's transformation....
     */
    Im3DMeshMatrix = RwMatrixCreate();

    if( Im3DMeshMatrix == NULL )
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

        RwMatrixTranslate(Im3DMeshMatrix, &pos, rwCOMBINEREPLACE);
    }

    /*
     * Create the matrix for the light's transformation....
     */
    Im3DLightMatrix = RwMatrixCreate();

    if( Im3DLightMatrix == NULL )
    {
        return FALSE;
    }

    /*
     * Create the mesh for the point light's representation....
     */
    Im3DPointLightCreate();

    Im3DLightReset();

    /*
     * Start with a torus...
     */
    Im3DMeshCreate();

    return TRUE;
}

/*
 *****************************************************************************
 */
