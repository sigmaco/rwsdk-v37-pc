
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
 * dmorph.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the DMorph plugin
 ****************************************************************************/

#ifndef DMORPH_H
#define DMORPH_H

#include "rwcore.h"
#include "rpworld.h"

extern RwInt32      CurrentAnimation;

#ifdef __cplusplus
extern              "C"
{
#endif                          /* __cplusplus */

extern RwReal       contribution[];
extern RwReal       duration[];
extern RpClump     *SurfaceBaseClump;
extern RpClump     *FaceBaseClump;
extern RpGeometry  *DeltaGeom[2];
extern RpGeometry  *SurfaceBaseGeom;

extern RwBool       CreateMorph(void);
extern void         ChangeSurfaceContributions(void);
extern void         ChangeAnimationDurations(void);
extern RwBool       AdvanceAnimation(RwReal deltaTime);
extern void         DestroyDMorph(void);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */

#endif                          /* DMORPH_H */
