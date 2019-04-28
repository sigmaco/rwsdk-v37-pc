
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
 * Copyright (c) 2000 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * physics.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: Example to demonstrate how user-plugins can be used to extend
 *          RenderWare objects.
 *                         
 ****************************************************************************/

#ifndef PHYSICS_H
#define PHYSICS_H

#include "rwcore.h"
#include "rpworld.h"

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RwReal 
RpClumpPhysicsGetGravity(void);

extern RwReal 
RpClumpPhysicsGetMinSpeed(void);

extern RwChar *
RpClumpPhysicsGetCaption(void);

extern RwInt32 
RpClumpPhysicsPluginAttach(void);

extern RwBool
RpClumpPhysicsIncSpeed(RpClump *clump, const RwReal speed);

extern RwBool
RpClumpPhysicsSetSpeed(RpClump *clump, const RwReal speed);

extern RwReal
RpClumpPhysicsGetSpeed(RpClump *clump);

extern RwReal
RpClumpPhysicsGetBounciness(RpClump *clump);

extern RwBool
RpClumpPhysicsSetBounciness(RpClump *clump, const RwReal bounce);

extern RwBool
RpClumpPhysicsGetActive(RpClump *clump);

extern RwBool
RpClumpPhysicsSetActive(RpClump *clump, const RwBool update);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif /* PHYSICS_H */
