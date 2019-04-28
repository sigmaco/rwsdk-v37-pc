
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
 * Copyright (c) 2002 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * main.c
 *
 * Copyright (C) 2002 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Starting point for any new RenderWare demo using the demo
 * skeleton.
 *
 ****************************************************************************/

#include <string.h>

#ifdef _WINDOWS
#include <d3d9.h>
#endif

#include "rwcore.h"
#include "rpworld.h"
#include "rtfsyst.h"

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rphanim.h"
#include "rpskin.h"
#include "rpnormmap.h"

#include "rtcharse.h"

#ifndef XBOX_DRVMODEL_H
#include "rtquat.h"
#include "rtslerp.h"
#endif

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#include "main.h"
#include "hanim.h"
#include "normalmapgen.h"

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


struct ClumpInfoType
{
    const RwChar *fileName;
    const RwChar *fileNameNormalMapTextureSpace;
    const RwChar *fileNameAnimation;
    RwReal pos;
    RwReal angleX, angleY;
};
typedef struct ClumpInfoType ClumpInfoType;


static RwRGBA ForegroundColor = {200, 200, 200, 255};
static RwRGBA BackgroundColor = { 64,  64,  64,   0};

static const RwV3d Xaxis = { 1.0f, 0.0f, 0.0f };
static const RwV3d Yaxis = { 0.0f, 1.0f, 0.0f };

static RwBool FPSOn = FALSE;

static RwInt32 FrameCounter = 0;
static RwInt32 FramesPerSecond = 0;

static RtCharset *Charset = NULL;

static RpWorld *World = NULL;
static RwBool WorldRender = FALSE;
static RwReal FarPlane = 1000.f;

static RwCamera *Camera = NULL;

static RpClump *Clump= NULL;

static RpLight *AmbientLight = (RpLight *)NULL;
static RwReal AmbientIntensity = 0.30f;
static RwBool AmbientLightOn = TRUE;

static RpLight *MainLight = (RpLight *)NULL;
static RwRGBAReal MainLightColor;

static RwBool NormalMapOn = TRUE;
static RwBool NormalMapEnvMapOn = FALSE;
static RwBool NormalMapModulateEnvMap = FALSE;

static const RwChar *WorldFileName = RWSTRING("models/example.bsp");

static const ClumpInfoType ClumpsInfo[] =
{
    {
        RWSTRING("models/clobsterwalk.dff"),
        RWSTRING("clobster_ts"),
        RWSTRING("clobsterwalk"),
        20.0f,
        0.0f, -90.0f
    },
    {
        RWSTRING("models/dice.dff"),
        RWSTRING("dice_ts"),
        NULL,
        2.25f,
        0.0f, 0.0f
    },
    {
        RWSTRING("models/sphere.dff"),
        RWSTRING("bumpdog_ts"),
        NULL,
        2.f,
        90.0f, -90.0f
    },
    {
        RWSTRING("models/bucky.dff"),
        RWSTRING("wood_ts"),
        NULL,
        3.25f,
        0.0f, 0.0f
    },
    {
        RWSTRING("models/goblet.dff"),
        RWSTRING("ripples_ts"),
        NULL,
        2.75f,
        -90.0f, 0.0f
    }
};

static RwUInt32 ActiveClump = 0;

#define ENVMAP_WITHFRAME    2

static const RwChar *EnvMapFileName[] =
{
    RWSTRING("gloss"),
    RWSTRING("office"),
    RWSTRING("highlite")
};
static RwUInt32 ActiveEnvMap = 0;

static RwReal EnvMapShininess = 0.75f;

static RwFrame *HighliteFrame = NULL;

/*
 *****************************************************************************
 */
static RwBool
LoadClump(void);

static void
DestroyClump(void);

static RwBool
LoadWorld(const RwChar *bspPath);

static RpMaterial *
MaterialGenerateNormalMapTextureSpace(RpMaterial *material, void *data);

static RpWorldSector *
WorldSectorInitialize(RpWorldSector *worldsector, void *data);

/*
 *****************************************************************************
 */
static RwCamera *
CreateCamera(RpWorld *world)
{
    RwCamera *camera;

    camera = CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);
    if( camera )
    {
        RwFrame *clumpFrame;
        const RwBBox *bbox;

        RwCameraSetNearClipPlane(camera, 1.0f);
        
        RwCameraSetFarClipPlane(camera, 100.0f);

        bbox = RpWorldGetBBox(World);

        FarPlane = max((bbox->sup.z - bbox->inf.z), (bbox->sup.x - bbox->inf.x));

        clumpFrame = RwCameraGetFrame(camera);

        if (WorldRender)
        {
            const RwV3d NewPos = {0.0f, 500.0f, 0.0f};

            RwFrameTranslate(clumpFrame, &NewPos, rwCOMBINEREPLACE);
        }
        else
        {
            const RwV3d NewPos = {0.0f, 0.0f, -ClumpsInfo[ActiveClump].pos};

            RwFrameTranslate(clumpFrame, &NewPos, rwCOMBINEREPLACE);
        }

        RpWorldAddCamera(world, camera);

        return camera;
    }

    return NULL;
}

/*
 *****************************************************************************
 */
static RpLight*
CreateMainLight(RpWorld *world)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTDIRECTIONAL);

    if( light )
    {
        RwFrame *frame;
        RwRGBAReal color;

        MainLightColor.red = color.red = 1.f;
        MainLightColor.green = color.green = 1.f;
        MainLightColor.blue = color.blue = 1.f;
        MainLightColor.alpha = color.alpha = 1.0f;

        RpLightSetColor(light, &color);

        frame = RwFrameCreate();

        if( frame )
        {
            RwFrameRotate(frame, &Xaxis, 30.0f, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &Yaxis, 80.0f, rwCOMBINEPOSTCONCAT);

            RpLightSetFrame(light, frame);

            RpLightSetRadius(light, FarPlane / 5.f);

            RpLightSetConeAngle(light, 3.1415927f / 4);

            RpWorldAddLight(world, light);

            return light;
        }

        RpLightDestroy(light);
    }

    return (RpLight *)NULL;
}


/*
 *****************************************************************************
 */
static RpLight*
CreateAmbientLight(RpWorld *world)
{
    RpLight *light;

    light = RpLightCreate(rpLIGHTAMBIENT);

    if( light )
    {
        RwRGBAReal color;

        color.red = color.green = color.blue = AmbientIntensity;
        color.alpha = 1.0f;

        RpLightSetColor(light, &color);

        RpWorldAddLight(world, light);

        return light;
    }

    return (RpLight *)NULL;
}

/*
 *****************************************************************************
 */
static void 
TexDictionaryGetName(RwChar *texDictionaryName, const RwChar *bspPath)
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
#elif (defined(XBOX_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_xbox.txd");
#elif (defined(OPENGL_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_ogl.txd");
#elif (defined(GCN_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_gcn.txd");
#elif (defined(SKY2_DRVMODEL_H))
    const RwChar ext[] = RWSTRING("_ps2.txd");
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
}

/*
 *****************************************************************************
 */
static RwTexDictionary *
LoadTextureDictionary(const RwChar *filename)
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

/*
 *****************************************************************************
 */
RwBool
SaveTextureDictionary(const RwChar *filename)
{
    RwStream *stream = (RwStream *)NULL;
    RwChar *path = (RwChar *)NULL;
    RwBool success = FALSE;

    path = RsPathnameCreate(filename);
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
static RwBool 
SaveWorld(void)
{
    RwStream *stream = (RwStream *)NULL;
    RwBool success = TRUE;
    RwChar *path;

    path = RsPathnameCreate(RWSTRING("new.bsp"));
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
static RwBool
LoadWorld(const RwChar *bspPath)
{
    RwStream *stream = (RwStream *)NULL;
    RpWorld *world = (RpWorld *)NULL;
    RwChar *path = (RwChar *)NULL;
    RwTexDictionary *prevTexDict = (RwTexDictionary *)NULL;
    RwTexDictionary *texDict = (RwTexDictionary *)NULL;
    RwChar TexDictionaryName[256];

    path = RsPathnameCreate(bspPath);

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
        /* remove some flags we don't want for this example  */
        RwUInt32 flags = RpWorldGetFlags(world);

        flags &= ~rpWORLDPRELIT;
        flags &= ~rpWORLDTEXTURED2;
        flags &= ~(rpWORLDTEXCOORDSETS(0xff));

        flags |= rpWORLDLIGHT;
        flags |= rpWORLDTEXTURED;
        flags |= rpWORLDTEXCOORDSETS(1);

        RpWorldSetFlags(world, flags);

        world->numTexCoordSets = 1;

        /*
         * Remove all the lights and camera from the current world before 
         * destroying it and the texture dictionary it was using...
         */
        if (World != NULL)
        {
            if( AmbientLightOn )
            {
                RpWorldRemoveLight(World, AmbientLight);
            }

            if( MainLight != NULL )
            {
                RpWorldRemoveLight(World, MainLight);
            }

            RpWorldRemoveCamera(World, Camera);

            /*
             * Cast away the old world...
             */
            RpWorldDestroy(World);
        }

        if (prevTexDict != NULL)
        {
            RwTexDictionaryDestroy(prevTexDict);
        }

        /*
         * ...and introduce the new world order...
         */
        World = world;

        /* Apply normal map texture space plugin*/
        if (NormalMapOn)
        {
            RpNormMapWorldEnable(World);

            RpWorldForAllWorldSectors(World,
                                      WorldSectorInitialize,
                                      NULL);

            RpWorldForAllMaterials(World,
                                      MaterialGenerateNormalMapTextureSpace,
                                      NULL);
        }

        if (texDict == NULL)
        {
            SaveTextureDictionary(TexDictionaryName);
        }

#if 0
        SaveWorld();
#endif

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
static RwBool
AmbientLightOnCB(RwBool justCheck)
{
    if( justCheck )
    {
        return TRUE;
    }

    if( AmbientLightOn )
    {
        RpWorldAddLight(World, AmbientLight);
    }
    else
    {
        RpWorldRemoveLight(World, AmbientLight);
    }

    return TRUE;
}

/*
 ***************************************************************************
 */
static RpAtomic *
AtomicApplyNormalMapTexture(RpAtomic *atomic, void *data)
{
	RwTexture *texture = (RwTexture *)data;

    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
                              RpNormMapMaterialSetNormMapTexture,
                              texture);

    return (atomic);
}

/*
 ***************************************************************************
 */
static RpAtomic *
AtomicApplyEnvMapTexture(RpAtomic *atomic, void *data)
{
	RwTexture *texture = (RwTexture *)data;

    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
                              RpNormMapMaterialSetEnvMapTexture,
                              texture);

    return (atomic);
}

/*
 ***************************************************************************
 */
static RpMaterial *
MaterialApplyEnvMapShininess(RpMaterial *material, RwReal *shininess)
{
    RpNormMapMaterialSetEnvMapCoefficient(material, *shininess);

    return material;
}

/*
 ***************************************************************************
 */
static RpAtomic *
AtomicApplyEnvMapShininess(RpAtomic *atomic, void *data)
{
	RwReal *shininess = (RwReal *)data;

    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
                              MaterialApplyEnvMapShininess,
                              shininess);

    return (atomic);
}

/*
 ***************************************************************************
 */
static RpAtomic *
AtomicApplyEnvMapFrame(RpAtomic *atomic, void *data)
{
	RwFrame *frame = (RwFrame *)data;

    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
                              RpNormMapMaterialSetEnvMapFrame,
                              frame);

    return (atomic);
}

/*
 ***************************************************************************
 */
static RpMaterial *
MaterialApplyModulateEnvMap(RpMaterial *material, RwBool *modulate)
{
    RpNormMapMaterialModulateEnvMap(material, *modulate);

    return material;
}

/*
 ***************************************************************************
 */
static RpAtomic *
AtomicApplyModulateEnvMap(RpAtomic *atomic, void *data)
{
	RwBool *modulate = (RwBool *)data;

    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
                              MaterialApplyModulateEnvMap,
                              modulate);

    return (atomic);
}

/*
 ***************************************************************************
 */
static RpMaterial *
MaterialGenerateNormalMapTextureSpace(RpMaterial *material, void *data)
{
    RwTexture *texture, *normalmap, *envmap;
    const RwChar *name;
    RwUInt32 len;
    RwChar normalmapname[rwTEXTUREBASENAMELENGTH];
    RwTexDictionary *texDictionary;

    texture = material->texture;

    if (texture != NULL)
    {
        name = RwTextureGetName(texture);
        len = rwstrlen(name);

        if (len > rwTEXTUREBASENAMELENGTH - 4)
        {
            len = rwTEXTUREBASENAMELENGTH - 4;
        }
        memcpy(normalmapname, name, len);
        memcpy(normalmapname + len, "_ts", 4);

        texDictionary = RwTextureGetDictionary(texture);
    }
    else
    {
        rwstrcpy(normalmapname, "notexture_ts");

        texDictionary = RwTexDictionaryGetCurrent();
    }

    normalmap = RwTexDictionaryFindNamedTexture(texDictionary, normalmapname);

    if (normalmap == NULL)
    {
        normalmap = RwTextureRead(normalmapname, RWSTRING(""));
    }

    if (normalmap == NULL)
    {
        normalmap = NormalMapTextureSpaceCreateFromTexture(texture, 0.1f);

        if (normalmap != NULL)
        {
            RwTextureSetName(normalmap, normalmapname);

            RwTexDictionaryAddTexture(texDictionary, normalmap);
        }
    }

    RwTextureSetFilterMode(normalmap, rwFILTERLINEARMIPLINEAR);

    RpNormMapMaterialSetNormMapTexture(material, normalmap);

    if (NormalMapEnvMapOn)
    {
        envmap = RwTexDictionaryFindNamedTexture(texDictionary, EnvMapFileName[ActiveEnvMap]);

        if (envmap == NULL)
        {
            envmap = RwTextureRead(EnvMapFileName[ActiveEnvMap], RWSTRING(""));
        }

        RwTextureSetFilterMode(envmap, rwFILTERLINEARMIPLINEAR);
        
        if (ActiveEnvMap == ENVMAP_WITHFRAME)
        {
            RwTextureSetAddressing(envmap, rwTEXTUREADDRESSCLAMP);
        }
        else
        {
            RwTextureSetAddressing(envmap, rwTEXTUREADDRESSWRAP);
        }

        RpNormMapMaterialSetEnvMapTexture(material, envmap);

        RpNormMapMaterialSetEnvMapCoefficient(material, EnvMapShininess);

        if (ActiveEnvMap == ENVMAP_WITHFRAME)
        {
            RpNormMapMaterialSetEnvMapFrame(material, HighliteFrame);
        }
        else
        {
            RpNormMapMaterialSetEnvMapFrame(material, NULL);
        }

        RpNormMapMaterialModulateEnvMap(material, NormalMapModulateEnvMap);
    }
    else
    {
        RpNormMapMaterialSetEnvMapTexture(material, NULL);
    }

    return material;
}

/*
 ***************************************************************************
 */
static RpAtomic *
AtomicNormalMapTextureSpaceEnable(RpAtomic *atomic, void *data)
{
    RpNormMapAtomicInitialize(atomic, (RpNormMapAtomicPipeline)data);

    return (atomic);
}

/*
 ***************************************************************************
 */
static RpAtomic *
AtomicSetPipeline(RpAtomic *atomic, void *data)
{
    RpAtomicSetPipeline(atomic, (RxPipeline *)data);

    return (atomic);
}

/*
 ***************************************************************************
 */
static RpWorldSector *
WorldSectorSetPipeline(RpWorldSector *worldsector, void *data)
{
    RpWorldSectorSetPipeline(worldsector, (RxPipeline *)data);

    return (worldsector);
}

/*
 ***************************************************************************
 */
static RpWorldSector *
WorldSectorInitialize(RpWorldSector *worldsector, void *data)
{
    RpNormMapWorldSectorInitialize(worldsector);

    return (worldsector);
}

/*
 *****************************************************************************
 */
static RwBool
NormalMapOnCB(RwBool justCheck)
{
    if( justCheck )
    {
        return TRUE;
    }

    RwResourcesEmptyArena();

    if (NormalMapOn)
    {
        if (ClumpsInfo[ActiveClump].fileNameAnimation)
        {
            RpClumpForAllAtomics(Clump, AtomicSetPipeline,
            (void *)RpNormMapGetAtomicPipeline(rpNORMMAPATOMICSKINNEDPIPELINE));
        }
        else
        {
            RpClumpForAllAtomics(Clump, AtomicSetPipeline,
            (void *)RpNormMapGetAtomicPipeline(rpNORMMAPATOMICSTATICPIPELINE));
        }

        RpWorldForAllWorldSectors(World, WorldSectorSetPipeline,
            (void *)RpNormMapGetWorldSectorPipeline());
    }
    else
    {
        if (ClumpsInfo[ActiveClump].fileNameAnimation)
        {
            #if defined(D3D9_DRVMODEL_H)
            RpClumpForAllAtomics(Clump, AtomicSetPipeline,
                    (void *)RpSkinGetD3D9Pipeline(rpSKIND3D9PIPELINEGENERIC));
            #elif defined(XBOX_DRVMODEL_H)
            RpClumpForAllAtomics(Clump, AtomicSetPipeline,
                    (void *)RpSkinGetXboxPipeline(rpSKINXBOXPIPELINEGENERIC));
            #endif
        }
        else
        {
            RpClumpForAllAtomics(Clump, AtomicSetPipeline, (void *)RpAtomicGetDefaultPipeline());
        }

        RpWorldForAllWorldSectors(World, WorldSectorSetPipeline,
            (void *)RpWorldGetDefaultSectorPipeline());
    }

    if (NormalMapOn)
    {
        RpWorldForAllMaterials(World,
                                  MaterialGenerateNormalMapTextureSpace,
                                  NULL);
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
NewEnvMapCB(RwBool justCheck)
{
    if( justCheck )
    {
        return (NormalMapOn && NormalMapEnvMapOn);
    }
    else
    {
        RwTexture *envMapTexture;

        /* World */
        envMapTexture = RwTexDictionaryFindNamedTexture(RwTexDictionaryGetCurrent(),
                                            EnvMapFileName[ActiveEnvMap]);

        if (envMapTexture == NULL)
        {
            envMapTexture = RwTextureRead(EnvMapFileName[ActiveEnvMap], RWSTRING(""));
        }

        RwTextureSetFilterMode(envMapTexture, rwFILTERLINEARMIPLINEAR);

        if (ActiveEnvMap == ENVMAP_WITHFRAME)
        {
            RwTextureSetAddressing(envMapTexture, rwTEXTUREADDRESSCLAMP);
        }
        else
        {
            RwTextureSetAddressing(envMapTexture, rwTEXTUREADDRESSWRAP);
        }

        RpWorldForAllMaterials(World, RpNormMapMaterialSetEnvMapTexture, envMapTexture);

        RpWorldForAllMaterials(World, MaterialApplyEnvMapShininess, &EnvMapShininess);

        if (ActiveEnvMap == ENVMAP_WITHFRAME)
        {
            RpWorldForAllMaterials(World, RpNormMapMaterialSetEnvMapFrame, HighliteFrame);
        }
        else
        {
            RpWorldForAllMaterials(World, RpNormMapMaterialSetEnvMapFrame, NULL);
        }

        /* Clump */
        envMapTexture = RwTextureRead(EnvMapFileName[ActiveEnvMap], RWSTRING(""));

        RwTextureSetFilterMode(envMapTexture, rwFILTERLINEARMIPLINEAR);

        if (ActiveEnvMap == ENVMAP_WITHFRAME)
        {
            RwTextureSetAddressing(envMapTexture, rwTEXTUREADDRESSCLAMP);
        }
        else
        {
            RwTextureSetAddressing(envMapTexture, rwTEXTUREADDRESSWRAP);
        }

        RpClumpForAllAtomics(Clump, AtomicApplyEnvMapTexture, envMapTexture);
        
        RwTextureDestroy(envMapTexture);

        RpClumpForAllAtomics(Clump, AtomicApplyEnvMapShininess, &EnvMapShininess);

        if (ActiveEnvMap == ENVMAP_WITHFRAME)
        {
            RpClumpForAllAtomics(Clump, AtomicApplyEnvMapFrame, HighliteFrame);
        }
        else
        {
            RpClumpForAllAtomics(Clump, AtomicApplyEnvMapFrame, NULL);
        }

        return TRUE;
    }

}

/*
 *****************************************************************************
 */
static RwBool
NormalMapEnvMapOnCB(RwBool justCheck)
{
    if( justCheck )
    {
        return NormalMapOn;
    }

    if (NormalMapEnvMapOn)
    {
        NewEnvMapCB(FALSE);
    }
    else
    {
        RpWorldForAllMaterials(World, RpNormMapMaterialSetEnvMapTexture, NULL);

        RpClumpForAllAtomics(Clump, AtomicApplyEnvMapTexture, NULL);
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
EnvMapShininessCB(RwBool justCheck)
{
    if( justCheck )
    {
        return (NormalMapOn && NormalMapEnvMapOn);
    }

    RpWorldForAllMaterials(World, MaterialApplyEnvMapShininess, &EnvMapShininess);

    RpClumpForAllAtomics(Clump, AtomicApplyEnvMapShininess, &EnvMapShininess);

    return TRUE;
}

/*
 *****************************************************************************
 */
static void
UpdateHighlightFrame(RwFrame *lightFrame, RwFrame *cameraFrame)
{
    RwMatrix *lightMatrix;
    RwMatrix *cameraMatrix;
    RwMatrix *highlightMatrix;

    lightMatrix = RwFrameGetLTM(lightFrame);
    cameraMatrix = RwFrameGetLTM(cameraFrame);
    highlightMatrix = RwFrameGetLTM(HighliteFrame);

    /*
     * Simulate Specular only for pixel shaders 1.4
     */
#ifdef XBOX_DRVMODEL_H

    RwMatrixCopy(highlightMatrix, lightMatrix);

#else
    if ( (((const D3DCAPS9 *)RwD3D9GetCaps())->PixelShaderVersion & 0xFFFF) < 0x0104 )
    {
        RwMatrixCopy(highlightMatrix, lightMatrix);
    }
    else
    {
        RtQuat lightQuat, cameraQuat, highlightQuat;
        RtQuatSlerpCache slerpCache;

        RtQuatConvertFromMatrix(&lightQuat, lightMatrix);
        RtQuatConvertFromMatrix(&cameraQuat, cameraMatrix);

        RtQuatSetupSlerpCache(&cameraQuat, &lightQuat, &slerpCache);

        RtQuatSlerp(&highlightQuat, &cameraQuat, &lightQuat, 0.5f, &slerpCache);

        RtQuatConvertToMatrix(&highlightQuat, highlightMatrix);
    }
#endif
}

/*
 *****************************************************************************
 */
static RwBool
NormalMapModulateEnvMapCB(RwBool justCheck)
{
    if( justCheck )
    {
        return (NormalMapOn && NormalMapEnvMapOn);
    }

    RpWorldForAllMaterials(World, MaterialApplyModulateEnvMap, &NormalMapModulateEnvMap);

    RpClumpForAllAtomics(Clump, AtomicApplyModulateEnvMap, &NormalMapModulateEnvMap);

    if (ActiveEnvMap == ENVMAP_WITHFRAME)
    {
        UpdateHighlightFrame(RpLightGetFrame(MainLight), RwCameraGetFrame(Camera));
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
void
MainLightRotate(RwReal x, RwReal y)
{
    RwRaster *cameraRaster;
    RwMatrix *matrixCamera;
    RwMatrix *matrix;
    RwReal anglex, angley;

    cameraRaster = RwCameraGetRaster(Camera);

    matrixCamera = RwFrameGetLTM(RwCameraGetFrame(Camera));

    matrix = RwFrameGetLTM(RpLightGetFrame(MainLight));

    RwMatrixCopy(matrix, matrixCamera);

    anglex = ( ( ( -180.0f * y ) / RwRasterGetHeight(cameraRaster) ) + 90.0f );
    if (anglex > 90.0f)
    {
        anglex = 90.0f;
    }
    else if (anglex < -90.0f)
    {
        anglex = -90.0f;
    }

    angley = ( ( ( 180.0f * x ) / RwRasterGetWidth(cameraRaster) ) - 90.0f );
    if (angley > 90.0f)
    {
        angley = 90.0f;
    }
    else if (angley < -90.0f)
    {
        angley = -90.0f;
    }

    RwMatrixRotate(matrix, &Xaxis, anglex, rwCOMBINEPOSTCONCAT);
    RwMatrixRotate(matrix, &Yaxis, angley, rwCOMBINEPOSTCONCAT);

    if (ActiveEnvMap == ENVMAP_WITHFRAME)
    {
        UpdateHighlightFrame(RpLightGetFrame(MainLight), RwCameraGetFrame(Camera));
    }
}

/*
 *****************************************************************************
 */
void
ClumpsRotate(RwReal x, RwReal y)
{
    if (WorldRender)
    {
        RwFrame *frame;
        const RwV3d *right;
        RwV3d pos, delta;
        RwReal angleX, angleY;

        angleX = ( -x / 10 );
        angleY = ( y / 20 );

        frame = RwCameraGetFrame(Camera);

        /*
         * Remember where the camera is...
         */
        pos = *RwMatrixGetPos(RwFrameGetMatrix(frame));

        /*
         * Remove the translation component...
         */
        RwV3dScale(&delta, &pos, -1.0f);
        RwFrameTranslate(frame, &delta, rwCOMBINEPOSTCONCAT);

        /*
         * Rotate to the new direction...
         */
        right = RwMatrixGetRight(RwFrameGetMatrix(frame));
        RwFrameRotate(frame, right, angleY, rwCOMBINEPOSTCONCAT);
        RwFrameRotate(frame, &Yaxis, angleX, rwCOMBINEPOSTCONCAT);

        /*
         * Put the camera back to where it was...
         */
        RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
    }
    else
    {
        RwRaster *cameraRaster;
        RwFrame *clumpFrame;
        RwReal angleX, angleY;

        angleX = ( x / 10 );
        angleY = ( - y / 10 );

        cameraRaster = RwCameraGetRaster(Camera);

        clumpFrame = RpClumpGetFrame(Clump);

        RwFrameRotate(clumpFrame, &Xaxis, angleY, rwCOMBINEPOSTCONCAT);
        RwFrameRotate(clumpFrame, &Yaxis, angleX, rwCOMBINEPOSTCONCAT);
    }
}

/*
 *****************************************************************************
 */
void
ClumpsTranslate(RwReal x, RwReal y)
{
    RwV3d NewPos;
    RwFrame *frame;
    RwMatrix *matrix;
    const RwV3d *right;
    const RwV3d *up;

    frame = RwCameraGetFrame(Camera);

    matrix = RwFrameGetLTM(frame);
    right = RwMatrixGetRight(matrix);
    up = RwMatrixGetUp(matrix);

    if (WorldRender)
    {
        RwV3dScale(&NewPos, right, -(x * FarPlane / 1000.0f));
    }
    else
    {
        RwV3dScale(&NewPos, right, x * 0.05f);
    }

    if (WorldRender)
    {
        RwV3dIncrementScaled(&NewPos, up, -(y * FarPlane / 1000.0f));
    }
    else
    {
        RwV3dIncrementScaled(&NewPos, up, y * 0.05f);
    }

    RwFrameTranslate(frame, &NewPos, rwCOMBINEPOSTCONCAT);
}

/*
 *****************************************************************************
 */
void
ClumpsZoom(RwReal y)
{
    RwV3d NewPos;
    RwFrame *frame;
    RwMatrix *matrix;
    const RwV3d *at;

    frame = RwCameraGetFrame(Camera);

    matrix = RwFrameGetLTM(frame);
    at = RwMatrixGetAt(matrix);

    if (WorldRender)
    {
        RwV3dScale(&NewPos, at, -(y * FarPlane / 1000.f));
    }
    else
    {
        RwV3dScale(&NewPos, at, y * 0.5f);
    }

    RwFrameTranslate(frame, &NewPos, rwCOMBINEPOSTCONCAT);
}

/*
 *****************************************************************************
 */
static RwBool
LightsUpdateCB(RwBool justCheck)
{
    RwRGBAReal color;

    if( justCheck )
    {
        return TRUE;
    }

    color.red = color.green = color.blue = AmbientIntensity;
    color.alpha = 1.0f;

    RpLightSetColor(AmbientLight, &color);

    RpLightSetColor(MainLight, &MainLightColor);

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
ClumpLoadCB(RwBool justCheck)
{
    if( justCheck )
    {
        return (WorldRender == FALSE);
    }
    else
    {
        const RwV3d NewPos = {0.0f, 0.0f, -ClumpsInfo[ActiveClump].pos};
        RwFrame *frame;

        DestroyClump();

        LoadClump();

        frame = RwCameraGetFrame(Camera);
        RwFrameTranslate(frame, &NewPos, rwCOMBINEREPLACE);

        return TRUE;
    }

}

/*
 *****************************************************************************
 */
static RwBool
WorldRenderCB(RwBool justCheck)
{
    RwFrame *frame;

    if( justCheck )
    {
        return TRUE;
    }

    frame = RwCameraGetFrame(Camera);

    if (WorldRender == FALSE)
    {
        const RwV3d NewPos = {0.0f, 0.0f, -ClumpsInfo[ActiveClump].pos};

        RwFrameTranslate(frame, &NewPos, rwCOMBINEREPLACE);

        RwCameraSetNearClipPlane(Camera, 1.0f);

        RwCameraSetFarClipPlane(Camera, 100.0f);
    }
    else
    {
        const RwV3d NewPos = {0.0f, 500.0f, 0.0f};

        RwFrameTranslate(frame, &NewPos, rwCOMBINEREPLACE);

        RwCameraSetNearClipPlane(Camera, FarPlane / 3000);

        RwCameraSetFarClipPlane(Camera, FarPlane);
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

        RsGlobal.appName = RWSTRING("RenderWare Graphics NormalMap Example");

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
    static RwChar clumpLabel[] = RWSTRING("Clump_C");
    static RwChar worldLabel[] = RWSTRING("World_W");

    static RwChar ambientLightOnLabel[] = RWSTRING("Ambient light_A");
    static RwChar ambientLightLabel[]   = RWSTRING("Ambient intensity");

    static RwChar mainLightRedLabel[]       = RWSTRING("Main light Red");
    static RwChar mainLightGreenLabel[]     = RWSTRING("Main light Green");
    static RwChar mainLightBlueLabel[]      = RWSTRING("Main light Blue");

    static RwChar normalmapLabel[]                  = RWSTRING("Normal map_N");
    static RwChar normalmapenvmapLabel[]            = RWSTRING("Env map_E");
    static RwChar envmapshininessLabel[]            = RWSTRING("Shininess");
    static RwChar normalmapmodulateenvmapLabel[]    = RWSTRING("Modulate Env map_M");
    static RwChar envmaptextureLabel[]              = RWSTRING("Env map texture_T");

    static RwChar fpsLabel[] = RWSTRING("FPS_F");

    if( MenuOpen(TRUE, &ForegroundColor, &BackgroundColor) )
    {
        MenuAddEntryInt(clumpLabel, &ActiveClump, 
                         ClumpLoadCB, 0, (sizeof(ClumpsInfo) / sizeof(ClumpInfoType)) - 1, 1, NULL);

        MenuAddEntryBool(worldLabel, &WorldRender, WorldRenderCB);

        MenuAddSeparator();

        MenuAddEntryBool(ambientLightOnLabel, &AmbientLightOn, AmbientLightOnCB);

        MenuAddEntryReal(ambientLightLabel, &AmbientIntensity, 
                         LightsUpdateCB, 0.0f, 1.0f, 0.1f);

        MenuAddSeparator();

        MenuAddEntryReal(mainLightRedLabel, &(MainLightColor.red),
                         LightsUpdateCB, 0.0f, 1.0f, 0.1f);

        MenuAddEntryReal(mainLightGreenLabel, &(MainLightColor.green), 
                         LightsUpdateCB, 0.0f, 1.0f, 0.1f);

        MenuAddEntryReal(mainLightBlueLabel, &(MainLightColor.blue), 
                         LightsUpdateCB, 0.0f, 1.0f, 0.1f);

        MenuAddSeparator();

        MenuAddEntryBool(normalmapLabel, &NormalMapOn, NormalMapOnCB);

        MenuAddEntryBool(normalmapenvmapLabel, &NormalMapEnvMapOn, NormalMapEnvMapOnCB);

        MenuAddEntryReal(envmapshininessLabel, &EnvMapShininess, 
                         EnvMapShininessCB, 0.0f, 1.0f, 0.1f);

        MenuAddEntryBool(normalmapmodulateenvmapLabel, &NormalMapModulateEnvMap, NormalMapModulateEnvMapCB);

        MenuAddEntryInt(envmaptextureLabel, &ActiveEnvMap, 
                         NewEnvMapCB, 0, (sizeof(EnvMapFileName) / sizeof(const RwChar *)) - 1, 1, EnvMapFileName);

        MenuAddSeparator();

        MenuAddEntryBool(fpsLabel, &FPSOn, NULL);

        return TRUE;
    }

    return FALSE;
}

/*
 ***************************************************************************
 */
static RpAtomic *
AtomicNormalizeGeometry(RpAtomic *atomic, void *data)
{
    RpGeometry *geom = RpAtomicGetGeometry(atomic);
    const RwUInt32 numVerts = RpGeometryGetNumVertices(geom); 
	RpMorphTarget *morphTarget = RpGeometryGetMorphTarget(geom, 0);
	RwV3d *positions = RpMorphTargetGetVertices(morphTarget);
    RwUInt32 n;
    RwBBox bbox;
    RwV3d center, radious, invradious;
    RwSphere sphere;
    RwUInt32 flags;

    RwBBoxCalculate(&bbox, positions, numVerts);

    center.x = (bbox.sup.x + bbox.inf.x) / 2.f;
    center.y = (bbox.sup.y + bbox.inf.y) / 2.f;
    center.z = (bbox.sup.z + bbox.inf.z) / 2.f;

    radious.x = (bbox.sup.x - bbox.inf.x) / 2.f;
    radious.y = (bbox.sup.y - bbox.inf.y) / 2.f;
    radious.z = (bbox.sup.z - bbox.inf.z) / 2.f;

    if (radious.x > 1.f ||
        radious.y > 1.f ||
        radious.z > 1.f)
    {
        invradious.x = 1.f / radious.x;
        invradious.y = 1.f / radious.y;
        invradious.z = 1.f / radious.z;

        for (n = 0; n < numVerts; n++)
	    {
            positions[n].x = (positions[n].x - center.x) * invradious.x;
            positions[n].y = (positions[n].y - center.y) * invradious.y;
            positions[n].z = (positions[n].z - center.z) * invradious.z;
	    }

        sphere.center.x = 0.0f;
        sphere.center.y = 0.0f;
        sphere.center.z = 0.0f;
        sphere.radius = 1.f;
    }
    else
    {
        for (n = 0; n < numVerts; n++)
	    {
            positions[n].x = (positions[n].x - center.x);
            positions[n].y = (positions[n].y - center.y);
            positions[n].z = (positions[n].z - center.z);
	    }

        sphere.center.x = 0.0f;
        sphere.center.y = 0.0f;
        sphere.center.z = 0.0f;
        sphere.radius = max(max(radious.x, radious.y), radious.z);
    }

    RpMorphTargetSetBoundingSphere(morphTarget, &sphere);

    /* remove some flags we don't want for this example  */
    flags = RpGeometryGetFlags(geom);

    flags &= ~rpGEOMETRYPRELIT;
    flags &= ~rpGEOMETRYTEXTURED2;

    flags |= rpGEOMETRYLIGHT;
    flags |= rpGEOMETRYTEXTURED;

    RpGeometrySetFlags(geom, flags);

    geom->numTexCoordSets = 1;

    return (atomic);
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

    path = RsPathnameCreate(RWSTRING("new.dff"));
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
 ***************************************************************************
 */
static RwBool
LoadClump(void)
{
    RwStream           *stream = (RwStream *)NULL;
    RwChar             *pathName;

    /* Open stream */
    pathName = RsPathnameCreate(ClumpsInfo[ActiveClump].fileName);
    RsSetModelTexturePath(pathName);
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, pathName);
    if (stream)
    {
        /* Find clump chunk */
        if (RwStreamFindChunk(stream, rwID_CLUMP,
                              (RwUInt32 *)NULL, (RwUInt32 *)NULL))
        {
            /* Load clump */
            Clump = RpClumpStreamRead(stream);
        }

        /* Close stream */
        RwStreamClose(stream, NULL);
    }
    RsPathnameDestroy(pathName);

    if( Clump == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot load model."));

        return FALSE;
    }
    else
    {
        RwFrame *clumpFrame = NULL;
        const RwV3d origin = {0, 0, 0};
        RwTexture *NormalMapTexture;

        clumpFrame = RpClumpGetFrame(Clump);

        if (ClumpsInfo[ActiveClump].fileNameAnimation)
        {
            const RwV3d scale = {20.0f, 20.0f, 20.0f};

            LoadClumpAnimation(Clump, ClumpsInfo[ActiveClump].fileNameAnimation);

            RwFrameScale(clumpFrame, &scale, rwCOMBINEREPLACE);
            RwFrameRotate(clumpFrame, &Xaxis, ClumpsInfo[ActiveClump].angleX, rwCOMBINEPOSTCONCAT);
            RwFrameRotate(clumpFrame, &Yaxis, ClumpsInfo[ActiveClump].angleY, rwCOMBINEPOSTCONCAT);
            RwFrameTranslate(clumpFrame, &origin, rwCOMBINEPOSTCONCAT);
        }
        else
        {
            RpClumpForAllAtomics(Clump, AtomicNormalizeGeometry, NULL);

            RwFrameRotate(clumpFrame, &Xaxis, ClumpsInfo[ActiveClump].angleX, rwCOMBINEREPLACE);
            RwFrameRotate(clumpFrame, &Yaxis, ClumpsInfo[ActiveClump].angleY, rwCOMBINEPOSTCONCAT);
            RwFrameTranslate(clumpFrame, &origin, rwCOMBINEPOSTCONCAT);
        }

        /* NormalMap */
        NormalMapTexture = RwTextureRead(ClumpsInfo[ActiveClump].fileNameNormalMapTextureSpace, RWSTRING(""));

        RwTextureSetFilterMode(NormalMapTexture, rwFILTERLINEARMIPLINEAR);

        RpClumpForAllAtomics(Clump, AtomicApplyNormalMapTexture, NormalMapTexture);

        RwTextureDestroy(NormalMapTexture);

        /* EnvMap */
        if (NormalMapEnvMapOn)
        {
            RwTexture *envMapTexture;

            envMapTexture = RwTextureRead(EnvMapFileName[ActiveEnvMap], RWSTRING(""));

            RwTextureSetFilterMode(envMapTexture, rwFILTERLINEARMIPLINEAR);

            if (ActiveEnvMap == ENVMAP_WITHFRAME)
            {
                RwTextureSetAddressing(envMapTexture, rwTEXTUREADDRESSCLAMP);
            }
            else
            {
                RwTextureSetAddressing(envMapTexture, rwTEXTUREADDRESSWRAP);
            }

            RpClumpForAllAtomics(Clump, AtomicApplyEnvMapTexture, envMapTexture);

            RpClumpForAllAtomics(Clump, AtomicApplyEnvMapShininess, &EnvMapShininess);

            RpClumpForAllAtomics(Clump, AtomicApplyModulateEnvMap, &NormalMapModulateEnvMap);

            if (ActiveEnvMap == ENVMAP_WITHFRAME)
            {
                RpClumpForAllAtomics(Clump, AtomicApplyEnvMapFrame, HighliteFrame);
            }
            else
            {
                RpClumpForAllAtomics(Clump, AtomicApplyEnvMapFrame, NULL);
            }

            RwTextureDestroy(envMapTexture);
        }
        else
        {
            RpClumpForAllAtomics(Clump, AtomicApplyEnvMapTexture, NULL);
        }

        if (ClumpsInfo[ActiveClump].fileNameAnimation)
        {
            RpClumpForAllAtomics(Clump,
                                    AtomicNormalMapTextureSpaceEnable,
                                    (void *)rpNORMMAPATOMICSKINNEDPIPELINE);
        }
        else
        {
            RpClumpForAllAtomics(Clump,
                                    AtomicNormalMapTextureSpaceEnable,
                                    (void *)rpNORMMAPATOMICSTATICPIPELINE);
        }

        if (NormalMapOn == FALSE)
        {
            if (ClumpsInfo[ActiveClump].fileNameAnimation)
            {
                #if defined(D3D9_DRVMODEL_H)
                RpClumpForAllAtomics(Clump, AtomicSetPipeline,
                        (void *)RpSkinGetD3D9Pipeline(rpSKIND3D9PIPELINEGENERIC));
                #elif defined(XBOX_DRVMODEL_H)
                RpClumpForAllAtomics(Clump, AtomicSetPipeline,
                        (void *)RpSkinGetXboxPipeline(rpSKINXBOXPIPELINEGENERIC));
                #endif
            }
            else
            {
                RpClumpForAllAtomics(Clump, AtomicSetPipeline, (void *)RpAtomicGetDefaultPipeline());
            }
        }

#if 0
        SaveDFF();
#endif
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static void
DestroyClump(void)
{
    if (Clump != NULL)
    {
        DestroyClumpAnimation(Clump);

        RpClumpDestroy(Clump);

        Clump = NULL;
    }
}

/*
 *****************************************************************************
 */
static RwBool 
Initialize3D(void *param)
{
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

    /*
     * Create an empty world if not loading a BSP...
     */
    if( LoadWorld(WorldFileName) == FALSE )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return FALSE;
    }

    /*
     * Create a camera using the democom way...
     */
    Camera = CreateCamera(World);
    if( Camera == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    /*
     * Create an Ambient Light
     */
    AmbientLight = CreateAmbientLight(World);
    if( AmbientLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create Ambient Light."));
        return FALSE;
    }

    /*
     * Create Main Light
     */
    MainLight = CreateMainLight(World);
    if( MainLight == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create Ambient Light."));
        return FALSE;
    }

    /*
     * Create frame for highlight envmap
     */
    HighliteFrame = RwFrameCreate();

    /*
     * Load Clumps
     */
	RwImageSetPath(RWSTRING("models/textures/"));

    RwTextureSetMipmapping(TRUE);
    RwTextureSetAutoMipmapping(TRUE);

    if (LoadClump() == FALSE)
    {
        return FALSE;
    }

    if( !InitializeMenu() )
    {
        RsErrorMessage(RWSTRING("Error initializing menu."));

        return FALSE;
    }

#ifdef RWMETRICS
    RsMetricsOpen(Camera);
#endif

    return TRUE;
}


/*
 *****************************************************************************
 */
static void 
Terminate3D(void)
{
    RwFrame *frame;
    RpWorld *world;

    /*
     * Close or destroy RenderWare components in the reverse order they
     * were opened or created...
     */

#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    world = RpLightGetWorld(AmbientLight);
    if( world )
    {
        RpWorldRemoveLight(world, AmbientLight);
    }

    world = RpLightGetWorld(MainLight);
    if( world )
    {
        RpWorldRemoveLight(world, MainLight);
    }

    RpLightDestroy(AmbientLight);

    frame = RpLightGetFrame(MainLight);

    RpLightDestroy(MainLight);

    if( frame )
    {
        RwFrameDestroy(frame);
    }

    if( Camera )
    {
        RpWorldRemoveCamera(World, Camera);

        /*
         * This assumes the camera was created the democom way...
         */
        CameraDestroy(Camera);
    }

    DestroyClump();

    if( World )
    {
        RpWorldDestroy(World);
    }

    if ( HighliteFrame )
    {
        RwFrameDestroy(HighliteFrame);
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

    /* 
     * Attach hanim plug-in...
     */
    if( !RpHAnimPluginAttach() )
    {
        return FALSE;
    }

    /* 
     * Attach anim toolkit...
     */
    if( !RtAnimInitialize() )
    {
        return FALSE;
    }

    /* 
     * Attach skin plug-in...
     */
    if( !RpSkinPluginAttach() )
    {
        return FALSE;
    }

    /* 
     * Attach Normal Maps plug-in...
     */
    if( !RpNormMapPluginAttach() )
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
    RwCameraClear(Camera, &BackgroundColor, rwCAMERACLEARZ|rwCAMERACLEARIMAGE);

    if( RwCameraBeginUpdate(Camera) )
    {
        if( MenuGetStatus() != HELPMODE )
        {
            /*
             * Scene rendering here...
             */
            if (WorldRender)
            {
                RpWorldRender(World);
            }
            else
            {
                if (Clump != NULL)
                {
                    RpClumpRender(Clump);
                }
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

    if( CameraTranslating )
    {
        if (WorldRender)
        {
            ClumpsTranslate(50 * CameraTranslateDelta * deltaTime, 0.0f);
        }
        else
        {
            ClumpsTranslate(5 * CameraTranslateDelta * deltaTime, 0.0f);
        }
    }

    if( CameraZooming )
    {        
        if (WorldRender)
        {
            ClumpsZoom(100 * CameraZoomDelta * deltaTime);
        }
        else
        {
            ClumpsZoom(10 * CameraZoomDelta * deltaTime);
        }
    }


    if (!WorldRender)
    {
        if (ClumpsInfo[ActiveClump].fileNameAnimation)
        {
            UpdateClumpAnimation(Clump, deltaTime * 0.75f);
        }
    }

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
            CameraSize(Camera, (RwRect *)param, 
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
 */
