
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
 * hanim.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the usage of custom RpHAnimAnimation 
 *          keyframe interpolation schemes.
 * *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rphanim.h"
#include "rpskin.h"

#define SkinGetSkinToBoneMatrix(target, skin, index)    \
MACRO_START                                             \
{                                                       \
    const RwMatrix * const SkinToBoneMatrices =         \
        RpSkinGetSkinToBoneMatrices(skin);              \
    *target = SkinToBoneMatrices[index];                \
}                                                       \
MACRO_STOP

#define SkinGetBoneToSkinMatrix(target, skin, index)    \
MACRO_START                                             \
{                                                       \
    RwMatrix SkinToBone;                                \
    SkinGetSkinToBoneMatrix(&SkinToBone, skin, index);  \
    RwMatrixInvert(target, &SkinToBone);                \
}                                                       \
MACRO_STOP

#include "skeleton.h"
#include "menu.h"

#include "keyframe.h"
#include "hanim.h"

#define MAXNUMBONES         64

typedef struct BoneTranslation BoneTranslation;
struct BoneTranslation
{
    RwUInt32            parent;
    RwV3d               offset;
};

static RpClump     *Clump = NULL;
static RpSkin      *Skin = NULL;
static RpHAnimHierarchy *Hierarchy = NULL;
static RtAnimAnimation *Anim = NULL;
static RtAnimAnimation *Anim2 = NULL;

static RwUInt32     TotalNumBones = 0;
static BoneTranslation BoneTranslations[MAXNUMBONES];
static RwBool       TranslateBones = FALSE;

static RwBool       Anim2Saved = FALSE;

static const RwV3d  Yaxis = { 0.0f, 1.0f, 0.0f };

/*
 *****************************************************************************
 */
static RpAtomic *
SkinAtomicSetHierarchy(RpAtomic * atomic, void *data)
{
    RpHAnimHierarchy * const HAnimHierarchy = (RpHAnimHierarchy *) data;

    /* Cache most recent skin */
    Skin = RpSkinAtomicGetSkin(atomic);

    RpSkinAtomicSetHAnimHierarchy(atomic, HAnimHierarchy);

    return atomic;
}

/*
 *****************************************************************************
 */
static RpClump *
LoadClump(RwChar * filename)
{
    RwChar             *path;
    RpClump            *clump = NULL;

    path = RsPathnameCreate(filename);
    if (path)
    {
        RwStream           *stream;

        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
        RsPathnameDestroy(path);
        if (stream)
        {
            if (RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL))
            {
                clump = RpClumpStreamRead(stream);
            }

            RwStreamClose(stream, NULL);
        }
    }

    return clump;
}

/*
 *****************************************************************************
 */
static RtAnimAnimation *
LoadAnimFile(const RwChar * file)
{
    RtAnimAnimation   *anim = NULL;
    RwChar             *path;

    path = RsPathnameCreate(file);
    if (path)
    {
        anim = RtAnimAnimationRead(path);
        RsPathnameDestroy(path);
    }

    return anim;
}

/*
 *****************************************************************************
 */
static RwBool
SaveAnimFile(RtAnimAnimation * anim, const RwChar * file)
{
    RwBool              result = FALSE;
    RwChar             *path;

    path = RsPathnameCreate(file);
    if (path)
    {
        result = RtAnimAnimationWrite(anim, path);
        RsPathnameDestroy(path);
    }

    return result;
}

/*
 *****************************************************************************
 */
static RwFrame *
GetChildFrameHierarchy(RwFrame * frame, void *data)
{
    RpHAnimHierarchy   * hierarchy = *(RpHAnimHierarchy **) data;

    /*
     * Return the first hierarchy found that is attached to one of the atomic
     * frames...
     */
    hierarchy = RpHAnimFrameGetHierarchy(frame);
    if (hierarchy == NULL)
    {
        RwFrameForAllChildren(frame, GetChildFrameHierarchy, data);

        return frame;
    }

    *(void **) data = (void *) hierarchy;

    return NULL;
}

static RpHAnimHierarchy *
GetHierarchy(RpClump * clump)
{
    RpHAnimHierarchy   *hierarchy = NULL;

    /*
     * Return the hierarchy for this model...
     */
    RwFrameForAllChildren(RpClumpGetFrame(clump),
                          GetChildFrameHierarchy, (void *) &hierarchy);

    return hierarchy;
}

/*
 ******************************************************************************
 */
static void
SkinGetBonePositions(RpSkin * skin)
{
    RwUInt32            stack[MAXNUMBONES >> 1];
    RwUInt32            nStack = 0;
    RwInt32             parentIndex = 0;
    RwInt32             currentIndex;
    RwInt32             numBones;

    /*
     *  Assume root bone is at origin
     */
    BoneTranslations[0].offset.x = 0.0f;
    BoneTranslations[0].offset.y = 0.0f;
    BoneTranslations[0].offset.z = 0.0f;
    BoneTranslations[0].parent = -1;

    /*
     *  Get offset for each bone from its parent.
     */

    numBones = RpSkinGetNumBones(skin);

    for (currentIndex = 1; currentIndex < numBones; currentIndex++)
    {
        RwUInt32            flags;
        RwMatrix            currentBoneToSkin;
        RwMatrix            parentSkinToBone;

        /*
         *  Invert bone-to-skin matrix to get currentBoneToSkin.pos,
         * which will be the position of this bone's origin in skin space.
         */


        SkinGetBoneToSkinMatrix(&currentBoneToSkin, skin, currentIndex);

        /*
         *  Transform to get the position of this bone in the local space
         *  of it's parent bone, which we store for later use.
         */

        SkinGetSkinToBoneMatrix(&parentSkinToBone, skin, parentIndex);

        RwV3dTransformPoint(&BoneTranslations[currentIndex].offset,
                            &currentBoneToSkin.pos, &parentSkinToBone);
        /*
         *  Store the index of the parent bone also.
         */
        BoneTranslations[currentIndex].parent = parentIndex;

        /*
         *  Handle hierarchy structure
         */
        flags = Hierarchy->pNodeInfo[currentIndex].flags;

        if (flags & rpHANIMPUSHPARENTMATRIX)
        {
            stack[++nStack] = parentIndex;
        }

        if (flags & rpHANIMPOPPARENTMATRIX)
        {
            parentIndex = stack[nStack--];
        }
        else
        {
            parentIndex = currentIndex;
        }
    }
}

/*
 ******************************************************************************
 */
static RwBool
LoadAnimCB(RwBool justCheck)
{
    if (justCheck)
    {
        return Anim2Saved;
    }

    if (Anim2Saved)
    {
        if (Anim2)
        {
            RtAnimAnimationDestroy(Anim2);
        }

        Anim2 = LoadAnimFile(RWSTRING("models/rotation.anm"));
        RpHAnimHierarchySetCurrentAnim(Hierarchy, Anim2);
    }

    return TRUE;
}

/*
 ******************************************************************************
 */
static RwBool
SaveAnimCB(RwBool justCheck)
{
    if (!justCheck)
    {
        SaveAnimFile(Anim2, RWSTRING("models/rotation.anm"));
        Anim2Saved = TRUE;
    }

    return TRUE;
}

/*
 ******************************************************************************
 */
void
HAnimMenuOpen(void)
{
    static RwChar       transLabel[] = RWSTRING("Translate bones_T");
    static RwChar       saveLabel[] = RWSTRING("Save animation_S");
    static RwChar       loadLabel[] = RWSTRING("Load animation_L");

    MenuAddSeparator();
    MenuAddEntryBool(transLabel, &TranslateBones, NULL);
    MenuAddEntryTrigger(saveLabel, SaveAnimCB);
    MenuAddEntryTrigger(loadLabel, LoadAnimCB);

    return;
}

/*
 ******************************************************************************
 */
RwBool
HAnimOpen(RpWorld * world)
{
    static RwChar       _clump_dff[] = RWSTRING("models/clump.dff");

    /*
     *  Load the model into the world.
     */
    Clump = LoadClump(_clump_dff);
    if (!Clump)
    {
        return FALSE;
    }

    RwFrameRotate(RpClumpGetFrame(Clump), &Yaxis, 180.0f,
                  rwCOMBINEREPLACE);
    RpWorldAddClump(world, Clump);

    /*
     *  Get the hierarchy.
     */
    Hierarchy = GetHierarchy(Clump);
    TotalNumBones = Hierarchy->numNodes;

    /*
     *  Setup all skins on the hierarchy. Each skin will have a complete
     *  array of bone-to-skin matrices for the base pose, and we use the
     *  last skin accessed to extract bone positions.
     */
    RpClumpForAllAtomics(Clump, SkinAtomicSetHierarchy,
                         (void *) Hierarchy);

    SkinGetBonePositions(Skin);

    /*
     *  Register our new animation type.
     *  Load an existing animation and convert it to the new type.
     *  Set the hierarchy to use the new animation.
     */
    if (!HAnimRotKeyFrameRegister())
    {
        return FALSE;
    }

    Anim = LoadAnimFile(RWSTRING("models/clump.anm"));
    if (!Anim)
    {
        return FALSE;
    }

    Anim2 = HAnimExtractRotAnimFromStdAnim(Anim);
    if (!Anim2)
    {
        return FALSE;
    }

    RpHAnimHierarchySetCurrentAnim(Hierarchy, Anim2);
    RpHAnimHierarchySetCurrentAnimTime(Hierarchy, 0.0f);

    return TRUE;
}

/*
 *****************************************************************************
 */
void
HAnimClose(void)
{
    if (Anim)
    {
        RtAnimAnimationDestroy(Anim);
        Anim = NULL;
    }

    if (Anim2)
    {
        RtAnimAnimationDestroy(Anim2);
        Anim2 = NULL;
    }

    if (Clump)
    {
        RpWorldRemoveClump(RpClumpGetWorld(Clump), Clump);
        RpClumpDestroy(Clump);
        Clump = NULL;
        Hierarchy = NULL;
    }
}

/*
 *****************************************************************************
 */
void
HAnimUpdate(RwReal deltaTime)
{
    if (deltaTime > 0.0f)
    {
        /*
         *  Update the animation. This uses our custom animation type
         *  which animates rotations only.
         */
        RpHAnimHierarchyAddAnimTime(Hierarchy, deltaTime);
        RpHAnimHierarchyUpdateMatrices(Hierarchy);

        /*
         *  Now add translations to bones by transforming with the
         *  stored offset relative to each bone's parent.
         */
        if (TranslateBones)
        {
            RwUInt32            i;
            RwMatrix           * const matrices =
                RpHAnimHierarchyGetMatrixArray(Hierarchy);

            for (i = 1; i < TotalNumBones; i++)
            {
                RwV3dTransformPoint(&matrices[i].pos,
                                    &BoneTranslations[i].offset,
                                    &matrices[BoneTranslations[i].parent]);
            }
        }
    }

    return;
}

/*
 *****************************************************************************
 */
void
HAnimRender(void)
{
    RpClumpRender(Clump);

    return;
}
