
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
 * clmppick.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by: 
 *                                                                         
 * Purpose: Atomic picking tool box.
 *
 ****************************************************************************/

#ifndef CLMPPICK_H
#define CLMPPICK_H

#include "rwcore.h"
#include "rpworld.h"

extern RpAtomic *AtomicSelected;

extern RwInt32 currentAtomicNumber;
extern RwBBox CurrentAtomicBBox;

extern RwInt32 AtomicTotalTriangles;
extern RwInt32 AtomicTotalVertices;
extern RwInt32 AtomicTotalMorphTargets;

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern void AtomicGetBoundingBox(RpAtomic *atomic, RwBBox *bbox);
extern void UpdateSelectedStats(void);
extern RwBool SelectNextAtomic(void);
extern RwBool AtomicSelect(RwInt32 screenX, RwInt32 screenY);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* CLMPPICK_H */

