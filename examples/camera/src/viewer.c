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
 * viewer.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics camera example.
 *          can be calculated.
 ****************************************************************************/


#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"
#include "camera.h"

#include "viewer.h"


/*
 *****************************************************************************
 */
RwCamera *
ViewerCreate(RpWorld *world)
{
    RwCamera *camera;

    camera = CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);

    if( camera )
    {
        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 500.0f);

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}


/*
 *****************************************************************************
 */
RpWorld *
ViewerDestroy(RwCamera *camera, RpWorld *world)
{
    if( camera && world )
    {
        RpWorldRemoveCamera(world, camera);

        CameraDestroy(camera);
    }

    return world;
}


/*
 *****************************************************************************
 */
RwCamera * 
ViewerSize(RwCamera *camera, RwRect *rect, RwReal viewWindow, RwReal aspectRatio)
{
    CameraSize(camera, rect, viewWindow, aspectRatio);

    return camera;
}


/*
 *****************************************************************************
 */
RwCamera *
ViewerMove(RwCamera *camera, RwV3d *offset)
{
    CameraMove(camera, offset);

    return camera;
}


/*
 *****************************************************************************
 */
RwCamera *
ViewerRotate(RwCamera *camera, RwReal deltaX, RwReal deltaY)
{
    CameraTilt(camera, NULL, deltaY);
    CameraPan(camera, NULL, deltaX);
    
    return camera;
}


/*
 *****************************************************************************
 */
RwCamera *
ViewerTranslate(RwCamera *camera, RwReal deltaX, RwReal deltaY)
{
    RwV3d offset;

    offset.x = deltaX;
    offset.y = deltaY;
    offset.z = 0.0f;

    CameraMove(camera, &offset);
    
    return camera;
}


/*
 *****************************************************************************
 */
RwCamera *
ViewerSetPosition(RwCamera *camera, RwV3d *position)
{
    RwFrameTranslate(RwCameraGetFrame(camera), position, rwCOMBINEREPLACE);
    
    return camera;
}


/*
 *****************************************************************************
 */
