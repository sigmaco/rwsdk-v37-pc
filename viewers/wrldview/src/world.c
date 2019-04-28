
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
 * world.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 * Reviewed by: John Irwin (with substantial edits).
 *
 * Purpose: RenderWare3 BSP viewer.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rppvs.h"
#include "rtworld.h"
#include "rpmatfx.h"
#include "rpcollis.h"
#include "rpltmap.h"

#include "skeleton.h"
#include "rtfsyst.h"

#include "main.h"
#include "pvsgen.h"
#include "spline.h"
#include "world.h"
#include "movement.h"
#include "render.h"

static RwChar TexDictionaryName[256];
#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))
static RwChar EffectDictionaryName[256];
#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */

RpWorld *World = (RpWorld *)NULL;
RpWorldSector *CurrentWorldSector = (RpWorldSector *)NULL;
RwInt32 CurrentWorldSectorIndex;

RwSphere WorldSphere;

RwBool WorldLoaded = FALSE;
RwBool WorldHasNormals = FALSE;
RwBool WorldHasSpline = FALSE;
RwBool WorldHasTextures = FALSE;
RwBool WorldHasCollData = FALSE;
RwBool WorldHasLightmaps = FALSE;

/*
 *****************************************************************************
 */
void
WorldGetBoundingSphere(RpWorld *world, RwSphere *sphere)
{
    const RwBBox *bbox = (const RwBBox *)NULL;
    RwV3d temp;

    bbox = RpWorldGetBBox(world);

    RwV3dAdd(&temp, &bbox->sup, &bbox->inf);
    RwV3dScale(&sphere->center, &temp, 0.5f);

    RwV3dSub(&temp, &bbox->sup, &bbox->inf);

    sphere->radius = RwV3dLength(&temp) * 0.5f;

    return;
}


/*
 *****************************************************************************
 */
static RpWorldSector *
WorldSectorGetFirst(RpWorldSector *worldSector, void *data)
{
    *(RpWorldSector **)data = worldSector;

    return (RpWorldSector *)NULL;
}


RpWorldSector * 
WorldGetFirstWorldSector(RpWorld *world)
{
    RpWorldSector *worldSector;

    RpWorldForAllWorldSectors(world, WorldSectorGetFirst, &worldSector);

    return worldSector;
}


/*
 *****************************************************************************
 */
static RpWorldSector *
WorldSectorGetLast(RpWorldSector *worldSector, void *data)
{
    static RwInt32 count = 1;

    if( count == RtWorldGetNumWorldSectors(World) )
    {
        *(RpWorldSector **)data = worldSector;

        count = 1;

        return (RpWorldSector *)NULL;
    }

    count++;

    return worldSector;
}


static RpWorldSector * 
WorldGetLastWorldSector(RpWorld *world)
{
    RpWorldSector *worldSector;

    RpWorldForAllWorldSectors(world, WorldSectorGetLast, &worldSector);

    return worldSector;
}


/*
 *****************************************************************************
 */
static RpWorldSector *
WorldSectorSetNext(RpWorldSector *worldSector, void *data)
{
    RwBool *passedCurrentSector = (RwBool *)data;

    if( *passedCurrentSector )
    {
        CurrentWorldSector = worldSector;

        *passedCurrentSector = FALSE;

        return (RpWorldSector *)NULL;
    }

    if( worldSector == CurrentWorldSector )
    {
        *passedCurrentSector = TRUE;
    }

    return worldSector;
}


void 
SelectNextWorldSector(void)
{
    RwBool passedCurrentSector = FALSE;

    RpWorldForAllWorldSectors(World, WorldSectorSetNext, 
        (void *)&passedCurrentSector);

    if( passedCurrentSector )
    {
        /*
         * Current world-sector must be the last one, so
         * make the first one the current...
         */
        CurrentWorldSector = WorldGetFirstWorldSector(World);

        CurrentWorldSectorIndex = 1;
    }
    else
    {
        CurrentWorldSectorIndex++;
    }

    return;
}


/*
 *****************************************************************************
 */
static RpWorldSector *
WorldSectorSetPrevious(RpWorldSector *worldSector, 
                       void * data __RWUNUSED__)
{
    static RpWorldSector *prevSector = (RpWorldSector *)NULL;

    if( worldSector == CurrentWorldSector )
    {
        CurrentWorldSector = prevSector;

        prevSector = (RpWorldSector *)NULL;

        return (RpWorldSector *)NULL;
    }

    prevSector = worldSector;

    return worldSector;
}


void 
SelectPreviousWorldSector(void)
{
    RpWorldForAllWorldSectors(World, WorldSectorSetPrevious, NULL);

    if( CurrentWorldSector )
    {
        CurrentWorldSectorIndex--;
    }
    else
    {
        /*
         * Current world-sector must be the first one, so
         * make the last one the current...
         */
        CurrentWorldSector = WorldGetLastWorldSector(World);

        CurrentWorldSectorIndex = RtWorldGetNumWorldSectors(World);
    }

    return;
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
    RsPathnameDestroy(path);

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
LoadEffectDictionary(RwChar *filename)
{
    RpMTEffectDict *effectDict = (RpMTEffectDict *)NULL;
    RwStream *stream = (RwStream *)NULL;
    RwChar *path = (RwChar *)NULL;

    path = RsPathnameCreate(filename);

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

    RsPathnameDestroy(path);

    return effectDict;
}

#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */


/*
 *****************************************************************************
 */
static RwTexDictionary *
LoadTextureDictionary(RwChar *filename)
{
    RwTexDictionary *texDict = (RwTexDictionary *)NULL;
    RwStream *stream = (RwStream *)NULL;
    RwChar *path = (RwChar *)NULL;

    path = RsPathnameCreate(filename);

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

    RsPathnameDestroy(path);

    return texDict;
}

#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))

static void
EffectDictionaryGetName(RwChar *effectDictionaryName, RwChar *bspPath)
{
    /*
     * Creates a path for the effect dictionary which has the same name 
     * as the BSP and resides in the same directory as the BSP.
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

    rwstrcpy(effectDictionaryName, bspPath);

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
TexDictionaryGetName(RwChar *texDictionaryName, RwChar *bspPath)
{
    /*
     * Creates a path for the texture dictionary which has the same name 
     * as the BSP and resides in the same directory as the BSP.
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

    const RwChar ext[] = RWSTRING(".txd");
    
#endif

    rwstrcpy(texDictionaryName, bspPath);

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
LoadWorld(RwChar *bspPath)
{
    RwStream *stream = (RwStream *)NULL;
    RpWorld *world = (RpWorld *)NULL;
    RwChar *path = (RwChar *)NULL;
    RwTexDictionary *prevTexDict = (RwTexDictionary *)NULL;
    RwTexDictionary *texDict = (RwTexDictionary *)NULL;
#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))
    RpMTEffectDict *prevEffectDict = (RpMTEffectDict *)NULL;
    RpMTEffectDict *effectDict = (RpMTEffectDict *)NULL;
#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */

    /* set multi-texture effect file path on platforms that do it... */
#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))
    RwChar effectPath[256], *effectPathEnding = (char *)NULL, *psEffectPath;

    rwstrcpy(effectPath, bspPath);
    effectPathEnding = rwstrstr(effectPath, RWSTRING(".bsp"));
    if (effectPathEnding)
    {
        rwstrcpy(effectPathEnding, "/");
    }
    psEffectPath = RsPathnameCreate(effectPath);
    RpMTEffectSetPath(psEffectPath);
    RsPathnameDestroy(psEffectPath);
#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */

    path = RsPathnameCreate(bspPath);

    /* load multi-texture effect dictionary on platforms that do it... */
#if (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H))
    /*
     * Remember the current dictionary so that it can be 
     * reinstated if the BSP load fails...
     */
    prevEffectDict = RpMTEffectDictGetCurrent();

    /*
     * Attempt to load a texture dictionary...
     */
    EffectDictionaryGetName(EffectDictionaryName, path);

    effectDict = LoadEffectDictionary(EffectDictionaryName);
    if( effectDict )
    {
        /* set the new effect dictionary as the current */
        RpMTEffectDictSetCurrent(effectDict);
    }
#endif /* (defined(XBOX_DRVMODEL_H) || defined(GCN_DRVMODEL_H)) */

    /*
     * Remember the current dictionary so that it can be 
     * reinstated if the BSP load fails...
     */
    prevTexDict = RwTexDictionaryGetCurrent();

    /*
     * Attempt to load a texture dictionary...
     */
    TexDictionaryGetName(TexDictionaryName, path);
    
    texDict = LoadTextureDictionary(TexDictionaryName);
    if( texDict )
    {
        /*
         * A texture dictionary is available, so make it the current
         * one before the loading the BSP...
         */
        RwTexDictionarySetCurrent(texDict);
    }
    else
    {
        /*
         * No texture dictionary available, so create a new empty
         * dictionary and make it the current one. This dictionary
         * will be populated with textures (if any) when the BSP is 
         * loaded. If textures have been loaded along with the BSP, 
         * we can save this dictionary, so it may be loaded directly
         * next time round...
         */
        RwTexDictionarySetCurrent(RwTexDictionaryCreate());

        RsSetModelTexturePath(path);
    }

    /*
     * Now load the BSP...
     */
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_WORLD, 
                              (RwUInt32 *)NULL, (RwUInt32 *)NULL) )
        {
            world = RpWorldStreamRead(stream);

            RwStreamClose (stream, NULL);
        }
    }

    RsPathnameDestroy(path);

    if( world )
    {
        if( texDict )
        {
            /*
             * Pretend there's no textures, so we can disable the menu
             * item that saves the texture dictionary - we don't need to
             * resave it...
             */
            WorldHasTextures = FALSE;
        }
        else
        {
            WorldHasTextures = FALSE;

            RwTexDictionaryForAllTextures(RwTexDictionaryGetCurrent(), 
                TextureTest, (void *)&WorldHasTextures);
        }

        /*
         * Remove all the lights and camera from the current world before 
         * destroying it and the texture dictionary it was using...
         */
        if( AmbientLightOn )
        {
            RpWorldRemoveLight(World, AmbientLight);

            AmbientLightOn = FALSE;
        }

        if( MainLightOn )
        {
            RpWorldRemoveLight(World, MainLight);

            MainLightOn = FALSE;
        }

        RpWorldRemoveCamera(World, Camera);

        /*
         * Cast away the old world...
         */
        RpWorldDestroy(World);
        RwTexDictionaryDestroy(prevTexDict);

        /*
         * ...and introduce the new world order...
         */
        World = world;

        /*
         * If the world is prelit, lighting is not applied to the
         * world, but can be added later by the user...
         */
        if( !(RpWorldGetFlags(world) & rpWORLDPRELIT) )
        {
            RpWorldAddLight(world, AmbientLight);

            RpWorldAddLight(world, MainLight);

            AmbientLightOn = MainLightOn = TRUE;
        }

        /*
         * Test for vertex normals...
         */
        if( RpWorldGetFlags(world) & rpWORLDNORMALS )
        {
            WorldHasNormals = TRUE;
        }
        else
        {
            WorldHasNormals = FALSE;
        }

        RpWorldAddCamera(world, Camera);

        RpPVSSetProgressCallBack(world, PVSProgressCallback);

        /*
         * See if the world has a spline, assuming the name of the
         * spline is the same as the world...
         */       
        WorldHasSpline = LoadSpline(bspPath, world);

        /*
         * See if the world is lightmaped
         */       
        WorldHasLightmaps = (RpLtMapWorldLightMapsQuery(world)  > 0);

        /*  
         * Does the new world have PVS...
         */
        PVSOn = RpPVSQuery(world);

        /* does it have collision data? */
        WorldHasCollData = RpCollisionWorldQueryData(world);

        CurrentWorldSector = WorldGetFirstWorldSector(world);
        CurrentWorldSectorIndex = 1;
        SingleSectorOn = FALSE;

        WorldLoaded = TRUE;

        return TRUE;
    }

    /*
     * The BSP failed to load so reinstate the original texture dictionary...
     */
    RwTexDictionaryDestroy(RwTexDictionaryGetCurrent());
    RwTexDictionarySetCurrent(prevTexDict);

    return FALSE;
}


/*
 *****************************************************************************
 */
RwBool 
SaveWorld(void)
{
    RwStream *stream = (RwStream *)NULL;
    RwBool success = TRUE;
    RwChar *path;

    path = RsPathnameCreate(RWSTRING("./new.bsp"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        if( !RpWorldStreamWrite(World, stream) )
        {   
            RsErrorMessage(RWSTRING("Cannot write BSP file."));

            success = FALSE;
        }

        RwStreamClose(stream, NULL);
    }
    else
    {
        RsErrorMessage(RWSTRING("Cannot open stream to write BSP file."));

        success =  FALSE;
    }

    return success;
}

/*
 *****************************************************************************
 */
