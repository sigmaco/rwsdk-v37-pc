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
 * clmpdmrf.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Matt Thorman
 * Reviewed by:
 *
 * Purpose: Handling delta morph animations
 *
 ****************************************************************************/

#ifndef CLMPDMRF_H
#define CLMPDMRF_H

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern RwBool   DMorphClumpInitialize(RpClump *clump, RwChar *clumpFileName);
extern void     DMorphDestroy(void);

extern RwBool   DMorphLoadDMA(RpClump *clump, RwChar *dmaPath);

extern void     DMorphClumpUpdate(RwReal delta);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* CLMPDMRF_H */
