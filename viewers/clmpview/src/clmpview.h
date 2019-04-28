
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
 * clmpsview.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Clump view basics functions
 *
 ****************************************************************************/

#ifndef CLMPVIEW_H
#define CLMPVIEW_H

#include "rwcore.h"
#include "rpworld.h"
#include "rplodatm.h"

typedef struct
{
    RwInt32 totalAtomics;
    RwInt32 totalTriangles;
    RwInt32 totalVertices;
    RwInt32 totalRotationKeys;
    RwInt32 totalTranslationKeys;
    RwInt32 maxFramePerAtomic;
}
ClumpStatistics;

extern RwBool ClumpLoaded;
extern RwBool ClumpIsPreLit;

extern RpClump *Clump;

extern RpAtomic *LodRoot;
extern RwInt32 numLods;
extern RwBool lodChanged;

extern ClumpStatistics ClumpStats;
extern ClumpStatistics LODClumpStats[RPLODATOMICMAXLOD];

extern RwSphere ClumpSphere;

extern RwFrame *ManipFrame;

extern RwBool ClumpHasTextures;

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern void AtomicGetBoundingBox(RpAtomic *atomic, RwBBox *bbox);
    
extern RwBool ClumpViewInit(void);
extern void ClumpViewTerminate(void);

extern RwBool ClumpLoadDFF(RwChar *path);
extern RpAtomic *ClumpGetFirstAtomic(RpClump *clump);
extern void ClumpDeltaUpdate(RwReal delta);    
extern RwInt32 ClumpChooseLevelOfDetail(RpAtomic *lodRoot);
extern void ClumpDisplayOnScreenInfo(RwRaster *cameraRaster);
extern void ClumpReset(RpClump *clump);

extern RwBool SaveDFF(void);
extern RwBool SaveTextureDictionary(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* CLMPVIEW_H */

