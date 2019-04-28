
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
#define POINT_LIGHT_RADIUS_FACTOR (0.05f)
#define POINT_LIGHT_NUM_VERTICES (50)

/* 
 * Direct light cylinder properties...
 */ 
#define DIRECT_LIGHT_CYLINDER_LENGTH (5.0f)
#define DIRECT_LIGHT_CYLINDER_DIAMETER (1.5f)     
#define DIRECT_LIGHT_NUM_VERTICES (20)

/* 
 * Direct light cone properties...
 */ 
#define DIRECT_LIGHT_CONE_SIZE (3.0f)     
#define DIRECT_LIGHT_CONE_ANGLE (45.0f)    

/* 
 * Spot light properties...
 */
#define SPOT_LIGHTS_RADIUS_FACTOR (0.05f)

/*
 * Effects both spot lights & the direct light... 
 */
#define CONE_NUM_VERTICES (10)


static const RwV3d LightStartPos = {0.0f, 0.0f, 75.0f};


RpLight *BaseAmbientLight = NULL;
RwBool BaseAmbientLightOn = FALSE;

RpLight *CurrentLight = NULL;
RpLight *AmbientLight = NULL;
RpLight *PointLight = NULL;
RpLight *DirectLight = NULL;
RpLight *SpotLight = NULL;
RpLight *SpotSoftLight = NULL;

RwReal LightRadius = 100.0f;
RwReal LightConeAngle = 45.0f;
RwRGBAReal LightColor = {1.0f, 1.0f, 1.0f, 1.0f};

RwRGBA LightSolidColor = {255, 255, 0, 255};
RwBool LightOn = TRUE;
RwBool LightDrawOn = TRUE;
RwV3d LightPos = {0.0f, 0.0f, 75.0f};
RwInt32 LightTypeIndex = 1;

RwReal AdvanceLightSpeed;

RwBBox RoomBBox;



/*
 ***************************************************************************
 */
RpLight *
CreateBaseAmbientLight(void)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTAMBIENT);

    if( light )
    {
        RwRGBAReal color;

        color.red = color.green = color.blue = 0.5f;
        color.alpha = 1.0f;

        RpLightSetColor(light, &color);

        return light;
    }

    return NULL;
}


/*
 ***************************************************************************
 */
RpLight *
CreateAmbientLight(void)
{
    return RpLightCreate(rpLIGHTAMBIENT);
}


/*
 ***************************************************************************
 */
RpLight *
CreatePointLight(void)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTPOINT);

    if( light )
    {
        RwFrame *lightFrame;
        RwV3d pos;

        RpLightSetRadius(light, LightRadius);

        /* 
         * This one needs a frame...
         */
        lightFrame = RwFrameCreate();

        if( lightFrame )
        {
            pos.x = LightPos.x;
            pos.y = LightPos.y;
            pos.z = LightPos.z;

            RwFrameTranslate(lightFrame, &pos, rwCOMBINEREPLACE);

            RpLightSetFrame(light, lightFrame);

            return light;
        }

        RpLightDestroy(light);
    }

    return NULL;
}


/*
 ***************************************************************************
 */
RpLight *
CreateDirectLight(void)
{
    RpLight *light;
    RwV3d pos;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);

    if( light ) 
    {
        RwFrame *lightFrame;

        /* 
         * This one needs a frame...
         */
        lightFrame = RwFrameCreate();

        if( lightFrame )
        {
            RwFrameRotate(lightFrame, &Xaxis, 45.0f, rwCOMBINEREPLACE);

            /*
             * Position not required but useful when 
             * drawing light source... 
             */
            pos.x = LightPos.x;
            pos.y = LightPos.y;
            pos.z = LightPos.z;

            RwFrameTranslate(lightFrame, &pos, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, lightFrame);

            return light;
        }

        RpLightDestroy(light);
    }

    return NULL;
}


/*
 ***************************************************************************
 */
RpLight *
CreateSpotLight(void)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTSPOT);

    if( light )
    {
        RwFrame *lightFrame;
        RwV3d pos;

        RpLightSetRadius(light, LightRadius);
        RpLightSetConeAngle(light, LightConeAngle / 180.0f * rwPI);

        /* 
         * This one needs a frame...
         */
        lightFrame = RwFrameCreate();

        if( lightFrame ) 
        {
            RwFrameRotate(lightFrame, &Xaxis, 45.0f, rwCOMBINEREPLACE);

            pos.x = LightPos.x;
            pos.y = LightPos.y;
            pos.z = LightPos.z;

            RwFrameTranslate(lightFrame, &pos, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, lightFrame);

            return light;
        }

        RpLightDestroy(light);
    }

    return NULL;
}


/*
 ***************************************************************************
 */
RpLight *
CreateSpotSoftLight(void)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTSPOTSOFT);

    if( light )
    {
        RwFrame *lightFrame;
        RwV3d pos;

        RpLightSetRadius(light, LightRadius);
        RpLightSetConeAngle(light, LightConeAngle / 180.0f * rwPI);

        /*  
         * This one needs a frame...
         */
        lightFrame = RwFrameCreate();

        if( lightFrame )
        {
            RwFrameRotate(lightFrame, &Xaxis, 45.0f, rwCOMBINEREPLACE);

            pos.x = LightPos.x;
            pos.y = LightPos.y;
            pos.z = LightPos.z;

            RwFrameTranslate(lightFrame, &pos, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, lightFrame);

            return light;
        }

        RpLightDestroy(light);
    }

    return NULL;
}


/*
 ***************************************************************************
 */
void
LightsDestroy(void)
{
    RpWorld *world;
    RwFrame *frame;

    if( SpotSoftLight )
    {
        world = RpLightGetWorld(SpotSoftLight);
        if( world )
        {
            RpWorldRemoveLight(world, SpotSoftLight);
        }

        frame = RpLightGetFrame(SpotSoftLight);
        RpLightSetFrame(SpotSoftLight, NULL);
        RwFrameDestroy(frame);

        RpLightDestroy(SpotSoftLight);
    }

    if( SpotLight )
    {
        world = RpLightGetWorld(SpotLight);
        if( world )
        {
            RpWorldRemoveLight(world, SpotLight);
        }

        frame = RpLightGetFrame(SpotLight);
        RpLightSetFrame(SpotLight, NULL);
        RwFrameDestroy(frame);

        RpLightDestroy(SpotLight);
    }

    if( PointLight )
    {
        world = RpLightGetWorld(PointLight);
        if( world )
        {
            RpWorldRemoveLight(world, PointLight);
        }

        frame = RpLightGetFrame(PointLight);
        RpLightSetFrame(PointLight, NULL);
        RwFrameDestroy(frame);

        RpLightDestroy(PointLight);
    }

    if( DirectLight )
    {
        world = RpLightGetWorld(DirectLight);
        if( world )
        {
            RpWorldRemoveLight(world, DirectLight);
        }

        frame = RpLightGetFrame(DirectLight);
        RpLightSetFrame(DirectLight, NULL);
        RwFrameDestroy(frame);

        RpLightDestroy(DirectLight);
    }

    if( AmbientLight )
    {
        world = RpLightGetWorld(AmbientLight);
        if( world )
        {
            RpWorldRemoveLight(world, AmbientLight);
        }

        RpLightDestroy(AmbientLight);
    }

    if( BaseAmbientLight )
    {
        world = RpLightGetWorld(BaseAmbientLight);
        if( world )
        {
            RpWorldRemoveLight(world, BaseAmbientLight);
        }

        RpLightDestroy(BaseAmbientLight);
    }

    return;
}


/*
 ***************************************************************************
 */
RwBool
LightResetCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return LightOn;
    }

    if( CurrentLight )
    {
        RwFrame *frame;
        RwV3d pos;

        frame = RpLightGetFrame(CurrentLight);

        if( frame )
        {
            RwFrameSetIdentity(frame);

            RwFrameRotate(frame, &Xaxis, 45.0f, rwCOMBINEREPLACE);

            pos.x = LightStartPos.x;
            pos.y = LightStartPos.y;
            pos.z = LightStartPos.z;

            RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
        }
    }

    return TRUE;
}


/*
 ***************************************************************************
 */
void
LightsUpdate(void)
{
    RpWorld *world;
    static RwInt32 oldLightTypeIndex = -1;

    if( AdvanceLightSpeed != 0.0f )
    {
        /*
         * Advance light forwards or backwards in the z-direction...
         */
        LightTranslateZ(AdvanceLightSpeed);
    }
    
    if( (LightOn && oldLightTypeIndex != LightTypeIndex) || !CurrentLight )
    {
        /*
         * Change Current Light Type...
         */
        static RwFrame *oldLightFrame;
        RwFrame *newLightFrame;
        RwV3d oldPos;
    
        oldLightTypeIndex = LightTypeIndex;

        if( CurrentLight )
        {
            world = RpLightGetWorld(CurrentLight);
            RpWorldRemoveLight(world, CurrentLight);

            /*
             * Store old Light's frame for initialization of new current
             * light's position.
             * One exception is the direct light which has a fixed position...
             */
            if (RpLightGetFrame(CurrentLight) && CurrentLight != DirectLight )
            {
                oldLightFrame = RpLightGetFrame(CurrentLight);
            }
        }

        switch( LightTypeIndex )
        {
            case 0:
                CurrentLight = AmbientLight;
                break;

            case 1:
                CurrentLight = PointLight;
                break;

            case 2:
                CurrentLight = DirectLight;
                break;

            case 3:
                CurrentLight = SpotLight;
                break;

            case 4:
                CurrentLight = SpotSoftLight;
                break;
        }
        
        newLightFrame = RpLightGetFrame(CurrentLight);
        if( newLightFrame && oldLightFrame )
        {
            oldPos = *RwMatrixGetPos(RwFrameGetMatrix(oldLightFrame));       
            
            if( CurrentLight != DirectLight )
            {
                /*
                 * Position new light to equal previous light...
                 */
                RwFrameTranslate(newLightFrame, &oldPos, rwCOMBINEREPLACE);  
            }
        }        

        RpWorldAddLight(World, CurrentLight);
    }

    if( CurrentLight )
    {
        /*
         * Update Light parameters...
         */
        RpLightSetColor(CurrentLight, &LightColor);
        RpLightSetRadius(CurrentLight, LightRadius);
        RpLightSetConeAngle(CurrentLight, LightConeAngle / 180.0f * rwPI);
    }

    if( !LightOn )
    {
        if( CurrentLight )
        {
            world = RpLightGetWorld(CurrentLight);
            RpWorldRemoveLight(world, CurrentLight);
            
            CurrentLight = NULL;
        }
    }

    return;
}


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
DrawCurrentLight(void)
{
    static RwBool stateSet = FALSE;

    if( !stateSet )
    {
        /*
         * Because the clump's or Im3D rendering is not using any textures
         * this render state only got to be set once...
         */
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

        stateSet = TRUE;
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);

    switch( LightTypeIndex )
    {
        case DIRECT_LIGHT:
        {
            DrawDirectLight();
            break;
        }

        case SPOT_LIGHT:
        case SOFTSPOT_LIGHT:
        {
            DrawCone(LightConeAngle, 
                LightRadius * SPOT_LIGHTS_RADIUS_FACTOR, 1.0f);
            break;
        }

        case POINT_LIGHT:
        {
            DrawPointLight();
            break;
        }
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

    return;
}


/*
 *****************************************************************************
 */
void
LightRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix *cameraMatrix;
    RwV3d right, up, pos;
    RwFrame *lightFrame;

    /*
     * Rotate the light about an origin defined by its frame...
     */
    if( CurrentLight && CurrentLight != AmbientLight
        && CurrentLight != PointLight )
    {

        cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
        right = *RwMatrixGetRight(cameraMatrix);
        up = *RwMatrixGetUp(cameraMatrix);

        lightFrame = RpLightGetFrame(CurrentLight);
        pos = *RwMatrixGetPos(RwFrameGetMatrix(lightFrame));

        /*
         * First translate back to the origin...
         */
        RwV3dScale(&pos, &pos, -1.0f);
        RwFrameTranslate(lightFrame, &pos, rwCOMBINEPOSTCONCAT);

        /*
         * ...do the rotations...
         */
        RwFrameRotate(lightFrame, &up, xAngle, rwCOMBINEPOSTCONCAT);
        RwFrameRotate(lightFrame, &right, yAngle, rwCOMBINEPOSTCONCAT);

        /*
         * ...and translate back...
         */
        RwV3dScale(&pos, &pos, -1.0f);
        RwFrameTranslate(lightFrame, &pos, rwCOMBINEPOSTCONCAT);
    }

    return;
}


/*
 ***************************************************************************
 */
static void 
ClampPosition(RwV3d *pos, RwV3d *delta, RwBBox *bbox)
{
    if( (pos->x + delta->x) < bbox->inf.x )
    {
        delta->x = bbox->inf.x - pos->x;
    }
    else if( (pos->x + delta->x) > bbox->sup.x )
    { 
        delta->x = bbox->sup.x - pos->x;
    }

    if( (pos->y + delta->y) < bbox->inf.y )
    {
        delta->y = bbox->inf.y - pos->y;
    }
    else if( (pos->y + delta->y) > bbox->sup.y )
    { 
        delta->y = bbox->sup.y - pos->y;
    }

    if( (pos->z + delta->z) < bbox->inf.z )
    {
        delta->z = bbox->inf.z - pos->z;
    }
    else if( (pos->z + delta->z) > bbox->sup.z )
    { 
        delta->z = bbox->sup.z - pos->z;
    }

    return;
}


/*
 ***************************************************************************
 */
void
LightTranslateXY(RwReal xDelta, RwReal yDelta)
{
    if( CurrentLight && CurrentLight != AmbientLight && CurrentLight != DirectLight )
    {
        RwFrame *lightFrame, *cameraFrame;
        RwV3d right, up, delta;

        lightFrame = RpLightGetFrame(CurrentLight);
        cameraFrame = RwCameraGetFrame(Camera);

        right = *RwMatrixGetRight(RwFrameGetMatrix(cameraFrame));
        up = *RwMatrixGetUp(RwFrameGetMatrix(cameraFrame));

        /* 
         * Scale the movement of the light so it has the same 
         * direction as the mouse...
         */
        RwV3dScale(&right, &right, xDelta);
        RwV3dScale(&up, &up, yDelta);
        RwV3dAdd(&delta, &right, &up);

        ClampPosition(RwMatrixGetPos(RwFrameGetMatrix(lightFrame)), 
            &delta, &RoomBBox);

        RwFrameTranslate(lightFrame, &delta, rwCOMBINEPOSTCONCAT);
    }

    return;
}


/*
 ***************************************************************************
 */
void
LightTranslateZ(RwReal zDelta)
{
    if( CurrentLight && CurrentLight != AmbientLight && CurrentLight != DirectLight )
    {
        RwFrame *lightFrame, *cameraFrame;
        RwV3d delta;

        lightFrame = RpLightGetFrame(CurrentLight);
        cameraFrame = RwCameraGetFrame(Camera);

        RwV3dScale(&delta, RwMatrixGetAt(RwFrameGetMatrix(cameraFrame)), zDelta);

        ClampPosition(RwMatrixGetPos(RwFrameGetMatrix(lightFrame)), 
            &delta, &RoomBBox);

        RwFrameTranslate(lightFrame, &delta, rwCOMBINEPOSTCONCAT);
    }

    return;
}

/*
 ***************************************************************************
 */
