
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
 * collis3.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: Illustrates the detection of collisions with atomics and
 *          the creation of geometry collision data to speed up the
 *          intersection tests.
 *                         
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpcollis.h"
#include "rprandom.h"

#include "skeleton.h"

#include "collis3.h"

#define FARAWAY (RwRealMAXVAL)

#define BOWL_RADIUS (9.8f)
#define BALL_RADIUS (0.6f)

#define NUM_SPINNERS (13)
#define NUM_BALLS (5)

#define SPINNER_PERIOD_MIN (4.0f)
#define SPINNER_PERIOD_MAX (8.0f)
#define SPINNER_RANGE_MIN (90.0f)     /* degrees */
#define SPINNER_RANGE_MAX (360.0f)

typedef struct
{
    RpAtomic *atomic;
    RwReal omega2;
    RwReal speed;
    RwReal accel;
}
Spinner; 

typedef struct
{
    RpAtomic *atomic;
    RwV3d *pos;
    RwReal radius;
    RwV3d velocity;
}
Ball;

typedef struct
{
    RwV3d closestPoint;
    RwReal distance;
    RwMatrix *ltm;
}
CollisionParams;

typedef enum 
{
    XDIM = 0,
    YDIM,
    ZDIM,

    DIMENSIONFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
}
Dimension;

static RpAtomic *BowlAtomic = NULL;
static RpAtomic *SpinnerAtomic = NULL;
static RpAtomic *BallAtomic = NULL;

static Spinner Spinners[NUM_SPINNERS];
static Ball Balls[NUM_BALLS];

static const RwV3d ZeroVec = {0.0f, 0.0f, 0.0f};

static const RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
static const RwV3d Yaxis = {0.0f, 1.0f, 0.0f};
static const RwV3d Zaxis = {0.0f, 0.0f, 1.0f};

RwBool CollisionDataGenerated = FALSE;



/*
 ******************************************************************************
 */
static RwReal
RandomReal(RwReal min, RwReal max)
{
    RwUInt32 random;
    
    /* 
     * A random real value in the range min to max...
     */
    random = RpRandom();
    
    return min + (max - min) * (RwReal)random / (RwUInt32MAXVAL>>1);
}


/*
 *****************************************************************************
 */
static RpClump *
ClumpLoad(const RwChar *name)
{
    RpClump *clump = NULL;
    RwStream *stream;
    RwChar *path;

    path = RsPathnameCreate(name);
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            clump = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    return clump;
}


/*
 *****************************************************************************
 */
static RpAtomic *
GetFirstAtomic(RpAtomic *atomic, void *data)
{
    *(RpAtomic **)data = atomic;

    return NULL;
}


/*
 ******************************************************************************
 */
static RwBool
SpinnerInitialize(Spinner *spinner, RpWorld *world, RwReal theta, RwReal phi)
{
    RwFrame *frame;
    RwV3d pos;
    RwReal omega, period, range;

    /*
     * Clone base atomic and create a frame...
     */
    spinner->atomic = RpAtomicClone(SpinnerAtomic);
    frame = RwFrameCreate();
    RpAtomicSetFrame(spinner->atomic, frame);

    /*
     * Set position in bowl...
     */
    pos.x = pos.z = 0.0f;
    pos.y = -BOWL_RADIUS;
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(frame, &Xaxis, theta, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(frame, &Yaxis, phi, rwCOMBINEPOSTCONCAT);

    /*
     * Initialize data for oscillatory motion...
     */
    period = RandomReal(SPINNER_PERIOD_MIN, SPINNER_PERIOD_MAX);
    range = RandomReal(SPINNER_RANGE_MIN, SPINNER_RANGE_MAX);
    omega = 2.0f * rwPI / period;

    spinner->omega2 = omega * omega;
    spinner->speed = range * omega;
    spinner->accel = 0.0f;

    RpWorldAddAtomic(world, spinner->atomic);

    return TRUE;
}


/*
 ******************************************************************************
 */
static RwBool
BallInitialize(Ball *ball, RpWorld *world)
{
    RwFrame *frame;
    RwV3d scale, pos;
    RwReal radius;

    /*
     * Clone the base atomic and create a frame...
     */
    ball->atomic = RpAtomicClone(BallAtomic);
    frame = RwFrameCreate();
    RpAtomicSetFrame(ball->atomic, frame);

    /*
     * Get pointer to the balls position (for convenience)...
     */
    ball->pos = 
        RwMatrixGetPos(RwFrameGetMatrix(RpAtomicGetFrame(ball->atomic)));

    /* 
     * Scale to required radius...
     */
    ball->radius = BALL_RADIUS;
    radius = RpAtomicGetBoundingSphere(ball->atomic)->radius;
    scale.x = scale.y = scale.z = ball->radius / radius;
    RwFrameScale(frame, &scale, rwCOMBINEREPLACE);

    /*
     * Set a random initial position...
     */
    pos.x =  0.5f * BOWL_RADIUS * RandomReal(-1.0f, 1.0f);
    pos.y = -0.5f * BOWL_RADIUS;
    pos.z =  0.5f * BOWL_RADIUS * RandomReal(-1.0f, 1.0f);
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * Set velocity...
     */
    ball->velocity = ZeroVec;

    RpWorldAddAtomic(world, ball->atomic);

    return TRUE;
}


/*
 ******************************************************************************
 */
RwBool
CollisionObjectsCreate(RpWorld *world)
{
    RwInt32 i;
    RpClump *clump;
    RwChar *path;

    /*
     * Set path for textures...
     */
    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    /*
     * Load the bowl atomic and add to the world...
     */
    clump = ClumpLoad(RWSTRING("models/bowl.dff"));
    if( !clump )
    {
        return FALSE;
    }

    RpClumpForAllAtomics(clump, GetFirstAtomic, &BowlAtomic);
    RpWorldAddAtomic(world, BowlAtomic);

    /*
     * Load the base atomic for the spinners...
     */
    clump = ClumpLoad(RWSTRING("models/spinner.dff"));
    if( !clump )
    {
        return FALSE;
    }

    RpClumpForAllAtomics(clump, GetFirstAtomic, &SpinnerAtomic);

    /*
     * Create spinner at center...
     */
    SpinnerInitialize(&Spinners[0], world, 0.0f, 0.0f);

    /*
     * Create 6 spinners evenly placed in first ring...
     */
    for(i=1; i<7; i++)
    {
        SpinnerInitialize(&Spinners[i], world, 23.0f, (i-1) * 60.0f);
    }

    /*
     * Create 6 spinners evenly placed in outer ring...
     */
    for(i=7; i<13; i++)
    {
        SpinnerInitialize(&Spinners[i], world, 40.0f, (i-7) * 60.0f + 30.0f);
    }

    /*
     * Load base atomic for the balls...
     */
    clump = ClumpLoad(RWSTRING("models/ball.dff"));
    if( !clump )
    {
        return FALSE;
    }

    RpClumpForAllAtomics(clump, GetFirstAtomic, &BallAtomic);

    /*
     * Create the balls...
     */
    for(i=0; i<NUM_BALLS; i++)
    {
        BallInitialize(&Balls[i], world);
    }

    CollisionDataGenerated = FALSE;

    return TRUE;
}


/*
 ******************************************************************************
 */
static void
DestroyAtomicWithFrame(RpAtomic *atomic)
{
    RwFrame *frame;

    /*
     * Detatch and destroy the frame...
     */
    frame = RpAtomicGetFrame(atomic);
    RpAtomicSetFrame(atomic, NULL);
    RwFrameDestroy(frame);

    /*
     * Now destroy the atomic...
     */
    RpAtomicDestroy(atomic);

    return;
}


/*
 ******************************************************************************
 */
void
CollisionObjectsDestroy(RpWorld *world)
{
    RwUInt32 i;

    /*
     * Destroy the bowl...
     */
    RpWorldRemoveAtomic(world, BowlAtomic);
    RpClumpDestroy(RpAtomicGetClump(BowlAtomic));

    /*
     * Destroy spinners...
     */
    for(i=0; i<NUM_SPINNERS; i++)
    {
        RpWorldRemoveAtomic(world, Spinners[i].atomic);

        DestroyAtomicWithFrame(Spinners[i].atomic);
    }

    RpClumpDestroy(RpAtomicGetClump(SpinnerAtomic));

    /*
     * Destroy balls...
     */
    for(i=0; i<NUM_BALLS; i++)
    {
        RpWorldRemoveAtomic(world, Balls[i].atomic);

        DestroyAtomicWithFrame(Balls[i].atomic);
    }

    RpClumpDestroy(RpAtomicGetClump(BallAtomic));

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
                if (((( tri[i]->y <= pt->y ) && ( pt->y < tri[j]->y )) ||
                    ((  tri[j]->y <= pt->y ) && ( pt->y < tri[i]->y ))) &&
                    ( pt->z < ( tri[j]->z - tri[i]->z ) * ( pt->y - tri[i]->y ) /
                              ( tri[j]->y - tri[i]->y ) + tri[i]->z ))
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
               if (((( tri[i]->z <= pt->z ) && ( pt->z < tri[j]->z )) ||
                    (( tri[j]->z <= pt->z ) && ( pt->z < tri[i]->z ))) &&
                    ( pt->x < ( tri[j]->x - tri[i]->x ) * ( pt->z - tri[i]->z ) /
                              ( tri[j]->z - tri[i]->z ) + tri[i]->x ))
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
               if (((( tri[i]->y <= pt->y ) && ( pt->y < tri[j]->y )) ||
                    (( tri[j]->y <= pt->y ) && ( pt->y < tri[i]->y ))) &&
                    ( pt->x < ( tri[j]->x - tri[i]->x ) * ( pt->y - tri[i]->y ) /
                              ( tri[j]->y - tri[i]->y ) + tri[i]->x ))
               {
                  inside = !inside;
               }
            }

            break;
        }

        case DIMENSIONFORCEENUMSIZEINT:
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
SphereAtomicCollisionCB(RpIntersection *intersection,
                        RpCollisionTriangle *collTriangle, 
                        RwReal distance __RWUNUSED__, 
                        void *data)
{
    CollisionParams *params = (CollisionParams *)data;
    RwV3d v[3], *worldVerts[3], worldNormal;
    RwV3d *center = &intersection->t.sphere.center; 
    RwV3d projPoint;
    RwReal dist2plane;

    /*
     * Transform the collision triangle to world coordinates using LTM...
     */
    worldVerts[0] = &v[0];
    worldVerts[1] = &v[1];
    worldVerts[2] = &v[2];

    RwV3dTransformPoint(worldVerts[0], 
        collTriangle->vertices[0], params->ltm);

    RwV3dTransformPoint(worldVerts[1], 
        collTriangle->vertices[1], params->ltm);

    RwV3dTransformPoint(worldVerts[2], 
        collTriangle->vertices[2], params->ltm);

    RwV3dTransformVector(&worldNormal, 
        &collTriangle->normal, params->ltm);

    /*
     * Correct normal for any LTM scaling...
     */
    RwV3dNormalize(&worldNormal, &worldNormal);

    /*
     * Project point onto plane of triangle.
     * First the perpendicular distance (signed)...
     */
    dist2plane = RwV3dDotProduct(worldVerts[0], &worldNormal) -
        RwV3dDotProduct(center, &worldNormal);

    /*
     * ...then the projected point...
     */
    RwV3dScale(&projPoint, &worldNormal, dist2plane);
    RwV3dAdd(&projPoint, &projPoint, center);

    /* 
     * Does the projected point lie within the collision triangle...
     */
    if( PointWithinTriangle(&projPoint, worldVerts, &worldNormal) )
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
            RwReal  distance;
            RwV3d   closestPoint, temp;

            FindNearestPointOnLine(&closestPoint, 
                                   &projPoint, 
                                   worldVerts[i], 
                                   worldVerts[(i+1)%3]);

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
SphereAtomicCollision(RwSphere *sphere, RwV3d *delta, RpAtomic *atomic)
{
    RpIntersection intersection;
    CollisionParams params;

    /*
     * Test for intersections between a sphere shifted by delta, and
     * the atomic. Find the point on a triangle of the atomic
     * corresponding to maximum penetration and modify delta to eliminate
     * this penetration.
     *
     * This may leave the sphere still penetrating secondary triangles.
     */

    /* 
     * Construct the sphere to detect collisions...
     */            
    intersection.type = rpINTERSECTSPHERE;
    RwV3dAdd(&intersection.t.sphere.center, &sphere->center, delta);
    intersection.t.sphere.radius = sphere->radius;

    /*
     * Initialize collision parameters. The LTM is required by the intersection
     * callback to transform the atomic's triangles to world coordinates...
     */
    params.ltm = RwFrameGetLTM(RpAtomicGetFrame(atomic));
    params.distance = FARAWAY; 

    /* 
     * Test for collisions...
     */
    RpAtomicForAllIntersections(atomic, &intersection, 
        SphereAtomicCollisionCB, &params);
    
    if( params.distance < FARAWAY )
    {
        /* 
         * Process collision...
         */
        RwV3d contact;
        RwReal shift;

        /* 
         * Calculate vector from point of contact through center...
         */
        RwV3dSub(&contact, &intersection.t.sphere.center, &params.closestPoint);
        RwV3dNormalize(&contact, &contact);
        
        /*
         * Shift in contact direction...
         */
        shift = intersection.t.sphere.radius - params.distance;
        RwV3dIncrementScaled(delta, &contact, shift);
    } 

    return;            
}


/*
 ******************************************************************************
 */
static void
UpdateSpinners(RwReal deltaTime)
{
    RwInt32 i;

    /*
     * Update the orientation of the spinners according to their oscillation
     * parameters. A numerical difference approximation is adequate here...
     */
    for(i=0; i<NUM_SPINNERS; i++)
    {
        RwReal angle;

        Spinners[i].accel -= Spinners[i].omega2 * Spinners[i].speed * deltaTime;
        Spinners[i].speed += Spinners[i].accel * deltaTime;

        angle = Spinners[i].speed * deltaTime;

        RwFrameRotate(RpAtomicGetFrame(Spinners[i].atomic),
            &Yaxis, angle, rwCOMBINEPRECONCAT);
    }
    
    return;
}


/*
 ******************************************************************************
 */
static void
UpdateBalls(RwReal deltaTime  __RWUNUSED__)
{
    RwInt32 i;

    /* 
     * Update the positions of the balls, and resolve collisions with the
     * the bowl, the spinners and between balls.
     *
     * The example is intended to demonstrate intersection testing 
     * with atomics and the advantages of geometry collision data. Therefore,
     * the following algorithm is kept simple and dumb. It uses
     * a large number of intersection tests and does not guarantee that
     * geometry will not penetrate.
     */

    for(i=0; i<NUM_BALLS; i++)
    {
        RwV3d delta;
        RwSphere sphere;
        RwInt32 j;

        /*
         * Calculate sphere's new position... 
         */
        delta.x = delta.z = 0.0f;
        delta.y = -0.04f;

        sphere.center = *(Balls[i].pos);
        sphere.radius = Balls[i].radius;

        /*
         * Check for collisions between the ball and the world
         * Do it three times to move out of corners...
         */
        SphereAtomicCollision(&sphere, &delta, BowlAtomic);
        SphereAtomicCollision(&sphere, &delta, BowlAtomic);
        SphereAtomicCollision(&sphere, &delta, BowlAtomic);

        /*
         * Check for collisions between the ball and the spinner
         * Do each three times to move out of corners...
         */
        for(j=0; j<NUM_SPINNERS; j++)
        {
            SphereAtomicCollision(&sphere, &delta, Spinners[j].atomic);
            SphereAtomicCollision(&sphere, &delta, Spinners[j].atomic);
            SphereAtomicCollision(&sphere, &delta, Spinners[j].atomic);
        }

        /* 
         * Check for collisions between balls...
         */
        for(j=0; j<NUM_BALLS; j++)
        {
            if( j != i )
            {
                RwV3d sep;
                RwReal distSq;
                RwReal sumRadSq, sumRad;

                /*
                 * Find current separation...
                 */
                RwV3dSub(&sep, Balls[i].pos, Balls[j].pos);
                RwV3dAdd(&sep, &sep, &delta);
                distSq = RwV3dDotProduct(&sep, &sep);

                /* 
                 * Are balls intersecting...
                 */
                sumRad = Balls[i].radius + Balls[j].radius;
                sumRadSq = sumRad * sumRad;
                if( distSq < sumRadSq )
                {
                    RwReal dist;

                    rwSqrt(&dist, distSq);

                    RwV3dIncrementScaled(&delta, &sep, sumRad - dist);
                }
            }
        }

        /*
         * Translate the ball...
         */     
        RwFrameTranslate(RpAtomicGetFrame(Balls[i].atomic),
            &delta, rwCOMBINEPOSTCONCAT);
    }

    return;
}


/*
 ******************************************************************************
 */
RwBool
CollisionObjectsUpdate(RpWorld *world  __RWUNUSED__, 
                       RwReal deltaTime)
{
    /*
     * Impose limit if too much time has elapsed...
     */
    deltaTime = deltaTime < 0.05f ? deltaTime : 0.05f;

    /*
     * Update objects, testing for collisions...
     */
    UpdateSpinners(deltaTime);

    UpdateBalls(deltaTime);

    return TRUE;
}


/*
 ******************************************************************************
 */
RwBool
CollisionDataBuildCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return TRUE;
    }

    /*
     * Build the collision extension data for the geometry of the spinner
     * and the bowl.
     *  
     * This information allows fast isolation of the triangles in the geometry
     * which potentially intersect a given primitive before individual tests
     * are performed, thus improving performance.
     *
     * Without this data, every triangle must be tested separately...
     */
    if( RpCollisionGeometryBuildData(RpAtomicGetGeometry(SpinnerAtomic),
            NULL) == (RpGeometry *)NULL )
    {
        RsErrorMessage(RWSTRING("Could not build collision data for SpinnerAtomic."));

        CollisionDataGenerated = FALSE;

        return TRUE;
    }

    if( RpCollisionGeometryBuildData(RpAtomicGetGeometry(BowlAtomic),
            NULL) == (RpGeometry *)NULL )
    {
        RsErrorMessage(RWSTRING("Could not build collision data for BowlAtomic."));

        CollisionDataGenerated = FALSE;

        return TRUE;
    }

    CollisionDataGenerated = TRUE;

    return TRUE;
}


/*
 ******************************************************************************
 */
static RpClump *
ClumpSave(RpClump *clump, const RwChar *name)
{
    RwStream *stream;
    RwChar *path;

    path = RsPathnameCreate(name);
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        RpClumpStreamWrite(clump, stream);

        RwStreamClose(stream, NULL);
    }

    return clump;
}


/*
 ******************************************************************************
 */
RwBool
CollisionDataSaveCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return CollisionDataGenerated;
    }

    /*
     * The collision extension data for the geometry, once generated, is 
     * saved when the RpGeometry is written out with the atomics of the
     * spinner and bowl...
     */
    ClumpSave(RpAtomicGetClump(SpinnerAtomic), 
        RWSTRING("models/spinnerc.dff"));

    ClumpSave(RpAtomicGetClump(BowlAtomic), 
        RWSTRING("models/bowlc.dff"));

    return TRUE;
}

/*
 ******************************************************************************
 */
