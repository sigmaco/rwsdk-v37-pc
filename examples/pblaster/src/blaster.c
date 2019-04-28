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
 * ptank.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: The PBlaster example demonstrate a blaster gun effects, using both 
 * 3d matrix based particles to generate the laser beams and screen aligned 
 * particles for the charging effect.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpptank.h"

#include "skeleton.h"
#include "menu.h"

#include "burst.h"
#include "shot.h"

#include "blaster.h"

#ifdef SKY
#include "sky/skyburst.h"
#endif /*SKY*/

/*
 * Local Types
 */
typedef struct BlasterStateParam BlasterStateParam;

struct  BlasterStateParam
{
    RwReal timeLaps;

    RwReal ooTimeLaps;

    RwBool burstStatus;
    RwBool burstMorph;
    
    RwReal burstTimeFactor;
    RwV3d  burstRadius;
};

/*
 * Local defines
 */
#define MAX_SHOT                        (50)
#define PRT_PER_BLASTER_BURST           (100)

/*
 * Global variables
 */

blasterShot Shot;

/*
 * Local variables
 */
static burstObj *Burst;
static RwMatrix *BlasterMtx;
static RpClump  *BlasterClump;

static RwBool BlasterLocked = FALSE;
static RwBool TriggerReleased;

BlasterStateParam BlasterParams[BS_STATE_NUM_STATES];

const RwRGBA    BlasterBigShotInitialColor = { 128, 128, 255, 255 };
const RwRGBA    BlasterBigShotDeadColor = { 0, 0,0, 0 };
RwRGBAReal      BlasterBigShotDeltaColor;
const RwReal    BlasterBigShotDeathTime = 1.00f;
const RwReal    BlasterBigShotSpeed = 3.0f;
const RwReal    BlasterBigShotLength = 100.0f;


const RwRGBA    BlasterShotInitialColor = { 32, 32, 255, 255 };
const RwRGBA    BlasterShotDeadColor = { 32, 32, 255, 0 };
RwRGBAReal      BlasterShotDeltaColor;
const RwReal    BlasterShotDeathTime = 1.00f;
const RwReal    BlasterShotSpeed = 10.0f;
const RwReal    BlasterShotWidth = 5.0f;
const RwReal    BlasterShotLength = 50.0f;

/*
 *****************************************************************************
 */
static RpClump *
ClumpLoad(void)
{
    RwChar *path;
    RpClump *clump = NULL;
    RwStream *stream = NULL;
    
    path = RsPathnameCreate(RWSTRING("models/pgun.dff"));
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

    if( clump )
    {
        const RwV3d pos = {0.0f, 0.0f, 0.0f};
        const RwV3d Xaxis = {1.0f, 0.0f, 0.0f};
        const RwV3d Yaxis = {0.0f, 1.0f, 0.0f};

        RwFrameTranslate(RpClumpGetFrame(clump), &pos, rwCOMBINEREPLACE);
        RwFrameRotate(RpClumpGetFrame(clump), &Xaxis, -30.0f, rwCOMBINEPOSTCONCAT);
        RwFrameRotate(RpClumpGetFrame(clump), &Yaxis, -45.0f, rwCOMBINEPOSTCONCAT);

    }

    return clump;
}


/*
 *****************************************************************************
 */
static void
BasterFillBlasterParamTable(void)
{
    RwInt32 i;

    /*
     * Set Up Idle state animation data
     */
    BlasterParams[BS_STATE_IDLE].timeLaps = -1.0f;
    BlasterParams[BS_STATE_IDLE].burstStatus = FALSE;
    BlasterParams[BS_STATE_IDLE].burstMorph = FALSE;
    
    /*
     * Set Up Precharging state animation data
     */
    BlasterParams[BS_STATE_PRECHARGING].timeLaps = 0.5f;
    BlasterParams[BS_STATE_PRECHARGING].burstStatus = FALSE;
    BlasterParams[BS_STATE_PRECHARGING].burstMorph = FALSE;
    
    /*
     * Set Up Charging state animation data
     */
    BlasterParams[BS_STATE_CHARGING].timeLaps = 1.0f;
    BlasterParams[BS_STATE_CHARGING].burstStatus = TRUE;
    BlasterParams[BS_STATE_CHARGING].burstMorph = FALSE;
    BlasterParams[BS_STATE_CHARGING].burstTimeFactor = 7.5f;
    BlasterParams[BS_STATE_CHARGING].burstRadius.x = 5.0f;
    BlasterParams[BS_STATE_CHARGING].burstRadius.y = 5.0f;
    BlasterParams[BS_STATE_CHARGING].burstRadius.z = 10.0f;
    
    /*
     * Set Up SuperCharging state animation data
     */
    BlasterParams[BS_STATE_SUPERCHARGING].timeLaps = 5.0f;
    BlasterParams[BS_STATE_SUPERCHARGING].burstStatus = TRUE;
    BlasterParams[BS_STATE_SUPERCHARGING].burstMorph = TRUE;
    BlasterParams[BS_STATE_SUPERCHARGING].burstTimeFactor = 7.5f;
    BlasterParams[BS_STATE_SUPERCHARGING].burstRadius.x = 20.0f;
    BlasterParams[BS_STATE_SUPERCHARGING].burstRadius.y = 20.0f;
    BlasterParams[BS_STATE_SUPERCHARGING].burstRadius.z = 20.0f;

    /*
     * Set Up DisCharging state animation data
     */
    BlasterParams[BS_STATE_DISCHARGING].timeLaps = 0.50f;
    BlasterParams[BS_STATE_DISCHARGING].burstStatus = TRUE;
    BlasterParams[BS_STATE_DISCHARGING].burstMorph = TRUE;
    BlasterParams[BS_STATE_DISCHARGING].burstTimeFactor = -7.5f;
    BlasterParams[BS_STATE_DISCHARGING].burstRadius.x = 0.1f;
    BlasterParams[BS_STATE_DISCHARGING].burstRadius.y = 0.1f;
    BlasterParams[BS_STATE_DISCHARGING].burstRadius.z = 0.1f;

    /*
     * Set Up Charged state animation data
     */
    BlasterParams[BS_STATE_CHARGED].timeLaps = -1.0f;
    BlasterParams[BS_STATE_CHARGED].burstStatus = TRUE;
    BlasterParams[BS_STATE_CHARGED].burstMorph = FALSE;
    BlasterParams[BS_STATE_CHARGED].burstTimeFactor = 7.5f;
    BlasterParams[BS_STATE_CHARGED].burstRadius.x = 20.0f;
    BlasterParams[BS_STATE_CHARGED].burstRadius.y = 20.0f;
    BlasterParams[BS_STATE_CHARGED].burstRadius.z = 20.0f;

    /*
     * Set Up Fired state animation data
     */
    BlasterParams[BS_STATE_FIRED].timeLaps = 0.5f;
    BlasterParams[BS_STATE_FIRED].burstStatus = TRUE;
    BlasterParams[BS_STATE_FIRED].burstMorph = TRUE;
    BlasterParams[BS_STATE_FIRED].burstTimeFactor = 14.5f;
    BlasterParams[BS_STATE_FIRED].burstRadius.x = 0.0f;
    BlasterParams[BS_STATE_FIRED].burstRadius.y = 0.0f;
    BlasterParams[BS_STATE_FIRED].burstRadius.z = 0.0f;


    /*
     * Precalculing of One Over timelaps 
     */
    for(i=0;i<BS_STATE_NUM_STATES;i++)
    {
        if( BlasterParams[i].timeLaps != 0.0f )
        { 
            BlasterParams[i].ooTimeLaps = 1.0f / BlasterParams[i].timeLaps;
        }
        else
        {
            BlasterParams[i].ooTimeLaps = 0.0f;
        }
    }

    return;
}


/*
 *****************************************************************************
 */
static RwFrame *
getFirstChildCB(RwFrame *frame, void *data)
{
    RwMatrix **mtx = (void*)data;
    
    *mtx = RwFrameGetLTM(frame);
    
    RwFrameForAllChildren(frame,getFirstChildCB,data);

    /* 
     * Only looking for the first child so stop it here !
     */
    return NULL;
}


/*
 *****************************************************************************
 */
RwBool   
BlasterCreate(void)
{
    /*
     * Initialize the burst manager with 1 burst object3
     * and PRT_PER_BLASTER_BURST particles per bursts.
     */
    if(FALSE == BurstsInitialize(1,PRT_PER_BLASTER_BURST))
    {
        return FALSE;
    }

    /*
     * Initialize the shot manager with MAX_SHOT shots
     */
    if(FALSE == ShotsCreate(MAX_SHOT))
    {
        return FALSE;
    }

    /*
     * Load the blaster Clump
     */
    BlasterClump = ClumpLoad();
    if( NULL == BlasterClump )
    {
        return FALSE;
    }

    /*
     * Get the clump matrix for effects calculations.
     * As the exporters add a base frame : we're looking for the first child
     * of the first child of the clump's frame.
     */
    getFirstChildCB(RpClumpGetFrame(BlasterClump),(void *)&BlasterMtx);
    
    /*
     * Create the burst object for the blaster ( 1 blaster == 1 burst object )
     */
    Burst = BurstGetNewBurst();

    /*
     * Blaster Shot state is Idle
     */
    Shot.state = BS_STATE_IDLE;

    /*
     * Blaster Trigger is not pressed
     */
    TriggerReleased = FALSE;

    /*
     * Blaster is not locked
     */
    BlasterLocked = FALSE;

    /*
     * Set up animation settings
     */
    BasterFillBlasterParamTable();
    
    BlasterShotDeltaColor.red   = ((RwReal)BlasterShotDeadColor.red     - 
                   (RwReal)BlasterShotInitialColor.red  )/BlasterShotDeathTime;
    BlasterShotDeltaColor.green = ((RwReal)BlasterShotDeadColor.green   - 
                   (RwReal)BlasterShotInitialColor.green)/BlasterShotDeathTime;
    BlasterShotDeltaColor.blue  = ((RwReal)BlasterShotDeadColor.blue    - 
                   (RwReal)BlasterShotInitialColor.blue )/BlasterShotDeathTime;
    BlasterShotDeltaColor.alpha = ((RwReal)BlasterShotDeadColor.alpha   - 
                   (RwReal)BlasterShotInitialColor.alpha)/BlasterShotDeathTime;

    BlasterBigShotDeltaColor.red   = ((RwReal)BlasterBigShotDeadColor.red     -
             (RwReal)BlasterBigShotInitialColor.red  )/BlasterBigShotDeathTime;
    BlasterBigShotDeltaColor.green = ((RwReal)BlasterBigShotDeadColor.green   - 
             (RwReal)BlasterBigShotInitialColor.green)/BlasterBigShotDeathTime;
    BlasterBigShotDeltaColor.blue  = ((RwReal)BlasterBigShotDeadColor.blue    -
             (RwReal)BlasterBigShotInitialColor.blue )/BlasterBigShotDeathTime;
    BlasterBigShotDeltaColor.alpha = ((RwReal)BlasterBigShotDeadColor.alpha   -
             (RwReal)BlasterBigShotInitialColor.alpha)/BlasterBigShotDeathTime;

    return TRUE;
}


/*
 *****************************************************************************
 */
void
BlasterShotStart(void)
{
    /* 
     * On Blaster Trigger Pressed
     */
    if( BlasterLocked == FALSE )
    {
        /* 
         * Create a new small shot
         */
        
        shotObject *shot;

        shot = ShotShot(BlasterMtx,BlasterShotWidth,BlasterShotLength);

        shot->initialColor = BlasterShotInitialColor;

        shot->deltaColor = BlasterShotDeltaColor;

        shot->decayTime = BlasterShotDeathTime;
        shot->speed = BlasterShotSpeed;

        /* 
         * Set state to Precharging
         */
        Shot.state = BS_STATE_PRECHARGING;
        Shot.time = 0.0f;
        Burst->time = 0.0f;

        TriggerReleased = FALSE;

    }

    return;
}


/*
 *****************************************************************************
 */void 
BlasterShotEnd(void)
{
    /* 
     * On Blaster Trigger Released
     */
    if( BlasterLocked == FALSE )
    {
        TriggerReleased = TRUE;
    }

    return;
}


/*
 *****************************************************************************
 */
void 
BlasterUpdate(RwReal deltaT)
{
    RwV3d *pos;
    RwV3d *at;
    RwV3d tmp;
    BlasterStateParam *currentParams;
    BlasterStateParam *nextParams;
    RwBool stateChange = FALSE;
    RwInt32 nextState = 0;
    
    /* 
     * Update laser shots
     */
    ShotsUpdate(deltaT);
    
    /* 
     * Update Blaster time
     */
    Shot.time += deltaT;

    /* 
     * Get blaster current state
     */
    currentParams = &BlasterParams[Shot.state];


    /* 
     * Open the burst render list : if there was more than one blaster
     * This function call would be made in the "update all blasters" function 
     * rather than per blaster.
     */
    BurstBeginRenderList();
    
    /* 
     * Update particle burst if needed
     */
    if( TRUE == currentParams->burstStatus )
    {
        /* 
         * Burst follow blaster orientation
         */
        RwMatrixCopy(&Burst->orientation,BlasterMtx);
        pos = RwMatrixGetPos(&Burst->orientation);
        at = RwMatrixGetAt(&Burst->orientation);
        RwV3dScale(&tmp,at,Burst->radius.z);
        RwV3dAdd(pos,pos,&tmp);

        /* 
         * Animate burst sphere if needed
         */
        if( TRUE == currentParams->burstMorph )
        {
            RwV3dScale(&tmp,&Shot.deltaRadius,Shot.time);
            RwV3dAdd(&Burst->radius,&Shot.radius,&tmp);
        }

        /* 
         * Update burst time
         */
        Burst->time += deltaT * currentParams->burstTimeFactor;

        /* 
         * Generate burst and add it to the burst render list
         */
#if (defined SKY)
        SkyBurstAddToRenderList(&Burst, 1);
#else
        BurstAddToRenderList(&Burst, 1);
#endif                                  /* (defined(SKY)) */

    }
        
    /* 
     * close the burst render list 
     */
    BurstEndRenderList();
    
    /* 
     * Resolve blaster state. Based on blaster time, 
     * locked status and trigger status
     */
    switch(Shot.state)
    {
        case BS_STATE_PRECHARGING:
            if( Shot.time > BlasterParams[Shot.state].timeLaps )
            {
                stateChange = TRUE;
                nextState = BS_STATE_CHARGING;
            }

            if( TRUE == TriggerReleased )
            {
                stateChange = TRUE;
                nextState = BS_STATE_IDLE;
            }
            break;
        case BS_STATE_CHARGING:
            if( Shot.time > BlasterParams[Shot.state].timeLaps )
            {
                stateChange = TRUE;
                nextState = BS_STATE_SUPERCHARGING;
            }
            else if( TRUE == TriggerReleased )
            {
                stateChange = TRUE;
                nextState = BS_STATE_DISCHARGING;
            }
            break;
        case BS_STATE_DISCHARGING:
            if(( Burst->time < 0.0f ) ||
               ( Shot.time > BlasterParams[Shot.state].timeLaps ))
            {
                stateChange = TRUE;
                nextState = BS_STATE_IDLE;
            }
            break;
        case BS_STATE_SUPERCHARGING:
            if( Shot.time > BlasterParams[Shot.state].timeLaps )
            {
                stateChange = TRUE;
                nextState = BS_STATE_CHARGED;
            }
        case BS_STATE_CHARGED:
            if( TRUE == TriggerReleased )
            {
                stateChange = TRUE;
                nextState = BS_STATE_FIRED;
            }
            break;
        case BS_STATE_FIRED:
            if( Shot.time > BlasterParams[Shot.state].timeLaps )
            {
                stateChange = TRUE;
                nextState = BS_STATE_IDLE;
            }
            
            break;
        case BS_STATE_IDLE:
        default:
            break;
    }

    /* 
     * Resolve blaster state change.
     */
    if( TRUE == stateChange )
    {
        RwInt32 prevState;
        nextParams = &BlasterParams[nextState];
        
        /* 
         * Set Blaster new state
         */
        prevState = Shot.state;
        Shot.state = nextState;
        Shot.time = 0.0f;

        /* 
         * Set Burst new state as needed
         */
        if( TRUE == nextParams->burstStatus )
        {
            if( TRUE == nextParams->burstMorph )
            {
                RwV3dAssign(&Shot.radius,&Burst->radius);
                RwV3dSub(&Shot.deltaRadius,&nextParams->burstRadius,&Shot.radius);
                RwV3dScale(&Shot.deltaRadius,&Shot.deltaRadius,nextParams->ooTimeLaps);

            }
        }
        
        if( TRUE == TriggerReleased )
        {
            TriggerReleased = FALSE;
        }
        
        /* 
         * Resolve state change action
         */
        switch(Shot.state)
        {
            case BS_STATE_FIRED:
                Shot.deltaRadius.x = Shot.deltaRadius.y = 0.0f;
            case BS_STATE_DISCHARGING:
                BlasterLocked = TRUE;
                break;
            case BS_STATE_IDLE:
                BlasterLocked = FALSE;
                if( BS_STATE_FIRED == prevState )
                {
                    shotObject *shot;

                    shot = ShotShot(BlasterMtx,Burst->radius.x,BlasterBigShotLength);

                    shot->initialColor = BlasterBigShotInitialColor;

                    shot->deltaColor = BlasterBigShotDeltaColor;

                    shot->decayTime = BlasterBigShotDeathTime;
                    shot->speed = BlasterBigShotSpeed;

                }
                break;
            case BS_STATE_CHARGING:
                   RwV3dAssign(&Burst->radius, &BlasterParams[BS_STATE_CHARGING].burstRadius);
                break;
            case BS_STATE_PRECHARGING:
            case BS_STATE_SUPERCHARGING:
            case BS_STATE_CHARGED:
                break;
            default:
                break;
        }
    }
}


/*
 *****************************************************************************
 */
void     
BlasterRender(void)
{
    /* 
     * Render blaster clump
     */
    RpClumpRender(BlasterClump);

    /* 
     * Render burst clump
     */
    BurstsRender();
    
    /* 
     * Render Shot clump
     */
    ShotsRender();

    /* 
     * Reset Z test/write state
     */
     RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);


}


/*
 *****************************************************************************
 */
void     
BlasterDestroy(void)
{
    if( NULL != BlasterClump )
    {
        RpClumpDestroy(BlasterClump);
    }

    BurstsDestroy();

    ShotsDestroy();
}

/*
 *****************************************************************************
 */
void
BlasterRotate(RwReal xAngle, RwReal yAngle)
{
    RwMatrix *cameraMatrix;
    RwFrame *frame;

    RwV3d right, up, pos;

    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(Camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);
    
    frame = RpClumpGetFrame(BlasterClump);
    pos = *RwMatrixGetPos(RwFrameGetMatrix(frame));

    /*
     * First translate back to the origin..
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * ..do the rotations..
     */
    RwFrameRotate(frame, &right, yAngle, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(frame, &up, xAngle, rwCOMBINEPOSTCONCAT);

    /*
     * ..and translate back..
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    return;
}
