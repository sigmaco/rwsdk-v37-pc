
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
 * clmpsview.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Clump view basics functions
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rpskin.h"
#include "rpmatfx.h"

#include "rtcharse.h"
#include "rtfsyst.h"

#include "skeleton.h"
#include "menu.h"

#include "clmprndr.h"
#include "clmpmorf.h"
#include "clmpdmrf.h"
#include "clmpskin.h"
#include "clmphanm.h"
#include "clmppick.h"
#include "clmpcntl.h"
#include "clmpatch.h"

#include "main.h"
#include "scene.h"

#include "clmpview.h"

#include "rppatch.h"

#define ANIMSAMPLESPACING (0.1f)
#define MAXANIMSAMPLES (1000)
#define CLUMPTIMEBETWEENSCREENENDS (10.0f)

static RwV3d Zero = {0.0f, 0.0f, 0.0f};

static RwChar ClumpPathFile[256];

static RwChar TexDictionaryName[256];

#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))
static RwChar EffectDictionaryName[256];
#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */

RpAtomic *LodRoot;
RwInt32 numLods;

RpClump *Clump = (RpClump *)NULL;
RwSphere ClumpSphere;

RwBool ClumpLoaded = FALSE;
RwBool ClumpIsPreLit = FALSE;

ClumpStatistics ClumpStats;
ClumpStatistics LODClumpStats[RPLODATOMICMAXLOD];

RwBool lodChanged = FALSE;

RwFrame *ManipFrame;

RwBool ClumpHasTextures = FALSE;



/*
 *****************************************************************************
 */
static RpAtomic *
AtomicAddBSphereCentre(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RwV3d center;
        RwMatrix *LTM;
        RpMorphTarget *morphTarget;
        RwInt32 i, numMorphTargets;
        RwV3d atomicCentre;
        RwSphere *clumpSphere;

        clumpSphere = (RwSphere *)data;

        /*
         * Establish the average centre of this atomic over all morph targets
         */
        atomicCentre = Zero;

        numMorphTargets = RpGeometryGetNumMorphTargets (geometry);

        for( i = 0; i < numMorphTargets; i++ )
        {
            morphTarget = RpGeometryGetMorphTarget(geometry, i);
            center = RpMorphTargetGetBoundingSphere(morphTarget)->center;
            RwV3dAdd(&atomicCentre, &atomicCentre, &center);
        }

        RwV3dScale(&atomicCentre, &atomicCentre, 1.0f / numMorphTargets);

        /*
         * Tranform the average centre of the atomic to world space
         */
        LTM = RwFrameGetLTM(RpAtomicGetFrame(atomic));
        RwV3dTransformPoints(&atomicCentre, &atomicCentre, 1, LTM);

        /*
         * Add the average centre of the atomic up in order to calculate the centre of the clump
         */
        RwV3dAdd(&clumpSphere->center, &clumpSphere->center, &atomicCentre);
    }

    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicCompareBSphere(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        RwSphere *sphere, morphTargetSphere;
        RwV3d tempVec;
        RpMorphTarget *morphTarget;
        RwReal dist;
        RwMatrix *LTM;
        RwInt32 i, numMorphTargets;

        sphere = (RwSphere *)data;

        LTM = RwFrameGetLTM(RpAtomicGetFrame(atomic));

        numMorphTargets = RpGeometryGetNumMorphTargets(geometry);

        for( i = 0; i < numMorphTargets; i++ )
        {
            morphTarget = RpGeometryGetMorphTarget(geometry, i);
            morphTargetSphere = *RpMorphTargetGetBoundingSphere(morphTarget);

            RwV3dTransformPoints(&morphTargetSphere.center,
                &morphTargetSphere.center, 1, LTM);

            RwV3dSub(&tempVec, &morphTargetSphere.center, &sphere->center);

            dist = RwV3dLength(&tempVec) + morphTargetSphere.radius;
            if( dist > sphere->radius )
            {
                sphere->radius = dist;
            }
        }
    }

    return atomic;
}


/*
 *****************************************************************************
 */
static void
ClumpGetBoundingSphere(RpClump *clump, RwSphere *clumpSphere)
{
    RwInt32 numAnimSamples;

    /*
     * First find the mean of all the atomics' bounding sphere centers.
     * All morph targets of all atomics and all frame animations are taken into account.
     * The result is the clump's bounding sphere center...
     */
    clumpSphere->center = Zero;
    numAnimSamples = 0;

    {
        RwSphere curClumpSphere;

        /*
         * average over morph targets and atomics
         */
        curClumpSphere.center = Zero;
        RpClumpForAllAtomics(clump, AtomicAddBSphereCentre, &curClumpSphere);

        RwV3dScale(&curClumpSphere.center, 
            &curClumpSphere.center, 1.0f / RpClumpGetNumAtomics (clump));

        /*
         * Sum up the above average in order to calculate the overall
         * average over the frame animation...
         */
        RwV3dAdd(&clumpSphere->center,
            &clumpSphere->center, &curClumpSphere.center);

        numAnimSamples++;

    }

    RwV3dScale(&clumpSphere->center, &clumpSphere->center, 1.0f / numAnimSamples);

    /*
     * Now, given the clump's bounding sphere center, determine the radius
     * by finding the greatest distance from the center that encloses all
     * the atomics' bounding spheres.  All morph targets, atomics and animations
     * are taken into account
     */
    clumpSphere->radius = -RwRealMAXVAL;
    numAnimSamples = 0;

    RpClumpForAllAtomics(clump, AtomicCompareBSphere, clumpSphere);

    numAnimSamples++;

    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicTestGeometry(RpAtomic *atomic, 
                   void * data __RWUNUSED__)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);

    if( !geometry )
    {
        RsWarningMessage(RWSTRING("Some atomics have no geometry."));

        return (RpAtomic *)NULL;
    }

    if( !(RpGeometryGetFlags (geometry) & rpGEOMETRYNORMALS) )
    {
        RsWarningMessage(RWSTRING("Some atomics have no vertex normals."));
        return (RpAtomic *)NULL;
    }

    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicGetStatistics(RpAtomic *atomic, void *data)
{
    RpGeometry *geometry;
    RwFrame *frame;
    RwInt32 frameNum;

    geometry = RpAtomicGetGeometry(atomic);

    if( geometry )
    {
        ClumpStatistics *clumpStats;

        clumpStats = (ClumpStatistics *) data;

        clumpStats->totalAtomics++;

        clumpStats->totalTriangles += RpGeometryGetNumTriangles(geometry);
        clumpStats->totalVertices += RpGeometryGetNumVertices(geometry);

        frame = RwFrameGetRoot(RpAtomicGetFrame(atomic));
        frameNum = RwFrameCount(frame);
        if( frameNum > clumpStats->maxFramePerAtomic )
        {
            clumpStats->maxFramePerAtomic = frameNum;
        }
        
        ClumpIsPreLit = ClumpIsPreLit || 
            ( (RpGeometryGetFlags (geometry) & rpGEOMETRYPRELIT) ? TRUE  : FALSE );
             
    }

    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicSetRenderFlags(RpAtomic *atomic, 
                     void * data __RWUNUSED__)
{
    RpAtomicSetFlags(atomic, rpATOMICRENDER | rpATOMICCOLLISIONTEST);

    return atomic;
}


/*
 *****************************************************************************
 */
void
ClumpReset(RpClump *clump)
{
    RwFrame *clumpFrame;
    clumpFrame = RpClumpGetFrame(clump);

    RwFrameTranslate(clumpFrame, &Zero, rwCOMBINEREPLACE);

    return;
}

/*
 *****************************************************************************
 */
static void
ClumpInitialize(RpClump *clump)
{
    RwFrame *clumpFrame;

    /*
     * Destroy any previously existing clump...
     */
    if( ClumpLoaded )
    {
        RpWorldRemoveClump(World, Clump);
        
        HAnimDestroy();

        SkinDestroy();

        DMorphDestroy();

        PatchDestroy();

        RpClumpDestroy(Clump);
    }

    /*
     * Turns the lights Off...
     */
    if( AmbientLightOn )
    {
        AmbientLightOn = FALSE;
        RpWorldRemoveLight(World, AmbientLight);
    }

    if( MainLightOn )
    {
        MainLightOn = FALSE;
        RpWorldRemoveLight(World, MainLight);
    }

    Clump = clump;

    /*
     * Move the clump to the world's origin...
     */
    clumpFrame = RpClumpGetFrame(Clump);
    RwFrameTranslate(clumpFrame, &Zero, rwCOMBINEREPLACE);

    RpWorldAddClump(World, Clump);

    SkinClumpInitialize(Clump, ClumpPathFile);

    HAnimClumpInitialize(Clump, ClumpPathFile);

    MorphClumpInitialize(Clump);

    DMorphClumpInitialize(Clump, ClumpPathFile);

    PatchClumpInitialize(Clump);

    if( !LodRoot )
    {
        RpClumpForAllAtomics(Clump, AtomicSetRenderFlags, NULL);
    }

    /*
     * Warn the user if some or all the atomics comprising the the clump
     * have no geometry or the geometry is without vertex normals...
     */
    RpClumpForAllAtomics (Clump, AtomicTestGeometry, NULL);

    /*
     * Gather some statistics about the new clump...
     */
    ClumpStats.totalAtomics = 0;
    ClumpStats.totalTriangles = 0;
    ClumpStats.totalVertices = 0;
    ClumpStats.maxFramePerAtomic = 0;
    
    ClumpIsPreLit = FALSE;

    RpClumpForAllAtomics (Clump, AtomicGetStatistics, &ClumpStats);

    /*
     * Turns the lights back on if necessary...
     */
    if( ClumpIsPreLit )   
    {
        AmbientLightOn = FALSE; 
        MainLightOn = FALSE;
    }
    else
    {
        AmbientLightOn = TRUE; 
        RpWorldAddLight(World, AmbientLight);
    }
    
    AmbientIntensity = 0.75f;
    MainIntensity = 1.0f;

    RenderMode = RENDERSOLID;
    ClumpLoaded = TRUE;
    AtomicSelected = (RpAtomic *)NULL;

    /*
     * Determine the clump's overall bounding sphere
     * (for all morph targets and for all animations)...
     */
    ClumpGetBoundingSphere(Clump, &ClumpSphere);

    NormalsScaleFactor = 0.15f * ClumpSphere.radius;

    return;
}


/*
 *****************************************************************************
 */
static RpClump *
LoadDff(RwChar *path)
{
    RwStream *stream;
    RpClump *clump = (RpClump *)NULL;

    /*
     * Open a stream connected to the disk file...
     */
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    if( stream )
    {
        /*
         * Find a clump chunk in the stream...
         */
        if( !RwStreamFindChunk(stream, rwID_CLUMP, 
                               (RwUInt32 *)NULL, (RwUInt32 *)NULL) )
        {
            RwStreamClose(stream, NULL);
            return (RpClump *)NULL;
        }

        /*
         * Read the clump chunk...
         */
        clump = RpClumpStreamRead(stream);

        RwStreamClose(stream, NULL);
    }

    return clump;
}


/*
 *****************************************************************************
 */
RwInt32 
ClumpChooseLevelOfDetail(RpAtomic * lodRoot)
{
    RwInt32 result;
    RwV3d displacement, *atomicPos, *cameraPos, *cameraAt;
    RwReal dot, max, min;

    result = -1;

    if( lodRoot && Camera )
    {
        /*
         * Calculate how far the atomic is from the viewer.
         */
        atomicPos =
            RwMatrixGetPos(RwFrameGetLTM(RpAtomicGetFrame(lodRoot)));
        cameraPos =
            RwMatrixGetPos(RwFrameGetLTM(RwCameraGetFrame(Camera)));
        cameraAt =
            RwMatrixGetAt(RwFrameGetLTM(RwCameraGetFrame(Camera)));

        RwV3dSub(&displacement, atomicPos, cameraPos);

        dot = RwV3dDotProduct (&displacement, cameraAt);
        min = NearClip + ClumpSphere.radius * 2.0f;
        max = FarClip - ClumpSphere.radius;
        if( dot <= min || max <= min )
        {
            result = 0;
        }
        else if (dot >= max)
        {
            result = numLods;
        }
        else
        {
            result =
                (RwInt32) (sqrt ((dot - min) / (max - min)) *
                           (numLods + 1));
        }

        if (result != RpLODAtomicGetCurrentLOD (lodRoot))
        {
            lodChanged = TRUE;
        }
    }

    return result;
}


/*
 *****************************************************************************
 */
static RpAtomic *
CountAtomic(RpAtomic *atomic, void *count)
{
    (*(RwUInt32 *) count)++;
    return atomic;
}


/*
 *****************************************************************************
 */
static RwUInt32
ClumpCountAtomics(RpClump *clump)
{
    RwUInt32 count = 0;

    RpClumpForAllAtomics(clump, CountAtomic, (void *)&count);
    return count;
}


/*
 *****************************************************************************
 */
static RpAtomic *
GetFirstAtomic(RpAtomic *atomic, void *firstAtomic)
{
    *(RpAtomic **)firstAtomic = atomic;
    return (RpAtomic *)NULL;
}


/*
 *****************************************************************************
 */
RpAtomic *
ClumpGetFirstAtomic(RpClump * clump)
{
    RpAtomic *atomic = (RpAtomic *)NULL;

    RpClumpForAllAtomics(clump, GetFirstAtomic, (void *)&atomic);
    return atomic;
}


/*
 *****************************************************************************
 */
static RpClump *
LoadLodDffs(RwChar *lodPath, RwChar *lodPathEnding)
{
    RpClump *clump[RPLODATOMICMAXLOD];
    RwInt32 i;

    /*
     * Remove end of clump name and extension 
     * so we can append lod numbers and ext.
     */
    lodPathEnding[0] = '\0';

    /*
     * Load it as a LOD object, building new names
     */
    for( i = 0; i < RPLODATOMICMAXLOD; i++ )
    {
        RsSprintf(lodPathEnding, "%d.dff", (int) i);

        clump[i] = LoadDff(lodPath);
        if( clump[i] )
        {
            numLods = i; 
        }
    }

    /*
     * Level 0 LOD supplies the atomics and frame hierarchy
     */
    if( clump[0] )
    {
        RpGeometry *geom;
        RwUInt32 numAtomics;

        /*
         * All clump must have the same number of atomics
         */
        numAtomics = ClumpCountAtomics(clump[0]);
        for( i = 1; i < RPLODATOMICMAXLOD; i++ )
        {
            if( clump[i] && (ClumpCountAtomics(clump[i]) != numAtomics) )
            {
                /*
                 * Wrong number of atomics - tidy up and fail
                 */
                for( i = 0; i < RPLODATOMICMAXLOD; i++ )
                {
                    if( clump[i] )
                    {
                        RpClumpDestroy(clump[i]);
                        clump[i] = (RpClump *)NULL;
                    }
                }
                return (RpClump *)NULL;
            }
        }

        /*
         * Now set up the LODs - first level 0 - always got this
         */
        LodRoot = ClumpGetFirstAtomic(clump[0]);
        AtomicSetRenderFlags(LodRoot, NULL);
        geom = RpAtomicGetGeometry(LodRoot);

        RpLODAtomicSetGeometry(LodRoot, 0, geom);

        RpLODAtomicSetLODCallBack(LodRoot, ClumpChooseLevelOfDetail);

        /*
         * Now the other levels - for missing ones, use next available
         */
        for( i = 1; i < RPLODATOMICMAXLOD; i++ )
        {
            if (clump[i])
            {
                geom = 
                    RpAtomicGetGeometry(ClumpGetFirstAtomic(clump[i]));
            }

            RpLODAtomicSetGeometry(LodRoot, i, geom);
        }

        /*
         * Destroy all clumps except first one.  Geometries will
         * remain, because of reference counting.
         */
        for( i = 1; i < RPLODATOMICMAXLOD; i++ )
        {
            if( clump[i] )
            {
                LODClumpStats[i].totalAtomics = 0;
                LODClumpStats[i].totalTriangles = 0;
                LODClumpStats[i].totalVertices = 0;
                LODClumpStats[i].maxFramePerAtomic = 0;

                RpClumpForAllAtomics(clump[i],
                    AtomicGetStatistics, &LODClumpStats[i]);

                RpClumpDestroy(clump[i]);
                clump[i] = (RpClump *)NULL;
            }
        }
    }
    else
    {
        /*
         * No level zero atomic, tidy up and fail...
         */
        for( i = 0; i < RPLODATOMICMAXLOD; i++ )
        {
            if( clump[i] )
            {
                RpClumpDestroy(clump[i]);
                clump[i] = (RpClump *)NULL;
            }
        }

        return (RpClump *)NULL;
    }

    RpLODAtomicHookRender(LodRoot);

    return clump[0];
}


/*
 *****************************************************************************
 */
RwBool
SaveTextureDictionary(void)
{
    RwStream *stream = (RwStream *)NULL;
    RwChar *path = (RwChar *)NULL;
    RwBool success = FALSE;

    path = RsPathnameCreate(TexDictionaryName);
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, path);
    RsPathnameCreate(path);

    if( stream )
    {
        RwTexDictionary *texDict;

        texDict = RwTexDictionaryGetCurrent();

        if( RwTexDictionaryStreamWrite(texDict, stream) )
        {
            success = TRUE;
        }

        RwStreamClose(stream, NULL);
    }
    
    return success;
}

/*
 *****************************************************************************
 */


#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) 

static RpMTEffectDict *
LoadEffectDictionary(RwChar *path)
{
    RpMTEffectDict *effectDict = (RpMTEffectDict *)NULL;
    RwStream *stream = (RwStream *)NULL;

    if( RwFexist(path) )
    {
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
        if( stream )
        {
            if( RwStreamFindChunk(stream, rwID_MTEFFECTDICT, 
                                  (RwUInt32 *)NULL, (RwUInt32 *)NULL) )
            {
                effectDict = RpMTEffectDictStreamRead(stream);
            }

            RwStreamClose(stream, NULL);
        }
    }

    return effectDict;
}

#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */


/*
 *****************************************************************************
 */
static RwTexDictionary *
LoadTextureDictionary(RwChar *path)
{
    RwTexDictionary *texDict = (RwTexDictionary *)NULL;
    RwStream *stream = (RwStream *)NULL;

    if( RwFexist(path) )
    {
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
        if( stream )
        {
            if( RwStreamFindChunk(stream, rwID_TEXDICTIONARY, 
                                  (RwUInt32 *)NULL, (RwUInt32 *)NULL) )
            {
                texDict = RwTexDictionaryStreamRead(stream);
            }

            RwStreamClose(stream, NULL);
        }
    }

    return texDict;
}


#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))

static void
EffectDictionaryGetName(RwChar *effectDictionaryName, 
                        RwChar *dffPath)
{
    /*
     * Creates a path for the effect dictionary which has the same name 
     * as the DFF and resides in the same directory as the DFF.
     * Effect dictionaries are platform-dependent...
     */
    RwInt32 i;

#if (defined(XBOX_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_xbox.mtd");
#elif (defined(GCN_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_gcn.mtd");
#else
    /* #error Unsupported platform */
#endif

    rwstrcpy(effectDictionaryName, dffPath);

    i = rwstrlen(effectDictionaryName)-1;

    while( i >= 0 )
    {
        if( effectDictionaryName[i] == '.' )
        {
            effectDictionaryName[i] = '\0';

            break;
        }

        i--;
    }

    rwstrcat(effectDictionaryName, ext);

    return;
}

#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */

/*
 *****************************************************************************
 */
static void 
TexDictionaryGetName(RwChar *texDictionaryName, RwChar *dffPath)
{
    /*
     * Creates a path for the texture dictionary which has the same name 
     * as the BSP and resides in the same directory as the DFF.
     * Texture dictionaries are platform-dependent...
     */
    RwInt32 i;

#if (defined(D3D8_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_d3d8.txd");
#elif (defined(D3D9_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_d3d9.txd");
#elif (defined(OPENGL_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_ogl.txd");
#elif (defined(SKY))
    const RwChar ext[] = RWSTRING("_ps2.txd");
#elif (defined(_XBOX))
    const RwChar ext[] = RWSTRING("_xbox.txd");
#elif (defined(DOLPHIN))
    const RwChar ext[] = RWSTRING("_gcn.txd");
#elif (defined(SOFTRAS_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_sras.txd");
#else
    /* #error Unsupported platform */
#endif

    rwstrcpy(texDictionaryName, dffPath);

    i = rwstrlen(texDictionaryName)-1;

    while( i >= 0 )
    {
        if( texDictionaryName[i] == '.' )
        {
            texDictionaryName[i] = '\0';

            break;
        }

        i--;
    }

    rwstrcat(texDictionaryName, ext);

    return;
}


/*
 *****************************************************************************
 */
static RwTexture *
TextureTest(RwTexture * texture __RWUNUSED__, 
            void *data)
{
    /*
     * The first time this function is called, we immediately know
     * that this texture dictionary has at least one texture. This is 
     * all we need to know, so return NULL to stop looking...
     */
    *(RwBool *)data = TRUE;

    return (RwTexture *)NULL;
}


/*
 *****************************************************************************
 */
RwBool 
ClumpLoadDFF(RwChar *dffPath)
{
    RpClump *clump = (RpClump *)NULL;
    RwChar *path = (char *)NULL;
    RwChar lodPath[256], *lodPathEnding = (char *)NULL;
    RwTexDictionary *prevTexDict = (RwTexDictionary *)NULL;
    RwTexDictionary *texDict = (RwTexDictionary *)NULL;

#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))
    RpMTEffectDict *prevEffectDict = (RpMTEffectDict *)NULL;
    RpMTEffectDict *effectDict = (RpMTEffectDict *)NULL;
#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */

    /* set multitexture effect file path on platforms that do it... */
#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))
    RwChar effectPath[256], *effectPathEnding = (char *)NULL, *psEffectPath;

    rwstrcpy(effectPath, dffPath);
    effectPathEnding = rwstrstr(effectPath, RWSTRING(".dff"));
    if (effectPathEnding)
    {
        strcpy(effectPathEnding, "/");
    }
    psEffectPath = RsPathnameCreate(effectPath);
    RpMTEffectSetPath(psEffectPath);
    RsPathnameDestroy(psEffectPath);
#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */

    path = RsPathnameCreate(dffPath);

#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))
    /*
     * Remember the current dictionaries so that they can be 
     * reinstated if the DFF load fails...
     */
    prevEffectDict = RpMTEffectDictGetCurrent();

    /*
     * Attempt to load effect and texture dictionaries...
     */
    EffectDictionaryGetName(EffectDictionaryName, path);

    effectDict = LoadEffectDictionary(EffectDictionaryName);
    if( effectDict )
    {
        /* set the new dictionary as current */
        RpMTEffectDictSetCurrent(effectDict);
    }
#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */

    /*
     * Remember the current dictionaries so that they can be 
     * reinstated if the DFF load fails...
     */
    prevTexDict = RwTexDictionaryGetCurrent();  
    
    /*
     * Attempt to load effect and texture dictionaries...
     */
    TexDictionaryGetName(TexDictionaryName, path);
    
    texDict = LoadTextureDictionary(TexDictionaryName);
    if( texDict )
    {
        /*
         * A texture dictionary is available, so make it the current
         * one before the loading the DFF...
         */
        RwTexDictionarySetCurrent(texDict);
    }
    else
    {
        /*
         * No texture dictionary available, so create a new empty
         * dictionary and make it the current one. This dictionary
         * will be populated with textures (if any) when the DFF is 
         * loaded. If textures have been loaded along with the DFF, 
         * we can save this dictionary, so it may be loaded directly
         * next time round...
         */
        RwTexDictionarySetCurrent(RwTexDictionaryCreate());

        RsSetModelTexturePath(path);
    }

    rwstrcpy(lodPath, path);
    lodPathEnding = rwstrstr(lodPath, RWSTRING("0.dff"));
    if( !lodPathEnding )
    {
        lodPathEnding = rwstrstr(lodPath, RWSTRING("0.DFF"));
    }

    if( !lodPathEnding )
    {
        clump = LoadDff(path);

        LodRoot = (RpAtomic *)NULL;
    }
    else
    {
        clump = LoadLodDffs(lodPath, lodPathEnding);
    }

    if( clump )
    {
        if( texDict )
        {
            /*
             * Pretend there's no textures, so we can disable the menu
             * item that saves the texture dictionary - we don't need to
             * resave it...
             */
            ClumpHasTextures = FALSE;
        }
        else
        {
            ClumpHasTextures = FALSE;

            RwTexDictionaryForAllTextures(RwTexDictionaryGetCurrent(), 
                TextureTest, (void *)&ClumpHasTextures);
        }

        rwstrcpy(ClumpPathFile, path);
        RsPathnameDestroy(path);

        /*
         * Setup the clump for viewing...
         */
        ClumpInitialize(clump);

        RwTexDictionaryDestroy(prevTexDict);

        return TRUE;
    }

    RsPathnameDestroy(path);

    /*
     * The DFF failed to load so reinstate the original texture dictionary...
     */
    RwTexDictionaryDestroy(RwTexDictionaryGetCurrent());
    RwTexDictionarySetCurrent(prevTexDict);

    return FALSE;
}


/*
 *****************************************************************************
 */
void
ClumpDeltaUpdate(RwReal delta)
{
    HAnimClumpUpdate(delta);
    SkinClumpUpdate(delta);
    MorphClumpUpdate(delta);
    DMorphClumpUpdate(delta);

    if( AtomicSelected &&
        (MorphOn || SkinOn || HAnimOn ||
         lodChanged) )
    {
        AtomicGetBoundingBox(AtomicSelected, &CurrentAtomicBBox);
        UpdateSelectedStats();

        lodChanged = FALSE;
    }

    return;
}


/*
 *****************************************************************************
 */
RwBool 
ClumpViewInit(void)
{
    ClumpPathFile[0] = '\0';

    return TRUE;
}


/*
 *****************************************************************************
 */
void
ClumpViewTerminate(void)
{

    HAnimDestroy();

    SkinDestroy();

    DMorphDestroy();

    PatchDestroy();

    if( Clump )
    {
        RpWorldRemoveClump(World, Clump);

        RpClumpDestroy(Clump);
    }
    
    return;
}


/*
 *****************************************************************************
 */
void
ClumpDisplayOnScreenInfo(RwRaster * cameraRaster __RWUNUSED__)
{
    RwInt32 linesFromBottom = 1;
    RwChar caption[128];
    RwInt32 usedLOD;
    ClumpStatistics *clumpStats;

    if( LodRoot )
    {
        usedLOD = RpLODAtomicGetCurrentLOD(LodRoot);

        if( usedLOD > 0 )
        {
            clumpStats = &LODClumpStats[usedLOD];
        }
        else
        {
            clumpStats = &ClumpStats;
        }
    }
    else
    {
        clumpStats = &ClumpStats;
    }

    if( AtomicSelected )
    {
        /*
         * Print the atomic statistics...
         */
        RsSprintf(caption, RWSTRING("Atomic %c"), 'A' + currentAtomicNumber);
        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPLEFT);

        RsSprintf(caption, RWSTRING("Triangles: %d"), AtomicTotalTriangles);
        RsCharsetPrint(Charset, caption, 0, 1, rsPRINTPOSTOPLEFT);

        RsSprintf(caption, RWSTRING("Vertices: %d"), AtomicTotalVertices);
        RsCharsetPrint(Charset, caption, 0, 2, rsPRINTPOSTOPLEFT);

        if( ClumpHasMorphAnimation )
        {
            RsSprintf(caption,
                RWSTRING("Morph targets: %d"), AtomicTotalMorphTargets);
            RsCharsetPrint(Charset, caption, 0, 3, rsPRINTPOSTOPLEFT);
        }
    }
    else
    {
        /*
         * Print the clump statistics...
         */
        RsSprintf(caption, RWSTRING("Atomics: %d"), clumpStats->totalAtomics);
        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPLEFT);

        RsSprintf(caption, 
            RWSTRING("Triangles: %d"), clumpStats->totalTriangles);
        RsCharsetPrint(Charset, caption, 0, 1, rsPRINTPOSTOPLEFT);

        RsSprintf(caption, 
            RWSTRING("Vertices: %d"), clumpStats->totalVertices);
        RsCharsetPrint(Charset, caption, 0, 2, rsPRINTPOSTOPLEFT);

        RsSprintf(caption, RWSTRING("Radius: %0.2f"), ClumpSphere.radius);
        RsCharsetPrint(Charset, caption, 0, 3, rsPRINTPOSTOPLEFT);

        if( ClumpHasSkinAnimation )
        {
            RsSprintf(caption, RWSTRING("Bones: %d"), AtomicTotalSkinBones);
            RsCharsetPrint(Charset, caption, 0, 4, rsPRINTPOSTOPLEFT);

            RsSprintf(caption, 
                RWSTRING("Keyframes: %d"), AtomicTotalAtomicTotalKeyFrame);
            RsCharsetPrint(Charset, caption, 0, 5, rsPRINTPOSTOPLEFT);
        }
    }

    linesFromBottom = 1;

    /*
     * Print the animation and animation speed ...
     */
    if( ClumpHasMorphAnimation && MorphOn )
    {
        RsSprintf(caption, RWSTRING("MPS: %.1f"), MorphsPerSecond);
        linesFromBottom++;
    }
    else
    {
        RsSprintf(caption, RWSTRING(""));
    }

    RsCharsetPrint(
        Charset, caption, 0, -linesFromBottom, rsPRINTPOSBOTTOMRIGHT);

    linesFromBottom++;

    /*
     * Print the lod..
     */
    if( LodRoot )
    {
        RsSprintf(caption, RWSTRING("Level Of Detail: %d"),
            RpLODAtomicGetCurrentLOD(LodRoot));
        RsCharsetPrint(
            Charset, caption, 0, -linesFromBottom, rsPRINTPOSBOTTOMRIGHT);
        linesFromBottom++;
    }

    linesFromBottom++;

    /*
     * Print user control messages
     */
    if( ClumpPick )
    {
        RsSprintf(caption, RWSTRING("Pick Atomic"));

        RsCharsetPrint(
            Charset, caption, 0, -linesFromBottom, rsPRINTPOSBOTTOMRIGHT);
        linesFromBottom++;
    }

    if( ClumpRotate || ClumpDirectRotate)
    {
        RsSprintf(caption, RWSTRING("Rotate Atomic"));

        RsCharsetPrint(
            Charset, caption, 0, -linesFromBottom, rsPRINTPOSBOTTOMRIGHT);
        linesFromBottom++;
    }

    if( ClumpTranslate || ClumpDirectTranslate )
    {
        RsSprintf(caption, RWSTRING("Translate Atomic"));

        RsCharsetPrint(
            Charset, caption, 0, -linesFromBottom, rsPRINTPOSBOTTOMRIGHT);
        linesFromBottom++;
    }

    return;
}


/*
 *****************************************************************************
 */
RwBool 
SaveDFF(void)
{
    RwStream *stream = (RwStream *)NULL;
    RwBool success = TRUE;
    RwChar *path;

    path = RsPathnameCreate(RWSTRING("./new.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( !RpClumpStreamWrite(Clump, stream) )
        {   
            RsErrorMessage(RWSTRING("Cannot write DFF file."));

            success = FALSE;
        }

        RwStreamClose(stream, NULL);
    }
    else
    {
        RsErrorMessage(RWSTRING("Cannot open stream to write DFF file."));

        success =  FALSE;
    }

    return success;
}

/*
 *****************************************************************************
 */
