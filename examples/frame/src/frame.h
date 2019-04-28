
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
 * frame.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the frame hierarchy of a series of atomics 
 * in a clump.
 *****************************************************************************/

#ifndef FRAME_H
#define FRAME_H

#include "rwcore.h"
#include "rpworld.h"

typedef struct NextAndPrevious
{
    RpAtomic *previous;
    RpAtomic *next;
} 
NextAndPrevious;

extern NextAndPrevious NextAndPreviousAtomic[ATOMICNUM];
extern RpAtomic *SelectedAtomic;


#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern void RenderFrameHierarchy(void);
extern void FrameRotate(RwReal xAngle, RwReal yAngle);
extern void LinkFrameHierarchy(void);

extern RwUInt32 GetAtomicIndex(RpAtomic *atomic);
extern RwObject* GetFirstAtomic(RwObject *object, void *data);

extern RwBool ResetFrameHierarchyCallback(RwBool testEnable);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* FRAME_H */
