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
 * 
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


#ifndef DECAL_H
#define DECAL_H

typedef struct shotResult shotResult;

struct shotResult
{
    RwV3d posP1;
    RwV3d rightP1;
    RwV3d upP1;

    RwV3d posP2;
    RwV3d rightP2;
    RwV3d upP2;
     
};

#define SO_STATE_DEAD       (0)
#define SO_STATE_SHOT       (1)

typedef struct shotObject shotObject;

struct shotObject
{
    RwReal time;
    RwUInt32 state;
    
    RwMatrix orientation;
    
    RwReal width;
    RwReal length;
    RwReal speed;

    RwRGBA initialColor;
    RwRGBAReal deltaColor;

    RwReal decayTime;
};

#if defined(__cplusplus)
extern "C"
{
#endif /* defined(__cplusplus) */

extern RwBool
ShotsCreate(RwInt32 maxNumShot);

extern void
ShotsDestroy(void);

extern void 
ShotsUpdate(RwReal deltaT);

extern shotObject *
ShotShot(RwMatrix *orientation,RwReal width,RwReal length);

extern void 
ShotsRender(void);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* DECAL_H */
