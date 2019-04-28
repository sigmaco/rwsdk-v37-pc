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
 * alpha.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: An example to illustrate how to sort a collection of atomics 
 *          with alpha components, such that they are rendered in the 
 *          correct back-to-front order.
 *                         
 ****************************************************************************/

#ifndef ALPHA_H
#define ALPHA_H

#include "rwcore.h"
#include "rpworld.h"

extern RwChar *RenderOrderString;

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern void ClumpGetAlphaAtomics(RpClump *clump);

extern void ClumpSetAtomicRenderCallback(RpClump *clump, RwBool alphaSort);

extern void DestroyAlphaAtomicsList(void);

extern void RenderAlphaSortedAtomics(void);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */


#endif /* ALPHA_H */
