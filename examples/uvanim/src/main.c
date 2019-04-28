
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
 * main.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: uv animation example
 *
 ****************************************************************************/

#include <string.h>
#include <rwcore.h>
#include <rpworld.h>
#if (defined(SKY))
#include <rppds.h>
#endif /* (defined(SKY)) */

#ifdef RWLOGO
#include <rplogo.h>
#endif

#include <rtcharse.h>

#include <rpmatfx.h>
#include <rpuvanim.h>
#include <rtdict.h>

#include "skeleton.h"
#include "main.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.5f)

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

RtCharset *Charset = NULL;

RwV3d XAxis = {1.0f, 0.0f, 0.0f};
RwV3d YAxis = {0.0f, 1.0f, 0.0f};
RwV3d ZAxis = {0.0f, 0.0f, 1.0f};

typedef struct Scene Scene;

struct Scene {
    const RwChar *name;
    const RwChar *worldFile;
    const RwChar *alphaClumpFile;
    const RwChar *backClumpFile;
    const RwChar *uvanimFile;
    RtDict *uvanimDict;
    RpWorld *world;
    RpClump *alphaClump;
    RpClump *backClump;
    RwCamera *camera;
    RwSList *animatedMaterials;
    RwBool mayAutoLight;
    RwBool autoLit;
    RwBool autoCam;
    RpUVAnim *autoAnim;
};

static Scene Scenes[]={
    { "tunnel",             "models/tunnelbb.bsp", "models/tunalpha.dff", "models/tunnel.dff", "models/tunnel.uva",    NULL, NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, FALSE, NULL },
    { "lava",               "models/lavawrld.bsp", NULL,                    "models/lavacam.dff", "models/lavawrld.uva", NULL, NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, FALSE, NULL },
    { "auto-gen animation", "models/wormhole.bsp", NULL,                    "models/wormhole.dff",  NULL,                    NULL, NULL, NULL, NULL, NULL, NULL, TRUE, FALSE, FALSE, NULL } };
#define NUMSCENES (sizeof(Scenes)/sizeof(Scene))

#define DEFAULTSCENEINDEX 0
static RwInt32 CurrentSceneIndex = DEFAULTSCENEINDEX;
static Scene *CurrentScene = &Scenes[DEFAULTSCENEINDEX];
static RwReal AnimScale=1.0f;

RwCamera *Camera;
RwCamera *FirstCamera;

/*
 *****************************************************************************
 * World, clump and material for-all helpers
 *****************************************************************************
 */
typedef struct MaterialCBAndData MaterialCBAndData;

struct MaterialCBAndData
{
    RpMaterialCallBack cb;
    void *data;
};

static RpAtomic*
AtomicForAllMaterials(RpAtomic *atomic, void *data)
{
    MaterialCBAndData *cbAndData = (MaterialCBAndData *)data;

    RpGeometryForAllMaterials(
        RpAtomicGetGeometry(atomic), cbAndData->cb, cbAndData->data);

    return atomic;
}

static RpClump*
ClumpForAllMaterials(RpClump *clump, void *data)
{
    RpClumpForAllAtomics(clump, AtomicForAllMaterials, data);

    return clump;
}

static RpWorld*
WorldForAllMaterials(RpWorld *world, void *data)
{
    MaterialCBAndData *cbAndData = (MaterialCBAndData *)data;

    RpWorldForAllMaterials(world, cbAndData->cb, cbAndData->data);
    RpWorldForAllClumps(world, ClumpForAllMaterials, data);

    return world;
}

typedef struct AtomicCBAndData AtomicCBAndData;

struct AtomicCBAndData
{
    RpAtomicCallBack cb;
    void *data;
};

static RpClump*
WorldForAllClumps(RpClump *clump, void *data)
{
    AtomicCBAndData *cb=(AtomicCBAndData*)data;

    RpClumpForAllAtomics(clump, cb->cb, cb->data);

    return clump;
}

static RpWorld*
WorldForAllAtomics(RpWorld *world, void *data)
{
    RpWorldForAllClumps(world, WorldForAllClumps, data);

    return world;
}

static RpLight *
WorldHasLightCallBack(RpLight *light __RWUNUSED__, void *data)
{
    RwBool *yep = (RwBool *)data;

    *yep = TRUE;

    return (RpLight *)NULL;  /* early out */
}

static RwBool
WorldHasLights(RpWorld *world)
{
    RwBool yep = FALSE;
    RpWorldForAllLights(world, WorldHasLightCallBack, &yep);
    return yep;
}

static RwCamera *
GetFirstCamera(RwCamera *camera, void *data)
{
    *(RwCamera **)data = camera;
    return camera;
}

static RpClump *
ClumpGetFirstCamera(RpClump *clump, void *data)
{
    RwCamera **camera = (RwCamera **)data;

    RpClumpForAllCameras(clump, GetFirstCamera, camera);

    if (*camera)
    {
        return FALSE; /* early out */
    }
    else
    {
        return clump;
    }
}

static RwCamera *
WorldGetFirstCamera(RpWorld *world)
{
    RwCamera *camera=NULL;
    RpWorldForAllClumps(world, ClumpGetFirstCamera, &camera);
    return camera;
}

static RpLight *
DestroyLight(RpLight *light, void *data)
{
    RwFrame *frame;

    RpWorldRemoveLight((RpWorld *)data, light);

    frame = RpLightGetFrame(light);
    if( frame )
    {
        RpLightSetFrame(light, NULL);

        RwFrameDestroy(frame);
    }

    RpLightDestroy(light);

    return light;
}

RwReal
WorldGetExtent(RpWorld *world)
{
    const RwBBox *bbox=RpWorldGetBBox(world);
    RwV3d  extentVec;
    RwReal extent;

    RwV3dSub(&extentVec, &bbox->sup, &bbox->inf);
    extent = RwV3dLength(&extentVec);

    return extent;
}

static void
WorldGetCentre(RpWorld *world, RwV3d *out)
{
    const RwBBox *bbox=RpWorldGetBBox(world);

    RwV3dAdd(out, &bbox->sup, &bbox->inf);
    RwV3dScale(out, out, 0.5f);
}

/*
 *****************************************************************************
 * Helper functions for creating a list of animated materials and updating
 * animations via this list
 *****************************************************************************
 */
static RpMaterial *
AddAnimatedMaterialToList(RpMaterial *material, void *data)
{
    RwSList *list = (RwSList *)data;
    if (RpMaterialUVAnimExists(material))
    {
        RpMaterial **current=(RpMaterial**)rwSListGetBegin(list);

        /* No duplicates */
        while (current != rwSListGetEnd(list))
        {
            if (material==*current)
            {
                /* Don't add, and that's OK */
                return material;
            }
            ++current;
        }

        /* Add it */
        *(RpMaterial**)rwSListGetNewEntry(list, rwID_NAOBJECT)=material;
    }

    return material;
}

static RwSList *
CreateAnimatedMaterialsList(RpWorld *world, RpClump *alphaClump)
                            /* Note - alphaClump is included because it isn't
                                      added to the world */
{
    RwSList *list=rwSListCreate(sizeof(RpMaterial *), rwID_NAOBJECT);

    if (list)
    {
        MaterialCBAndData cbAndData = { AddAnimatedMaterialToList, NULL };
        cbAndData.data = list;
        WorldForAllMaterials(world, &cbAndData);
        
        if (alphaClump)
        {
            ClumpForAllMaterials(alphaClump, &cbAndData);
        }
    }

    return list;
}

static void
MaterialsInterpolatorsAddAnimTime(RwSList *animatedMaterials, RwReal deltaTime)
{
    RpMaterial **material = (RpMaterial **)rwSListGetBegin(animatedMaterials);

    while (material!=rwSListGetEnd(animatedMaterials))
    {
        RpMaterialUVAnimAddAnimTime(*material, deltaTime*AnimScale);

        ++material;
    }
}

static void
MaterialsAnimApply(RwSList *animatedMaterials)
{
    RpMaterial **material = (RpMaterial **)rwSListGetBegin(animatedMaterials);

    while (material!=rwSListGetEnd(animatedMaterials))
    {
        RpMaterialUVAnimApplyUpdate(*material);

        ++material;
    }
}

/*
 *****************************************************************************
 * Automatic animation creation
 *****************************************************************************
 */
typedef struct NodesKeyframes NodesKeyframes;

struct NodesKeyframes
{
    void *Keyframes;
    RwInt32 numKeyFrames;
};

static RwBool
AnimOrderAndSetKeyframes(RtAnimAnimation *anim, NodesKeyframes *nodes, RwInt32 numNodes)
{
    RwInt32     *numFrames;
    RpUVAnimKeyFrame     **prevKey;
    RwInt32     i,j;
    RwChar      msg[256];
    RpUVAnimKeyFrame     *destination;
    RwUInt8     **currentKey;
    RwInt32     keyframeSize;
    RwInt32     keyframesLeft;
    RwInt32 minNodes;
    RwReal minTime;

    for(i=0;i<numNodes;i++)
    {
        if(nodes[i].numKeyFrames <2)
        {
            RsSprintf(msg,"Error : node %d has less than 2 keyframes",i);
            RsErrorMessage(msg);

            return FALSE;
        }
    }

    prevKey = (RpUVAnimKeyFrame **)RwMalloc(sizeof(RpUVAnimKeyFrame *)*numNodes,
                                    rwID_NAOBJECT);
    currentKey = (RwUInt8 **)RwMalloc(sizeof(RwUInt8*)*numNodes,
                                       rwID_NAOBJECT);
    numFrames = (RwInt32*)RwMalloc(sizeof(RwInt32)*numNodes,
                                   rwID_NAOBJECT);

    memset(prevKey,0,sizeof(RpUVAnimKeyFrame *)*numNodes);
    keyframesLeft = 0;

    for(i=0;i<numNodes;i++)
    {
        currentKey[i] = nodes[i].Keyframes;
        numFrames[i] = nodes[i].numKeyFrames;
        keyframesLeft += nodes[i].numKeyFrames;
    }
    destination = anim->pFrames;
    keyframeSize = anim->interpInfo->animKeyFrameSize;

    /* Copy the first two key frames to the animation... */
    for(j=0;j<2;j++)
    {
        for(i=0;i<numNodes;i++)
        {
            {
                RwMatrix matrix;
                RpUVAnimLinearKeyFrameToMatrixMacro(&matrix,  (RpUVAnimKeyFrame *)currentKey[i]);

                /* Copy the found keyframe to the animation */
                RpUVAnimKeyFrameInit(anim, destination,
                                     prevKey[i],
                                     ((RpUVAnimKeyFrame *)currentKey[i])->time,
                                     &matrix);
            }
            prevKey[i] = destination;

            destination++;
            currentKey[i] += keyframeSize;

            numFrames[i]--;
            keyframesLeft--;
        }
    }

    for(j=0;j<keyframesLeft;j++)
    {
        minNodes = -1;
        minTime = RwRealMAXVAL;
        /* Search for the recently processed keyframe with the smallest time */
        for(i=0;i<numNodes;i++)
        {
            if(((prevKey[i])->time < minTime)
                && (numFrames[i] != 0))
            {
                minTime = prevKey[i]->time;
                minNodes = i;
            }
        }

        {
            RwMatrix matrix;
            RpUVAnimLinearKeyFrameToMatrixMacro(&matrix,  (RpUVAnimKeyFrame *)currentKey[minNodes]);

            /* Copy the found keyframe to the animation */
            RpUVAnimKeyFrameInit(anim, destination,
                                 prevKey[minNodes],
                                 ((RpUVAnimKeyFrame *)currentKey[minNodes])->time,
                                 &matrix);
        }

        prevKey[minNodes] = destination;

        destination++;
        currentKey[minNodes] += keyframeSize;

        numFrames[minNodes]--;
    }

    /* Point the start->prevnode to the end nodes */
    destination = anim->pFrames;
    for(i=0;i<numNodes;i++)
    {
        destination->prevFrame = prevKey[i];

        destination ++;
    }

    /* A bit of clean up */
    RwFree(prevKey);
    RwFree(currentKey);
    RwFree(numFrames);

    return TRUE;
}


static RtAnimAnimation *
AnimCreate(const RwChar *name, RwUInt32 nodes, RwUInt32 frames, RwReal duration, RwUInt32 *map)
{
    const RwReal deltaTime = duration / (frames-1);

    RtAnimAnimation *anim;

    NodesKeyframes *keyframes = RwMalloc(sizeof(NodesKeyframes)*nodes, rwID_NAOBJECT);
        ;
    /* Create Animation */
    anim = RpUVAnimCreate(name, nodes, nodes*frames, duration, map, rpUVANIMPARAMKEYFRAMES);

    /* Generate keyframes for each node */
    {
        RwUInt32 node;
        for(node=0; node<nodes; ++node)
        {
            NodesKeyframes *kfc=&keyframes[node];
            RpUVAnimKeyFrame *f;

            kfc->Keyframes = (RpUVAnimKeyFrame*)(RwMalloc(sizeof(RpUVAnimKeyFrame)*frames, rwID_NAOBJECT));
            kfc->numKeyFrames = frames;

            f = kfc->Keyframes;

            {
                RwUInt32 frame;
                RwReal time=0.0f;
                for(frame=0; frame<frames; ++frame)
                {
                    RwV3d trans = { 0,0,0};
                    RwMatrix mat;
                    RpUVAnimLinearKeyFrameData *d=&f->data.linear;

                    trans.x = -(RwReal)frame/(frames-1);
                    trans.y = -(RwReal)frame/(frames-1)*2.0f;
                    RwMatrixTranslate(&mat, &trans, rwCOMBINEREPLACE );
                    d->uv[0] = RwMatrixGetRight(&mat)->x;
                    d->uv[1] = RwMatrixGetRight(&mat)->y;
                    d->uv[2] = RwMatrixGetUp(&mat)->x;
                    d->uv[3] = RwMatrixGetUp(&mat)->y;
                    d->uv[4] = RwMatrixGetPos(&mat)->x;
                    d->uv[5] = RwMatrixGetPos(&mat)->y;

                    f->time = time;
                    time += deltaTime;
                    f++;
                }
            }
        }

    }

    AnimOrderAndSetKeyframes(anim, keyframes, nodes);

    {
        RwUInt32 i;
        for (i=0; i<nodes; ++i)
        {
            RwFree(keyframes[i].Keyframes);
        }
        RwFree(keyframes);
    }

    return anim;
}

static RpMaterial*
AnimateMaterial(RpMaterial *material, void *data)
{
    RpUVAnim *anim=(RpUVAnim *)data;

    RpMatFXMaterialSetEffects(material, rpMATFXEFFECTUVTRANSFORM);

    RpMaterialSetUVAnim(material, anim, 0);
    /*RpMatFXMaterialSetUVTransformMatrices(material, 0, 0);*/

    return material;
}

static RpAtomic *
AnimateAtomic(RpAtomic *atomic, void __RWUNUSED__ *data )
{
    RpMatFXAtomicEnableEffects(atomic);

    return atomic;
}

static RpUVAnim *
CreateAutoAnim()
{
    RwUInt32 map[]={ 0 };

    RpUVAnim *anim = AnimCreate("auto", 1, 4, 6, map);

    return anim;
}

static RpWorld *
AnimateAllMaterials(RpWorld *world, RpUVAnim *anim)
{
    MaterialCBAndData matCbAndData = { AnimateMaterial, NULL };
    AtomicCBAndData atomicCbAndData = { AnimateAtomic, NULL };
    matCbAndData.data = anim;

    WorldForAllAtomics(world, &atomicCbAndData);
    WorldForAllMaterials(world, &matCbAndData);

    return world;
}

/*
 *****************************************************************************
 * Scene management
 *****************************************************************************
 */

/*
 *****************************************************************************
 */
static RpWorld *
CreateWorld(void)
{
    RpWorld *world;
    RwBBox bb;

    bb.inf.x = bb.inf.y = bb.inf.z = -100.0f;
    bb.sup.x = bb.sup.y = bb.sup.z = 100.0f;

    world = RpWorldCreate(&bb);

    return world;
}

/*
 *****************************************************************************
 */
static RwBool
CreateLights(RpWorld *world)
{
    RpLight *light = NULL;

    /*
     * Add ambient light source...
     */
    light = RpLightCreate(rpLIGHTAMBIENT);
    if( light )
    {
        RpWorldAddLight(world, light);
    }
    else
    {
        return FALSE;
    }

    /*
     * Add directional light source...
     */
    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    if( light )
    {
        RwFrame *frame;

        frame = RwFrameCreate();
        if( frame )
        {
            RwFrameRotate(frame, &XAxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &YAxis, 30.0f, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, frame);

            RpWorldAddLight(world, light);
        }
        else
        {
            RpLightDestroy(light);

            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwCamera *
CreateCamera(RpWorld *world)
{
    RwCamera *camera;

    camera = RwCameraCreate();
    if( camera )
    {
        RwReal extent = WorldGetExtent(world);
        RwFrame *frame = RwFrameCreate();
        RwV3d centre;

        RwCameraSetNearClipPlane(camera, extent*0.001f);
        RwCameraSetFarClipPlane(camera, extent);

        RpWorldAddCamera(world, camera);

        WorldGetCentre(world, &centre);

        RwFrameTranslate(frame, &centre, rwCOMBINEREPLACE);
        RwCameraSetFrame(camera, frame);

        return camera;
    }

    return NULL;
}

static void
DestroyCamera(RwCamera *camera, RpWorld *world)
{
    RwFrame *frame = RwCameraGetFrame(camera);
    if (frame)
    {
        RwCameraSetFrame(camera, (RwFrame *)NULL);
        RwFrameDestroy(frame);
    }

    RpWorldRemoveCamera(world, camera);

    RwCameraSetRaster(camera, (RwRaster *)NULL);
    RwCameraSetZRaster(camera, (RwRaster *)NULL);

    RwCameraDestroy(camera);
}

static RtDict *
LoadUVAnimDict(const RwChar *filename)
{
    RwChar *path;
    RtDict *dict = NULL;
    RwStream *stream = NULL;

    path = RsPathnameCreate(filename);
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_UVANIMDICT, NULL, NULL) )
        {
            dict = RtDictSchemaStreamReadDict(RpUVAnimGetDictSchema(), stream);
        }

        RwStreamClose(stream, NULL);
    }

    return dict;
}

static RpWorld *
LoadWorld(const RwChar *filename)
{
    RwChar *path;
    RpWorld *world = NULL;
    RwStream *stream = NULL;

    path = RsPathnameCreate(filename);
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL) )
        {
            world = RpWorldStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    return world;
}

static RpClump *
LoadClump(const RwChar *filename)
{
    RwChar *path;
    RpClump *clump = NULL;
    RwStream *stream = NULL;

    path = RsPathnameCreate(filename);
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

    return clump;
}

static RwBool
LoadScene(Scene *scene)
{
    /*
     * Load the uvanim dict to be used by the scene
     */
    if (scene->uvanimFile)
    {
        scene->uvanimDict = LoadUVAnimDict(scene->uvanimFile);
    }
    else
    {
        scene->uvanimDict = RtDictSchemaCreateDict(RpUVAnimGetDictSchema());
    }
    if (!scene->uvanimDict)
    {
        RsErrorMessage(RWSTRING("Error creating UV animation dictionary."));
        return FALSE;
    }
    else
    {
        /* Set current dictionary which world/clump loading will use */
        RtDictSchemaSetCurrentDict(RpUVAnimGetDictSchema(), scene->uvanimDict);
    }

    /* If an anim file wasn't specified; need to autocreate anims */
    if (!scene->uvanimFile)
    {
        scene->autoAnim = CreateAutoAnim();

        RtDictAddEntry(RtDictSchemaGetCurrentDict(RpUVAnimGetDictSchema()),
                                                  scene->autoAnim);
        RpUVAnimDestroy(scene->autoAnim); /* dictionary can own it */
    }

    /*
     * Load the world
     */
    if (scene->worldFile)
    {
        scene->world = LoadWorld(scene->worldFile);
    }
    else
    {
        /*
         * Create an empty world
         */
        scene->world = CreateWorld();
    }

    if(!scene->world)
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

    /*
     * Load the alpha-containing clump
     */
    if (scene->alphaClumpFile)
    {
        scene->alphaClump = LoadClump(scene->alphaClumpFile);

        if( !scene->alphaClump )
        {
            RsErrorMessage(RWSTRING("Cannot create alpha clump."));
    
            return FALSE;
        }

    }

    /*
     * Load the back clump
     */
    if (scene->backClumpFile)
    {
        scene->backClump = LoadClump(scene->backClumpFile);
        if( scene->backClump )
        {
            RpWorldAddClump(scene->world, scene->backClump);
        }
        else
        {
            RsErrorMessage(RWSTRING("Cannot create back clump."));
    
            return FALSE;
        }
    }

    /*
     * If no lights; create some
     */
    if (!WorldHasLights(scene->world) && scene->mayAutoLight)
    {
        if (!CreateLights(scene->world))
        {
            RsErrorMessage(RWSTRING("Cannot create lights."));

            return FALSE;
        }
        scene->autoLit = TRUE;
    }


    /*
     * If no camera; create one
     */
    scene->camera = WorldGetFirstCamera(scene->world);
    if (!scene->camera)
    {
        scene->camera = CreateCamera(scene->world);
        if(scene->camera)
        {
            scene->autoCam = TRUE;
        }
        else
        {
            RsErrorMessage(RWSTRING("Cannot create camera."));

            return FALSE;
        }
    }
    (void)RwCameraSetRaster(scene->camera, RwCameraGetRaster(FirstCamera));
    (void)RwCameraSetZRaster(scene->camera, RwCameraGetZRaster(FirstCamera));


    /* If an anim file wasn't specified; need to autocreate anims */
    if (!scene->uvanimFile)
    {
        AnimateAllMaterials(scene->world, scene->autoAnim);
    }

    /* For convenience, make a list of all animated materials in the scene */
    scene->animatedMaterials = CreateAnimatedMaterialsList(scene->world, scene->alphaClump);
    if (!scene->animatedMaterials)
    {
        RsErrorMessage(RWSTRING("Error creating animated materials list."));
        return FALSE;
    }

    return TRUE;
}

static RwBool
InitScene(Scene *scene)
{
    return LoadScene(scene);
}

static void
DestructScene(Scene *scene)
{
    if (scene->animatedMaterials)
    {
        rwSListDestroy(scene->animatedMaterials);
    }

    if (scene->autoLit)
    {
        RpWorldForAllLights(scene->world, DestroyLight, scene->world);
    }

    if (scene->autoCam)
    {
        DestroyCamera(scene->camera, scene->world);
    }
   
    if (scene->backClump )
    {
        RpWorldRemoveClump(scene->world, scene->backClump);
        RpClumpDestroy(scene->backClump);
    }
    
    if (scene->alphaClump )
    {
        RpClumpDestroy(scene->alphaClump);
    }

    if (scene->world )
    {
        RpWorldDestroy(scene->world);
    }

    if (scene->uvanimDict)
    {
        RtDictDestroy(scene->uvanimDict);
    }
}

/*
 *****************************************************************************
 */
static RwBool
SceneFunctionCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return (TRUE);
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
AnimScaleFunctionCallback(RwBool testEnable)
{
    if( testEnable )
    {
        return (TRUE);
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
Initialize(void)
{
    if( RsInitialize() )
    {
        if( !RsGlobal.maximumWidth )
        {
            RsGlobal.maximumWidth = DEFAULT_SCREEN_WIDTH;
        }

        if( !RsGlobal.maximumHeight )
        {
            RsGlobal.maximumHeight = DEFAULT_SCREEN_HEIGHT;
        }

        RsGlobal.appName = RWSTRING("RenderWare Graphics UV Animation Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
InitializeMenu(void)
{
    static RwChar fpsLabel[] = RWSTRING("FPS_F");
    static RwChar sceneFunctionLabel[] = RWSTRING("Scene");
    static RwChar speedFunctionLabel[] = RWSTRING("Animation speed");
    static const RwChar *sceneNames[NUMSCENES];

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        RwUInt32 i;
        for (i=0; i<NUMSCENES; ++i)
        {
            sceneNames[i] = Scenes[i].name;
        }

        MenuAddEntryBool(fpsLabel, &FPSOn, NULL);

        MenuAddSeparator();

        MenuAddEntryInt(
            sceneFunctionLabel,
            &CurrentSceneIndex,
            &SceneFunctionCallback,
            0,
            NUMSCENES-1,
            1,
            sceneNames);

        MenuAddEntryReal(
            speedFunctionLabel,
            &AnimScale,
            &AnimScaleFunctionCallback,
            0.0f,
            10.0f,
            0.05f);

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
Initialize3D(void *param)
{
    RwChar *path;
    RwUInt32 scene;

    if( !RsRwInitialize(param) )
    {
        RsErrorMessage(RWSTRING("Error initializing RenderWare."));

        return FALSE;
    }

    Charset = RtCharsetCreate(&ForegroundColor, &BackgroundColor);
    if( Charset == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create raster charset."));

        return FALSE;
    }

    /* Set Default path for textures */
    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    /* Setup application camera and init from current scene */
    FirstCamera = CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);
    if (!FirstCamera)
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }
    Camera = FirstCamera;

    /* Initialize each scene */
    for (scene=0; scene!=NUMSCENES; ++scene)
    {
        InitScene(&Scenes[scene]);
    }

    if( !InitializeMenu() )
    {
        RsErrorMessage(RWSTRING("Error initializing menu."));

        return FALSE;
    }

#ifdef RWMETRICS
    RsMetricsOpen(Camera);
#endif

#if (defined(SKY))
    RpSkySelectTrueTSClipper(TRUE);
    RpSkySelectTrueTLClipper(TRUE);
#endif

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
Terminate3D(void)
{
    RwUInt32 scene;

    /*
     * Close or destroy RenderWare components in the reverse order they
     * were opened or created...
     */

#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    for(scene=0; scene<NUMSCENES; ++scene)
    {
        DestructScene(&Scenes[scene]);
    }

    if (FirstCamera)
    {
        CameraDestroy(FirstCamera);
    }

    if( Charset )
    {
        RwRasterDestroy(Charset);
    }

    RsRwTerminate();

    return;
}


/*
 *****************************************************************************
 */
static RwBool
AttachPlugins(void)
{
    /*
     * Attach world plug-in...
     */
    if( !RpWorldPluginAttach() )
    {
        return FALSE;
    }

#ifdef RWLOGO
    /*
     * Attach logo plug-in...
     */
    if( !RpLogoPluginAttach() )
    {
        return FALSE;
    }
#endif

    if ( !RpMatFXPluginAttach() )
    {
        return FALSE;
    }

    /* For Sky builds register the matfx PDS pipes */
#if (defined(SKY))
    RpMatfxPipesAttach();
#endif /* (defined(SKY)) */


    if ( !RpUVAnimPluginAttach() )
    {
        return FALSE;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
DisplayOnScreenInfo(void)
{
    RwChar caption[256];

    if( FPSOn )
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);

        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }

    return;
}


/*
 *****************************************************************************
 */
static void
Render(void)
{
    /* Set camera from current scene */
    CurrentScene = &Scenes[CurrentSceneIndex];
    Camera = CurrentScene->camera;

    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE );

        if( MenuGetStatus() != HELPMODE )
        {
            RpWorldRender(CurrentScene->world);
            
            if (CurrentScene->alphaClump)
            {
                RpClumpRender(CurrentScene->alphaClump);
            }

            DisplayOnScreenInfo();
        }

        MenuRender(Camera, NULL);

#ifdef RWMETRICS
        RsMetricsRender();
#endif

        RwCameraEndUpdate(Camera);
    }

    /*
     * Display camera's raster...
     */
    RsCameraShowRaster(Camera);

    FrameCounter++;

    return;
}


/*
 *****************************************************************************
 */
static void
Idle(void)
{
    RwUInt32 thisTime;
    RwReal deltaTime;

    static RwBool firstCall = TRUE;
    static RwUInt32 lastFrameTime, lastAnimTime;

    if( firstCall )
    {
        lastFrameTime = lastAnimTime = RsTimer();

        firstCall = FALSE;
    }

    thisTime = RsTimer();

    /*
     * Has a second elapsed since we last updated the FPS...
     */
    if( thisTime > (lastFrameTime + 1000) )
    {
        /*
         * Capture the frame counter...
         */
        FramesPerSecond = FrameCounter;

        /*
         * ...and reset...
         */
        FrameCounter = 0;

        lastFrameTime = thisTime;
    }

    /*
     * Animation update time in seconds...
     */
    deltaTime = (thisTime - lastAnimTime) * 0.001f;

    /*
     * Update any animations here...
     */
	MaterialsInterpolatorsAddAnimTime(CurrentScene->animatedMaterials, deltaTime);
	MaterialsAnimApply(CurrentScene->animatedMaterials);

    lastAnimTime = thisTime;

    Render();

    return;
}


/*
 *****************************************************************************
 */
RsEventStatus
AppEventHandler(RsEvent event, void *param)
{
    switch( event )
    {
        case rsINITIALIZE:
        {
            return Initialize() ? rsEVENTPROCESSED : rsEVENTERROR;
        }

        case rsCAMERASIZE:
        {
            CameraSize(FirstCamera, (RwRect *)param,
                DEFAULT_VIEWWINDOW, DEFAULT_ASPECTRATIO);

            return rsEVENTPROCESSED;
        }

        case rsRWINITIALIZE:
        {
            return Initialize3D(param) ? rsEVENTPROCESSED : rsEVENTERROR;
        }

        case rsRWTERMINATE:
        {
            Terminate3D();

            return rsEVENTPROCESSED;
        }

        case rsPLUGINATTACH:
        {
            return AttachPlugins() ? rsEVENTPROCESSED : rsEVENTERROR;
        }

        case rsINPUTDEVICEATTACH:
        {
            AttachInputDevices();

            return rsEVENTPROCESSED;
        }

        case rsIDLE:
        {
            Idle();

            return rsEVENTPROCESSED;
        }

        default:
        {
            return rsEVENTNOTPROCESSED;
        }
    }
}

/*
 *****************************************************************************
 *  UVAnim, Material and Dictionary management samples
 */

#if 0
    /*
     * UV animation dictionary management
     *  -creation
     *  -streaming
     *  -destruction
     */
    {
        RtDict *dict = RtDictSchemaCreateDict(RpUVAnimGetDictSchema());
        RwUInt32 map[3] = { 0, 1, 2 };
        RpUVAnim *anim;

        anim = AnimCreate("MyAnim3", 1, 10, 20.0, map);

        RtDictAddEntry(dict, anim);
        RpUVAnimDestroy(anim); /* Dictionary now owns it */

        /* Test streaming */
        {
            RtDict *dict2=NULL;
            RwMemory memory0 = {0, 0};

            /* Test write */
            RwStream *stream=RwStreamOpen(rwSTREAMMEMORY, rwSTREAMWRITE, &memory0);
            RtDictStreamWrite(dict, stream);
            RwStreamClose(stream, &memory0);

            /* Test read */
            stream=RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &memory0);
            RwStreamFindChunk(stream, rwID_UVANIMDICT, 0, 0);
            dict2 = RtDictSchemaStreamReadDict(RpUVAnimGetDictSchema(), stream);
            RwStreamClose(stream, &memory0);

            RwFree(memory0.start);

            RtDictDestroy(dict2);
        }

        RtDictDestroy(dict);
    }
#endif
#if 0
    /*
     * Create a Material with a uvanim from a UV animation dictionary
     */
    {
        RpMaterial *mat = RpMaterialCreate();
        RwUInt32 map[3] = { 0, 1, 2 };
        RpUVAnim *anim = AnimCreate("MyAnim4", 2, 10, 5, map);
        RtDict *dict = RtDictSchemaCreateDict(RpUVAnimGetDictSchema());
        RpMaterialSetUVAnim(mat, anim, 0);
        RtDictAddEntry(dict, anim);
        RpUVAnimDestroy(anim); /* Dictionary now owns it */

        /* Test streaming */
        {
            RpMaterial *mat2=NULL;
            RwMemory memory0 = {0, 0};
            RwMemory memory1 = {0, 0};
            RwBool theSameLength=FALSE;
            RwBool theSameContents=FALSE;

            /* Test write */
            RwStream *stream=RwStreamOpen(rwSTREAMMEMORY, rwSTREAMWRITE, &memory0);
            RpMaterialStreamWrite(mat, stream);
            RwStreamClose(stream, &memory0);

            /* Test read */
            stream=RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &memory0);
            RwStreamFindChunk(stream, rwID_MATERIAL, 0, 0);
            mat2 = RpMaterialStreamRead(stream);
            RwStreamClose(stream, &memory0);

            RwFree(memory0.start);

            RpMaterialDestroy(mat2);
        }
        RtDictDestroy(dict);
    }
#endif

