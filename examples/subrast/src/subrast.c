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
 * subrast.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how sub-rasters can be used to render multiple
 *          views.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "skeleton.h"
#include "camera.h"

#include "subrast.h"

RwCamera *Camera = NULL;
RwCamera *SubCameras[NUMOFSUBCAMERAS] = {NULL, NULL, NULL, NULL}; 



/*
 *****************************************************************************
 */
static void
CameraSetViewWindow(RwCamera *camera, RwReal width, RwReal height, RwReal vw)
{
    RwV2d viewWindow;
    RwVideoMode videoMode;

    RwEngineGetVideoModeInfo(&videoMode, RwEngineGetCurrentVideoMode());

    if( videoMode.flags & rwVIDEOMODEEXCLUSIVE )
    {
        viewWindow.x = vw;
        viewWindow.y = vw / DEFAULT_ASPECTRATIO;
    }
    else
    {
        if( width > height )
        {
            viewWindow.x = vw;
            viewWindow.y = (height * vw) / width;
        }
        else
        {
            viewWindow.x = (width * vw) / height;
            viewWindow.y = vw;
        }
    }

    RwCameraSetViewWindow(camera, &viewWindow);

    return;
}


/*
 *****************************************************************************
 */
void
UpdateSubRasters(RwCamera *mainCamera, RwInt32 mainWidth, RwInt32 mainHeight)
{
    RwRect rect[4];
    RwReal width, height, border;
    RwInt32 i;

    /* 
     * Size of the border between sub-rasters.... 
     */
    border = mainHeight * 0.05f;

    /*
     * Calculate the width and height for each sub-raster...
     */
    width =  ((RwReal)mainWidth  - border * 3.0f) / 2.0f;
    height = ((RwReal)mainHeight - border * 3.0f) / 2.0f;

    /* 
     * Define the top left rect...
     */
    rect[0].x = (RwInt32)border;
    rect[0].y = (RwInt32)border;
    rect[0].w = (RwInt32)width;
    rect[0].h = (RwInt32)height;

    /* 
     * Define the top right rect...
     */
    rect[1].x = (RwInt32)(width + border * 2.0f);
    rect[1].y = (RwInt32)border;
    rect[1].w = (RwInt32)width;
    rect[1].h = (RwInt32)height;

    /* 
     * Define the bottom left rect...
     */
    rect[2].x = (RwInt32)border;
    rect[2].y = (RwInt32)(height + border * 2.0f);
    rect[2].w = (RwInt32)width;
    rect[2].h = (RwInt32)height;

    /* 
     * Define the bottom right rect...
     */
    rect[3].x = (RwInt32)(width + border * 2.0f);
    rect[3].y = (RwInt32)(height + border * 2.0f);
    rect[3].w = (RwInt32)width;
    rect[3].h = (RwInt32)height;

    /*
     * Work out the view window for the sub-cameras.
     * The first one is a perspective view....
     */
    CameraSetViewWindow(SubCameras[0], width, height, DEFAULT_VIEWWINDOW);

    for(i=1; i<NUMOFSUBCAMERAS; i++)
    {
        /*
         * These sub-cameras will have parallel views so make
         * their view window a little bit larger, so we get to see summit...
         */
        CameraSetViewWindow(SubCameras[i], width, height, 
            DEFAULT_VIEWWINDOW + 0.4f);
    }

    for(i=0; i<NUMOFSUBCAMERAS; i++)
    {
        RwRasterSubRaster(RwCameraGetRaster(SubCameras[i]), 
            RwCameraGetRaster(mainCamera), &rect[i]);

        RwRasterSubRaster(RwCameraGetZRaster(SubCameras[i]), 
            RwCameraGetZRaster(mainCamera), &rect[i]);
    }

    return;
}


/*
 *****************************************************************************
 */
static void
PositionSubCameras(void)
{
    RwReal dist = 2.5f;
    RwFrame *frame;
    RwV3d pos;

    /*
     * An arbitrary perspective view... 
     */
    pos.x = pos.y = 0.0f;
    pos.z = -4.0f;

    frame = RwCameraGetFrame(SubCameras[0]);
    RwFrameTranslate(frame, &pos, rwCOMBINEREPLACE);

    /*
     * A parallel view looking along the world's increasing Z-axis... 
     */
    pos.x = pos.y = 0.0f;
    pos.z = -dist;

    frame = RwCameraGetFrame(SubCameras[1]);
    RwFrameTranslate(frame, &pos, rwCOMBINEREPLACE);

    /*
     * To view the clump along the world's increasing X-axis... 
     */
    pos.x = -dist;
    pos.y = pos.z = 0.0f;

    frame = RwCameraGetFrame(SubCameras[2]);
    RwFrameRotate(frame, &YAxis, 90.0f, rwCOMBINEREPLACE);
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * To view the clump along the world's increasing Y-axis... 
     */
    pos.x = pos.z = 0.0f;
    pos.y = -dist;

    frame = RwCameraGetFrame(SubCameras[3]);
    RwFrameRotate(frame, &XAxis, -90.0f, rwCOMBINEREPLACE);
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
RwBool
CreateCameras(RpWorld *world)
{
    RwInt32 i;

    /*
     * This is the main camera, whose rasters are used soley as the 
     * parent of the sub-cameras' rasters. It does no rendering of itself,
     * so is not added and positioned in the world...
     */
    Camera = CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);

    if( Camera == NULL )
    {
        return FALSE;
    }

    /*
     * The sub-cameras' rasters are created with no memory allocated...
     */
    for(i=0; i<NUMOFSUBCAMERAS; i++)
    {
        SubCameras[i] = CameraCreate(0, 0, TRUE);
        
        if( SubCameras[i] == NULL )
        {
            return FALSE;
        }

        RwCameraSetNearClipPlane(SubCameras[i], 0.1f);
        RwCameraSetFarClipPlane(SubCameras[i], 30.0f);

        RpWorldAddCamera(world, SubCameras[i]);

        /*
         * The first sub-camera gives a perspective view, the others
         * are parallel views..
         */
        if( i > 0 )
        {
            RwCameraSetProjection(SubCameras[i], rwPARALLEL);
        }
    }

    PositionSubCameras();
    
    return TRUE;
}


/*
 *****************************************************************************
 */
void 
DestroyCameras(RpWorld *world)
{
    RwInt32 i;

    for(i=0; i<NUMOFSUBCAMERAS; i++ )
    {
        if( SubCameras[i] )
        {
            RpWorldRemoveCamera(world, SubCameras[i]);

            CameraDestroy(SubCameras[i]);
        }
    }

    if( Camera )
    {
        CameraDestroy(Camera);
    }

    return;
}

/*
 *****************************************************************************
 */
