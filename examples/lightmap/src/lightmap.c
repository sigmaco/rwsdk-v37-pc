
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
 * lightmap.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example to demonstrate the functions of light maps as
   as they are provided in RpLightmap and RtLightmap.
 *
*****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"
#include "menu.h"

#include "rpltmap.h"
#include "rtltmap.h"
#include "rtpitexd.h"

#include "main.h"
#include "lightmap.h"


/*===========================================================================*
 *--- Global Variables ------------------------------------------------------*
 *===========================================================================*/

RwBool   gCreatingLightMaps = FALSE;
RwBool   gClearingLightMaps = FALSE;
RwBool   gLightingWorld     = FALSE;
RwBool   gUseAreaLights     = FALSE;
RwBool   gUseRpLights       = TRUE;
RwBool   gUseDynamicLights  = TRUE;
RwReal   gLightProgress     = 0; /* Percentage progress on lighting */
RwBool   gResetLighting     = FALSE;
RwReal   gLightingProgress  = 0.0f;
RtLtMapAreaLightGroup  *gAreaLights      = (RtLtMapAreaLightGroup *)NULL;
RtLtMapLightingSession  gLightingSession;

RwUInt32 gRenderStyle       = rpLTMAPSTYLERENDERBASE|rpLTMAPSTYLERENDERLIGHTMAP;
RwBool   gPointSampling     = TRUE;
RwUInt32 gLumCalcIndex      = 0;

RwBool  gMakeDarkMap        = FALSE;
RwUInt32 gTexDictIndex       = 0;

/*===========================================================================*
 *--- Local Defines ---------------------------------------------------------*
 *===========================================================================*/

#define CONE_NUM_VERTICES              10
#define POINT_LIGHT_NUM_VERTICES       32
#define DIRECT_LIGHT_NUM_VERTICES      20
#define DIRECT_LIGHT_CYLINDER_DIAMETER (1.5f)
#define DIRECT_LIGHT_CYLINDER_LENGTH   (5.0f)
#define DIRECT_LIGHT_CONE_SIZE         (4.0f)
#define DIRECT_LIGHT_CONE_ANGLE        (0.25f*rwPI)

/* Ideally, this would be scaled with processor power to
 * give a fairly constant framerate during lighting */
#define SAMPLESPERSLICE 1000

/* Write-back on startup is disabled by default since on some
 * platforms (e.g GCN) it will appear to cause a hang unless
 * the target control environment is set up appropriately */
#define CREATETXDx

/* When defined, the example will generate lightmap for a complete scene
 * rather than just what's lie in the camera frustrum */
#define FULLSCENELIGHTINGx

/*===========================================================================*
 *--- Local Variables -------------------------------------------------------*
 *===========================================================================*/

static RwChar gAtomicPath[] = RWSTRING("models/");
static RwChar gAtomicName[] = RWSTRING("vase");

/* Dynamic lights */
static RwUInt32 gNumDynamicLights = 0;
static RpLight *gDynamicLights[2] = {(RpLight *)NULL, (RpLight *)NULL};
static RwV3d    gDynamicVectors[3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
static RwV3d    gExampleVectors[3] = {{   0.0f,  650.0f, -8000.0f},
                                      {   0.0f,    0.0f,  3000.0f},
                                      { 600.0f,    0.0f,     0.0f}};

/* Clumps containing atomics for the default scene */
RpClump *gAtomicClumps[4] = {NULL, NULL, NULL, NULL};
RwUInt32 gNumAtomics = 0;
/* Positioning information for these clumps */
static RwReal gAtomicScales[4] = {1.0f, 0.7f, 1.5f, 0.8f};
static RwV3d  gAtomicAngles[4] = {{-90, 0, 0}, {-90, 0, 0}, {-90, 0, 0}, {-15, 0, 0}};
static RwV3d  gAtomicPositions[4] = {{    0.0f,  390.0f,     0.0f},
                                     {-9021.0f,  274.0f,  1750.0f},
                                     {-8990.0f,  580.0f,  2300.0f},
                                     { 6430.0f,  159.0f,   541.0f}};

static RwChar gLightMapPrefix[] = RWSTRING("xmp_");

#if (defined(SKY2_DRVMODEL_H) || defined(NULLSKY_DRVMODEL_H))
RwChar gPlatformString[] = RWSTRING("ps2");
#elif (defined(D3D8_DRVMODEL_H))
RwChar gPlatformString[] = RWSTRING("d3d8");
#elif (defined(D3D9_DRVMODEL_H))
RwChar gPlatformString[] = RWSTRING("d3d9");
#elif (defined(GCN_DRVMODEL_H) || defined(NULLGCN_DRVMODEL_H))
RwChar gPlatformString[] = RWSTRING("gcn");
#elif (defined(OPENGL_DRVMODEL_H))
RwChar gPlatformString[] = RWSTRING("ogl");
#elif (defined(SOFTRAS_DRVMODEL_H))
RwChar gPlatformString[] = RWSTRING("sfrs");
#elif (defined(XBOX_DRVMODEL_H) || defined(NULLXBOX_DRVMODEL_H))
RwChar gPlatformString[] = RWSTRING("xbox");
#elif (defined(NULL_DRVMODEL_H))
RwChar gPlatformString[] = RWSTRING("null");
#else
#error Example does not support this platform!
#endif

/* Stuff used in texDict loading/saving */
static RwTexDictionary *gBaseTexDict = (RwTexDictionary *)NULL;
static RwChar          *gBaseTexDictPath = (RwChar *)NULL;

/*===========================================================================*
 *--- Functions -------------------------------------------------------------*
 *===========================================================================*/

/*
 *****************************************************************************
 */
void
CameraInitPosition(RwCamera *camera, RpWorld *world)
{
    const RwBBox *bBox;
    const RwV2d *viewWindow;
    RwV3d pos, right, at;
    RwFrame *frame;
    RwReal size, distance, rTemp;

    /* How big's the world? */
    bBox = RpWorldGetBBox(world);
    RwV3dSub(&pos, &(bBox->sup), &(bBox->inf));
    size = RwV3dLength(&pos);

    /* Rotate the camera so it looks straight down... */
    frame = RwCameraGetFrame(camera);
    RwFrameSetIdentity(frame);
    right = *RwMatrixGetRight(RwFrameGetMatrix(frame));
    RwFrameRotate(frame, &right, 90.0f, rwCOMBINEREPLACE);

    /* Move it to the center of the world... */
    RwV3dSub(&pos, &(bBox->sup), &(bBox->inf));
    RwV3dScale(&pos, &pos, 0.5f);
    RwV3dAdd(&pos, &pos, &(bBox->inf));
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    /* Back it up till it can see the whole world... */
    viewWindow = RwCameraGetViewWindow(camera);
    rTemp = viewWindow->x;
    if (viewWindow->y < rTemp) rTemp = viewWindow->y;
    distance = (0.5f*size) / rTemp;
    at = *RwMatrixGetAt(RwFrameGetMatrix(frame));
    RwV3dScale(&at, &at, -distance);
    RwFrameTranslate(frame, &at, rwCOMBINEPOSTCONCAT);

    /* Set the clip planes to give good precision... */
    RwCameraSetNearClipPlane(camera, RwRealMINVAL);
    RwCameraSetFarClipPlane( camera, (1.5f*size));
    RwCameraSetNearClipPlane(camera, (1.5f*size) / 1500.0f);

    /* Set camera movement speed as appropriate */
    CameraMaxSpeed = size / 3;

    return;
}


/*
 *****************************************************************************
 */
static RpClump *
ClumpDestroyCB(RpClump *clump, void *data)
{
    RpWorldRemoveClump((RpWorld *)data, clump);
    RpClumpDestroy(clump);
    return(clump);
}


/*
 *****************************************************************************
 */
static RpLight *
LightDestroyCB(RpLight *light, void *data)
{
    RwBool preserve = (RwBool)data;
    RwFrame *frame;

    if ((FALSE != preserve) &&
        ((rpLIGHTAMBIENT == RpLightGetType(light)) ||
         (rpLIGHTDIRECTIONAL == RpLightGetType(light))))
    {
        return(light);
    }

    if (NULL != RpLightGetClump(light))
    {
        RpClumpRemoveLight(RpLightGetClump(light), light);
    }
    if (NULL != RpLightGetWorld(light))
    {
        RpWorldRemoveLight(RpLightGetWorld(light), light);
    }
    frame = RpLightGetFrame(light);
    RpLightSetFrame(light, (RwFrame *)NULL);
    RwFrameDestroy(frame);
    RpLightDestroy(light);

    return(light);
}


/*
 *****************************************************************************
 */
static RwTexDictionary *
TexDictDestroyCB(RwTexDictionary *texDict, void *data __RWUNUSED__)
{
    RwTexDictionaryDestroy(texDict);
    return(texDict);
}


/*
 *****************************************************************************
 */
void
ResetScene(RpWorld *world, RwCamera *camera)
{
    RwFrame *frame;
    RwUInt32 i;

    /* Initialise the lighting scene to 'no lighting', and 'not
     * working out any lighting' */
    gLightingSession.startObj = 0;
    gLightingSession.totalObj = 0;

    /* Kill any area lights we have floating around */
    if (NULL != gAreaLights)
    {
        RtLtMapAreaLightGroupDestroy(gAreaLights);
        gAreaLights = (RtLtMapAreaLightGroup *)NULL;
    }

    /* Clean up lights globals */
    for (i = 0;i < gNumDynamicLights;i++)
    {
        if (RpLightGetWorld(gDynamicLights[i]) == world)
        {
            RpWorldRemoveLight(world, gDynamicLights[i]);
        }
        frame = RpLightGetFrame(gDynamicLights[i]);
        gDynamicLights[i] = RpLightSetFrame(gDynamicLights[i], (RwFrame *)NULL);
        RwFrameDestroy(frame);
        RpLightDestroy(gDynamicLights[i]);
        gDynamicLights[i] = (RpLight *)NULL;
    }
    gNumDynamicLights = 0;

    if (NULL != world)
    {
        DestroyLightMaps ();

        /* Kill lights/atomics clumps if loaded */
        RpWorldForAllClumps(world, ClumpDestroyCB, (void *)world);

        RtLtMapLightingSessionDeInitialize(&gLightingSession);

    }

    /* Clean up atomics globals */
    for (i = 0;i < gNumAtomics;i++) gAtomicClumps[i] = (RpClump *)NULL;
    gNumAtomics = 0;


    /* Loading a new world cancels any lighting currently going on */
    gResetLighting = TRUE;
    /* Destroy the existing world */
    if (NULL != world)
    {
        /* Destroy any other lights in the world */
        RpWorldForAllLights(world, LightDestroyCB, (void *)FALSE);

        /* Remove the camera before destroying the world */
        if (NULL != camera)
        {
            RpWorldRemoveCamera(world, camera);
        }
        RpWorldDestroy(world);
    }

    /* Destroy any existing texture dictionaries (so that lightmaps don't
     * get mistakenly carried across from the old world to the new one) */
    RwTexDictionaryForAllTexDictionaries(TexDictDestroyCB, NULL);
    if (NULL != gBaseTexDictPath)
    {
        RsPathnameDestroy(gBaseTexDictPath);
        gBaseTexDictPath = (RwChar *)NULL;
    }
    gBaseTexDict = (RwTexDictionary *)NULL;

    return;
}


/*
 *****************************************************************************
 */
static RwRGBA *
TextureCalcEmitterColor(RwRGBA *result, RwTexture *texture)
{
    RwRaster *raster;
    RwImage  *image;
    RwUInt8  *pixels;
    RwUInt32  width, height, stride, x, y, i;
    RwReal    redSum = 0, greenSum = 0, blueSum = 0, lumSum = 0, maxColSum = 0;
    RwRGBA    *curPixel;

    result->red = 0;
    result->green = 0;
    result->blue = 0;
    result->alpha = 255;


    /* This function calculates a representative color for
     * a texture which is to be used as a light emitter. */

    RSASSERT(NULL != texture);
    raster = RwTextureGetRaster(texture);
    RSASSERT(NULL != raster);
    width = RwRasterGetWidth( raster);
    height = RwRasterGetHeight(raster);

    image = RwImageCreate(width, height, 32);
    RSASSERT(NULL != image);
    RwImageAllocatePixels(image);
    stride = RwImageGetStride(image);

    RwImageSetFromRaster(image, raster);
    pixels = RwImageGetPixels(image);
    i = 0;
    for (y = 0;y < height;y++)
    {
        curPixel = (RwRGBA *)&(pixels[y*stride]);
        for (x = 0;x < width;x++)
        {
            RwReal maxCol, lum, weightLum;

            /* This stores a kinda hue (ratio of color channel to max colour channel)
             * and does a weighted mean of these, the weights being the exponential of
             * luminance (i.e the true luminance on-screen). It then reconstructs the
             * mean colour from the averaged max color channel value and the hue sums.
             * The result's pretty good (e.g a texture, with two small buttons on it,
             * one red (lit), one green (unlit) and a gray border, comes out dull red). */
            rwSqrt(&lum, ((RwReal)curPixel->red*curPixel->red +
                          (RwReal)curPixel->green*curPixel->green +
                          (RwReal)curPixel->blue*curPixel->blue));

            weightLum = (RwReal)RwPow(1.1f, lum);
            /* Often, the brightest pixels are de-saturated so it seems like emitter
             * color is too gray... sqrt()ing perceived luminance here helps. */
            rwSqrt(&weightLum, weightLum);

            maxCol = curPixel->red;
            if (curPixel->green > maxCol) maxCol = curPixel->green;
            if (curPixel->blue  > maxCol) maxCol = curPixel->blue;
            if (maxCol > 0)
            {
                redSum    += weightLum*(curPixel->red   / maxCol);
                greenSum  += weightLum*(curPixel->green / maxCol);
                blueSum   += weightLum*(curPixel->blue  / maxCol);
                lumSum    += weightLum;
                maxColSum += maxCol;
            }

            curPixel++;
            i++;
        }
    }
    RwImageDestroy(image);

    result->red   = (RwUInt8)((maxColSum / i)*(redSum   / lumSum));
    result->green = (RwUInt8)((maxColSum / i)*(greenSum / lumSum));
    result->blue  = (RwUInt8)((maxColSum / i)*(blueSum  / lumSum));

    return(result);
}

/*
 *****************************************************************************
 */
static RpMaterial *
ScaleMaterialsDensityCB (RpMaterial *mat, void *data)
{
    RwReal origDensity;

    origDensity = RtLtMapMaterialGetLightMapDensityModifier (mat);

    /* Set up the lightmapping density for this material */
    RtLtMapMaterialSetLightMapDensityModifier (mat, origDensity * (*(RwReal *) data));

    return mat;
}

/*
 *****************************************************************************
 */
static RpAtomic *
ScaleAtomicsMaterialDensityCB (RpAtomic *atomic, void *data)
{
    RpGeometryForAllMaterials (RpAtomicGetGeometry (atomic), ScaleMaterialsDensityCB, data);
    return atomic;
}

/*
 *****************************************************************************
 */
static RpClump *
ScaleClumpMaterialsDensityCB (RpClump *clump, void *data)
{
    RpClumpForAllAtomics (clump, ScaleAtomicsMaterialDensityCB, data);
    return clump;
}

/*
 *****************************************************************************
 */
static RpMaterial *
SetupMaterialsCB(RpMaterial *mat, void *data __RWUNUSED__)
{
    /* Most textures need to be lightmapped */
    RtLtMapMaterialSetFlags(mat, RtLtMapMaterialGetFlags(mat) |
                                 rtLTMAPMATERIALLIGHTMAP);

    if (mat->texture)
    {
        RwChar *name = mat->texture->name;

        /* Most of the geometry in this level is best lit flat-shaded */
        if (rwstrstr(name, RWSTRING("shied")) ||
            rwstrstr(name, RWSTRING("floor")) )
        {
            RtLtMapMaterialSetFlags(mat, RtLtMapMaterialGetFlags(mat) |
                                         rtLTMAPMATERIALFLATSHADE);
        }

        /* The skydome geometry doesnt need to be lightmapped */
        if (!rwstrcmp(name, RWSTRING("SKYSUN2")))
        {
            RtLtMapMaterialSetFlags(mat, RtLtMapMaterialGetFlags(mat) &
                                        ~rtLTMAPMATERIALLIGHTMAP);
        }

        /* Increase lightmap sample density for vases */
        if (!rwstrcmp(name, RWSTRING("TRAVERTN")))
        {
            /* Set up the lightmapping density for this material
             * This will be multiplied by a global density later */
            RtLtMapMaterialSetLightMapDensityModifier (mat, 4.0f);
        }

        /* Glass should not block light */
        if (!rwstrcmp(name, RWSTRING("glass")))
        {
            RwRGBA matCol;
            RtLtMapMaterialSetFlags(mat, RtLtMapMaterialGetFlags(mat) |
                                         rtLTMAPMATERIALNOSHADOW);
            matCol = *RpMaterialGetColor(mat);
            matCol.alpha >>= 1;
            RpMaterialSetColor(mat, &matCol);
        }

        /* Some textures emit light */
        if (rwstrstr(name, RWSTRING("light")))
        {
            RwRGBA emitterColor;

            RtLtMapMaterialSetFlags(mat, RtLtMapMaterialGetFlags(mat) |
                                         rtLTMAPMATERIALAREALIGHT);
            /* Emitters generally work better non-lightmapped */
            RtLtMapMaterialSetFlags(mat, RtLtMapMaterialGetFlags(mat) &
                                        ~rtLTMAPMATERIALLIGHTMAP);

            TextureCalcEmitterColor(&emitterColor, mat->texture);

            RtLtMapMaterialSetAreaLightColor(mat, emitterColor);

            if (rwstrstr(name, RWSTRING("chamber")))
            {
                /* Make the glass chambers very bright */
                RtLtMapMaterialSetAreaLightRadiusModifier(mat, 2.0f);
            }
        }
    }
    else
    {
        /* This is the skylight glass. We need to make it let sky and
         * sun lights through (all directional lights) */
        RtLtMapMaterialSetFlags(mat, RtLtMapMaterialGetFlags(mat) |
                                     rtLTMAPMATERIALSKY);
    }

    return(mat);
}

/*
 *****************************************************************************
 */
static RpAtomic *
SetupExampleAtomicCB(RpAtomic *atomic, void *data)
{
    RpGeometry *geom    = RpAtomicGetGeometry(atomic);
    RwUInt32   counter = *(RwUInt32 *)data;

    /* Set up other material properties */
    RpGeometryForAllMaterials(geom, SetupMaterialsCB, NULL);

    /* Disable vertex lighting (and turn on lightmapping) */
    RtLtMapAtomicSetFlags(atomic, (RtLtMapAtomicGetFlags(atomic)
        | rtLTMAPOBJECTLIGHTMAP) & ~rtLTMAPOBJECTVERTEXLIGHT);

    if (1 == counter)
    {
        /* Stop the second atomic in the scene from casting shadows */
        RtLtMapAtomicSetFlags(atomic, RtLtMapAtomicGetFlags(atomic)
            | rtLTMAPOBJECTNOSHADOW);
    }

    return(atomic);
}

/*
 *****************************************************************************
 */
static RwBool
LoadAtomics(RpWorld *world)
{
    RwChar   string[256], subString[32], *defaultPath, *newPath;
    RwV3d     xAxis = {1,0,0}, yAxis = {0,1,0}, zAxis = {0,0,1}, scale;
    RpClump  *clump;
    RwFrame  *frame;
    RwUInt32  i;

    rwstrcpy(string, gAtomicPath);
    rwstrcat(string, gAtomicName);
    rwstrcat(string, RWSTRING(".dff"));
    defaultPath = RsPathnameCreate(string);

    for (i = 0;i < 4;i++)
    {
        rwstrcpy(string, gAtomicPath);
        rwstrcat(string, gAtomicName);
        RsSprintf(subString, "%02d.dff", i);
        rwstrcat(string, subString);
        newPath = RsPathnameCreate(string);

        /* First search for numbered atomics (i.e those which have previously
         * been saved by the example, for which lightmap UVs and lightmap
         * plugin data have been set up) */
        clump = LoadClump(newPath);
        if (NULL == clump)
        {
            /* Else default to the freshly-exported version */
            if (i == 0)
            {
                clump = LoadClump(defaultPath);
                RpWorldAddClump(world, clump);
            }
            else
            {
                clump = RpClumpClone (gAtomicClumps [0]);
            }
            frame = RpClumpGetFrame(clump);
            scale.x = scale.y = scale.z = gAtomicScales[i];
            RwFrameScale(frame, &scale, rwCOMBINEREPLACE);
            RwFrameRotate(frame, &xAxis, gAtomicAngles[i].x, rwCOMBINEPOSTCONCAT);
            RwFrameRotate(frame, &zAxis, gAtomicAngles[i].z, rwCOMBINEPOSTCONCAT);
            RwFrameRotate(frame, &yAxis, gAtomicAngles[i].y, rwCOMBINEPOSTCONCAT);
            RwFrameTranslate(frame, &(gAtomicPositions[i]),  rwCOMBINEPOSTCONCAT);

            /* Make lightmaps high-res for atomics, enable light-emitting
             * materials and vertex lighting for the first atomic */
            RpClumpForAllAtomics(clump, SetupExampleAtomicCB, &i);
        }
        else
        {
            RpWorldAddClump(world,clump); 
        }
        RSASSERT(NULL != clump);
        RSASSERT(NULL == gAtomicClumps[i]);
        gAtomicClumps[i] = clump;

        RsPathnameDestroy(newPath);

        gNumAtomics++;
    }

    RsPathnameDestroy(defaultPath);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MakeSkyLights(RpWorld *world, RwRGBAReal *color, RwUInt32 numRings)
{
    const RwV3d   xAxis = {1, 0, 0}, yAxis = {0, 1, 0};
    const RwBBox *box = RpWorldGetBBox(world);
    const RwReal  maxAngle = 60;
    RwRGBAReal    lightColor;
    RwV3d         center, size, temp;
    RwReal        xAngle, yAngle;
    RwReal        length;
    RpLight      *light;
    RwFrame      *frame;
    RwUInt32      i, j, count;

    RwV3dAdd(&center, &(box->sup), &(box->inf));
    RwV3dScale(&center, &center, 0.5f);

    RwV3dSub(&size, &(box->sup), &(box->inf));
    length = RwV3dLength(&size);

    RwV3dScale(&temp, &yAxis, 0.55f*length);

    count = 1;
    for (i = 1;i < numRings;i++)
    {
        RwUInt32 numLightsInRing;
        RwReal   offset;

        xAngle = maxAngle*(i / (RwReal)(numRings - 1));

        /* Calculate spacing around the rings to equal that between rings */
        offset = 2*rwPI*(RwReal)RwSin(xAngle*rwPI / 180);
        offset /= (maxAngle*rwPI / 180) / (numRings - 1);

        /* Round to an integer */
        numLightsInRing = RwFastRealToUInt32(offset + 0.5f);
        count += numLightsInRing;
    }

    /* Share the color out among the sky lights */
    lightColor = *color;
    RwRGBARealScale(&lightColor, &lightColor, 1.0f / count);

    for (i = 0;i < numRings;i++)
    {
        RwUInt32 numLightsInRing;
        RwReal   offset;

        if (0 == i)
        {
            xAngle = 0;
            numLightsInRing = 1;
            offset = 0;
        }
        else
        {
            xAngle = maxAngle*(i / (RwReal)(numRings - 1));

            /* Calculate spacing around the rings to equal that between rings */
            offset = 2*rwPI*(RwReal)RwSin(xAngle*rwPI / 180);
            offset /= (maxAngle*rwPI / 180) / (numRings - 1);

            /* Round to an integer */
            numLightsInRing = RwFastRealToUInt32(offset + 0.5f);
            /* Try and avoid ugly alignments */
            offset = offset - numLightsInRing;
        }

        for (j = 0;j < numLightsInRing;j++)
        {
            yAngle = (360 / (RwReal)numLightsInRing)*(j + offset - 1);

            light = RpLightCreate(rpLIGHTDIRECTIONAL);
            RSASSERT(NULL != light);
            RpLightSetColor(light, &lightColor);
            RpLightSetRadius(light, 0.01f*length);
            RpLightSetFlags(light, 0);
            frame = RwFrameCreate();
            RSASSERT(NULL != frame);
            RpLightSetFrame(light, frame);
            RwFrameRotate(   frame, &xAxis, 90,     rwCOMBINEREPLACE);
            RwFrameTranslate(frame, &temp,          rwCOMBINEPOSTCONCAT);
            RwFrameRotate(   frame, &xAxis, xAngle, rwCOMBINEPOSTCONCAT);
            RwFrameRotate(   frame, &yAxis, yAngle, rwCOMBINEPOSTCONCAT);
            RwFrameTranslate(frame, &center,        rwCOMBINEPOSTCONCAT);
            RpWorldAddLight(world, light);
        }
    }

    return(TRUE);
}


/*
 *****************************************************************************
 */
RwBool
LightsUpdate(RwReal deltaTime)
{
    static RwReal time = 0;
    RwFrame *frame;
    RwV3d pos, tmp;

    time += deltaTime;
    if (time > (2*rwPI)) time -= 2*rwPI;

    if (gNumDynamicLights > 1)
    {
        /* We move our dynamic-lighting point light on an elliptical path around the world */
        pos = gDynamicVectors[0];
        RwV3dScale(&tmp, &(gDynamicVectors[1]), (RwReal)(RwSin(time)));
        RwV3dAdd(&pos, &pos, &tmp);
        RwV3dScale(&tmp, &(gDynamicVectors[2]), (RwReal)(1.21f*RwCos(time)));
        RwV3dAdd(&pos, &pos, &tmp);

        frame = RpLightGetFrame(gDynamicLights[1]);
        RwFrameTranslate(frame, &pos, rwCOMBINEREPLACE);
    }

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RpLight *
LightToggleCB(RpLight *light, void *data __RWUNUSED__)
{
    RpLightSetFlags(light,
        RpLightGetFlags(light) ^ (rpLIGHTLIGHTWORLD | rpLIGHTLIGHTATOMICS));
    return(light);
}

/*
 *****************************************************************************
 */
static RpLight *
EnumerateLightsCB(RpLight *light, void *data)
{
    RwUInt32 *lightNum = (RwUInt32 *) data;
   
    (*lightNum) ++;
    return(light);
}

/*
 *****************************************************************************
 */
static RwBool
SetupLights(RpWorld *world)
{
    static const RwV3d   xAxis = {1, 0, 0}, yAxis = {0, 1, 0}, origin = {0, 0, 0};
    const        RwBBox *box   = RpWorldGetBBox(world);
    static RwBool startup    = TRUE;
    RwRGBAReal    ambColor   = {0.05f, 0.05f, 0.05f, 1.0f};
    RwRGBAReal    pointColor = {1.00f, 1.00f, 1.00f, 1.0f};
    RwRGBAReal    sunColor   = {1.00f, 1.00f, 0.40f, 1.0f};
    RwRGBAReal    skyColor   = {0.80f, 0.80f, 1.00f, 1.0f};
    RpLight      *light;
    RwFrame      *frame;
    RwV3d         center, size, temp;
    RwReal        length;
    RwUInt32      i = 0;

    /* Scope the size of the world */
    RwV3dAdd(&center, &(box->sup), &(box->inf));
    RwV3dScale(&center, &center, 0.5f);
    RwV3dSub(&size, &(box->sup), &(box->inf));
    length = RwV3dLength(&size);
    RwV3dScale(&temp, &yAxis, 0.6f*length);


    /* Create lights to add light dynamically on top of lightmaps */

    /* An ambient light */
    RSASSERT(NULL == gDynamicLights[0]);
    gDynamicLights[0] = RpLightCreate(rpLIGHTAMBIENT);
    RSASSERT(NULL != gDynamicLights[0]);
    RpLightSetColor(gDynamicLights[0], &ambColor);
    (void)RpLightSetFlags(gDynamicLights[0], rpLIGHTLIGHTWORLD | rpLIGHTLIGHTATOMICS);
    frame = RwFrameCreate();
    RSASSERT(NULL != frame);
    (void)RpLightSetFrame(gDynamicLights[0], frame);
    RpWorldAddLight(world, gDynamicLights[0]);

    /* A point light */
    RSASSERT(NULL == gDynamicLights[1]);
    gDynamicLights[1] = RpLightCreate(rpLIGHTPOINT);
    RSASSERT(NULL != gDynamicLights[1]);
    RpLightSetRadius(gDynamicLights[1], 0.1f*length);
    RpLightSetColor(gDynamicLights[1], &pointColor);
    frame = RwFrameCreate();
    RSASSERT(NULL != frame);
    (void)RpLightSetFrame(gDynamicLights[1], frame);
    RwFrameTranslate(frame, &center, rwCOMBINEREPLACE);
    (void)RpLightSetFlags(gDynamicLights[1], rpLIGHTLIGHTWORLD | rpLIGHTLIGHTATOMICS);
    RpWorldAddLight(world, gDynamicLights[1]);
    /* This light will be moved dynamically in LightsUpdate. Set up two
     * vectors so we can move it around an ellipse fitted to the world. */
    gDynamicVectors[0]   = center;
/* TODO[3]: This copy-by-element oddness is to work around a
 *          compiler error in the latest CW */
/*    gDynamicVectors[1]   = origin;*/
    gDynamicVectors[1].x   = origin.x;
    gDynamicVectors[1].y   = origin.y;
    gDynamicVectors[1].z   = origin.z;
    gDynamicVectors[1].x = size.x * 0.375f;
    gDynamicVectors[2]   = origin;
    gDynamicVectors[2].z = size.z * 0.375f;
    if (FALSE != startup)
    {
        /* We have a nice path set up for the example level */
        gDynamicVectors[0] = gExampleVectors[0];
        gDynamicVectors[1] = gExampleVectors[1];
        gDynamicVectors[2] = gExampleVectors[2];
        startup = FALSE;
    }

    if (FALSE == gUseDynamicLights)
    {
        RpWorldRemoveLight(world, gDynamicLights[0]);
        RpWorldRemoveLight(world, gDynamicLights[1]);
    }

    gNumDynamicLights = 2;


    /* If there were lights in the world already, these will be
     * used for lightmap generation, so return */
    RpWorldForAllLights(world, EnumerateLightsCB, &i);
    if (i > 2) return(TRUE);


    /* Now create static lights for use during lightmap generation */

    /* Create a point light at the center of the world */
    light = RpLightCreate(rpLIGHTPOINT);
    RSASSERT(NULL != light);
    RpLightSetRadius(light, 0.5f*length);
    RpLightSetColor(light, &pointColor);
    RpLightSetFlags(light, 0);
    frame = RwFrameCreate();
    RSASSERT(NULL != frame);
    RpLightSetFrame(light, frame);
    RwFrameTranslate(frame, &center, rwCOMBINEREPLACE);
    RpWorldAddLight(world, light);

    /* Create a directional 'sun' light */
    light = RpLightCreate(rpLIGHTDIRECTIONAL);
    RSASSERT(NULL != light);
    RpLightSetRadius(light, 0.1f*length);
    RpLightSetColor(light, &sunColor);
    RpLightSetFlags(light, 0);
    frame = RwFrameCreate();
    RSASSERT(NULL != frame);
    RpLightSetFrame(light, frame);
    RwFrameRotate(frame, &xAxis, 45, rwCOMBINEREPLACE);
    RwFrameRotate(frame, &yAxis, 45, rwCOMBINEPOSTCONCAT);
    RwFrameTranslate(frame, &center, rwCOMBINEPOSTCONCAT);
    RwFrameTranslate(frame, &temp, rwCOMBINEPOSTCONCAT);
    RpWorldAddLight(world, light);

    /* Create a dome of dim, directional 'sky' lights so that we get
     * soft near-ambient light from the sky. */
    MakeSkyLights(world, &skyColor, 3);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static void
DrawCone(RwReal coneAngle, RwReal coneSize, RwRGBA *color, RwMatrix *matrix)
{
    static RwIm3DVertex    cone[CONE_NUM_VERTICES + 1];
    static RwImVertexIndex indices[3*(CONE_NUM_VERTICES*2)];
    RwV3d  *right, *up, *at, *pos;
    RwV3d   point;
    RwReal  cosValue, sinValue, coneAngleD;
    RwV3d   dRight, dUp, dAt;
    RwInt32 i, j;

    right = RwMatrixGetRight(matrix);
    up    = RwMatrixGetUp(matrix);
    at    = RwMatrixGetAt(matrix);
    pos   = RwMatrixGetPos(matrix);

    coneAngleD = (RwReal)RwCos(coneAngle);

    for(i = 0;i < CONE_NUM_VERTICES; i++)
    {
        cosValue = (RwReal)(RwCos(i*2*rwPI / CONE_NUM_VERTICES)*
                            RwSin(coneAngle));

        sinValue = (RwReal)(RwSin(i*2*rwPI / CONE_NUM_VERTICES)*
                            RwSin(coneAngle));

        RwV3dScale(&dUp,    up,    sinValue*coneSize);
        RwV3dScale(&dRight, right, cosValue*coneSize);
        RwV3dScale(&dAt,    at,  coneAngleD*coneSize);

        point.x = pos->x + dAt.x + dUp.x + dRight.x;
        point.y = pos->y + dAt.y + dUp.y + dRight.y;
        point.z = pos->z + dAt.z + dUp.z + dRight.z;

        /* Set color & alpha of all points... */
        RwIm3DVertexSetRGBA(&(cone[i + 1]),
            color->red, color->green, color->blue, 128);

        RwIm3DVertexSetPos(&(cone[i + 1]), point.x, point.y, point.z);
    }

    /* Set the color of the apex to something noticeable */
    RwIm3DVertexSetRGBA(&(cone[0]),
                         (color->red   + 128),
                         (color->green + 128),
                         (color->blue  + 128),
                         128);

    /* Set cone apex to light position... */
    RwIm3DVertexSetPos(&(cone[0]),  pos->x, pos->y, pos->z);

    /* Set up trilist indices for the inside of the cone... */
    j = 0;
    for (i = 0;i < CONE_NUM_VERTICES;i++)
    {
        indices[j++] = 0;
        indices[j++] = i + 2;
        indices[j++] = i + 1;
    }
    indices[j - 2] = 1;
    /* ...and the outside of the cone... */
    for (i = 0;i < CONE_NUM_VERTICES;i++)
    {
        indices[j++] = 0;
        indices[j++] = i + 1;
        indices[j++] = i + 2;
    }
    indices[j - 1] = 1;

    if(RwIm3DTransform(cone, (CONE_NUM_VERTICES + 1), NULL, 0))
    {
        /* Draw inside/outside of cone... */
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,
            indices, 3*(CONE_NUM_VERTICES*2));

        RwIm3DEnd();
    }

    return;
}

/*
 *****************************************************************************
 */
static void
DrawSpotLight(RpLight *light)
{
    RwRGBA color;

    RwRGBAFromRwRGBAReal(&color, RpLightGetColor(light));

    DrawCone(RpLightGetConeAngle(light),
             RpLightGetRadius(light),
             &color,
             RwFrameGetLTM(RpLightGetFrame(light)));
    return;
}

/*
 *****************************************************************************
 */
static void
DrawDirectionalLight(RpLight *light)
{
    static RwIm3DVertex    cylinder[DIRECT_LIGHT_NUM_VERTICES*2];
    static RwImVertexIndex indices[3*(2*DIRECT_LIGHT_NUM_VERTICES +
                                      2*(DIRECT_LIGHT_NUM_VERTICES - 2))];
    RwMatrix *matrix;
    RwV3d     right, up, at, *pos;
    RwRGBA    color;
    RwV3d     point;
    RwReal    radius, cosValue, sinValue, coneOffset;
    RwV3d     dRight, dUp, dAt;
    RwUInt32  i, j;

    matrix = RwFrameGetLTM(RpLightGetFrame(light));

    radius = RpLightGetRadius(light);
    RwV3dScale(&right, RwMatrixGetRight(matrix), radius);
    RwV3dScale(&up,    RwMatrixGetUp(matrix),    radius);
    RwV3dScale(&at,    RwMatrixGetAt(matrix),    radius);
    pos   = RwMatrixGetPos(matrix);

    RwRGBAFromRwRGBAReal(&color, RpLightGetColor(light));

    coneOffset = DIRECT_LIGHT_CONE_SIZE*(RwReal)RwCos(DIRECT_LIGHT_CONE_ANGLE);

    for(i = 0;i < (DIRECT_LIGHT_NUM_VERTICES*2);i += 2)
    {
        cosValue = (RwReal)RwCos(rwPI*i / (RwReal)DIRECT_LIGHT_NUM_VERTICES);
        sinValue = (RwReal)RwSin(rwPI*i / (RwReal)DIRECT_LIGHT_NUM_VERTICES);

        RwV3dScale(&dUp,    &up,    sinValue*DIRECT_LIGHT_CYLINDER_DIAMETER);
        RwV3dScale(&dRight, &right, cosValue*DIRECT_LIGHT_CYLINDER_DIAMETER);
        RwV3dScale(&dAt,    &at,   -coneOffset);

        /* Cylinder base vertices... */
        point.x = pos->x + dAt.x + dUp.x + dRight.x;
        point.y = pos->y + dAt.y + dUp.y + dRight.y;
        point.z = pos->z + dAt.z + dUp.z + dRight.z;
        RwIm3DVertexSetPos(&(cylinder[i]), point.x, point.y, point.z);

        /* Cylinder top vertices */
        RwV3dScale(&dAt, &at,
            -(DIRECT_LIGHT_CYLINDER_LENGTH + DIRECT_LIGHT_CONE_SIZE) );
        point.x = pos->x + dAt.x + dUp.x + dRight.x;
        point.y = pos->y + dAt.y + dUp.y + dRight.y;
        point.z = pos->z + dAt.z + dUp.z + dRight.z;
        RwIm3DVertexSetPos(&(cylinder[i + 1]), point.x, point.y, point.z);
    }

    /* Set color & alpha of all points... */
    for(i = 0;i < (2*DIRECT_LIGHT_NUM_VERTICES);i++)
    {
        RwIm3DVertexSetRGBA(
            &(cylinder[i]), color.red, color.green, color.blue, 128);
    }

    /* Set up indices - first the outside of the cylinder */
    j = 0;
    for (i = 0;i < 2*DIRECT_LIGHT_NUM_VERTICES;)
    {
        indices[j++] = i++;
        indices[j++] = i++;
        indices[j++] = (i % (2*DIRECT_LIGHT_NUM_VERTICES));

        indices[j++] = (i % (2*DIRECT_LIGHT_NUM_VERTICES));
        indices[j++] = i - 1;
        indices[j++] = ((i + 1) % (2*DIRECT_LIGHT_NUM_VERTICES));
    }
    /* Then the base */
    for (i = 0;i < (DIRECT_LIGHT_NUM_VERTICES - 2);i++)
    {
        indices[j++] = 0;
        indices[j++] = 2*(i + 1);
        indices[j++] = 2*(i + 2);
    }
    /* Then the top */
    for (i = 0;i < (DIRECT_LIGHT_NUM_VERTICES - 2);i++)
    {
        indices[j++] = 1;
        indices[j++] = 1 + 2*(i + 2);
        indices[j++] = 1 + 2*(i + 1);
    }

    /* Draw the cylinder... */
    if(RwIm3DTransform(cylinder, 2*DIRECT_LIGHT_NUM_VERTICES, NULL, 0))
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indices,
                                     3*(2*DIRECT_LIGHT_NUM_VERTICES +
                                        2*(DIRECT_LIGHT_NUM_VERTICES - 2)));
        RwIm3DEnd();
    }

    /* Draw inverted cone to act as arrow head... */
    DrawCone(DIRECT_LIGHT_CONE_ANGLE, -radius*DIRECT_LIGHT_CONE_SIZE,
            &color, matrix);

    return;
}


/*
 *****************************************************************************
 */
static void
DrawPointLight(RpLight *light, RwCamera *camera)
{
    static RwIm3DVertex    shape[        POINT_LIGHT_NUM_VERTICES];
    static RwImVertexIndex indices[4 + 2*POINT_LIGHT_NUM_VERTICES];
    RwV3d     point, *pos;
    RwMatrix *matrix, *camMatrix, *alignMatrix;
    RwRGBA    color;
    RwReal    radius;
    RwInt32   i, j;

    matrix = RwFrameGetLTM(RpLightGetFrame(light));
    camMatrix = RwFrameGetLTM(RwCameraGetFrame(camera));
    alignMatrix = RwMatrixCreate();

    pos    = RwMatrixGetPos(matrix);
    radius = RpLightGetRadius(light);

    RwRGBAFromRwRGBAReal(&color, RpLightGetColor(light));

    for(i = 0;i < POINT_LIGHT_NUM_VERTICES;i++)
    {
        /* Generate a circle in a plane, perpendicular to the camera's
         * view direction, with the radius of the light */
        RwV3dScale(&point, RwMatrixGetRight(camMatrix), radius);
        RwMatrixRotate(alignMatrix, RwMatrixGetAt(camMatrix),
                       (360*i / (RwReal)POINT_LIGHT_NUM_VERTICES),
                       rwCOMBINEREPLACE);
        RwMatrixTranslate(alignMatrix, pos, rwCOMBINEPOSTCONCAT);
        RwV3dTransformPoint(&point, &point, alignMatrix);

        RwIm3DVertexSetRGBA(
            &(shape[i]), color.red, color.green, color.blue, 255);
        RwIm3DVertexSetPos(&(shape[i]), point.x, point.y, point.z);
    }

    /* Fill in indices for the circle */
    j = 0;
    for (i = 0;i < POINT_LIGHT_NUM_VERTICES;i++)
    {
        indices[j++] = i;
        indices[j++] = i + 1;
    }
    indices[j - 1] = 0;

    /* Fill in indices for  two lines across the middle,
     * at right-angles, which locate the light for us */
    indices[j++] = 0;
    indices[j++] = POINT_LIGHT_NUM_VERTICES >> 1;
    indices[j++] = POINT_LIGHT_NUM_VERTICES >> 2;
    indices[j++] = 3*(POINT_LIGHT_NUM_VERTICES >> 2);

    if(RwIm3DTransform(
        shape, POINT_LIGHT_NUM_VERTICES, NULL, rwIM3D_ALLOPAQUE))
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, indices,
                                     4 + 2*POINT_LIGHT_NUM_VERTICES);
        RwIm3DEnd();
    }

    RwMatrixDestroy(alignMatrix);

    return;
}

/*
 *****************************************************************************
 */
static void
DrawAmbientLight(RpLight *light)
{
    static RwIm3DVertex    shape[8];
    static RwImVertexIndex indices[3*12] = {0,1,2,  1,3,2,
                                            4,6,5,  5,6,7,
                                            0,6,4,  0,2,6,
                                            1,5,7,  1,7,3,
                                            1,0,4,  1,4,5,
                                            3,6,2,  3,7,6};
    const  RwBBox *box;
    RwV3d   infSup[2], center, span;
    RwRGBA  color;
    RwInt32 i;

    /* This function draws an ambient light as polygons covering
     * the inside of the bounding box of this world */

    RwRGBAFromRwRGBAReal(&color, RpLightGetColor(light));

    RSASSERT(NULL != gLightingSession.world);
    box = RpWorldGetBBox(gLightingSession.world);
    RwV3dAdd(&center, &(box->sup), &(box->inf));
    RwV3dScale(&center, &center, 0.5f);
    RwV3dSub(&span, &(box->sup), &(box->inf));
    RwV3dScale(&span, &span, 0.55f);
    RwV3dSub(&(infSup[0]), &center, &span);
    RwV3dAdd(&(infSup[1]), &center, &span);
    for (i = 0;i < 8;i++)
    {
        /* Set up vertices for the world's BBox */
        RwIm3DVertexSetPos(&(shape[i]),
                           infSup[((i&1)>>0)].x,
                           infSup[((i&2)>>1)].y,
                           infSup[((i&4)>>2)].z);
        RwIm3DVertexSetRGBA(&(shape[i]),
                            color.red,
                            color.green,
                            color.blue,
                            128);
    }

    if(RwIm3DTransform(shape, 8, NULL, 0))
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indices, 36);
        RwIm3DEnd();
    }

    return;
}

/*
 *****************************************************************************
 */
static RpLight *
DisplayLightCB(RpLight *light, void *data)
{
    RwCamera *camera = (RwCamera *)data;

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void *)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void *)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     (void *)NULL);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void *)FALSE);

    switch(RpLightGetType(light))
    {
    case rpLIGHTPOINT:
        {
            DrawPointLight(light, camera);
            break;
        }
    case rpLIGHTSPOT:
    case rpLIGHTSPOTSOFT:
        {
            DrawSpotLight(light);
            break;
        }
    case rpLIGHTDIRECTIONAL:
        {
            DrawDirectionalLight(light);
            break;
        }
    case rpLIGHTAMBIENT:
        {
            DrawAmbientLight(light);
            break;
        }
    default:
        /* Maybe platform-specific lights would get here... */
        break;
    }

    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

    return(light);
}

/*
 *****************************************************************************
 */
RwBool
DisplayLights(RwCamera *camera)
{
    RSASSERT(NULL != gLightingSession.world);
    RpWorldForAllLights(gLightingSession.world, DisplayLightCB, camera);

    return(TRUE);
}


/*
 *****************************************************************************
 */
RwBool
ToggleDynamicLights(RpWorld *world)
{
    RwUInt32 i;

    if (FALSE == gUseDynamicLights)
    {
        for (i = 0;i < gNumDynamicLights;i++)
        {
            RpWorldRemoveLight(world, gDynamicLights[i]);
        }
    }
    else
    {
        for (i = 0;i < gNumDynamicLights;i++)
        {
            RpWorldAddLight(world, gDynamicLights[i]);
        }

    }

    return(TRUE);
}

/*
 *****************************************************************************
 */
static RwTexture *
saveImageCB(RwTexture *texture, void *data)
{
    RwChar    *path = (RwChar *)data;
    RwChar    *fullPath;
    RwChar    *search;
    RwImage   *lightMap32;
    RwRaster  *ras;
    RwUInt32   texturenameLength, pathLength, i;

    /* We need enough space for the path and filename and extension */
    texturenameLength = rwstrlen(texture->name);
    pathLength = rwstrlen(path) + texturenameLength + 4 + 1;
    fullPath = (RwChar *)RwMalloc(pathLength*sizeof(RwChar), rwID_NAOBJECT);
    rwstrcpy(fullPath, path);
    for (i = 0;i < (texturenameLength + 4);i++) rwstrcat(fullPath, RWSTRING(" "));

    /* Find where we'll insert the filename */
    search = &(fullPath[rwstrlen(fullPath) - 1]);
    while (*search == ' ') search--;
    search++;
    pathLength = search - fullPath;

    /* Extend the path to be able to hold the full filename */
    rwstrcpy(&(fullPath[pathLength]), texture->name);
    rwstrcat(fullPath, RWSTRING(".png"));

    ras = RwTextureGetRaster(texture);
    lightMap32 = RwImageCreate(RwRasterGetWidth(ras), RwRasterGetHeight(ras), 32);
    RSASSERT(NULL != lightMap32);
    lightMap32 = RwImageAllocatePixels(lightMap32);
    RSASSERT(NULL != lightMap32);
    lightMap32 = RwImageSetFromRaster(lightMap32, ras);
    RSASSERT(NULL != lightMap32);

    RwImageWrite(lightMap32, fullPath);

    RwImageDestroy(lightMap32);

    RwFree(fullPath);

    return(texture);
}


/*
 *****************************************************************************
 */
RwBool
SaveLightMapImages(RwChar *path)
{
    RwTexDictionary *texDict;

    RSASSERT(NULL != path);

    texDict = RwTexDictionaryGetCurrent();

    RwTexDictionaryForAllTextures(texDict, saveImageCB, path);

    return(TRUE);
}

/*
 *****************************************************************************
 */
RwBool
SaveLightMapTexDict(RwChar *path)
{
    RwTexDictionary *texDict;
    RwStream        *stream;

    RSASSERT(NULL != path);

    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, path);
    if(NULL != stream)
    {
        texDict = RwTexDictionaryGetCurrent();

        if (gTexDictIndex == 0)
        {
            if (!RtPITexDictionaryStreamWrite(texDict, stream))
            {
                RwChar string[256];
                RsSprintf(string, "Unable to save PI texture dictionary: %s", path);
                RwDebugSendMessage(
                    rwDEBUGMESSAGE, "RpLtMapWorldSaveTexDictionary", string);
            }
        }
        else
        {
            if (!RwTexDictionaryStreamWrite(texDict, stream))
            {
                RwChar string[256];
                RsSprintf(string, "Unable to save texture dictionary: %s", path);
                RwDebugSendMessage(
                    rwDEBUGMESSAGE, "RpLtMapWorldSaveTexDictionary", string);
            }
        }

        RwStreamClose(stream, NULL);
    }
    else
    {
        RwDebugSendMessage(rwDEBUGMESSAGE, "RpLtMapWorldSaveTexDictionary",
            "Unable to open texture dictionary file for writing");
        return(FALSE);
    }

    return(TRUE);
}


/*
 *****************************************************************************
 */
RwBool
MakeDarkMaps(void)
{
    /*
     * Invert the lightmaps into darkmaps before texture processing.
     */
    RtLtMapSetLightMapProcessCallBack(RtLtMapSkyLightMapMakeDarkMap);

    RtLtMapLightingSessionLightMapProcess(&gLightingSession);

    return(TRUE);
}


/*
 *****************************************************************************
 */
RwBool
ProcessBaseTexture(void)
{
    /* This pre-processes world base textures as required for
     * the PS2 lightmap vector code. Lightmaps should already be
     * in inverted 'darkmap' form (whether loaded from texdicts
     * or images, if just created or cleared). */
    RtLtMapSkyLightingSessionBaseTexturesProcess(&(gLightingSession));

    return(TRUE);
}


/*
 *****************************************************************************
 */
RwBool
SaveScene(void)
{
    RwStream *stream;
    RwUInt32  i;

    /* Save the world, now w/ a second set of lightmap UVs */
    stream = RwStreamOpen(
        rwSTREAMFILENAME, rwSTREAMWRITE, (const void *)CurrentWorldPath);
    if (NULL == stream) return(FALSE);

    if (NULL != gLightingSession.world)
    {
        if (!RpWorldStreamWrite(gLightingSession.world, stream))
        {
            RwStreamClose(stream, NULL);
            /* Restore the app's prelighting state */
            return(FALSE);
        }
        RwStreamClose(stream, NULL);
    }

    /* Save the default example atomics (under different names),
     * now w/ a second set of lightmap UVs and plugin data */
    for (i = 0;i < gNumAtomics;i++)
    {
        if (NULL != gAtomicClumps[i])
        {
            RwChar string[256], subString[32], *newPath;

            rwstrcpy(string, gAtomicPath);
            rwstrcat(string, gAtomicName);
            RsSprintf(subString, "%02d.dff", i);
            rwstrcat(string, subString);
            newPath = RsPathnameCreate(string);

            stream = RwStreamOpen(
                rwSTREAMFILENAME, rwSTREAMWRITE, (const void *)newPath);
            if (NULL == stream) return(FALSE);

            if (!RpClumpStreamWrite(gAtomicClumps[i], stream))
            {
                RwStreamClose(stream, NULL);
                RsPathnameDestroy(newPath);
                return(FALSE);
            }
            RwStreamClose(stream, NULL);

            RsPathnameDestroy(newPath);
        }
    }

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RpWorldSector *
SectorPreLightClearCB(RpWorldSector *sector, void *data)
{
    RwBool clearVertexLitObjects = *(RwBool *)data;

    /* Initialize prelights */
    if (NULL != sector->preLitLum)
    {
        /* We only clear vertex-lit objects if we're explicitly told to */
        if ((FALSE != clearVertexLitObjects) ||
            (!(RtLtMapWorldSectorGetFlags(sector) & rtLTMAPOBJECTVERTEXLIGHT)) )
        {
            RwRGBA clearCol = {128, 128, 128, 255}, black = {0, 0, 0, 255};
            RwUInt32 i;

            /* We clear lightmapped objects to black, non-lightmapped ones to grey
             * (the checkerboard makes cleared, lightmapped objects visible) */
            if (NULL != RpLtMapWorldSectorGetLightMap(sector))
            {
                clearCol = black;
            }

            for (i = 0;i < (RwUInt32)sector->numVertices;i++)
            {
                sector->preLitLum[i] = clearCol;
            }
        }
    }

    return(sector);
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicPreLightClearAll128CB(RpAtomic *atomic, void *data)
{
    RwBool clearVertexLitObjects = *(RwBool *)data;
    RpGeometry *geom = RpAtomicGetGeometry(atomic);
    static RwRGBA clearCol = {128, 128, 128, 255};
    RwUInt32 i;

    /* Are there any prelights to clear? */
    if (RpGeometryGetPreLightColors(geom) == NULL)
        return atomic;

    /* We only clear vertex-lit objects if we're explicitly told to */
    if ((FALSE == clearVertexLitObjects) &&
        (RtLtMapAtomicGetFlags(atomic) & rtLTMAPOBJECTVERTEXLIGHT))
        return atomic;

    for (i = 0;i < (RwUInt32)RpGeometryGetNumVertices(geom);i++)
    {
        RpGeometryGetPreLightColors(geom)[i] = clearCol;
    }

    return(atomic);
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicPreLightClearLMappedBlackCB(RpAtomic *atomic, void *data)
{
    RwBool clearVertexLitObjects = *(RwBool *)data;
    RpGeometry *geom = RpAtomicGetGeometry(atomic);
    RwRGBA clearCol = {0, 0, 0, 255};
    RwUInt32 i;

    /* Are there any prelights to clear? */
    if (RpGeometryGetPreLightColors(geom) == NULL)
        return atomic;

    /* We only clear vertex-lit objects if we're explicitly told to */
    if ((FALSE == clearVertexLitObjects) &&
        (RtLtMapAtomicGetFlags(atomic) & rtLTMAPOBJECTVERTEXLIGHT))
        return atomic;

    /* Objects without a light map (those outside the frustrum usually)
     * are not made black */
    if (RpLtMapAtomicGetLightMap(atomic) == NULL)
        return atomic;

    for (i = 0;i < (RwUInt32)RpGeometryGetNumVertices(geom);i++)
    {
        RpGeometryGetPreLightColors(geom)[i] = clearCol;
    }

    return(atomic);
}


/*
 *****************************************************************************
 */
static RpClump *
ClumpPreLightClearAll128CB(RpClump *clump, void *data)
{
    RpClumpForAllAtomics(clump, AtomicPreLightClearAll128CB, data);
    return(clump);
}

/*
 *****************************************************************************
 */
static RpClump *
ClumpPreLightClearLMappedBlackCB(RpClump *clump, void *data)
{
    RpClumpForAllAtomics(clump, AtomicPreLightClearLMappedBlackCB, data);
    return(clump);
}


/*
 *****************************************************************************
 */
static RpWorldSector *
SectorSetupCB(RpWorldSector *sector, void * data __RWUNUSED__)
{
    /* All objects get lightmapped by default */
    RtLtMapWorldSectorSetFlags(sector,
        RtLtMapWorldSectorGetFlags(sector) | rtLTMAPOBJECTLIGHTMAP);

    return(sector);
}

/*
 *****************************************************************************
 */
static RpWorld *
_loadWorld(RwChar *file)
{
    RpWorld  *world  = (RpWorld *)NULL;
    RwTexDictionary *ltMapTexDict = (RwTexDictionary *)NULL;
    RwStream *stream;
    RwChar   *path;
    RwChar   *imagePath;
    RwChar   *texDictPath;
    RwChar   *lmTexDictPath;
    RwChar   *lmPITexDictPath;
    RwChar   *lightsDffPath;
    RwBool    texDict = FALSE;
    RwChar    string1[256], string2[64], buildpath[256];
    RwChar   *search    = (RwChar *)NULL;
    RwChar   *newSearch = (RwChar *)NULL;
    RwChar   *nameStart = (RwChar *)NULL;
    RwChar    separator[2] = {0, 0};

    if (file == NULL)
        return(NULL);

    /* Load in the user-specified world and texture dictionary */

    path = RsPathnameCreate(file);

    separator[0] = RsPathGetSeparator();
    rwstrcpy(string1, path);
    search = string1;
    while (NULL != (newSearch = rwstrchr(search, separator[0]))) search = newSearch + 1;
    nameStart = search;
    search = rwstrchr(search, '.');
   *search = '\0';
    /* From "[path]/[filename].bsp", this is "[filename]" */
    rwstrcpy(string2, nameStart);
    /* This is "[path]/[filename]/" */
    rwstrcat(string1, separator);

    /* Create an appropriate path for texture images */
    imagePath = RsPathnameCreate(string1);

    /* This is "[path]/[filename]/[filename][target].txd" */
    /* Create an appropriate path for a texture dictionary */
    rwsprintf (buildpath, "%s%s%s.txd", string1, string2, gPlatformString);
    texDictPath = RsPathnameCreate(buildpath);

    /* This is "[path]/[filename]/[filename][target]lm.txd" */
    /* Create an appropriate path for a lightmap texture dictionary */
    rwsprintf (buildpath, "%s%s%slm.txd", imagePath, string2, gPlatformString);
    lmTexDictPath = RsPathnameCreate(buildpath);

    /* This is "[path]/[filename]/[filename]pilm.txd" */
    /* Create an appropriate path for a lightmap PI texture dictionary */
    rwsprintf (buildpath, "%s%spilm.txd", imagePath, string2);
    lmPITexDictPath = RsPathnameCreate(buildpath);

    /* This is "[path]/[filename]/[filename].dff" */
    /* Create an appropriate path for DFF containing lights */
    rwsprintf (buildpath, "%s%s.dff", imagePath, string2);
    lightsDffPath = RsPathnameCreate(buildpath);

    RwImageSetPath(imagePath);

    /* Loading in the lightmap texdict is the app's responsibility...
     * we assume a certain naming convention here ("[filename]lm")
     * when searching for said texdict... if there's no texdict, the
     * worldstreamread func will end up searching for individual,
     * platform-independent images in the same path as for the BSP's
     * base textures. */

    /* This texture dictionary contains both the base textures and the
     * lightmap textures.
     *
     * For this example, we assume the texture dictionary is stored in a
     * platform independant format. This is so we can generate lightmaps on
     * a host platform, eg a PC, for use on a target platform, eg PlayStation 2.
     *
     * Since PC cannot write out native platform textures, we need to
     * export them in a platform independant format. They will be converted
     * into the platform's format during reading.
     *
     * In a final game, this texture dictionary should have been converted to
     * the native platform format via a seperate tool before being loaded.
     */

    /* Attempt to load in a lightmap PI texture dictionary */
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, lmTexDictPath);
    if(NULL != stream)
    {
        if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, NULL, NULL))
        {
            ltMapTexDict = RwTexDictionaryStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    if (NULL == ltMapTexDict)
    {
        /* Attempt to load in a lightmap PI texture dictionary */
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, lmPITexDictPath);
        if(NULL != stream)
        {
            if(RwStreamFindChunk(stream, rwID_PITEXDICTIONARY, NULL, NULL))
            {
                ltMapTexDict = RtPITexDictionaryStreamRead(stream);
            }

            RwStreamClose(stream, NULL);
        }
    }

    if (NULL == ltMapTexDict)
    {
        /* Attempt to load in a texture dictionary */
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, texDictPath);
        if(NULL != stream)
        {
            if(RwStreamFindChunk(stream, rwID_TEXDICTIONARY, NULL, NULL))
            {
                ltMapTexDict = RwTexDictionaryStreamRead(stream);
            }

            RwStreamClose(stream, NULL);
        }
   }

    /* Make it the current dictionary... */
    if(NULL != ltMapTexDict)
    {
        RwTexDictionarySetCurrent(ltMapTexDict);
        texDict = TRUE;
    }
    else
    {
        /* Create a blank texDict - we need a holder for our lightmaps
         * once we load them from images or create them anyway... */
        RwTexDictionarySetCurrent(RwTexDictionaryCreate());
    }

    /* Attempt to load in the BSP file... */
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    if(NULL != stream)
    {
        if( RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL) )
        {
            world = RpWorldStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    if (NULL != world)
    {
        /* Detect bad data */
        if (!(RpWorldGetFlags(world) & rpWORLDTEXTURED2))
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, "_loadWorld",
                "World must contain at least two texture coordinate sets");
            RpWorldDestroy(world);
            world = (RpWorld *)NULL;
        }
#if (defined(SKY2_DRVMODEL_H) || defined(NULLSKY_DRVMODEL_H))
        /* The PS2 pipes only work w/ tristrips atm */
        if (!(world->flags & rpWORLDTRISTRIP))
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, "_loadWorld",
                "The PS2 lightmap pipeline only supports tristripped geometry.");
            RpWorldDestroy(world);
            world = (RpWorld *)NULL;
        }
#endif /* (defined(SKY2_DRVMODEL_H) || defined(NULLSKY_DRVMODEL_H)) */
    }

    /* Load in a DFF containing lights if there is one */
    if (NULL != world)
    {
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, lightsDffPath);
        if(NULL != stream)
        {
            if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
            {
                RpClump *clump = RpClumpStreamRead(stream);
                if (NULL != clump)
                {
                    RpWorldAddClump(world, clump);
                }
            }

            RwStreamClose(stream, NULL);
        }
    }

    RsPathnameDestroy(lightsDffPath);
    RsPathnameDestroy(lmTexDictPath);
    RsPathnameDestroy(lmPITexDictPath);
    RsPathnameDestroy(texDictPath);
    RsPathnameDestroy(imagePath);
    RsPathnameDestroy(path);

#if (defined(SKY2_DRVMODEL_H) || defined(NULLSKY_DRVMODEL_H))
    if (NULL != world)
    {
        /* Worlds are around the camera and have big polygons,
         * so we enable true clipping (not fast-culling): */
        RpSkySelectTrueTSClipper(TRUE);
        RpSkySelectTrueTLClipper(TRUE);
    }
#endif /* (defined(SKY2_DRVMODEL_H) || defined(NULLSKY_DRVMODEL_H)) */

    return(world);
}


/*
 *****************************************************************************
 */
RpWorld *
LoadWorld(RwChar *file, RpWorld *oldWorld, RwCamera *camera)
{
    static RwBool startup = TRUE;
    RwBool clearVertexLitObjects = FALSE;
    RpWorld *newWorld;
    RwReal worldDensity;
    RwReal        length;
    const RwBBox *box;
    RwV3d         size;

    /* Destroy the clumps, lights and area lights in the scene. */
    ResetScene(oldWorld, camera);

    /* Load in a new world... */
    newWorld = _loadWorld(file);
    if(newWorld == NULL)
    {
        RwBBox box = {{100, 100, 100}, {0, 0, 0}};

        /* If the load fails, create a blank world here */
        newWorld = RpWorldCreate(&box);

        RwDebugSendMessage(rwDEBUGERROR, "LoadWorld", "Failed to load world");
    }

    /* [Re-]initialize our lighting session */
    RtLtMapLightingSessionInitialize(&gLightingSession, newWorld);

#ifdef FULLSCENELIGHTING
    gLightingSession.camera = NULL;
#else
    /* Set up the camera in our lighting session. Note that we leave
     * the atomic/sector lists NULL as we wish to light the world and
     * all atomics inside it (well, those within the camera frustum) */
    gLightingSession.camera = camera;
#endif /* FULLSCENELIGHTING     */

    /* Place the camera in the new world */
    if (NULL != camera)
    {
        RpWorldAddCamera(newWorld, camera);
        CameraInitPosition(camera, newWorld);
    }

    /* Set up some atomics for the default BSP */
    if (startup)
    {
        LoadAtomics(newWorld);
    }

    /* Set up a default LightMap size for the world */
    box = RpWorldGetBBox(newWorld);
    RwV3dSub(&size, &(box->sup), &(box->inf));
    length = RwV3dLength(&size);

    /* This density is reasonable for the example level (it lights
     * fairly quickly and all objects fit into one 512x512 lightmap),
     * but can be modified from the menu. */
    RtLtMapLightMapSetDefaultSize(LightMapSize);

    /* Set up sector properties for the BSP */
    RpWorldForAllWorldSectors(newWorld, SectorSetupCB, NULL);

    /* Set up material properties for the BSP */
    RpWorldForAllMaterials(newWorld, SetupMaterialsCB, NULL);

    /* Use LightMapping tool to calculate a density that hopefully fits
     * all world onto a single lightmap */
    worldDensity = RtLtMapWorldCalculateDensity (newWorld);
    RpWorldForAllMaterials(newWorld, ScaleMaterialsDensityCB     , (void *) &worldDensity);
    /* Scale only the first clump as the others are clones and share the materials */
    if (gNumAtomics > 0)
    {
        ScaleClumpMaterialsDensityCB (gAtomicClumps [0], (void *) &worldDensity);
    }

    /* Alter the naming of lightmaps */
    RtLtMapSetDefaultPrefixString(gLightMapPrefix);

    /* Create a set of lights if the new world doesn't have any already */
    SetupLights(newWorld);

    /* Get it rendering with the current style */
    RpLtMapSetRenderStyle(
        gRenderStyle|(gPointSampling?rpLTMAPSTYLEPOINTSAMPLE:0), newWorld);

    /* Clear prelights to grey (for non-lightmapped, non-vertex-lit objects)
     * or black (for lightmapped, non-vertex-lit objects) */
    RpWorldForAllWorldSectors(
        newWorld, SectorPreLightClearCB, &clearVertexLitObjects);
    RpWorldForAllClumps(
        newWorld, ClumpPreLightClearAll128CB,  &clearVertexLitObjects);
    RpWorldForAllClumps(
        newWorld, ClumpPreLightClearLMappedBlackCB,  &clearVertexLitObjects);

    startup = FALSE;

    return(newWorld);
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicSetupCB(RpAtomic *atomic, void *data __RWUNUSED__)
{
#if (defined(SKY2_DRVMODEL_H) || defined(NULLSKY_DRVMODEL_H))
    RpGeometry *geom = RpAtomicGetGeometry(atomic);
    RwBool *valid = (RwBool *)data;
    if (!(RpGeometryGetFlags(geom) & rpGEOMETRYTRISTRIP))
    {
       *valid = FALSE;
        return((RpAtomic *)NULL);
    }
#endif /* (defined(SKY2_DRVMODEL_H) || defined(NULLSKY_DRVMODEL_H)) */

    /* All new objects get lightmapped by default */
    if (!RtLtMapAtomicGetFlags(atomic))
    {
        RtLtMapAtomicSetFlags(atomic,
            RtLtMapAtomicGetFlags(atomic) | rtLTMAPOBJECTLIGHTMAP);
    }

    return(atomic);
}


/*
 *****************************************************************************
 */
RpClump *
LoadClump(const RwChar *file)
{
    RwStream *stream;
    RpClump *clump = (RpClump *)NULL;

    /* This cancels any lighting going on, since loading a new
     * atomic could affect the current lighting session. */
    gResetLighting = TRUE;

    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, file);
    if(NULL != stream)
    {
        if(RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL))
        {
            clump = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    /* The PS2 pipes only work w/ tristrips atm */
    if (NULL != clump)
    {
        RwBool valid = TRUE;

        RpClumpForAllAtomics(clump, AtomicSetupCB, &valid);
        if (FALSE == valid)
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, "LoadClump",
                "The PS2 lightmap pipeline only supports tristripped geometry.");
            RpClumpDestroy(clump);
            clump = (RpClump *)NULL;
        }
    }

    return(clump);
}


/*
 *****************************************************************************
 */
RwBool
LightScene(RwCamera *camera)
{
    static RwCamera *lightingCamera    = (RwCamera *)NULL;
    RwInt32  deltaObjs;
    RwUInt32 i;
   
    if (gLightingWorld == FALSE || gResetLighting == TRUE ||
        (gLightingSession.totalObj > 0 && gLightingSession.startObj >= gLightingSession.totalObj))
    {
        /* Lighting is now complete, or someone loaded a new world or something,
         * so destroy any variables that were used for lighting */
        gLightingSession.totalObj = 0;
        gLightingWorld    = FALSE;
        gResetLighting    = FALSE;
        gLightingProgress = 0;
        if (NULL != lightingCamera)
        {
            RwFrame *frame = RwCameraGetFrame(lightingCamera);
            RwCameraSetFrame(lightingCamera, (RwFrame *)NULL);
            RwFrameDestroy(frame);
            RpWorldRemoveCamera(gLightingSession.world, lightingCamera);
            RwCameraDestroy(lightingCamera);
            lightingCamera = (RwCamera *)NULL;
        }
        return TRUE;
    }

    if (RpLtMapWorldLightMapsQuery(gLightingSession.world) == 0)
    {
        /* Create lightmaps if they don't exist already */
        gCreatingLightMaps = TRUE;
        return TRUE;
    }

    /* Beginning to lightmap a new collection of geometries / atomics? */
    if (gLightingSession.totalObj == 0)
    {
        /* We don't call ClearWorld() here because it is unnecessary.
         * Any new lighting will overwrite old lighting values. So,
         * if you move any lights and then relight, you'll see the
         * lightmaps change in only the affected areas. You could, for
         * efficiency, only relight the objects in the ROI of the change.*/

        /* Capture the current camera to allow it to safely move around during lighting */
        if ((NULL != camera) && (NULL == lightingCamera))
        {
            RwMatrix matrix;
            RwFrame *frame;

#ifdef FULLSCENELIGHTING
            lightingCamera = NULL;
#else
            /* Note: RpCameraClone() automatically adds the clone
             * camera to the world owning the source camera */
            lightingCamera = RwCameraClone(camera);
            RSASSERT(NULL != lightingCamera);
#endif

            /* Fix it in place */
            frame = RwFrameCreate();
            RSASSERT(NULL != frame);
            RwCameraSetFrame(lightingCamera, frame);
            RwMatrixCopy(&matrix, RwFrameGetLTM(RwCameraGetFrame(camera)));
            RwFrameTransform(frame, &matrix, rwCOMBINEREPLACE);
            /* Gather sectors-in-frustum */
            RwCameraBeginUpdate(camera);
            RwCameraEndUpdate(camera);
        }

        gLightingSession.startObj = 0;
    }

    if ((NULL == gAreaLights) &&
        (FALSE != gUseAreaLights))
    {
        /* This can be used to globally reduce/increase the number of area light
         * samples which will be created (this will reduce/increase the time taken
         * to light the scene). */
        RtLtMapSetAreaLightDensityModifier(1.0f);

        /* This can be used to globally reduce/increase the brightness of area lights
         * (this will also reduce/increase the time taken to light the scene). */
        RtLtMapSetAreaLightRadiusModifier(4.0f);

        /* This sets the tolerance for visual error in internal calculations which
         * limit the region of influence of area lights. Higher values look worse but
         * result in faster lightmap illumination. */
        RtLtMapSetAreaLightErrorCutoff(4);

        /* Create the area lights using the default sample density */
        gAreaLights = RtLtMapAreaLightGroupCreate(&gLightingSession, 0);
    }

    /* gLightingSession.startObj persists from the
     * last slice and we process some more samples */
    gLightingSession.numObj = 1;

    /* parameters.totalSamples gets filled by RtLtMapWorldIlluminate */

    /* Toggle light flags so that static lights get used and dynamic ones don't */
    if (FALSE != gUseRpLights)
    {
        RpWorldForAllLights(gLightingSession.world, LightToggleCB, NULL);
    }
    else
    {
        /* If RpLights aren't being used for illumination,
         * just toggle the dynamic lights off */
        for (i = 0;i < gNumDynamicLights;i++)
        {
            (void)RpLightSetFlags(gDynamicLights[i],
                RpLightGetFlags(gDynamicLights[i]) ^
                (rpLIGHTLIGHTWORLD | rpLIGHTLIGHTATOMICS));
        }
    }

    /* Use the temporary clone camera */
    gLightingSession.camera = lightingCamera;

    /* Perform one 'slice' of illumination */
    if (FALSE != gUseAreaLights)
    {
        deltaObjs = RtLtMapIlluminate(&gLightingSession, gAreaLights,
            (RwUInt32) LightMapScale);
    }
    else
    {
        deltaObjs = RtLtMapIlluminate(&gLightingSession, (RtLtMapAreaLightGroup *)NULL,
            (RwUInt32) LightMapScale);
    }
    RSASSERT(-1 != deltaObjs);

    /* Don't use the temporary clone camera outside of this function */
    gLightingSession.camera = camera;

    /* Toggle light flags back so that dynamic lights get used and static ones don't */
    if (gUseRpLights)
    {
        RpWorldForAllLights(gLightingSession.world, LightToggleCB, NULL);
    }
    else
    {
        /* Toggle the dynamic lights back on */
        for (i = 0;i < gNumDynamicLights;i++)
        {
            (void)RpLightSetFlags(gDynamicLights[i],
                RpLightGetFlags(gDynamicLights[i]) ^
                (rpLIGHTLIGHTWORLD | rpLIGHTLIGHTATOMICS));
        }
    }

    gLightingSession.startObj += deltaObjs;

    gLightingProgress = (RwReal) gLightingSession.startObj /
        (RwReal)gLightingSession.totalObj;

    return TRUE;
}

/*
 *****************************************************************************
 */
RwBool
CreateLightMaps(void)
{
    RwCamera     *oldCamera;

    if (!gCreatingLightMaps)
        return TRUE;

    /* Create lightmaps for the entire scene */
    oldCamera = gLightingSession.camera;
    gLightingSession.camera = NULL;
    RtLtMapLightMapsCreate(&gLightingSession, LightMapDensity, (RwRGBA *)NULL);
    gLightingSession.camera = oldCamera;

    gCreatingLightMaps = FALSE;

    /* Do this primarily to ensure prelights are black (if they're
     * white, you can't see the lightmaps!) */
    gClearingLightMaps = TRUE;

    return(TRUE);
}


/*
 *****************************************************************************
 */
RwBool
ClearLightMaps(void)
{
    RwBool clearVertexLitObjects = TRUE;

    if (!gClearingLightMaps)
        return TRUE;

    /* If the scene doesn't have lightmaps already, create 'em */
    if (RpLtMapWorldLightMapsQuery (gLightingSession.world) == 0)
    {
        gCreatingLightMaps = TRUE;
        return TRUE;
    }

    /* Clear lightmaps to a black'n'white checkerboard */
    RtLtMapLightMapsClear(&gLightingSession, (RwRGBA *) NULL);

    /* Also clear prelights to black (for lightmapped objects)
     * or grey (for non-lightmapped objects) */
    RSASSERT(NULL != gLightingSession.world);
    RpWorldForAllWorldSectors(gLightingSession.world,
                              SectorPreLightClearCB,
                             &clearVertexLitObjects);
    RpWorldForAllClumps(gLightingSession.world,
                        ClumpPreLightClearAll128CB,
                       &clearVertexLitObjects);
    RpWorldForAllClumps(gLightingSession.world,
                        ClumpPreLightClearLMappedBlackCB,
                       &clearVertexLitObjects);

    gClearingLightMaps = FALSE;

    return(TRUE);
}


/*
 *****************************************************************************
 */
RwBool
DestroyLightMaps(void)
{
    RwBool clearVertexLitObjects = FALSE;
    RwCamera *backupCam;

    gResetLighting = TRUE;

    /* Destroy the lightmaps in the scene */
    /* Temporarily disable the camera so that all lightmaps in the scene are destroyed */
    backupCam = gLightingSession.camera;
    gLightingSession.camera = (RwCamera *)NULL;
    RtLtMapLightMapsDestroy(&gLightingSession);
    gLightingSession.camera = backupCam;

    /* Clear prelights to white again (for non-vertex-lit objects) */
    RSASSERT(NULL != gLightingSession.world);
    RpWorldForAllWorldSectors(
        gLightingSession.world, SectorPreLightClearCB, &clearVertexLitObjects);
    RpWorldForAllClumps(
        gLightingSession.world, ClumpPreLightClearAll128CB,  &clearVertexLitObjects);
    /* Get the new prelights reinstanced */
    RwResourcesEmptyArena();

    return(TRUE);
}

