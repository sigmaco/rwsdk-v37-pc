
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
 * dmorph.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the DMorph plugin
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rtworld.h"
#include "rphanim.h"

#include "skeleton.h"
#include "dmorph.h"
#include "rpdmorph.h"

static RpAtomic    *FaceBaseAtomic = NULL;
static RpAtomic    *SurfaceBaseAtomic = NULL;

static RpDMorphAnimation *FaceBaseAnimation = NULL;

/*
 *****************************************************************************
 */
static void
AttachSurfaceMorphTargets(void)
{
    /*
     * Add DMorph targets to the base surface geometry...
     */
    RwV3d              *verts;
    RwV3d              *norms;

    /* create space for 2 delta morph target */
    RpDMorphGeometryCreateDMorphTargets(SurfaceBaseGeom, 2);

    verts =
        RpMorphTargetGetVertices(RpGeometryGetMorphTarget
                                 (DeltaGeom[0], 0));
    norms =
        RpMorphTargetGetVertexNormals(RpGeometryGetMorphTarget
                                      (DeltaGeom[0], 0));

    if (!RpDMorphGeometryAddDMorphTarget(SurfaceBaseGeom, 0, verts,
                                         norms, NULL, NULL,
                                         rpGEOMETRYPOSITIONS |
                                         rpGEOMETRYNORMALS))
    {
        RsErrorMessage(RWSTRING("Failed to add target"));
    }
    else
    {
        RpGeometryDestroy(DeltaGeom[0]);
    }
    verts =
        RpMorphTargetGetVertices(RpGeometryGetMorphTarget
                                 (DeltaGeom[1], 0));
    norms =
        RpMorphTargetGetVertexNormals(RpGeometryGetMorphTarget
                                      (DeltaGeom[1], 0));

    if (!RpDMorphGeometryAddDMorphTarget(SurfaceBaseGeom, 1, verts,
                                         norms, NULL, NULL,
                                         rpGEOMETRYPOSITIONS |
                                         rpGEOMETRYNORMALS))
    {
        RsErrorMessage(RWSTRING("Failed to add target"));
    }
    else
    {
        RpGeometryDestroy(DeltaGeom[1]);
    }
}

/*
 *****************************************************************************
 */
static RpAtomic *
GetFirstAtomic(RpAtomic * atomic, void *data)
{
    /*
     * Find the first atomic...
     */

    *(RpAtomic **) data = atomic;

    return NULL;
}

/*
 *****************************************************************************
 */
RwBool
CreateMorph(void)
{
    RwChar             *path;

    /*
     * Since our models only have one atomic, let's make things simpler...
     */
    RpClumpForAllAtomics(FaceBaseClump, GetFirstAtomic,
                         &FaceBaseAtomic);
    RpClumpForAllAtomics(SurfaceBaseClump, GetFirstAtomic,
                         &SurfaceBaseAtomic);

    /* Overload atomic so that it can be DMorphed */
    RpDMorphAtomicInitialize(SurfaceBaseAtomic);

    /*
     * Load DMorphing animation file...
     */
    path = RsPathnameCreate(RWSTRING("models/face.dma"));
    FaceBaseAnimation = RpDMorphAnimationRead(path);
    RsPathnameDestroy(path);

    if (FaceBaseAnimation == NULL)
    {
        return FALSE;
    }

    RpDMorphAtomicSetAnimation(FaceBaseAtomic, FaceBaseAnimation);

    /*
     * Set up the surface morph geometry...
     */
    AttachSurfaceMorphTargets();

    return TRUE;
}

/*
 *****************************************************************************
 */
void
ChangeSurfaceContributions(void)
{
    /*
     * Alters the surface contributions... 
     */
    RwInt32             max =
        RpDMorphGeometryGetNumDMorphTargets(RpAtomicGetGeometry
                                            (SurfaceBaseAtomic));
    RwReal             *dlist;
    RwInt32             i;

    dlist = RpDMorphAtomicGetDMorphValues(SurfaceBaseAtomic);

    for (i = 0; i < max; i++)
    {
        *dlist++ = contribution[i];
    }
}

/*
 *****************************************************************************
 */
void
ChangeAnimationDurations(void)
{
    /*
     * Change durations of the face animation...
     */
    RwUInt32            frames;
    RwUInt32            i, tgts;

    for (tgts = 0; tgts <= 1; tgts++)
    {
        frames = RpDMorphAnimationGetNumFrames(FaceBaseAnimation, tgts);
        for (i = 0; i < frames; i++)
        {
            RpDMorphAnimationFrameSetDuration(FaceBaseAnimation, tgts,
                                              i, duration[tgts]);
        }
    }
}

/*
 *****************************************************************************
 */
RwBool
AdvanceAnimation(RwReal deltaTime)
{
    /*
     * Update the animation of the face...
     */
    RpDMorphAtomicAddTime(FaceBaseAtomic, deltaTime);

    return TRUE;
}

/*
 *****************************************************************************
 */
void
DestroyDMorph(void)
{
    /*
     * Destroy animations and morphs, etc...
     */
    if (FaceBaseAnimation)
    {
        RpDMorphAnimationDestroy(FaceBaseAnimation);
    }

    RpDMorphGeometryDestroyDMorphTargets(RpAtomicGetGeometry
                                         (SurfaceBaseAtomic));

    return;
}
