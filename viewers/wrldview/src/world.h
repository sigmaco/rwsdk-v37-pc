
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
 * world.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 *                                                                         
 * Purpose: RenderWare3 BSP viewer.
 *                         
 ****************************************************************************/

#ifndef WORLD_H
#define WORLD_H

#include "rwcore.h"
#include "rpworld.h"

extern RpWorld *World;
extern RpWorldSector *CurrentWorldSector;
extern RwInt32 CurrentWorldSectorIndex;

extern RwSphere WorldSphere;

extern RwBool WorldLoaded;
extern RwBool WorldHasNormals;
extern RwBool WorldHasSpline;
extern RwBool WorldHasTextures;
extern RwBool WorldHasLightmaps;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */ 

extern void WorldGetBoundingSphere(RpWorld *world, RwSphere *sphere);

extern RwBool LoadWorld(RwChar *file);
extern RwBool SaveWorld(void);

extern RwBool SaveTextureDictionary(void);

extern void SelectNextWorldSector(void);
extern void SelectPreviousWorldSector(void);

extern RpWorldSector *WorldGetFirstWorldSector(RpWorld *world);

#ifdef __cplusplus
}
#endif  /* __cplusplus */    

#endif  /* WORLD_H */
