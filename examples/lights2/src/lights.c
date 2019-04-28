
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
 * lights.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose :To illustrate the effects of different global and local lights on
 *          a landscape.
 *
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"

#include "lights.h"
#include "main.h"

/* 
 * Number of points used in drawing a circle
 */
#define NUMPOINTS (100)  

#define ARROW_LENGTH (40.0f)
#define ARROW_FRACTION (0.3f)
#define ARROW_WIDTH (10.0f)

RpLight *AmbientLight = NULL;
RpLight *DirectionalLight = NULL;
RpLight *PointLight = NULL;

RwBool DirectLightOn = TRUE;
RwBool PointLightOn = TRUE;

RwReal PointRadius = 70.0f;


/*
 ****************************************************************************
 */
static void
DirectionalToggle(RwBool on)
{
    static RwBool removed = FALSE;

    if (removed && on)
    {
        /* 
         * Turn the light back on...
         */
        RpWorldAddLight(World, DirectionalLight);
        
        removed = FALSE;
    }
    else if (on == FALSE && removed == FALSE)
    {
        /* 
         * Turn the light off...
         */
        RpWorldRemoveLight(World, DirectionalLight);
        
        removed = TRUE;
    }

    return;
}


/*
 ****************************************************************************
 */
static void
PointToggle(RwBool on)
{
    static RwBool removed = FALSE;

    if (removed && on)
    {
        /* 
         * Turn the light back on...
         */
        RpWorldAddLight(World, PointLight);

        removed = FALSE;
    }
    else if (on == FALSE && removed == FALSE)
    {
        /* 
         * Turn the light off...
         */
        RpWorldRemoveLight(World, PointLight);

        removed = TRUE;
    }

    return;
}


/*
 ****************************************************************************
 */
void
DrawLightDirection(void)
{
    /*
     * Draws an arrow to show the direction of the directional light...
     */
    const RwBBox bBox = *RpWorldGetBBox(World);

    RwIm3DVertex vertex[4];
    RwMatrix *matrix;
    RwV3d *at;
    RwV3d pos, point;

    matrix = RwFrameGetLTM(RpLightGetFrame(DirectionalLight));
    at = RwMatrixGetAt(matrix);

    pos.x = bBox.inf.x - 100.0f;
    pos.y = bBox.sup.y;
    pos.z = (bBox.sup.z + bBox.inf.z) / 2.0f;

    /*
     * Set the vertex positions for the arrow's tail...
     */
    RwIm3DVertexSetPos(&vertex[0],  pos.x, pos.y, pos.z);
    RwIm3DVertexSetRGBA(&vertex[0], 255, 255, 255, 255);

    RwIm3DVertexSetPos(&vertex[1], pos.x + at->x * ARROW_LENGTH,
        pos.y + at->y * ARROW_LENGTH, pos.z + at->z * ARROW_LENGTH);

    RwIm3DVertexSetRGBA(&vertex[1], 255, 255, 255, 255);

    /*
     * Calculate the point for the arrow's head...
     */
    point.x = pos.x + (at->x * ARROW_LENGTH * (1.0f - ARROW_FRACTION));
    point.y = pos.y + (at->y * ARROW_LENGTH * (1.0f - ARROW_FRACTION));
    point.z = pos.z + (at->z * ARROW_LENGTH * (1.0f - ARROW_FRACTION));

    /*
     * Set the vertex positions for the arrow's head...
     */
    RwIm3DVertexSetPos(&vertex[2], point.x + ARROW_WIDTH, point.y, point.z);
    RwIm3DVertexSetRGBA(&vertex[2], 255, 255, 255, 255);
    
    RwIm3DVertexSetPos(&vertex[3], point.x - ARROW_WIDTH, point.y, point.z);
    RwIm3DVertexSetRGBA(&vertex[3], 255, 255, 255, 255);

    if (RwIm3DTransform(vertex, 4, NULL, rwIM3D_ALLOPAQUE))
    {
        RwIm3DRenderLine(0, 1);
        RwIm3DRenderLine(1, 2);
        RwIm3DRenderLine(1, 3);

        RwIm3DEnd();
    }

    return;
}


/*
 ****************************************************************************
 */
void
DrawLightRadius(void)
{
    /*
     * Draws a circle to represent the radius of the point light...
     */
    const RwBBox *bBox;
    RwIm3DVertex circleXZ[NUMPOINTS+1];
    RwV3d point;
    RwV3d *pos;
    RwMatrix *matrix;
    RwInt32 i;
    RwReal middle;

    /* 
     * To draw a full circle, if just NUMPOINTS would draw only half...
     */
    RwReal divide = (RwReal)NUMPOINTS / 2;
    
    matrix = RwFrameGetLTM(RpLightGetFrame(PointLight));
    pos = RwMatrixGetPos(matrix);

    bBox = RpWorldGetBBox(World);
    middle = (bBox->sup.y + bBox->inf.y) / 2; 

    for(i=0; i<=NUMPOINTS; i++)
    {
        /* 
         * Calculate point for x-z plane circle...
         */
        point.x = pos->x + (RwReal)RwCos(i / divide * rwPI) * PointRadius;

        point.z = pos->z + (RwReal)RwSin(i / divide * rwPI) * PointRadius; 

        RwIm3DVertexSetRGBA(&circleXZ[i], 255, 255, 255, 255);
        RwIm3DVertexSetPos(&circleXZ[i], point.x, middle, point.z);
    }

    if( RwIm3DTransform(circleXZ, NUMPOINTS+1, NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPEPOLYLINE);

        RwIm3DEnd();
    }

    return;
}


/*
 ****************************************************************************
 */
void
TranslatePointLight(RwReal dX, RwReal dY)
{
    const RwBBox *bBox;
    RwFrame *lightFrame;
    RwFrame *cameraFrame;
    RwV3d temp; 
    RwV3d right, up, pos;

    bBox = RpWorldGetBBox(World);
    lightFrame = RpLightGetFrame(PointLight);
    cameraFrame = RwCameraGetFrame(Camera);

    right = *RwMatrixGetRight(RwFrameGetMatrix(cameraFrame));
    up = *RwMatrixGetUp(RwFrameGetMatrix(cameraFrame));

    /*
     * Project the RIGHT and UP vectors onto a plane perpendicular
     * to the the Y-axis...
     */
    right.y = 0.0f;
    RwV3dNormalize(&right, &right);

    up.y = 0.0f;
    RwV3dNormalize(&up, &up);

    /* 
     * Scale the movement of the point light so it has the same 
     * direction as the mouse...
     */
    RwV3dScale(&right, &right, dX);
    RwV3dScale(&up, &up, dY);

    RwV3dAdd(&temp, &right, &up);
    
    /* 
     * Keep the position of the light within the world bounding-box...
     */
    pos = *RwMatrixGetPos(RwFrameGetMatrix(lightFrame));

    if( (pos.x + temp.x) < bBox->inf.x )
    {
        temp.x = bBox->inf.x - pos.x;
    }
    else if( (pos.x + temp.x) > bBox->sup.x )
    { 
        temp.x = bBox->sup.x - pos.x;
    }

    if( (pos.z + temp.z) < bBox->inf.z )
    {
        temp.z = bBox->inf.z - pos.z;
    }
    else if( (pos.z + temp.z) > bBox->sup.z )
    { 
        temp.z = bBox->sup.z - pos.z;
    }

    RwFrameTranslate(lightFrame, &temp, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
void
RotateDirectLight(RwReal angleX, RwReal angleY)
{
    RwFrame *lightFrame;
    RwMatrix *cameraMatrix;
    RwV3d *right;
    RwV3d *up;

    lightFrame = RpLightGetFrame(DirectionalLight);
    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));

    /* 
     * Get the camera axis to perform rotations around the x- and y-axis...
     */
    right = RwMatrixGetRight(cameraMatrix);
    up = RwMatrixGetUp(cameraMatrix);

    RwFrameRotate(lightFrame, up, angleX, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(lightFrame, right, angleY, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 ****************************************************************************
 */
void
UpdateLights(void)
{
    DirectionalToggle(DirectLightOn);

    PointToggle(PointLightOn);

    RpLightSetRadius(PointLight, PointRadius);

    return;
}


/*
 ****************************************************************************
 */
RwBool
CreateLights(RpWorld *world)
{
    AmbientLight = RpLightCreate(rpLIGHTAMBIENT);
    if (AmbientLight)
    {
        RwRGBAReal color = {0.4f, 0.4f, 0.4f, 1.0f};

        RpLightSetColor(AmbientLight, &color);
        
        RpWorldAddLight(world, AmbientLight);
    }
    else
    {
        AmbientLight = NULL;

        return FALSE;
    }

    DirectionalLight = RpLightCreate (rpLIGHTDIRECTIONAL);
    if (DirectionalLight)
    {
        RwRGBAReal color = {0.8f, 0.8f, 0.8f, 1.0f};
        RwFrame *lightFrame;
    
        lightFrame = RwFrameCreate();
        if (lightFrame)
        {
            RwV3d axis;

            axis.x = 1.0f;
            axis.y = axis.z = 0.0f;
            RwFrameRotate(lightFrame, &axis, 10.0f, rwCOMBINEREPLACE);

            RpLightSetFrame(DirectionalLight, lightFrame);

            RpLightSetColor(DirectionalLight, &color);

            RpWorldAddLight(world, DirectionalLight);
        }
        else
        {
            RpLightDestroy(DirectionalLight);
            DirectionalLight = NULL;

            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    PointLight = RpLightCreate(rpLIGHTPOINT);
    if (PointLight)
    {
        RwFrame *lightFrame;
        
        RpLightSetRadius(PointLight, PointRadius);
        lightFrame = RwFrameCreate();
        if (lightFrame)
        {
            RwV3d pos;

            pos.x = -10.0f;
            pos.y = 40.0f;
            pos.z = -150.0f;

            RwFrameTranslate(lightFrame, &pos, rwCOMBINEREPLACE);
            RpLightSetFrame(PointLight, lightFrame);

            RpWorldAddLight(world, PointLight);
        }
        else
        {
            RpLightDestroy(PointLight);
            PointLight = NULL;

            return FALSE;
        }
    }

    return TRUE;
}

/*
 ****************************************************************************
 */

