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
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Illustrates simple collision detection between an RpWorld and
 *          a line or sphere.
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpcollis.h"

#include "skeleton.h"

#include "movement.h"

#define FARAWAY (12345.0f)

/* These control how faster we fall or rise when the terrain beneath
 * us changes height; and how fast we move forward.  If we move forward
 * much faster than we move up and down, then we risk passing through
 * the terrain.  Whether this happens or not depends on the gradient
 * of the terrain beneath us.  Try to keep SPEED less than DESCENTRATE
 * and you'll probably live to fight another day.
 */
#define DESCENTRATE (0.8f)
#define SPEED (0.5f)

RwReal CameraFloatHeight = 15.0f;
static RwReal CameraSpeed = 0.0f;

/*
 *****************************************************************************
 */
static RpCollisionTriangle *
CameraFloatCB(RpIntersection *intersection,
              RpWorldSector *sector,
              RpCollisionTriangle *collTriangle,
              RwReal distance, void *data)
{
    RwReal closest = *(RwReal *)data;

    if (distance < closest)
    {
        *(RwReal *)data = distance;
    }

    return collTriangle;
}

/*
 *****************************************************************************
 */
static void
CameraFloat(RwCamera *cam, RwReal height, RpWorld *world)
{
    RwFrame        *f         = RwCameraGetFrame(cam);
    RwV3d          *camPos    = RwMatrixGetPos(RwFrameGetMatrix(f));
    RwV3d          *axis      = RwMatrixGetRight(RwFrameGetMatrix(f));
    RwV3d           inf       = RpWorldGetBBox(world)->inf;
    RwV3d           sup       = RpWorldGetBBox(world)->sup;
    RwReal          h         = FARAWAY;
    RpIntersection  intersect;
    RwV3d           start, end;
    RwLine          line;

    start = *camPos;
    start.y = sup.y;
    end = *camPos;
    end.y = inf.y;

    line.start = start;
    line.end = end;

    intersect.type = rpINTERSECTLINE;
    intersect.t.line = line;

    RpCollisionWorldForAllIntersections(world, &intersect, CameraFloatCB, &h);

    if (h < FARAWAY)
    {
        RwV3d v; /* point of intersection */
        RwReal l;
        RwReal above;

        /* compute v, the intersection point */
        RwV3dSub(&v, &end, &start);
        RwV3dScale(&v, &v, h);
        RwV3dAdd(&v, &v, &start);

        above = camPos->y - v.y;

        l = (above - height) * DESCENTRATE;
        camPos->y -= l;

        RwMatrixUpdate(RwFrameGetMatrix(f));
    }

    RwFrameUpdateObjects(f);
}

/*
 *****************************************************************************
 */
static void
CameraRestrict(RwCamera *cam, RpWorld *world)
{
    RwFrame *f = RwCameraGetFrame(cam);
    RwMatrix *m = RwFrameGetMatrix(f);
    RwV3d *pos = RwMatrixGetPos(m);
    RwBBox bbox = *RpWorldGetBBox(world);

    pos->x = (pos->x < bbox.inf.x) ? bbox.inf.x : pos->x;
    pos->z = (pos->z < bbox.inf.z) ? bbox.inf.z : pos->z;

    pos->x = (pos->x > bbox.sup.x) ? bbox.sup.x : pos->x;
    pos->z = (pos->z > bbox.sup.z) ? bbox.sup.z : pos->z;
}

/*
 *****************************************************************************
 */
static void
CameraRail(RwCamera *cam, RwReal speed)
{
    RwFrame *f = RwCameraGetFrame(cam);
    RwMatrix *m = RwFrameGetMatrix(f);
    RwV3d off = *RwMatrixGetAt(m);

    RwV3dScale(&off, &off, speed);
    RwFrameTranslate(f, &off, rwCOMBINEPOSTCONCAT);
}

/*
 *****************************************************************************
 */
void
MoveCameraStart(RwBool forward)
{
    if (forward)
    {
        CameraSpeed = SPEED;
    }
    else
    {
        CameraSpeed = -SPEED;
    }
}

/*
 *****************************************************************************
 */
void
MoveCameraStop(void)
{
    CameraSpeed = 0.0f;
}

/*
 *****************************************************************************
 */
void
MoveCameraUpdate(RwCamera *cam, RpWorld *wor)
{
    /* first move the camera across the terrain ... */
    CameraRail(cam, CameraSpeed);

    /* ... but keeping inside the world */
    CameraRestrict(cam, wor);

    /* Finally ensure that the camera floats above the ground */
    CameraFloat(cam, CameraFloatHeight, wor);
}
