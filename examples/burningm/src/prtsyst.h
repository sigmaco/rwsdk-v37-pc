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
 * prtsyst.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example of a particle system, using a ptank for the rendering, 
 *          using a skinned character as an emitter.
 *
*****************************************************************************/
#ifndef PRTSYST_H
#define PRTSYST_H

#include "rwcore.h"


/*
 *****************************************************************************
 */
typedef struct prtObj prtObj;
struct prtObj
{
    RwReal age;       /* age      */		
    RwV3d  spd;       /* speed    */	
    RwBool state;     /* active   */  
};


/*
 *****************************************************************************
 */
typedef struct prtObjInit prtObjInit;
struct prtObjInit
{
	RwV3d  pos;       /* position */
    RwReal age;       /* age      */		
    RwV3d  spd;       /* speed    */	
    RwBool state;     /* active   */  
};


/*
 *****************************************************************************
 */
typedef struct prtSystObj prtSystObj;
struct prtSystObj
{	
    RwReal		friction;		/* media friction coefficient           */	
    RwV3d 		gravity;		/* gravitic force			            */ 	
    RwV3d		wind;			/* wind speed 				            */	  

    RwRGBAReal  color;		    /* particules main color	            */						

    RwReal      colorInitial;	/* Luminance Linear Interptrs           */  	        
    RwReal      colorDelta;	    /* Luminance Linear Interptrs           */  	        

	prtObj      *prtList;  		/* particle buffer 		                */
	prtObjInit  *prtListNext;   /* New particle buffer 		            */
    RwInt32		maxPrtNum;	    /* maximum particles number             */
    RwInt32		actPrtNum;      /* number of active particles           */           

	RpAtomic    *atomic;        /* emiter Geometry                      */ 
    RwReal		speed;      	/* emission speed 		                */
    RwReal		rndSpeed;      	/* random speed componant               */

    RwReal 		disp;      	 	/* displacement of particles position   */

    RwReal 		prtLife;      	/* particle life duration 	            */        
    RwInt32  	emitionRate;    /* emission timer 			            */
    RwReal 		systemTime;     /* internal timer 			            */
    RwReal      frameRate;      /* particle frame rate                  */
    RwReal      frameLocker;    /* internal frame rate counter          */

    RwUInt32    seed;
    RwUInt32    currentVtx;

    RwBool		active;			/* Status flag			                */ 
    
    RpAtomic    *ptank;         /* Ptank pointer                        */

};


#ifdef    __cplusplus
extern              "C"
{
#endif /* __cplusplus */

extern prtSystObj *
PrtSystemCreate(RwInt32 numPrt, 
                RwReal life, 
                RwReal frameRate, 
                RpAtomic *atomic, 
                RwTexture *texture);

void
PrtSystemSetParticleFadeRange(prtSystObj *syst, RwReal initf, RwReal finalf);

void 
PrtSystemUpdate(prtSystObj *syst,RwReal deltaT);


extern void 
PrtSystemRender(prtSystObj *syst);

extern void
PrtSystemDestroy(prtSystObj *syst);

extern void
EmitNewWin(prtObjInit *dest, prtSystObj *syst, RwInt32 newPrt);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* PRTSYST_H */
