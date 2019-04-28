
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
 * collis2.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the use of the RpCollision plugin to collide 
 *          an atomic with the static geometry in a world.
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpcollis.h"
#include "rprandom.h"

#include "skeleton.h"

#include "collis2.h"

typedef struct
{
    RwV3d currentPos;
    RwV3d currentVel;

    RwV3d previousPos;
    RwV3d previousVel;

    RwV3d force;
    RwV3d collisionNormal;

    RpAtomic *atomic;
} 
Object;

static Object Ball;

static RwReal ClosestTriangleDistance;

static RwBool Penetrating;
static RwBool Contacting;



/*
 *****************************************************************************
 */
static void 
GetRandomVector(RwV3d *vec, RwReal length)
{
    RwUInt32 random;

    /*
     * Returns a vector with a random direction, and with the 
     * specified length...
     */

    random = RpRandom();
    vec->x = 2.0f * ((RwReal)random / (RwUInt32MAXVAL>>1)) - 1.0f;

    random = RpRandom();
    vec->y = 2.0f * ((RwReal)random / (RwUInt32MAXVAL>>1)) - 1.0f;

    random = RpRandom();
    vec->z = 2.0f * ((RwReal)random / (RwUInt32MAXVAL>>1)) - 1.0f;

    RwV3dNormalize(vec, vec);
    RwV3dScale(vec, vec, length);

    return;
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
 *****************************************************************************
 */
void
ResetGravity(void)
{
    if( GravityOn )
    {
        Ball.force.x = 0.0f;
        Ball.force.y = -150.0f;
        Ball.force.z = 0.0f;
    }
    else
    {
        Ball.force.x = 0.0f;
        Ball.force.y = 0.0f;
        Ball.force.z = 0.0f;
    }

    return;
}


/*
 *****************************************************************************
 */
void
ResetBall(void)
{
    RwBBox bBox;
    RwV3d pos;

    /*
     * Place the atomic in the middle of the world...
     */
    bBox = *RpWorldGetBBox(RpAtomicGetWorld(Ball.atomic));

	RwV3dAdd(&pos, &bBox.sup, &bBox.inf);
	RwV3dScale(&pos, &pos, 0.5f);

    RwFrameTranslate(RpAtomicGetFrame(Ball.atomic), &pos, rwCOMBINEREPLACE);

    Ball.previousPos = pos;

    /*
     * Random initial velocity direction...
     */
    GetRandomVector(&Ball.previousVel, 300.0f);

    ResetGravity();

    return;
}


/*
 *****************************************************************************
 */
RwBool 
CreateBallAtomic(RpWorld *world)
{
    RpClump *clump = NULL;
    RwStream *stream;
    RwChar *path;

    path = RsPathnameCreate(RWSTRING("models/ball.dff"));
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
    else
    {
        return FALSE;
    }

    if( clump )
    {
        RpAtomic *atomic = NULL, *copy = NULL;

        RpClumpForAllAtomics(clump, GetFirstAtomic, &atomic);

        if( atomic )
        {
            copy = RpAtomicClone(atomic);
            RpAtomicSetFrame(copy, RwFrameCreate());

            RpAtomicSetFlags(copy, rpATOMICCOLLISIONTEST|rpATOMICRENDER);
            RpWorldAddAtomic(world, copy);

            RpClumpDestroy(clump);

            Ball.atomic = copy;

            ResetBall();

            return TRUE;
        }
    }

    return FALSE;
}


/*
 *****************************************************************************
 */ 
RwBool
DestroyBallAtomic(void)
{
    if( Ball.atomic )
    {
        RwFrame *frame;

        RpWorldRemoveAtomic(RpAtomicGetWorld(Ball.atomic), Ball.atomic);
    
        frame = RpAtomicGetFrame(Ball.atomic);
        if( frame )
        {
            RpAtomicSetFrame(Ball.atomic, NULL);

            RwFrameDestroy(frame);
        }

        RpAtomicDestroy(Ball.atomic);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
RespondToCollision(void)
{
    /*
     * Responds to a collision by reflecting the atomic's velocity about the
     * normal of the triangle it had a collision with...
     * (Vout = Vin - 2*Dot(N, Vin)N)
     */

    RwV3d temp;

    RwV3dScale(&temp, &Ball.collisionNormal, 
        -2.0f * RwV3dDotProduct(&Ball.collisionNormal, &Ball.currentVel));

    RwV3dAdd(&Ball.currentVel, &temp, &Ball.currentVel);

    if( DampingOn )
    {
        /*
         * Damp it (in a fudge-factor sort of way!)...
         */
        RwV3dScale(&Ball.currentVel, &Ball.currentVel, 0.95f);
    }

    return;
}


/*
 *****************************************************************************
 */
static RpCollisionTriangle *
WorldIntersectionCallBack(RpIntersection *intersection __RWUNUSED__, 
                          RpWorldSector *sector  __RWUNUSED__,
                          RpCollisionTriangle *triangle, 
                          RwReal distance,
                          void *data  __RWUNUSED__)
{
    /*
     * The parameter 'distance' is equal to the perpendicular distance of 
     * the atomic's bounding-sphere center from the collision triangle plane, 
     * divided by the sphere's radius. 
     * Hence, if distance = 1, the sphere is just touching.
     */

    const RwReal contactEpsilon = 0.01f;

    if( distance < (1.0f - contactEpsilon) )
    {
        /*
         * The atomic's sphere has penetrated the world's geometry...
         */
        Penetrating = TRUE;
       
        return NULL;
    }

    /*
     * If we get here, the sphere must be contacting the triangle's plane 
     * within the given tolerance. Select the triangle that has the 
     * smallest distance from the atomic's bounding sphere center. 
     * THIS ASSUMES A STRICTLY CONCAVE DISTRIBUTION OF WORLD TRIANGLES...
     */
    if( distance < ClosestTriangleDistance )
    {
        Ball.collisionNormal = triangle->normal;
        ClosestTriangleDistance = distance;

        Contacting = TRUE;
    }
 
    return triangle;
}


/*
 *****************************************************************************
 */
static void
FindWorldContact(void)
{
    RpIntersection intersection;

    /*
     * Update the ball's frame so that we can test
     * for collisions properly...
     */
    RwFrameTranslate(RpAtomicGetFrame(Ball.atomic), 
        &Ball.currentPos, rwCOMBINEREPLACE);

    /*
     * Initialize intersection indicators...
     */
    ClosestTriangleDistance = RwRealMAXVAL;
    Contacting = FALSE;
    Penetrating = FALSE;

    intersection.type = rpINTERSECTATOMIC;
    intersection.t.object = Ball.atomic;

    /*
     * Find intersections, if any...
     */
    RpCollisionWorldForAllIntersections(RpAtomicGetWorld(Ball.atomic), 
        &intersection, WorldIntersectionCallBack, NULL);

    return;
}


/*
 *****************************************************************************
 */
static void
Integrate(RwReal deltaTime)
{
    RwV3d temp;

    /*
     * Update position...
     */
    RwV3dScale(&temp, &Ball.previousVel, deltaTime);
    RwV3dAdd(&Ball.currentPos, &Ball.previousPos, &temp);

    /*
     * Update velocity...
     */
    RwV3dScale(&temp, &Ball.force, deltaTime);
    RwV3dAdd(&Ball.currentVel, &Ball.previousVel, &temp);

    return;
}


/*
 *****************************************************************************
 */
static void 
Advance(RwReal delta)
{
    RwReal currentTime, targetTime;

    currentTime = 0.0f;
    targetTime = delta;

    while( currentTime < delta )
    {
        Integrate(targetTime - currentTime);

        FindWorldContact();

        if( Penetrating )
        {
            /*
             * The atomic has penetrated the world geometry (integrated too
             * far) so subdivide for new target time...
             */
            targetTime = (currentTime + targetTime) * 0.5f;

            RwFrameTranslate(RpAtomicGetFrame(Ball.atomic), 
                &Ball.previousPos, rwCOMBINEREPLACE);

            /*
             * Stop the subdivision of delta from running away...
             */
            if( RwRealAbs(currentTime - targetTime) < 0.00001f )
            {
                ResetBall();

                break;
            }
        }
        else
        {
            if( Contacting )
            {
                RespondToCollision();
            }

            currentTime = targetTime;
            targetTime = delta;

            Ball.previousPos = Ball.currentPos;
            Ball.previousVel = Ball.currentVel;
        }
    }

    return;
}


/*
 *****************************************************************************
 */
void
Run(RwReal deltaTime)
{
    const RwReal maxTimeStep = 0.01f;
    RwReal inc;

    while( deltaTime > 0 )
    {
        inc = deltaTime;

        if( inc > maxTimeStep )
        {
            inc = maxTimeStep;
        }

        Advance(inc);

        deltaTime -= inc;
    }

    return;
}

/*
 *****************************************************************************
 */
