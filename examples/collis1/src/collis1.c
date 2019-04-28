
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
 * collis1.c
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

#include "collis1.h"

#define FARAWAY (RwRealMAXVAL)
#define SPEED (25.0f)

enum _Dimension
{
    XDIM = 0,
    YDIM,
    ZDIM,

    _DIMENSIONFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum _Dimension Dimension;

typedef struct
{
    RwV3d closestPoint;
    RwReal distance;
}
CollisionParams;

enum CollisionModes CurrentCollisionMode = SPHERE;
enum MovementModes CurrentMovementMode = STOPPED;

RwReal CameraRadius = 15.0f;



/*
 *****************************************************************************
 */
static RpCollisionTriangle *
CameraCollisionLineCallback(RpIntersection *intersection __RWUNUSED__,
                            RpWorldSector *sector __RWUNUSED__, 
                            RpCollisionTriangle *collTriangle,
                            RwReal distance, 
                            void *data)
{
    RwReal *closest = (RwReal *)data;

    /* 
     * Keep track of closest point...
     */
    if( distance < *closest )
    {
        *closest = distance;
    }

    return collTriangle;
}


/*
 *****************************************************************************
 */
static void
CameraCollisionLine(RwV3d *camPos, RwV3d *camDelta, RpWorld *world)
{
    RpIntersection intersection;
    RwLine line;
    RwReal range;

    /* 
     * Construct a vertical line through the position of the camera.
     * The line extends the full height of the world bounding-box...
     */
    RwV3dAdd(&line.start, camPos, camDelta);
    line.start.y = RpWorldGetBBox(world)->sup.y;

    line.end = line.start;
    line.end.y = RpWorldGetBBox(world)->inf.y;
    
    /*
     * Create intersection object...
     */
    intersection.type = rpINTERSECTLINE;
    intersection.t.line = line;
    
    /* 
     * Test for collisions...
     */
    range = FARAWAY;

    RpCollisionWorldForAllIntersections(world, 
        &intersection, CameraCollisionLineCallback, &range);

    if( range < FARAWAY )
    {
        RwV3d pos;

        /* 
         * Compute the position of the closest intersection point...
         */
        RwV3dSub(&pos, &line.end, &line.start);
        RwV3dScale(&pos, &pos, range);
        RwV3dAdd(&pos, &pos, &line.start);
        
        /*
         * Compute new camera delta vector...
         */
        pos.y += CameraRadius;
        RwV3dSub(&pos, &pos, camPos);
        RwV3dAdd(camDelta, &pos, camDelta);
    }
    else
    {
        /*
         * Should never get to here...
         */
        RsErrorMessage(RWSTRING("Fallen off world!"));
    }

    return;
}


/*
 *****************************************************************************
 */
static RwBool
PointWithinTriangle(RwV3d *pt, RwV3d *tri[3], RwV3d *normal)
{
    /* 
     * Simple algorithm to determine whether or not a point lies within a 
     * triangle. The idea is to see how many edges are crossed when a 
     * semi-infinite line is taken from the point along a particular axis.
     * If an even number of lines are crossed then the point is outside,
     * otherwise it is inside...
     */

    Dimension dimension;
    RwReal absX, absY, absZ;
    RwBool inside = FALSE;
    RwInt32 i, j;

    /*
     * Determine weakest dimension, so we can work in 2D...
     */     
    absX = (RwReal)RwFabs(normal->x);
    absY = (RwReal)RwFabs(normal->y);
    absZ = (RwReal)RwFabs(normal->z);
    
    dimension = 
        absZ > absY ? (absZ > absX ? ZDIM:XDIM) : (absY > absX ? YDIM:XDIM);

    switch( dimension )
    {
        case XDIM:
        {    
            /* 
             * Process every edge - (0,2) (1,0) (2,1) ...
             */
            for(i=0, j=2; i<3; j=i++)
            {
                if( (((tri[i]->y <= pt->y) && (pt->y < tri[j]->y)) ||
                     ((tri[j]->y <= pt->y) && (pt->y < tri[i]->y))) &&
                     (pt->z < (tri[j]->z - tri[i]->z) * (pt->y - tri[i]->y) / 
                     (tri[j]->y - tri[i]->y) + tri[i]->z) )
               {
                  inside = !inside;
               }
            }

            break;
        }

        case YDIM:
        {    
            for(i=0, j=2; i<3; j=i++)
            {
               if( (((tri[i]->z <= pt->z) && (pt->z < tri[j]->z)) ||
                    ((tri[j]->z <= pt->z) && (pt->z < tri[i]->z))) &&
                    (pt->x < (tri[j]->x - tri[i]->x) * (pt->z - tri[i]->z) / 
                    (tri[j]->z - tri[i]->z) + tri[i]->x) )
               {
                  inside = !inside;
               }
            }

            break;
        }

        case ZDIM:
        {    
            for(i=0, j=2; i<3; j=i++)
            {
               if( (((tri[i]->y <= pt->y) && (pt->y < tri[j]->y)) ||
                    ((tri[j]->y <= pt->y) && (pt->y < tri[i]->y))) &&
                    (pt->x < (tri[j]->x - tri[i]->x) * (pt->y - tri[i]->y) / 
                    (tri[j]->y - tri[i]->y) + tri[i]->x) )
               {
                  inside = !inside;
               }
            }

            break;
        }

    case _DIMENSIONFORCEENUMSIZEINT:
        break;
        

    }

    return inside;    
}


/*
 *****************************************************************************
 */
static RwV3d *
FindNearestPointOnLine(RwV3d *result, RwV3d *point, RwV3d *start, RwV3d *end)
{
    RwReal mu;
    RwV3d line;

    RwV3dSub(&line, end, start);

    /*
     * Project point onto the line. The value of mu is the 
     * distance of the projected point from the start of the line 
     * scaled by the length of the line...
     */
    mu = RwV3dDotProduct(point, &line) - RwV3dDotProduct(start, &line);

    if( mu <= 0 )
    {
        /*
         * Point lies on the line outside the start point...
         */
        *result = *start;
    }
    else
    {
        RwReal lineLength2;

        lineLength2 = RwV3dDotProduct(&line, &line);

        if( mu < lineLength2 )
        {
            /*
             * Point lies on the line between start and end.
             * Calculate point along line...
             */
            mu /= lineLength2;

            RwV3dScale(result, &line, mu);
            RwV3dAdd(result, result, start);
        }
        else
        {
            /*
             * Point lies on the line outside end point...
             */
            *result = *end;
        }
    }

    return result;
}


/*
 *****************************************************************************
 */
static RpCollisionTriangle *
CameraCollisionSphereCallback(RpIntersection *intersection,
                              RpWorldSector *sector  __RWUNUSED__, 
                              RpCollisionTriangle *collTriangle,
                              RwReal distance __RWUNUSED__, 
                              void *data)
{
    CollisionParams *params;
    RwV3d *center, projPoint, *normal;
    RwReal dist2plane;

    params = (CollisionParams *)data;
    center = &intersection->t.sphere.center;
    normal = &collTriangle->normal;

    /*
     * Project point onto plane of triangle.
     * First the perpendicular distance (signed)...
     */
    dist2plane = RwV3dDotProduct(collTriangle->vertices[0], normal) -
                 RwV3dDotProduct(center, normal);

    /*
     * ...then the projected point...
     */
    RwV3dScale(&projPoint, normal, dist2plane);
    RwV3dAdd(&projPoint, &projPoint, center);

    /* 
     * Does the projected point lie within the collision triangle...
     */
    if( PointWithinTriangle(&projPoint, collTriangle->vertices, normal) )
    {
        RwReal distance;

        distance = (RwReal)RwFabs(dist2plane);

        if( distance < params->distance )
        {
            params->closestPoint = projPoint;
            params->distance = distance;
        }
    }
    else
    {
        /*
         * Projected point lies outside the triangle, so find the nearest
         * point on the triangle boundary...
         */

        RwInt32 i;

        /* 
         * Process every triangle edge in turn...
         */
        for(i=0; i<3; i++)
        {
            RwReal distance;
            RwV3d closestPoint, temp;

            FindNearestPointOnLine(&closestPoint,
                                   &projPoint, 
                                   collTriangle->vertices[i], 
                                   collTriangle->vertices[(i+1)%3]);

            /*
             * Calculate distance between point within the triangle
             * and the projected point...
             */
            RwV3dSub(&temp, center, &closestPoint);
            distance = RwV3dLength(&temp);

            if( distance < params->distance )
            {
                params->closestPoint = closestPoint;
                params->distance = distance;
            }
        }
    }

    return collTriangle;
}


/*
 *****************************************************************************
 */
static void
CameraCollisionSphere(RwV3d *camPos, RwV3d *camDelta, RpWorld *world)
{
    RpIntersection intersection;
    RwSphere sphere;
    CollisionParams params; 

    /* 
     * Construct the sphere to detect collisions...
     */            
    RwV3dAdd(&sphere.center, camPos, camDelta);
    sphere.radius = CameraRadius;            

    /*
     * Create intersection object...
     */            
    intersection.type = rpINTERSECTSPHERE;
    intersection.t.sphere = sphere;

    /* 
     * Test for intersections with the world...
     */
    params.distance = FARAWAY;

    RpCollisionWorldForAllIntersections(world, 
        &intersection, CameraCollisionSphereCallback, &params);
    
    if( params.distance < FARAWAY )
    {
        RwV3d contact, nudge;
        RwReal dot;

        /* 
         * Unit vector from closest point to sphere center...
         */
        RwV3dSub(&contact, &intersection.t.sphere.center, &params.closestPoint);
        RwV3dNormalize(&contact, &contact);

        /*
         * Calculate a vector sufficient to move the sphere away 
         * from a penetrating position...
         */
        RwV3dScale(&nudge, &contact, 
            intersection.t.sphere.radius - params.distance);
        
        /*
         * Cancel movement in contact direction...
         */
        dot = RwV3dDotProduct(camDelta, &contact);
        if( dot < 0.0f )
        {
            RwV3dScale(&nudge, &contact, dot);
            RwV3dSub(camDelta, camDelta, &nudge);
            
            return;
        }
        
        /*
         * Adjust position for camera radius changes...
         */
        if( !camDelta->x && !camDelta->y && !camDelta->z )
        {
            RwV3dAdd(camDelta, camDelta, &nudge);
            
            return;
        }
    } 

    return;            
}


/*
 *****************************************************************************
 */
static void
CameraConfine(RwV3d *pos, RwV3d *delta, RpWorld *world)
{
    const RwBBox *bbox;

    /* 
     * Does not bound y-values to world bounding box - may prevent camera with 
     * large radius from climbing to top of highest peak in world...
     */

    bbox = RpWorldGetBBox(world);

    if( (pos->x - CameraRadius + delta->x) < bbox->inf.x )
    {
        delta->x = bbox->inf.x - pos->x + CameraRadius;
    }
    else if( (pos->x + CameraRadius + delta->x) > bbox->sup.x )
    { 
        delta->x = bbox->sup.x - pos->x - CameraRadius;
    }

    if( (pos->z - CameraRadius + delta->z) < bbox->inf.z )
    {
        delta->z = bbox->inf.z - pos->z + CameraRadius;
    }
    else if( (pos->z + CameraRadius + delta->z) > bbox->sup.z )
    { 
        delta->z = bbox->sup.z - pos->z - CameraRadius;
    }
    
    return;
}


/*
 *****************************************************************************
 */
void
CameraUpdate(RwCamera *camera, RpWorld *world, RwReal deltaTime)
{
    RwReal cameraSpeed;

    static RwReal oldRadius = 0.0f;

    switch( CurrentMovementMode )
    {
        case FORWARDS:
        {
            cameraSpeed = SPEED * deltaTime;

            break;
        }

        case BACKWARDS:
        {
            cameraSpeed = -SPEED * deltaTime;

            break;
        }

        case STOPPED:     
        default:
        {
            cameraSpeed = 0.0f;

            break;
        }
    }

    /*
     * Only perform calculations if camera is moving or radius changed...
     */
    if( cameraSpeed != 0.0f || oldRadius != CameraRadius )
    {
        RwMatrix *matrix;
        RwV3d *pos, delta;

        matrix = RwFrameGetMatrix(RwCameraGetFrame(camera));
        pos = RwMatrixGetPos(matrix);

        delta = *RwMatrixGetAt(matrix);

        /*
         * Camera's change in position (assuming unrestricted movement)... 
         */
        RwV3dScale(&delta, &delta, cameraSpeed);
        
        /*
         * Keep camera inside the world...
         */
        CameraConfine(pos, &delta, world);
    
        /*
         * Check for collisions between the camera and the world
         * (camera's change in position assuming restricted movement)...
         */
        switch( CurrentCollisionMode )
        {
            case LINE:
            {    
                CameraCollisionLine(pos, &delta, world);

                break;
            }
    
            case SPHERE:
            {
                CameraCollisionSphere(pos, &delta, world);

                break;
            } 

            case COLLISIONMODESFORCEENUMSIZEINT:
            {
                break;
            }
            
        }
    
        /*
         * Finally translate the camera...
         */     
        RwFrameTranslate(RwCameraGetFrame(camera), &delta, rwCOMBINEPOSTCONCAT);
        
        oldRadius = CameraRadius;
    }

    return;
}

/*
 *****************************************************************************
 */

