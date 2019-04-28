
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
 * movement.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 * Reviewed by: John Irwin (with substantial edits).
 *
 * Purpose: RenderWare3 BSP viewer.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "main.h"
#include "movement.h"
#include "spline.h"
#include "world.h"

RwReal TotalTilt;

RwBool CameraFlying = FALSE;
RwReal FlyingSpeed;


/*
 *****************************************************************************
 */
void 
CameraPoint(RwReal tilt, RwReal turn)
{
    RwFrame *frame;
    RwV3d delta, pos, *right;
    RwV3d yAxis = {0.0f, 1.0f, 0.0f};

    /*
     * Limit the camera's tilt so that it never quite reaches
     * exactly +90 or -90 degrees...
     */
    if( TotalTilt + tilt > 89.0f )
    {
        tilt = 89.0f - TotalTilt;

    }
    else if( TotalTilt + tilt < -89.0f )
    {
        tilt = -89.0f - TotalTilt;
    }

    TotalTilt += tilt;

    if( WorldHasSpline )
    {
        /*
         * The camera is rotated relative to the base-frame moving along
         * the spline path, so we use the camera's frame...
         */
        frame = RwCameraGetFrame(Camera);
    }
    else
    {
        /*
         * Use the base-frame to rotate the camera...
         */
        frame = RwFrameGetParent(RwCameraGetFrame(Camera));
    }

    /*
     * Remember where the camera is...
     */
    pos = *RwMatrixGetPos(RwFrameGetMatrix(frame));

    /*
     * Remove the translation component...
     */
    RwV3dScale(&delta, &pos, -1.0f);
    RwFrameTranslate(frame, &delta, rwCOMBINEPOSTCONCAT);

    /*
     * Rotate to the new direction...
     */
    right = RwMatrixGetRight(RwFrameGetMatrix(frame));
    RwFrameRotate(frame, right, -tilt, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(frame, &yAxis, turn, rwCOMBINEPOSTCONCAT);

    /*
     * Put the camera back to where it was...
     */
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
void
TranslateCameraZ(RwReal dist)
{
    RwFrame *frame;
    RwV3d at;

    /*
     * Move the camera along it's look-at vector the given distance...
     */
    if( WorldHasSpline )
    {
        /*
         * The camera is moved relative to the base-frame moving along
         * the spline path, so we use the camera's frame...
         */
        frame = RwCameraGetFrame(Camera);
    }
    else
    {
        /*
         * Use the base-frame to advance the camera...
         */
        frame = RwFrameGetParent(RwCameraGetFrame(Camera));
    }

    at = *RwMatrixGetAt(RwFrameGetMatrix(frame));

    RwV3dScale(&at, &at, dist);

    RwFrameTranslate(frame, &at, rwCOMBINEPOSTCONCAT);

    return;
}


/*
 *****************************************************************************
 */
void 
CameraFly(RwCamera *camera, RwReal deltaTime)
{
    /*
     * Camera flying is controlled from a system timer...
     */
    if( WorldHasSpline )
    {
        RwV3d yAxis = {0.0f, 1.0f, 0.0f};

        /*
         * Advance the camera forward along the spline path...
         */
        RwFrame *baseFrame;
        RwMatrix *baseMatrix;

        baseFrame = RwFrameGetParent(RwCameraGetFrame(camera));
        baseMatrix = RwFrameGetMatrix(baseFrame);

        SplinePos += FlyingSpeed * deltaTime;

        RpSplineFindFrame(WayPointSpline, rpSPLINEPATHNICEENDS, 
            SplinePos, &yAxis, baseMatrix);

        RwMatrixUpdate(baseMatrix);
        RwFrameUpdateObjects(baseFrame);
    }
    else
    {
        /*
         * Advance the camera forward along the look-at vector...
         */
        RwFrame *baseFrame;
        RwV3d at;
        RwReal dist;

        dist = FlyingSpeed * deltaTime;

        baseFrame = RwFrameGetParent(RwCameraGetFrame(camera));

        at = *RwMatrixGetAt(RwFrameGetMatrix(baseFrame));

        RwV3dScale(&at, &at, dist);

        RwFrameTranslate(baseFrame, &at, rwCOMBINEPOSTCONCAT);
    }

    return;
}

/*
 *****************************************************************************
 */
