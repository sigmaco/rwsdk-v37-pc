#include "rwcore.h"
#include "rpworld.h"
#include "rpptank.h"

#include "skeleton.h"
#include "menu.h"

#include "shot.h"

static RpAtomic *shotPTank;
static RwTexture *ShotPTankTexture;

static shotObject *shotObjList;

static RwInt32 ActiveShot;
static RwInt32 MaxActiveShot;


/*
 *****************************************************************************
 */
RwBool
ShotsCreate(RwInt32 maxNumShot)
{
    RwTexCoords shotUVs[2];

    RwFrame     *frame;
    RwChar      *path;

    /*
     * Each laser shot is created using two 3d oriented particles 
     * to simulate a beam.
     *
     * Create shot global PTank :
     * One PTank contains all the shots, reducing 
     * the number of RpAtomic render to one, 
     * whatever the number of shot is.
     */

    /*
     * Create the Ptank atomic : Matrix(including position) per particles, 
     * color per particles,
     * constant set of texture coordinates.
     */
    shotPTank = RpPTankAtomicCreate( maxNumShot*2,
                                       rpPTANKDFLAGMATRIX |
                                       rpPTANKDFLAGCOLOR |
                                       rpPTANKDFLAGCNSVTX2TEXCOORDS,
                                       0);

    if( NULL == shotPTank )
    {
        return FALSE;
    }

    /*
     * Create a frame and attach it to the ptank 
     */
    frame = RwFrameCreate();

    RwMatrixSetIdentity( RwFrameGetMatrix( frame ) );

    RpAtomicSetFrame( shotPTank,frame );

    /*
     * All the particles are using the same texture coordinate set
     */
    shotUVs[0].u = 0.0f;
    shotUVs[0].v = 0.0f;
    shotUVs[1].u = 1.0f;
    shotUVs[1].v = 1.0f;

    RpPTankAtomicSetConstantVtx2TexCoords( shotPTank, shotUVs );

    /*
     * Load the texture...
     */
    path = RsPathnameCreate(RWSTRING("textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    
    ShotPTankTexture = RwTextureRead(RWSTRING("particle"), RWSTRING("particle"));

    if( ShotPTankTexture )
    {
        RwTextureSetFilterMode(ShotPTankTexture, rwFILTERLINEAR  );
    }

    /*
     * Set the texture
     */
    RpPTankAtomicSetTexture( shotPTank , ShotPTankTexture );

    /*
     * the PTank is using Vertex Alpha Blending
     */
    RpPTankAtomicSetVertexAlpha( shotPTank, TRUE );

    /*
     * Set a basic blending mode for the PTank
     */
    RpPTankAtomicSetBlendModes( shotPTank, rwBLENDSRCALPHA, rwBLENDONE );

    /*
     * Deactivate all the particle in the PTank 
     */
    RpPTankAtomicSetActiveParticlesCount(shotPTank,0);

    shotObjList = RwMalloc(maxNumShot*sizeof(shotObject),rwID_NAOBJECT);
    
    if( shotObjList == NULL )
    {
        return FALSE;
    }
    ActiveShot = 0;
    MaxActiveShot = maxNumShot;

    return TRUE;
}


/*
 *****************************************************************************
 */
void
ShotsDestroy(void)
{
    if( NULL != shotPTank )
    {
        RwFrame *frame = RpAtomicGetFrame(shotPTank);

        if (frame != NULL)
        {
            RpAtomicSetFrame(shotPTank, NULL);
            RwFrameDestroy(frame);
        }

        RpPTankAtomicDestroy( shotPTank );
    }

    if( NULL != ShotPTankTexture )
    {
        RwTextureDestroy(ShotPTankTexture);
        ShotPTankTexture = NULL;
    }

    if(  NULL != shotObjList )
    {
        RwFree(shotObjList);
        shotObjList = NULL;
    }

}


/*
 *****************************************************************************
 */
static void
ShotSetup(shotObject *shot, RwMatrix *orientation)
{
    RwV3d *pos;
    RwV3d *at;
    RwV3d *right;
    RwV3d *up;
    
    RwV3d *shotPos;
    RwV3d *shotAt;
    RwV3d *shotRight;
    RwV3d *shotUp;
    
    RwV3d tmp;
    RwV3d endShot;

    /*
     * Setup a laser beam matrix.
     * the laser beam will start at the matrix position.
     * and end at this position plus the matrix at vector multiplied by the 
     * length of the shot.
     */

    /*
     * Get the orientation matrix elements
     */
    pos = RwMatrixGetPos(orientation);
    at = RwMatrixGetAt(orientation);
    right = RwMatrixGetRight(orientation);
    up = RwMatrixGetUp(orientation);

    /*
     * Get the laser beam matrix elements
     */
    shotPos = RwMatrixGetPos(&shot->orientation);
    shotAt = RwMatrixGetAt(&shot->orientation);
    shotRight = RwMatrixGetRight(&shot->orientation);
    shotUp = RwMatrixGetUp(&shot->orientation);

    /*
     * Calculate end shot position.
     */
    RwV3dScale( &endShot, at, shot->length );
    RwV3dAdd( &endShot, &endShot, pos );

    /*
     * Shot particles position are in the middles of the beam
     */
    RwV3dAdd(&tmp,&endShot,pos);
    RwV3dScale(shotPos,&tmp,0.5f);

    /* shot right vector*/
    RwV3dScale(shotRight,right,shot->width);

    /* shot at vector  */
    RwV3dSub(shotAt,&endShot,pos);

    /* shot up vector */
    RwV3dScale(shotUp,up,shot->width);


    shot->time = 0.0f;
    shot->state = SO_STATE_SHOT;


}


/*
 *****************************************************************************
 */
extern shotObject *
ShotShot(RwMatrix *orientation, RwReal width,RwReal length)
{
    shotObject *shot = &shotObjList[ActiveShot];

    /*
     * Get a new shot from the shot object list 
     * and set it up according to it's orientation, width and length
     */
    ActiveShot++;
    if( ActiveShot == MaxActiveShot )
    {
        ActiveShot = 0;
    }
    
    shot->length = length;
    shot->width = width;

    ShotSetup(shot, orientation);
    
    return shot;
}


/*
 *****************************************************************************
 */
void 
ShotsUpdate(RwReal deltaT)
{
    RwInt32 i;
    RwInt32 activeShots;
    RpPTankLockStruct shotColorLock;
    RpPTankLockStruct shotMatrixLock;
    shotObject *shot;

    RwRGBA *color1Out;
    RwRGBA *color2Out;
    RwRGBA color;
    RwMatrix *mtx1Out;
    RwMatrix *mtx2Out;
    RwV3d *pos;
    RwV3d *right;
    RwV3d *up;
    RwV3d translate;

    
    /*
     * Lock the PTank atomic matrix and colors
     */
    RpPTankAtomicLock(shotPTank, &shotColorLock, rpPTANKLFLAGCOLOR, rpPTANKLOCKWRITE);
    RpPTankAtomicLock(shotPTank, &shotMatrixLock, rpPTANKLFLAGMATRIX, rpPTANKLOCKWRITE);

    activeShots = 0;
    for(i=0;i<MaxActiveShot;i++)
    {
        shot = &shotObjList[i];

        /*
         * Add the shot to the ptank, according to it's state
         */
        switch(shot->state)
        {
            case SO_STATE_SHOT:
                /*
                 * Update the shot time
                 */
                shot->time += deltaT;

                /*
                 * Kill it if needed
                 */
                if( shot->time > shot->decayTime )
                {
                    shot->state = SO_STATE_DEAD;
                    continue;
                }

                /*
                 * Calculate the shot color
                 */
                color = shot->initialColor;
                color.red   += (RwUInt8)(shot->deltaColor.red*shot->time);
                color.green += (RwUInt8)(shot->deltaColor.green*shot->time);
                color.blue  += (RwUInt8)(shot->deltaColor.blue*shot->time);
                color.alpha += (RwUInt8)(shot->deltaColor.alpha*shot->time);
            
                /*
                 * Get the first particle's data
                 */
                color1Out = (RwRGBA *)shotColorLock.data;
                mtx1Out = (RwMatrix*)shotMatrixLock.data;
                shotColorLock.data += shotColorLock.stride;
                shotMatrixLock.data += shotMatrixLock.stride;

                /*
                 * Get the second particle's data
                 */
                color2Out = (RwRGBA *)shotColorLock.data;
                mtx2Out = (RwMatrix*)shotMatrixLock.data;
                shotColorLock.data += shotColorLock.stride;
                shotMatrixLock.data += shotMatrixLock.stride;
                

                /*
                 * Both particles use the same color
                 */
                *color1Out = *color2Out = color;

                /*
                 * Calculate the shot translation 
                 */
                RwV3dScale(&translate,RwMatrixGetAt(&shot->orientation),shot->time*shot->speed);

                /*
                 * Get the first particles matrix elements
                 */
                pos = RwMatrixGetPos(mtx1Out);
                right = RwMatrixGetRight(mtx1Out);
                up = RwMatrixGetUp(mtx1Out);
                
                /*
                 * Set the first particle position, length and width
                 */
                RwV3dAdd(pos,RwMatrixGetPos(&shot->orientation),&translate);
                RwV3dAssign(up,RwMatrixGetAt(&shot->orientation));
                RwV3dAssign(right,RwMatrixGetRight(&shot->orientation));

                /*
                 * Get the second particles matrix elements
                 */
                pos = RwMatrixGetPos(mtx2Out);
                right = RwMatrixGetRight(mtx2Out);
                up = RwMatrixGetUp(mtx2Out);

                /*
                 * Set the second particle position, length and width
                 */
                RwV3dAdd(pos,RwMatrixGetPos(&shot->orientation),&translate);
                RwV3dAssign(up,RwMatrixGetAt(&shot->orientation));
                RwV3dAssign(right,RwMatrixGetUp(&shot->orientation));

                activeShots++;
                break;
            case SO_STATE_DEAD:
            default:
            break;
        }
        
        
    }

    /*
     * Unlock the ptank 
     */
    RpPTankAtomicUnlock(shotPTank);

    /*
     * Set the number of active particles in the Ptank.
     */
    RpPTankAtomicSetActiveParticlesCount(shotPTank,activeShots*2);
}


/*
 *****************************************************************************
 */
void
ShotsRender(void)
{
    /*
     * Early out if there is no shots to render
     */
    if( 0 == RpPTankAtomicGetActiveParticlesCount(shotPTank)) return;

    /* 
     * No backface culling, 
     * No Z Buffer write
     */
    RwRenderStateSet( rwRENDERSTATECULLMODE, ( void* )rwCULLMODECULLNONE );
    RwRenderStateSet( rwRENDERSTATEZWRITEENABLE, (void *) FALSE );

    /* 
     * Render the Ptank atomic
     */
    RpAtomicRender( shotPTank );

    
    /*
     * Reset back face culling and zwrite
     */
    RwRenderStateSet( rwRENDERSTATECULLMODE, ( void* )rwCULLMODECULLBACK );
    RwRenderStateSet( rwRENDERSTATEZWRITEENABLE, (void *) TRUE );
}
