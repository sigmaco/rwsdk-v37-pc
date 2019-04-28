
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
 * clmpcntl.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Handles rotation and translation of the clump.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "main.h"
#include "scene.h"

#include "clmpcntl.h"
#include "clmpview.h"

#define CLUMPTRANSLATIONSPEEDFACTOR (1.5f)
#define CLUMPINCFACTOR (0.1f)
#define CLUMPDECFACTOR (0.03f)

#define CLUMPROTATIONMAXSPEED (100.0f)
#define CLUMPROTATIONINC (CLUMPROTATIONMAXSPEED * CLUMPINCFACTOR)
#define CLUMPROTATIONDEC (CLUMPROTATIONMAXSPEED * CLUMPDECFACTOR)

/* 
 * Factor to equalise input from input devices - mouse\analogue stick
 */
#if (defined (SKY ))
#define CLUMPSPINFACTOR (50.0f)
#else
#define CLUMPSPINFACTOR (5.0f)
#endif /* SKY */

static RwV3d Zero = { 0.0f, 0.0f, 0.0f };

static RwBool TranslationChanged = FALSE;
static RwBool RotationChanged = FALSE;

static RwReal ClumpTranslationZ = 0.0f;
static RwV3d ClumpRotation = { 0.0f, 0.0f, 0.0f };

static RwReal ClumpTranslationMaxSpeed;
static RwReal ClumpTranslationInc;
static RwReal ClumpTranslationDec;

RwBool ClumpRotate = FALSE;
RwBool ClumpTranslate = FALSE;

RwBool ClumpDirectTranslate = FALSE;
RwBool ClumpDirectRotate = FALSE;

RwReal ClumpTranslateDeltaZ = 0.0f;
RwV3d ClumpRotateDelta = { 0.0f, 0.0f, 0.0f };

RwBool ClumpPick = FALSE;


/*
 *****************************************************************************
 */
static void
GetZTranslation(void)
{
    TranslationChanged = FALSE;

    if( ClumpTranslate )
    {
        ClumpTranslationZ += ClumpTranslateDeltaZ * ClumpTranslationInc;
        TranslationChanged = TRUE;

        /* 
         * Bound the translation...
         */
        if( fabs(ClumpTranslationZ) > ClumpTranslationMaxSpeed )
        {
            ClumpTranslationZ = (ClumpTranslationZ > 0.0f)?
                               ClumpTranslationMaxSpeed :
                               -ClumpTranslationMaxSpeed;
        }
    }

    /* 
     * If input ends, slow the moving clump to a stop...
     */
    if( !ClumpTranslate && ClumpTranslationZ != 0.0f )
    {
        if( fabs(ClumpTranslationZ) < 1.0f )
        {
            ClumpTranslationZ = 0.0f;
        }
        else
        {
            ClumpTranslationZ += (ClumpTranslationZ < 0.0f)?
                                  ClumpTranslationDec :
                                  -ClumpTranslationDec ;
        }        

        ClumpTranslateDeltaZ = 0.0f;

        TranslationChanged = TRUE;        
    }

    return;
}


/*
 *****************************************************************************
 */
static void
GetRotation(void)
{
    RotationChanged = FALSE;

    if( ClumpRotate )
    {
        ClumpRotation.x += ClumpRotateDelta.x * CLUMPROTATIONINC;
        ClumpRotation.y += ClumpRotateDelta.y * CLUMPROTATIONINC;
        RotationChanged = TRUE;

        /* 
         * Bound the rotation...
         */
        if( fabs(ClumpRotation.x) > CLUMPROTATIONMAXSPEED )
        {
            ClumpRotation.x = (ClumpRotation.x > 0.0f)?
                               CLUMPROTATIONMAXSPEED :
                               -CLUMPROTATIONMAXSPEED;
        }
        if( fabs(ClumpRotation.y) > CLUMPROTATIONMAXSPEED )
        {
            ClumpRotation.y = (ClumpRotation.y > 0.0f)?
                               CLUMPROTATIONMAXSPEED :
                               -CLUMPROTATIONMAXSPEED;
        }
    }

    /* 
     * If input ends, slow the moving clump to a stop...
     */
    if( !ClumpRotate && !SpinOn && (ClumpRotation.x != 0.0f || ClumpRotation.y != 0.0f) )
    {
        if( fabs(ClumpRotation.x) < 1.0f )
        {
            ClumpRotation.x = 0.0f;
        }
        else
        {
            ClumpRotation.x += (ClumpRotation.x < 0.0f)?
                                  CLUMPROTATIONDEC :
                                  -CLUMPROTATIONDEC ;
        }

        if( fabs(ClumpRotation.y) < 1.0f )
        {
            ClumpRotation.y = 0.0f;
        }
        else
        {
            ClumpRotation.y += (ClumpRotation.y < 0.0f)?
                                  CLUMPROTATIONDEC :
                                  -CLUMPROTATIONDEC ;
        }

        ClumpRotateDelta.x = ClumpRotateDelta.y = ClumpRotateDelta.z = 0.0f;
        RotationChanged = TRUE;        
    }

    return;
}

/*
 *****************************************************************************
 */
static void
ClumpControlTranslateZ(RwReal deltaZ)
{
    RwFrame *clumpFrame = (RwFrame *)NULL;
    
    clumpFrame = RpClumpGetFrame(Clump);

    if( clumpFrame )
    {
        RwV3d translation = {0.0f, 0.0f, 0.0f};
        translation.z = -deltaZ;
        RwFrameTranslate(clumpFrame, &translation, rwCOMBINEPOSTCONCAT);
    }
    return;
}

/*
 *****************************************************************************
 */
static void
ClumpControlRotate(RwReal deltaX, RwReal deltaY)
{
    RwFrame *clumpFrame = (RwFrame *)NULL;
    
    clumpFrame = RpClumpGetFrame(Clump);

    if( clumpFrame )
    {
        RwMatrix *cameraMatrix;
        RwV3d right, up, clumpPos;

        cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
        right = *RwMatrixGetRight(cameraMatrix);
        up = *RwMatrixGetUp(cameraMatrix);

        /*
         * Rotate about the clump's bounding sphere center...
         */
        RwV3dTransformPoints(&clumpPos, &(ClumpSphere.center), 1, RwFrameGetLTM(clumpFrame));

        /*
         * First translate back to the origin...
         */
        RwV3dScale(&clumpPos, &clumpPos, -1.0f);
        RwFrameTranslate(clumpFrame, &clumpPos, rwCOMBINEPOSTCONCAT);

        /*
         * ...do the rotations...
         */
        RwFrameRotate(clumpFrame, &up, deltaY, rwCOMBINEPOSTCONCAT);
        RwFrameRotate(clumpFrame, &right, -deltaX, rwCOMBINEPOSTCONCAT);

        /*
         * ...and translate back...
         */
        RwV3dScale(&clumpPos, &clumpPos, -1.0f);
        RwFrameTranslate(clumpFrame, &clumpPos, rwCOMBINEPOSTCONCAT);
        
    }    
    return;
}


/*
 *****************************************************************************
 */
void
ClumpControlUpdate(RwReal delta)
{
    if (ClumpLoaded)
    {
        GetZTranslation();

        if( TranslationChanged )
        {
            ClumpControlTranslateZ(ClumpTranslationZ * delta);
        }
        
        GetRotation();

        if( RotationChanged || SpinOn)
        {
            ClumpControlRotate(ClumpRotation.x * delta, ClumpRotation.y * delta);
        }
    }
    return;
}

/*
 *****************************************************************************
 */
void
ClumpControlDirectTranslateZ(RwReal deltaZ)
{    
    ClumpControlTranslateZ(deltaZ);
    
    return;
}


/*
 *****************************************************************************
 */
void
ClumpControlDirectRotate(RwReal deltaX, RwReal deltaY)
{
    ClumpControlRotate(deltaX, deltaY);

    if( SpinOn )
    {
        /*
         * Make sure spin is large enough to be noticed...
         */
        if( fabs(deltaX) > 0.25f )
        {
            ClumpRotation.x = deltaX * CLUMPSPINFACTOR;
        }
        if( fabs(deltaY) > 0.25f )
        {
            ClumpRotation.y = deltaY * CLUMPSPINFACTOR;
        }
    }
    
    return;
}

/*
 *****************************************************************************
 */
void
ClumpControlReset(void)
{
    /*
     * Initialize parameters for manipulation based on clump bounding sphere
     */
    ClumpTranslationMaxSpeed = ClumpSphere.radius * CLUMPTRANSLATIONSPEEDFACTOR;
    ClumpTranslationInc = ClumpTranslationMaxSpeed * CLUMPINCFACTOR;
    ClumpTranslationDec = ClumpTranslationMaxSpeed * CLUMPDECFACTOR;

    ClumpTranslationZ = 0.0f;
    ClumpRotation = Zero;

    return;
}

/*
 *****************************************************************************
 */
