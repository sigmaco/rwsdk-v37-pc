
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
 * clmpatch.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Handling patch based geometry
 *
 ****************************************************************************/

#ifndef CLMPATCH_H
#define CLMPATCH_H

#include "rwcore.h"
#include "rpworld.h"

extern RwBool ClumpHasPatch;

extern RwInt32 AtomicPatchLOD;

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern RwBool PatchClumpInitialize(RpClump *clump);
extern void PatchDestroy(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* CLMPATCH_H */

