
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
 * Purpose: To illustrate how two H-anim sequences can be run together,
 *          with the second animation being a delta of the first.
 * *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rphanim.h"
#include "rpskin.h"
#include "skeleton.h"
#include "menu.h"

#include "hanim.h"

#define MAXNUMBONES         64

typedef struct _BoneMod
{
    RwReal  length;
    RwReal  width;
    RwV3d   angles;

} BoneMod;

static RpClump             *Clump = NULL;
static RpHAnimHierarchy    *Hierarchy = NULL;
static RtAnimAnimation    *Anim = NULL;
static RwUInt32             TotalNumBones = 0;

/*
 *  User scaling and rotation of all bones.
 */
static BoneMod              BoneMods[MAXNUMBONES];
static BoneMod              NullBoneMod = { 1.0f, 1.0f, {0.0f, 0.0f, 0.0f} };

/*
 *  Menu controls.
 */
static RwReal               AnimSpeed = 1.0f;
static RwReal               BaseScale = 1.0f;
static RwInt32              CurrentBone = 3;
static BoneMod              CurrentBoneMod = { 1.0f, 0.6f, {0.0f, 0.0f, 0.0f} };

/*
 *  Immediate mode buffer for drawing current bone.
 */
static RwUInt32             NumIm3DVerts = 0;
static RwIm3DVertex         Im3DVerts[MAXNUMBONES];
static RwImVertexIndex      Im3DIndices[MAXNUMBONES << 1];

static const RwV3d          Xaxis = {1.0f, 0.0f, 0.0f};
static const RwV3d          Yaxis = {0.0f, 1.0f, 0.0f};
static const RwV3d          Zaxis = {0.0f, 0.0f, 1.0f};

/*
 *****************************************************************************
 */
static RpClump *
LoadClump(RwChar *filename)
{
    RwChar      *path;
    RpClump     *clump = NULL;

    path = RsPathnameCreate(filename);
    if (path)
    {
        RwStream    *stream;

        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
        RsPathnameDestroy(path);
        if( stream )
        {
            if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
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
static RwFrame *
GetChildFrameHierarchy(RwFrame *frame, void *data)
{    
    RpHAnimHierarchy *hierarchy = *(RpHAnimHierarchy **)data;

    /*
     * Return the first hierarchy found that is attached to one of the atomic
     * frames...
     */
    hierarchy = RpHAnimFrameGetHierarchy(frame);
    if( hierarchy == NULL )
    {
        RwFrameForAllChildren(frame, GetChildFrameHierarchy, data);

        return frame;
    }

    *(void **)data = (void *)hierarchy;

    return NULL;
}


static RpHAnimHierarchy *
GetHierarchy(RpClump *clump)
{
    RpHAnimHierarchy *hierarchy = NULL;
    
    /*
     * Return the hierarchy for this model...
     */
    RwFrameForAllChildren(RpClumpGetFrame(clump), GetChildFrameHierarchy,
        (void *)&hierarchy);

    return hierarchy;
}


/*
 *****************************************************************************
 */
static RpAtomic *
SetHierarchyForSkinAtomic(RpAtomic *atomic, void *data)
{
    RpSkinAtomicSetHAnimHierarchy(atomic, (RpHAnimHierarchy *)data);

    return atomic;
}

/*
 *****************************************************************************
 */
static RtAnimAnimation *
LoadAnimFile(const RwChar *file)
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
 ******************************************************************************
 */
static void
RenderCurrentBoneInit(void)
{
    RwUInt32        i, j;

    /*
     *  Set up indexed white lines joining first vertex to all others.
     */
    for (i=0, j=0; i<MAXNUMBONES; i++)
    {
        Im3DIndices[j++] = 0;
        Im3DIndices[j++] = (RwImVertexIndex) i + 1;
        RwIm3DVertexSetRGBA(&Im3DVerts[i], 255, 255, 255, 255);
    }

    return;
}

/*
 ******************************************************************************
 */
static void
RenderCurrentBone(void)
{
    RwMatrix    *matrices = RpHAnimHierarchyGetMatrixArray(Hierarchy);
    RwV3d       *rootPos;

    /*
     *  Render the currently selected bone as white lines to child bones.
     *  First set the vertex for the origin of the current bone.
     */
    rootPos = RwMatrixGetPos(&matrices[CurrentBone]);
    RwIm3DVertexSetPos(&Im3DVerts[0], rootPos->x, rootPos->y, rootPos->z);
    NumIm3DVerts = 1;

    /* 
     *  Does the current bone have any children?
     */
    if (Hierarchy->pNodeInfo[CurrentBone].flags & rpHANIMPOPPARENTMATRIX)
    {
        RwV3d    pos;

        /*
         *  No children. We just draw a line along the bone's x axis.
         */
        pos = *rootPos;
        RwV3dIncrementScaled(
            &pos, RwMatrixGetRight(&matrices[CurrentBone]), 3.0f);
        RwIm3DVertexSetPos(&Im3DVerts[NumIm3DVerts], pos.x, pos.y, pos.z);
        NumIm3DVerts++;
    }
    else
    {
        RwInt32     parentsOnStack = 0;
        RwUInt32    currentIndex = CurrentBone;

        /*
         *  Pick up all children of the current bone and draw a line
         *  to each of them.
         */
        while (parentsOnStack > -1)
        {
            RwUInt32    flags;

            currentIndex++;
            flags = Hierarchy->pNodeInfo[currentIndex].flags;

            /*
             *  Is this bone an immediate child of the current bone?
             */
            if (parentsOnStack == 0)
            {
                RwV3d   *pos = RwMatrixGetPos(matrices + currentIndex);

                RwIm3DVertexSetPos(&Im3DVerts[NumIm3DVerts], 
                    pos->x, pos->y, pos->z);
                NumIm3DVerts++;

                if (!(flags & rpHANIMPUSHPARENTMATRIX))
                {
                    /* 
                     *  This was the last sibling, so quit.
                     */
                    break;
                }
            }

            if (flags & rpHANIMPUSHPARENTMATRIX)
            {
                parentsOnStack++;
            }

            if (flags & rpHANIMPOPPARENTMATRIX)
            {
                parentsOnStack--;
            }
        }
    }

    /*
     *  Now render the lines.
     */
    if (RwIm3DTransform(Im3DVerts, NumIm3DVerts, NULL, 
            rwIM3D_ALLOPAQUE | rwIM3D_VERTEXXYZ))
    {
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) FALSE);
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, Im3DIndices, 
            (NumIm3DVerts - 1) << 1);
        RwIm3DEnd();
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) TRUE);
    }

    return;
}

/*
 ******************************************************************************
 */
static RwBool
CurrentBoneChangeCB(RwBool justCheck)
{
    if (!justCheck)
    {
        CurrentBoneMod = BoneMods[CurrentBone];
    }

    return TRUE;
}

static RwBool
BoneModChangeCB(RwBool justCheck)
{
    if (!justCheck)
    {
        BoneMods[CurrentBone] = CurrentBoneMod;
    }

    return TRUE;
}

static RwBool
BaseScaleChangeCB(RwBool justCheck)
{
    static RwReal   lastScale = 1.0f;

    if (!justCheck)
    {
        RwV3d   scl;

        scl.x = scl.y = scl.z = BaseScale / lastScale;
        lastScale = BaseScale;

        /*
         *  Scaling the root frame affects the entire model.
         */
        RwFrameScale(RpClumpGetFrame(Clump), &scl, rwCOMBINEPRECONCAT);
    }

    return TRUE;
}

static RwBool
BoneAllResetCB(RwBool justCheck)
{
    if (!justCheck)
    {
        RwUInt32    i;

        /*
         *  Set unit scaling on all bones.
         */
        for (i=0; i<TotalNumBones; i++)
        {
            BoneMods[i] = NullBoneMod;
        }

        CurrentBoneMod = BoneMods[CurrentBone];
    }

    return TRUE;
}

/*
 ******************************************************************************
 */
void
HAnimMenuOpen(void)
{
    static RwChar animLabel[]   = RWSTRING("Animation speed");
    static RwChar scaleLabel[]  = RWSTRING("Base scale");
    static RwChar resetLabel[]  = RWSTRING("Reset all bones_R");

    static RwChar boneLabel[]   = RWSTRING("Current bone index");
    static RwChar lengthLabel[] = RWSTRING("Scale length");
    static RwChar widthLabel[]  = RWSTRING("Scale width");
    static RwChar rotxLabel[]   = RWSTRING("Rotate x");
    static RwChar rotyLabel[]   = RWSTRING("Rotate y");
    static RwChar rotzLabel[]   = RWSTRING("Rotate z");

    static RwBool reset;

    MenuAddSeparator();

    MenuAddEntryReal(animLabel,
        &AnimSpeed, NULL, 0.01f, 10.0f, 0.1f);

    MenuAddEntryReal(scaleLabel,
        &BaseScale, BaseScaleChangeCB, 0.1f, 10.0f, 0.1f);

    MenuAddEntryBoolTransient(resetLabel, &reset, BoneAllResetCB);

    MenuAddSeparator();

    MenuAddEntryInt(boneLabel,
        &CurrentBone, CurrentBoneChangeCB, 0, TotalNumBones - 1, 1, NULL);

    MenuAddEntryReal(lengthLabel,
        &CurrentBoneMod.length, BoneModChangeCB, 0.1f, 10.0f, 0.1f);

    MenuAddEntryReal(widthLabel,
        &CurrentBoneMod.width, BoneModChangeCB, 0.1f, 10.0f, 0.1f);

    MenuAddEntryReal(rotxLabel,
        &CurrentBoneMod.angles.x, BoneModChangeCB, -180.0f, 180.0f, 1.0f);

    MenuAddEntryReal(rotyLabel,
        &CurrentBoneMod.angles.y, BoneModChangeCB, -180.0f, 180.0f, 1.0f);

    MenuAddEntryReal(rotzLabel,
        &CurrentBoneMod.angles.z, BoneModChangeCB, -180.0f, 180.0f, 1.0f);

    return;
}

/*
 ******************************************************************************
 */
RwBool
HAnimOpen(RpWorld *world)
{
    static RwChar _clump_dff[] = RWSTRING("models/clump.dff");
    RwV3d       yAxis = {0.0f, 1.0f, 0.0f};
    RwUInt32    i;

    /*
     *  Load the model.
     */
    Clump = LoadClump(_clump_dff);
    if( Clump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create clump."));

        return FALSE;
    }

    RwFrameRotate(RpClumpGetFrame(Clump), &yAxis, 180.0f, rwCOMBINEREPLACE);
    RpWorldAddClump(world, Clump);

    /*
     *  Set up hierarchy.
     */
    Hierarchy = GetHierarchy(Clump);
    RpClumpForAllAtomics(Clump, SetHierarchyForSkinAtomic, (void *)Hierarchy);

    /*
     *  Set up animation.
     */
    Anim = LoadAnimFile(RWSTRING("models/clump.anm"));
    if( Anim == NULL )
    {
        return FALSE;
    }

    RpHAnimHierarchySetCurrentAnim(Hierarchy, Anim);
    RpHAnimHierarchySetCurrentAnimTime(Hierarchy, 0.0f);

    /*
     *  Initialize modifiers on all bones.
     */
    TotalNumBones = Hierarchy->numNodes;

    for (i=0; i<TotalNumBones; i++)
    {
        BoneMods[i] = NullBoneMod;
    }

    BoneMods[CurrentBone] = CurrentBoneMod;

    /*
     *  Initialize immediate 3D rendering.
     */
    RenderCurrentBoneInit();

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
    RwUInt32    i;
    RwMatrix    *matrix;

    if (deltaTime <= 0.0f)
    {
        return;
    }

    /*
     *  First, the animation is updated, which internally sets up an
     *  array of interpolated keyframes in the RpHAnimHierarchy. For standard
     *  keyframes, this array contains a hierarchy of quaternions and 
     *  translations representing the current state of the model.
     */
    RpHAnimHierarchyAddAnimTime(Hierarchy, deltaTime*AnimSpeed);

    /*
     *  We now access the interpolated keyframes to modify the orientation
     *  of the bones. We operate directly on the quaternions, but they could 
     *  alternatively be converted to a matrix, modified, and converted back
     *  again if this were more convenient (see RtQuatUnitConvertToMatrix).
     */
    for (i=0; i<TotalNumBones; i++)
    {
        RpHAnimInterpFrame *iFrame = (RpHAnimInterpFrame *) 
            rtANIMGETINTERPFRAME(Hierarchy->currentAnim, i);

        RtQuatRotate(&iFrame->q, &Zaxis, BoneMods[i].angles.z, 
            rwCOMBINEPRECONCAT);
        RtQuatRotate(&iFrame->q, &Yaxis, BoneMods[i].angles.y, 
            rwCOMBINEPRECONCAT);
        RtQuatRotate(&iFrame->q, &Xaxis, BoneMods[i].angles.x, 
            rwCOMBINEPRECONCAT);
    }

    /*
     *  Bone scaling will be performed later once the interpolated keyframes
     *  have been converted to world space matrices. However, if a bone is
     *  scaled, then its children must be translated to maintain the correct
     *  linkage between bones. 
     *
     *  To do this, we walk down the keyframe hierarchy, adding a translation
     *  to those who's parent bone has been scaled.
     *
     *  It is assumed that in bone space, the x axis points along the bone
     *  and the y and z axes are perpendicular to it. This allows us to
     *  scale the width and length independently.
     */
    {
        RwUInt32            stack[MAXNUMBONES>>1];
        RwUInt32            nStack = 0;
        RwUInt32            parentIndex = 0;
        RwUInt32            currentIndex;

        for (currentIndex=1; currentIndex < TotalNumBones; currentIndex++)
        {
            RwUInt32            flags;
            RpHAnimInterpFrame *iFrame = (RpHAnimInterpFrame *) 
                rtANIMGETINTERPFRAME(Hierarchy->currentAnim, currentIndex);

            /*
             *  Modify interpolated keyframe translation.
             */
            iFrame->t.x *= BoneMods[parentIndex].length;
            iFrame->t.y *= BoneMods[parentIndex].width;
            iFrame->t.z *= BoneMods[parentIndex].width;

            /*
             *  Get the flags that give the structure of the hierarchy.
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
     *  Now expand the interpolated keyframe hierarchy to the array of 
     *  world space bone matrices. These are effectively LTMs for each bone.
     */
    RpHAnimHierarchyUpdateMatrices(Hierarchy);

    /*
     *  Scale the matrices to enlarge or reduce the bone size. By operating
     *  on the bone LTMs, we only affect a single bone for each scaling, and
     *  we can scale the length and width independently.
     */
    matrix = RpHAnimHierarchyGetMatrixArray(Hierarchy);

    for (i=0; i<TotalNumBones; i++)
    {
        RwV3d       scl;

        scl.x = BoneMods[i].length;
        scl.y = scl.z = BoneMods[i].width;
        RwMatrixScale(matrix, &scl, rwCOMBINEPRECONCAT);

        matrix++;
    }

    return;
}

/*
 *****************************************************************************
 */
void
HAnimRender()
{
    /*
     *  Render the clump.
     */
    RpClumpRender(Clump);

    /*
     *  Render a line for the bone currently being edited.
     */
    RenderCurrentBone();

    return;
}
