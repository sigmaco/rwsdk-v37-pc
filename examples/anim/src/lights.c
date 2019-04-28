
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
 * Purpose: To illustrate the different lights that are available for use in
 *          RenderWare.
 *
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "lights.h"
#include "main.h"

/*
 * All the drawing functions use these constants to draw the light
 * representations. The appearence of the lights can therefore be adjusted
 * at compile time by changing these constants.
 */
 
/*
 * Point light properties...
 */ 
#define POINT_LIGHT_RADIUS_FACTOR (1.0f)
#define POINT_LIGHT_NUM_VERTICES (50)

/* 
 * Direct light cylinder properties...
 */ 
#define DIRECT_LIGHT_CYLINDER_LENGTH (50.0f)
#define DIRECT_LIGHT_CYLINDER_DIAMETER (25.0f)     
#define DIRECT_LIGHT_NUM_VERTICES (20)

/* 
 * Direct light cone properties...
 */ 
#define DIRECT_LIGHT_CONE_SIZE (50.0f)
#define DIRECT_LIGHT_CONE_ANGLE (45.0f)    

/* 
 * Spot light properties...
 */
#define SPOT_LIGHTS_RADIUS_FACTOR (0.5f)

/*
 * Effects both spot lights & the direct light... 
 */
#define CONE_NUM_VERTICES (10)

static RpLight *CurrentLight;
static RwRGBA LightSolidColor;
static RwReal LightConeAngle;
static RwReal LightRadius;
 /*
 *****************************************************************************
 */
static void
DrawCone(RwReal coneAngle, RwReal coneSize, RwReal coneRatio)
{
    /*
     * Function to draw a cone.
     * The Cone Ratio parameter set the ratio between the cone height 
     * and width, if this is set to be negative the cone is drawn 
     * inverted - this means that the cone becomes narrower along the at 
     * vector(the direction of the light). Usually it is set to 1.0f which 
     * draws a cone getting wider along the at vector (the direction of 
     * the light).
     */

    RwMatrix *matrix;
    RwV3d *right, *up, *at, *pos;
    RwV3d point;
    RwReal cosValue, sinValue, coneAngleD;
    RwV3d dRight, dUp, dAt;
    RwIm3DVertex cone[CONE_NUM_VERTICES+1];
    RwImVertexIndex indices[CONE_NUM_VERTICES*3];
    RwInt32 i;

    matrix = RwFrameGetLTM(RpLightGetFrame(CurrentLight));

    right = RwMatrixGetRight(matrix);
    up    = RwMatrixGetUp(matrix);
    at    = RwMatrixGetAt(matrix);
    pos   = RwMatrixGetPos(matrix);

    for(i=1; i<CONE_NUM_VERTICES+1; i++)
    {
        cosValue = (RwReal)(RwCos(i/(CONE_NUM_VERTICES/2.0f) * rwPI) 
            * RwSin(coneAngle / 180.0f * rwPI));
        
        sinValue = (RwReal)(RwSin(i/(CONE_NUM_VERTICES/2.0f) * rwPI) 
            * RwSin(coneAngle / 180.0f * rwPI));

        RwV3dScale(&dUp, up, sinValue * coneSize);
        
        RwV3dScale(&dRight, right, cosValue * coneSize);

        coneAngleD = (RwReal)RwCos(coneAngle / 180.0f * rwPI);

        RwV3dScale(&dAt, at, coneAngleD * coneSize * coneRatio);

        point.x = pos->x + dAt.x + dUp.x + dRight.x;
        point.y = pos->y + dAt.y + dUp.y + dRight.y;
        point.z = pos->z + dAt.z + dUp.z + dRight.z;

        RwIm3DVertexSetPos(&cone[i], point.x, point.y, point.z);        
    }

    /* 
     * Set up vertex list...
     */
    for(i=0; i < CONE_NUM_VERTICES; i++)
    {
        indices[(i*3)]   = (RwImVertexIndex)0;
        indices[(i*3)+1] = (RwImVertexIndex)i+2;
        indices[(i*3)+2] = (RwImVertexIndex)i+1;
    }
    
    indices[(CONE_NUM_VERTICES*3)-2] = 1;

    /*
     * Set color & alpha of all points...
     */
    for(i=0; i<(CONE_NUM_VERTICES+1); i++)
    {
        RwIm3DVertexSetRGBA(&cone[i],
            LightSolidColor.red, LightSolidColor.green,
            LightSolidColor.blue, 128);
    }
    
    /*
     * Set cone apex to light position...
     */
    RwIm3DVertexSetPos(&cone[0],  pos->x, pos->y, pos->z);

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
    
    if( RwIm3DTransform(cone, (CONE_NUM_VERTICES+1), NULL, 0) )
    {
        /*
         * Draw inside of cone...
         */
        RwIm3DRenderPrimitive(rwPRIMTYPETRIFAN);
        RwIm3DRenderTriangle(0, CONE_NUM_VERTICES, 1);

        /*
         * Draw outside of cone...
         */
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, 
            indices, CONE_NUM_VERTICES*3);
        
        RwIm3DEnd();
    }

    /*
     * Change alpha of all points...
     */
    for(i=0; i<(CONE_NUM_VERTICES+1); i++)
    {
        RwIm3DVertexSetRGBA(&cone[i],
            LightSolidColor.red, LightSolidColor.green,
            LightSolidColor.blue, 255);
    }

     
    /*
     * Set cone apex to same level as cone base 
     */
    coneAngleD = (RwReal)RwCos(coneAngle / 180.0f * rwPI);

    RwV3dScale(&dAt, at, coneAngleD * coneSize * coneRatio);

    point.x = pos->x + dAt.x;
    point.y = pos->y + dAt.y;
    point.z = pos->z + dAt.z;

    RwIm3DVertexSetPos(&cone[0], point.x, point.y, point.z);

    /*
     * Draw base...
     */       
    if( RwIm3DTransform(cone, CONE_NUM_VERTICES+1, NULL, rwIM3D_ALLOPAQUE) )
    {
        if( coneRatio > 0 )
        {
            RwIm3DRenderPrimitive(rwPRIMTYPETRIFAN);
            RwIm3DRenderTriangle(0, CONE_NUM_VERTICES, 1);
        }
        else
        {
            RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRIFAN, 
                indices, CONE_NUM_VERTICES*3);
        }

        RwIm3DEnd();
    }
       
    /*
     * Move cone apex by small offset...
     */
    RwV3dScale(&dAt, at, -0.05f);
    point.x = pos->x + dAt.x;
    point.y = pos->y + dAt.y;
    point.z = pos->z + dAt.z;
    RwIm3DVertexSetPos(&cone[0], point.x, point.y, point.z);

    /*
     * Draw Lines...
     */    
    if( RwIm3DTransform(cone, CONE_NUM_VERTICES+1, NULL, rwIM3D_ALLOPAQUE) )
    {        
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE, 
            indices, CONE_NUM_VERTICES*3);
        
        RwIm3DEnd();
    }

    return;    
}


/*
 *****************************************************************************
 */
static void 
DrawDirectLight(void)
{
    /*
     * Draw cylinder behind cone -> to produce 3D arrow...
     */
    RwMatrix *matrix;
    RwV3d *right, *up, *at, *pos;
    RwV3d point;
    RwReal cosValue, sinValue;
    RwV3d dRight, dUp, dAt;
    RwIm3DVertex cone[(DIRECT_LIGHT_NUM_VERTICES*2)+1];
    RwImVertexIndex indices[DIRECT_LIGHT_NUM_VERTICES*3];
    RwInt32 i;

    matrix = RwFrameGetLTM(RpLightGetFrame(CurrentLight));

    right = RwMatrixGetRight(matrix);
    up    = RwMatrixGetUp(matrix);
    at    = RwMatrixGetAt(matrix);
    pos   = RwMatrixGetPos(matrix);
      
    for(i=0; i<(DIRECT_LIGHT_NUM_VERTICES*2); i+=2)
    {
        cosValue =
            (RwReal)(RwCos(i/(DIRECT_LIGHT_NUM_VERTICES/2.0f) * rwPI));
        sinValue =
            (RwReal)(RwSin(i/(DIRECT_LIGHT_NUM_VERTICES/2.0f) * rwPI));

        RwV3dScale(&dUp, up, sinValue * DIRECT_LIGHT_CYLINDER_DIAMETER);
        
        RwV3dScale(&dRight, right, cosValue * DIRECT_LIGHT_CYLINDER_DIAMETER);

        RwV3dScale(&dAt, at, -(DIRECT_LIGHT_CONE_SIZE + 1.0f));

        /*
         * Cylinder base vertices...
         */
        point.x = pos->x + dAt.x + dUp.x + dRight.x;
        point.y = pos->y + dAt.y + dUp.y + dRight.y;
        point.z = pos->z + dAt.z + dUp.z + dRight.z;

        RwIm3DVertexSetPos(&cone[i], point.x, point.y, point.z);
                       
        /*
         *  Cylinder top vertices 
         */
        RwV3dScale(&dAt, at, 
            -(DIRECT_LIGHT_CYLINDER_LENGTH + DIRECT_LIGHT_CONE_SIZE) );
            
        point.x = pos->x + dAt.x + dUp.x + dRight.x;
        point.y = pos->y + dAt.y + dUp.y + dRight.y;
        point.z = pos->z + dAt.z + dUp.z + dRight.z;
        
        RwIm3DVertexSetPos(&cone[i+1], point.x, point.y, point.z);
    }

    /*
     * Set color & alpha of all points...
     */
    for(i=0; i<(2*DIRECT_LIGHT_NUM_VERTICES)+1; i++)
    {
        RwIm3DVertexSetRGBA(&cone[i],
            LightSolidColor.red, LightSolidColor.green,
            LightSolidColor.blue, 128);
    }
    
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

    if( RwIm3DTransform(cone, 2*DIRECT_LIGHT_NUM_VERTICES, NULL, 0) )
    {
        /*
         * Draw cylinder...
         */
        RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP);
        
        /*
         * Close cylinder...
         */
        RwIm3DRenderTriangle((2*DIRECT_LIGHT_NUM_VERTICES)-2,
            (2*DIRECT_LIGHT_NUM_VERTICES)-1,0);

        RwIm3DRenderTriangle((2*DIRECT_LIGHT_NUM_VERTICES)-1,1,0);        
        
        RwIm3DEnd();
    }
    
    for(i=0; i<(DIRECT_LIGHT_NUM_VERTICES*2)+1; i++)
    {
        RwIm3DVertexSetRGBA(&cone[i],
            LightSolidColor.red, LightSolidColor.green,
            LightSolidColor.blue, 255); 
    }

    /*
     * Set cylinder base center point...
     */    
    RwV3dScale(&dAt, at, 
        -(DIRECT_LIGHT_CYLINDER_LENGTH + DIRECT_LIGHT_CONE_SIZE));

    point.x = pos->x + dAt.x;
    point.y = pos->y + dAt.y;
    point.z = pos->z + dAt.z;
    RwIm3DVertexSetPos(&cone[DIRECT_LIGHT_NUM_VERTICES*2], 
        point.x, point.y, point.z);    
    
    /* 
     * Set up vertex list...
     */
    for(i=0; i<DIRECT_LIGHT_NUM_VERTICES; i++)
    {
        indices[(i*3)]   = (RwImVertexIndex)DIRECT_LIGHT_NUM_VERTICES*2;
        indices[(i*3)+1] = (RwImVertexIndex)((i+1)*2) +1;    /* 3, 5, 7, 9, etc.*/
        indices[(i*3)+2] = (RwImVertexIndex)(i*2)+1;         /* 1, 3, 5, 7, etc.*/       
    }
    
    indices[(DIRECT_LIGHT_NUM_VERTICES*3)-2] = 1;

    /*
     * Draw base...
     */
    if( RwIm3DTransform(cone, (2*DIRECT_LIGHT_NUM_VERTICES)+1, NULL,
            rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, 
            indices, DIRECT_LIGHT_NUM_VERTICES*3);
        
        RwIm3DEnd();
    }  
     
    /*
     * Set cylinder top center point...
     */    
    RwV3dScale(&dAt, at, -(DIRECT_LIGHT_CONE_SIZE + 1.0f));
    point.x = pos->x + dAt.x;
    point.y = pos->y + dAt.y;
    point.z = pos->z + dAt.z;
    RwIm3DVertexSetPos(&cone[DIRECT_LIGHT_NUM_VERTICES*2], 
        point.x, point.y, point.z);    

    /* 
     * Set up vertex list...
     */
    for(i=0; i<DIRECT_LIGHT_NUM_VERTICES; i++)
    {
        indices[(i*3)]   = (RwImVertexIndex)(DIRECT_LIGHT_NUM_VERTICES*2);
        indices[(i*3)+1] = (RwImVertexIndex)i*2;         /* 0, 2, 4, 6, etc.*/
        indices[(i*3)+2] = (RwImVertexIndex)(i+1)*2;     /* 2, 4, 6, 8, etc.*/               
    }
    indices[(DIRECT_LIGHT_NUM_VERTICES*3)-1] = 0;
    
    /*
     * Draw cylinder top...
     */
    if( RwIm3DTransform(cone, (2*DIRECT_LIGHT_NUM_VERTICES)+1, NULL,
            rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, 
            indices, DIRECT_LIGHT_NUM_VERTICES*3);

        RwIm3DEnd();
    }

    /*
     * Draw inverted cone to act as arrow head...
     */
    DrawCone(DIRECT_LIGHT_CONE_ANGLE, DIRECT_LIGHT_CONE_SIZE, -2.0f);

    return;
}


/*
 *****************************************************************************
 */
static void 
DrawPointLight(void)
{
    RwIm3DVertex shape[POINT_LIGHT_NUM_VERTICES];
    RwV3d point, *pos;
    RwMatrix *matrix;
    RwInt32 i;

    matrix = RwFrameGetLTM(RpLightGetFrame(CurrentLight));
    pos = RwMatrixGetPos(matrix);

    for(i=0; i<POINT_LIGHT_NUM_VERTICES; i++)
    {
        point.x = pos->x +
            (RwReal)RwCos(i/(POINT_LIGHT_NUM_VERTICES/2.0f) * rwPI) * 
            LightRadius * POINT_LIGHT_RADIUS_FACTOR;

        point.y = pos->y +
            (RwReal)RwSin(i/(POINT_LIGHT_NUM_VERTICES/2.0f) * rwPI) * 
            LightRadius * POINT_LIGHT_RADIUS_FACTOR;
        
        point.z = pos->z;

        RwIm3DVertexSetRGBA(&shape[i],
            LightSolidColor.red, LightSolidColor.green,
            LightSolidColor.blue, LightSolidColor.alpha);

        RwIm3DVertexSetPos(&shape[i], point.x, point.y, point.z);
    }

    if( RwIm3DTransform(shape, POINT_LIGHT_NUM_VERTICES, NULL,
            rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPEPOLYLINE);
        RwIm3DRenderLine(POINT_LIGHT_NUM_VERTICES-1, 0);

        RwIm3DEnd();
    }

    return;
}


/*
 *****************************************************************************
 */
void
DrawLight(RpLight *light)
{
    RwRGBAReal lightColor;

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);
    

    CurrentLight = light;
    lightColor = *RpLightGetColor(CurrentLight);

    LightSolidColor.red = (RwChar)(lightColor.red * 255.0f);
    LightSolidColor.green = (RwChar)(lightColor.green * 255.0f);
    LightSolidColor.blue = (RwChar)(lightColor.blue * 255.0f);
    LightSolidColor.alpha = /*lightColor.red * 255.0f*/128;
    
    LightConeAngle = RpLightGetConeAngle(CurrentLight);
    LightRadius = RpLightGetRadius(CurrentLight);

    switch(RpLightGetType(CurrentLight))
    {
        case rpLIGHTDIRECTIONAL:
        {
            DrawDirectLight();
            break;
        }

        case rpLIGHTSPOT:
        case rpLIGHTSPOTSOFT:
        {
            DrawCone(LightConeAngle, 
                LightRadius * SPOT_LIGHTS_RADIUS_FACTOR, 1.0f);
            break;
        }

        case rpLIGHTPOINT:
        {
            DrawPointLight();
            break;
        }

        default:
            break;
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

    return;
}

/*
 ***************************************************************************
 */
