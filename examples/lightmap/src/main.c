
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
 * Purpose: Example to demonstrate the functions of light maps as
   as they are provided in RpLightmap and RtLightmap.
 *
*****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#if (defined(SKY2_DRVMODEL_H))
#include "rppds.h"
#endif /* (defined(SKY2_DRVMODEL_H)) */

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "rtpng.h"
#include "rtcharse.h"
#include "rprandom.h"
#include "rpcollis.h"
#include "rpmipkl.h"
#include "rppvs.h"
#include "rpltmap.h"
#include "rtltmap.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#include "lightmap.h"
#include "main.h"


#define DEFAULT_SCREEN_WIDTH (640)
#define DEFAULT_SCREEN_HEIGHT (480)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define DEFAULT_VIEWWINDOW (0.7f)

#define CAMERAMAXSPEED (99000.000f)
#define CAMERAMINSPEED (00000.001f)

#define LIGHTMAPMINDENSITY (1 / (RwReal)(1 << 8))
#define LIGHTMAPMAXDENSITY ((RwReal)(1 << 8))

#define LIGHTMAPMINSIZE (16)
#define LIGHTMAPMAXSIZE (1024)

#define NOWIREFRAME   0
#define WIREBOXES     1
#define WIRETRIANGLES 2

/* Number of lights to split each light into when jittering */
#define JITTER      (20)
/* Percentage of light radius to jitter position by */
#define POSJITTER   (0.05f)
/* Degrees to jitter light direction by */
#define JITTERANGLE (6)

/* Used in here and events.c to extend menu functionality */
RwInt32 gFakeEnums[MENUSTRINGARRAYENTRIES] = {FAKEENUMDEFAULTVAL,
                                              FAKEENUMDEFAULTVAL,
                                              FAKEENUMDEFAULTVAL};

RwChar CurrentWorldPath[256] = RWSTRING("");

/* Navigation */
RwReal CameraPitchRate = 0.0f;
RwReal CameraTurnRate = 0.0f;
RwReal CameraMaxSpeed = 0.0f;
RwReal CameraSpeed = 0.0f;
RwReal CameraStrafeSpeed = 0.0f;

/* Lightmap creation modifiers */
RwReal   LightMapDensity = 1.0f;
RwUInt32 LightMapSize = DEFAULTLIGHTMAPSIZE;
RwInt32  LightMapScale = 1;

static RwBool     gDisplayFPS     = FALSE;
static RwBool     gDrawLights     = FALSE;
static RwUInt32   gDrawWireframe  = NOWIREFRAME;
static RwInt32    FrameCounter    = 0;
static RwInt32    FramesPerSecond = 0;

static RwRGBA     ForegroundColor = {200, 200, 200, 255};
static RwRGBA     BackgroundColor = { 64,  64,  64,   0};
static RtCharset *Charset         = NULL;

static RpWorld   *World  = (RpWorld  *)NULL;
static RwCamera  *Camera = (RwCamera *)NULL;

/* Menu strings and whatnot */
static RwChar Blank[10] = RWSTRING("");
static RwChar *gMenuStrings[MENUSTRINGARRAYENTRIES][3] =
    {{&(Blank[0]), &(Blank[0]), &(Blank[0])},
     {&(Blank[0]), &(Blank[0]), &(Blank[0])},
     {&(Blank[0]), &(Blank[0]), &(Blank[0])}};


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

        RsGlobal.appName = RWSTRING("RenderWare Graphics Lightmap Example");

        return TRUE;
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
RwBool
ChangeCameraSpeed(RwBool justCheck)
{
    static RwChar string[10];

    if (FALSE != justCheck)
    {
        /* This makes sure the string's properly initialized */
        RsSprintf(string, "%9.3f", CameraMaxSpeed);
        gMenuStrings[MENUSTRINGARRAYCAMSPEED][1] = string;
        return(TRUE);
    }

    if (FAKEENUMMINVAL == gFakeEnums[MENUSTRINGARRAYCAMSPEED])
    {
        CameraMaxSpeed *= 0.5f;
        if (CameraMaxSpeed < CAMERAMINSPEED) CameraMaxSpeed = CAMERAMINSPEED;
    }
    else
    {
        CameraMaxSpeed *= 2.0f;
        if (CameraMaxSpeed > CAMERAMAXSPEED) CameraMaxSpeed = CAMERAMAXSPEED;
    }

    gFakeEnums[MENUSTRINGARRAYCAMSPEED] = FAKEENUMDEFAULTVAL;

    RsSprintf(string, "%9.3f", CameraMaxSpeed);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
ChangeLightMapDensity(RwBool justCheck)
{
    static RwChar string[10];

    if (FALSE != justCheck)
    {
        /* This makes sure the string's properly initialized */
        RsSprintf(string, "%9.3f", LightMapDensity);
        gMenuStrings[MENUSTRINGARRAYDENSITY][1] = string;
        return(TRUE);
    }

    if (FAKEENUMMINVAL == gFakeEnums[MENUSTRINGARRAYDENSITY])
    {
        LightMapDensity *= 0.5f;
        if (LightMapDensity < LIGHTMAPMINDENSITY)
        {
            LightMapDensity = LIGHTMAPMINDENSITY;
        }
    }
    else
    {
        LightMapDensity *= 2.0f;
        if (LightMapDensity > LIGHTMAPMAXDENSITY)
        {
            LightMapDensity = LIGHTMAPMAXDENSITY;
        }
    }

    gFakeEnums[MENUSTRINGARRAYDENSITY] = FAKEENUMDEFAULTVAL;

    RsSprintf(string, "%9.3f", LightMapDensity);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
ChangeLightMapSize(RwBool justCheck)
{
    static RwChar string[10];

    if (FALSE != justCheck)
    {
        /* This makes sure the string's properly initialized */
        RsSprintf(string, "%9d", LightMapSize);
        gMenuStrings[MENUSTRINGARRAYMAPSIZE][1] = string;
        return(TRUE);
    }

    if (FAKEENUMMINVAL == gFakeEnums[MENUSTRINGARRAYMAPSIZE])
    {
        LightMapSize >>= 1;
        if (LightMapSize < LIGHTMAPMINSIZE)
        {
            LightMapSize = LIGHTMAPMINSIZE;
        }
    }
    else
    {
        LightMapSize *= 2;
        if (LightMapSize > LIGHTMAPMAXSIZE)
        {
            LightMapSize = LIGHTMAPMAXSIZE;
        }
    }

    gFakeEnums[MENUSTRINGARRAYMAPSIZE] = FAKEENUMDEFAULTVAL;

    RsSprintf(string, "%9d", LightMapSize);

    RtLtMapLightMapSetDefaultSize(LightMapSize);

    return(TRUE);
}

/*
 *****************************************************************************
 */
static RwBool
ChangeLightMapScale(RwBool justCheck)
{
    if (FALSE != justCheck) return (TRUE);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MenuCreateLightMaps(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    gCreatingLightMaps = TRUE;

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MenuClearLightMaps(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    /* This cancels any lighting going on */
    gResetLighting = TRUE;
    gClearingLightMaps = TRUE;

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MenuDestroyLightMaps(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    DestroyLightMaps();

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MenuMakeDarkMaps(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    MakeDarkMaps();

    return(TRUE);
}

/*
 *****************************************************************************
 */
static RwBool
MenuProcBaseTex(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    ProcessBaseTexture();

    return(TRUE);
}

/*
 *****************************************************************************
 */
static RwBool
TexDictCallback(RwBool justCheck)
{
    if( justCheck )
    {
        return TRUE;
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
LumCalcCallback(RwBool justCheck)
{
    if( justCheck )
    {
        return TRUE;
    }

    switch( gLumCalcIndex )
    {
        case 0:
        {
            RtLtMapSkySetLumCalcCallBack(RtLtMapSkyLumCalcMaxCallBack);

            break;
        }

        case 1:
        {
            RtLtMapSkySetLumCalcCallBack(RtLtMapSkyLumCalcSigmaCallBack);

            break;
        }

        case 2:
        {
            RtLtMapSkySetLumCalcCallBack(RtLtMapSkyLumResetCallBack);

            break;
        }
    }

    return TRUE;
}

/*
 *****************************************************************************
 */
static RwBool
MenuRenderStyleCB(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    /* Toggle the current render style */
    RpLtMapSetRenderStyle(
        (gRenderStyle & (rpLTMAPSTYLERENDERBASE|rpLTMAPSTYLERENDERLIGHTMAP)) |
        (gPointSampling ? rpLTMAPSTYLEPOINTSAMPLE : 0),
        World);

    return(TRUE);
}

/*
 *****************************************************************************
 */
static RwBool
MenuPointSampleCB(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    RpLtMapSetRenderStyle(
        (gRenderStyle & (rpLTMAPSTYLERENDERBASE|rpLTMAPSTYLERENDERLIGHTMAP)) |
        (gPointSampling ? rpLTMAPSTYLEPOINTSAMPLE : 0),
        World);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MenuDynamicCB(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    ToggleDynamicLights(World);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MenuSaveLightMap(RwBool justCheck)
{
    RwChar  imagePath[256];
    RwChar *search, *newSearch;
    RwChar  separator[2] = {0, 0};

    if (FALSE != justCheck) return(TRUE);

    separator[0] = RsPathGetSeparator();
    rwstrcpy(imagePath, CurrentWorldPath);
    search = imagePath;
    while (NULL != (newSearch = rwstrchr(search, '.'))) search = newSearch + 1;
   *(search - 1) = '\0';
    /* This is "[path]/[filename]/" */
    rwstrcat(imagePath, separator);

    SaveLightMapImages(imagePath);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MenuSaveLightMapDict(RwBool justCheck)
{
    RwChar  texDictPath[256], name[64];
    RwChar *search, *newSearch;
    RwChar  separator[2] = {0, 0};

    if (FALSE != justCheck) return(TRUE);

    separator[0] = RsPathGetSeparator();
    /* CurrentWorldPath holds "[path]/[filename].bsp" */
    rwstrcpy(texDictPath, CurrentWorldPath);
    search = texDictPath;
    while (NULL != (newSearch = rwstrchr(search, '.'))) search = newSearch + 1;
    /* Truncate the file extension to give "[path]/[filename] */
   *(search - 1) = '\0';
    search = texDictPath;
    while (NULL != (newSearch = rwstrchr(search, separator[0]))) search = newSearch + 1;
    /* This is "[filename]" */
    rwstrcpy(name, search);
    /* This is "[path]/[filename]/" */
    rwstrcat(texDictPath, separator);

    if (gTexDictIndex == 0)
    {
        /* This is "[path]/[filename]/[filename]pilm.txd" */
        rwstrcat(texDictPath, name);
        rwstrcat(texDictPath, RWSTRING("pilm.txd"));
    }
    else
    {
        /* This is "[path]/[filename]/[filename][platform]lm.txd" */
        rwstrcat(texDictPath, name);
        rwstrcat(texDictPath, gPlatformString);
        rwstrcat(texDictPath, RWSTRING("lm.txd"));
    }

    SaveLightMapTexDict(texDictPath);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MenuSaveObjects(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    SaveScene();

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
MenuCancelLighting(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    gResetLighting = TRUE;

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RpLight *
LightJitterCB(RpLight *light, void *data)
{
    const RwReal magic = 0.9983126f;
    RpWorld *world = (RpWorld *)data;
    RpLight *lights[JITTER];
    RwReal   scalar = 1.0f / JITTER;
    RwUInt32 flags;

    /* Jitter lights to produce soft shadows by replacing lights
     * with lots of dimmer lights jittered in location and direction */

    flags = RpLightGetFlags(light);

    if ((magic != RpLightGetColor(light)->alpha) &&
        (!((rpLIGHTLIGHTWORLD | rpLIGHTLIGHTATOMICS) & flags)) &&
        (rpLIGHTAMBIENT       != RpLightGetType(light)) )
    {
        RwRGBAReal newColour;
        RwV3d     *pos, *at, *right, *up;
        RwV3d      minusPos, newPos;
        RwFrame   *frame;
        RwReal     radius, temp;
        RwInt32    i;

        /* Avoid jittering very dim lights
         * (e.g the already-jittered sky-dome lights) */
        temp = RpLightGetColor(light)->red +
               RpLightGetColor(light)->green +
               RpLightGetColor(light)->blue;
        if (temp < 0.3f) return(light);

        /* Use 'magic' as a flag so we don't duplicate the new lights! */
        newColour.alpha = magic;
        newColour.red   = scalar*RpLightGetColor(light)->red;
        newColour.green = scalar*RpLightGetColor(light)->green;
        newColour.blue  = scalar*RpLightGetColor(light)->blue;
        RpLightSetColor(light, &newColour);

        pos    = RwMatrixGetPos(RwFrameGetLTM(RpLightGetFrame(light)));
        RwV3dScale(&minusPos, pos, -1);
        at     = RwMatrixGetAt(   RwFrameGetLTM(RpLightGetFrame(light)));
        right  = RwMatrixGetRight(RwFrameGetLTM(RpLightGetFrame(light)));
        up     = RwMatrixGetUp(   RwFrameGetLTM(RpLightGetFrame(light)));
        radius = RpLightGetRadius(light);

        for (i = 1;i < JITTER;i++)
        {
            lights[i] = RpLightCreate(RpLightGetType(light));
            RSASSERT(NULL != lights[i]);
            frame = RwFrameCreate();
            RSASSERT(NULL != frame);
            lights[i] = RpLightSetFrame(lights[i], frame);
            (void)RpLightSetFlags(lights[i], flags);
            RpWorldAddLight(world, lights[i]);

            RpLightSetRadius(lights[i], radius);
            RpLightSetConeAngle(lights[i], RpLightGetConeAngle(light));
            RpLightSetColor(lights[i], &newColour);

            newPos = *pos;
            if ((rpLIGHTSPOT     == RpLightGetType(light)) ||
                (rpLIGHTSPOTSOFT == RpLightGetType(light)) )
            {
                RwV3d tempX, tempY;

                /* Jitter position (perpendicular to the spotlight's At direction) */
                RwV3dScale(&tempX, right,
                    2*POSJITTER*radius*((rand()/(RwReal)RAND_MAX) - 0.5f));
                RwV3dScale(&tempY,    up,
                    2*POSJITTER*radius*((rand()/(RwReal)RAND_MAX) - 0.5f));
                RwV3dAdd(&newPos, &newPos, &tempX);
                RwV3dAdd(&newPos, &newPos, &tempY);
            }
            else
            {
                /* Jitter position (within a cube for simplicity) */
                newPos.x += POSJITTER*radius*((rand()/(RwReal)RAND_MAX) - 0.5f);
                newPos.y += POSJITTER*radius*((rand()/(RwReal)RAND_MAX) - 0.5f);
                newPos.z += POSJITTER*radius*((rand()/(RwReal)RAND_MAX) - 0.5f);
            }

            /* Rotate the new lights about the existing one */
            RwMatrixCopy(RwFrameGetMatrix(frame),
                         RwFrameGetLTM(RpLightGetFrame(light)));
            /* Move back to the origin */
            RwFrameTranslate(frame, &minusPos, rwCOMBINEPOSTCONCAT);
            if (rpLIGHTDIRECTIONAL == RpLightGetType(light))
            {
                /* Rotate away from the original direction */
                RwFrameRotate(frame, right, JITTERANGLE, rwCOMBINEPOSTCONCAT);
                /* Rotate *around* the original direction */
                RwFrameRotate(frame, at, 360*(i / (RwReal)(JITTER - 1)),
                              rwCOMBINEPOSTCONCAT);
            }
            /* Translate to the final position */
            RwFrameTranslate(frame, &newPos, rwCOMBINEPOSTCONCAT);
        }
    }

    return(light);
}


/*
 *****************************************************************************
 */
static RwBool
MenuJitterLights(RwBool justCheck)
{
    if (FALSE != justCheck) return(TRUE);

    /* Jitter each light source loaded in - gives nice soft shadows! :) */
    RpWorldForAllLights(World, LightJitterCB, (void *)World);

    return(TRUE);
}


/*
 *****************************************************************************
 */
static RwBool
InitializeMenu(void)
{
    static RwChar fpsLabel[]          = RWSTRING("FPS_F");
    static RwChar camSpeedLabel[]     = RWSTRING("Camera Speed");

    static RwChar createLabel[]       = RWSTRING("Create Lightmaps_C");
    static RwChar clearLabel[]        = RWSTRING("Clear  Lightmaps_X");
    static RwChar darkMapLabel[]      = RWSTRING("Make DarkMaps_D");
    static RwChar procBaseTexLabel[]  = RWSTRING("Process Base Texture_U");
    static RwChar destroyLabel[]      = RWSTRING("Destroy Lightmaps_Q");
    static RwChar densityLabel[]      = RWSTRING("Lightmap Density");
    static RwChar sizeLabel[]         = RWSTRING("Lightmap Size");
    static RwChar scaleLabel[]         = RWSTRING("Lightmap SuperSample");

    static RwChar lightLabel[]        = RWSTRING("Lighting_L");
    static RwChar cancelLightLabel[]  = RWSTRING("Cancel Lighting_K");
    static RwChar areaLightLabel[]    = RWSTRING("Area Lights_A");
    static RwChar rpLightLabel[]      = RWSTRING("RpLights_R");
    static RwChar jitterLabel[]       = RWSTRING("Jitter Lights_J");

    static RwChar ltMapLabel[]        = RWSTRING("Save LightMapImage_I");
    static RwChar texDictLabel[]      = RWSTRING("Save TexDict_T");
    static RwChar objectsLabel[]      = RWSTRING("Save Objects_O");
    static RwChar styleLabel[]        = RWSTRING("Render Style_S");
    static const RwChar *styleNames[] = {RWSTRING("Base texture only"),
                                         RWSTRING("Lightmap only"),
                                         RWSTRING("Base & lightmap") };
    static RwChar sampleLabel[]       = RWSTRING("Point Sample_P");
    static RwChar dynamicLabel[]      = RWSTRING("Dynamic Lighting_Y");
    static RwChar drawLightLabel[]    = RWSTRING("Draw Lights_B");
    static RwChar drawWireframe[]     = RWSTRING("Draw Wireframe_W");
    static const RwChar *wireNames[]  = {RWSTRING("Off"),
                                         RWSTRING("Sector boxes"),
                                         RWSTRING("Triangles") };

    static RwChar lumCalcLabel[]      = RWSTRING("Lumin Calc Method");

    static const RwChar *lumCalcNames[] =
    {
        RWSTRING("Maximum Component"),
        RWSTRING("Vector Product"),
        RWSTRING("Reset")
    };

    static RwChar texDictMethodLabel[]      = RWSTRING("Tex Dict Method");

    static const RwChar *texDictMethodNames[] =
    {
        RWSTRING("PI Tex Dict"),
        RWSTRING("PD Tex Dict")
    };

    if(MenuOpen(TRUE, &ForegroundColor, &BackgroundColor))
    {
        MenuAddEntryBool(fpsLabel, &gDisplayFPS, NULL);
        MenuAddEntryInt(camSpeedLabel, &(gFakeEnums[MENUSTRINGARRAYCAMSPEED]),
            ChangeCameraSpeed, FAKEENUMMINVAL, FAKEENUMMAXVAL, 1,
            (const RwChar **)&(gMenuStrings[MENUSTRINGARRAYCAMSPEED][0]));
        MenuAddSeparator();

        MenuAddEntryTrigger(createLabel,  MenuCreateLightMaps);
        MenuAddEntryTrigger(clearLabel,   MenuClearLightMaps);

        MenuAddEntryTrigger(destroyLabel, MenuDestroyLightMaps);
        MenuAddEntryInt(densityLabel, &(gFakeEnums[MENUSTRINGARRAYDENSITY]),
            ChangeLightMapDensity, FAKEENUMMINVAL, FAKEENUMMAXVAL, 1,
            (const RwChar **)&(gMenuStrings[MENUSTRINGARRAYDENSITY][0]));
        MenuAddEntryInt(sizeLabel, &(gFakeEnums[MENUSTRINGARRAYMAPSIZE]),
            ChangeLightMapSize, FAKEENUMMINVAL, FAKEENUMMAXVAL, 1,
            (const RwChar **)&(gMenuStrings[MENUSTRINGARRAYMAPSIZE][0]));
        MenuAddEntryInt(scaleLabel, &LightMapScale,
            ChangeLightMapScale, 1, 8, 1,
            NULL);

        MenuAddSeparator();

        MenuAddEntryBool(lightLabel,     &gLightingWorld, NULL);
        MenuAddEntryTrigger(cancelLightLabel, MenuCancelLighting);
        MenuAddEntryBool(areaLightLabel, &gUseAreaLights, NULL);
        MenuAddEntryBool(rpLightLabel, &gUseRpLights, NULL);
        MenuAddEntryTrigger(jitterLabel,      MenuJitterLights);
        MenuAddSeparator();

        MenuAddEntryTrigger(ltMapLabel,   MenuSaveLightMap);
        MenuAddEntryTrigger(texDictLabel,     MenuSaveLightMapDict);
        MenuAddEntryTrigger(objectsLabel,     MenuSaveObjects);
        MenuAddSeparator();

        MenuAddEntryInt(styleLabel, (RwInt32 *)&gRenderStyle, MenuRenderStyleCB,
            rpLTMAPSTYLERENDERBASE,
            rpLTMAPSTYLERENDERBASE|rpLTMAPSTYLERENDERLIGHTMAP, 1, styleNames);
        MenuAddEntryBool(sampleLabel, &gPointSampling, MenuPointSampleCB);
        MenuAddEntryBool(dynamicLabel, &gUseDynamicLights, MenuDynamicCB);
        MenuAddEntryBool(drawLightLabel, &gDrawLights, NULL);
        MenuAddEntryInt(drawWireframe, (RwInt32 *)&gDrawWireframe, NULL,
                        NOWIREFRAME, WIRETRIANGLES, 1, wireNames);
        MenuAddSeparator();

        MenuAddEntryTrigger(darkMapLabel, MenuMakeDarkMaps);
        MenuAddEntryTrigger(procBaseTexLabel, MenuProcBaseTex);

        MenuAddEntryInt(lumCalcLabel, (RwInt32 *)&gLumCalcIndex,
                        LumCalcCallback, 0, 2, 1, lumCalcNames);
        MenuAddSeparator();

        MenuAddEntryInt(texDictMethodLabel, (RwInt32 *)&gTexDictIndex,
                        TexDictCallback, 0, 1, 1, texDictMethodNames);

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
    RwChar *worldPath = (RwChar *)NULL;

#if (defined(SKY2_DRVMODEL_H))
    _rwDMAPreAlloc(2 * 1024 * 1024, 4 * 1024, NULL);
#endif /* (defined(SKY2_DRVMODEL_H)) */

    if(!RsRwInitialize(param))
    {
        RsErrorMessage(RWSTRING("Error initializing RenderWare."));

        return FALSE;
    }

    /* Lightmaps are in PNG format */
    if(!RwImageRegisterImageFormat(
            RWSTRING("png"), RtPNGImageRead, RtPNGImageWrite))
    {
        RsErrorMessage(RWSTRING("Cannot register PNG image file format"));

        return FALSE;
    }

    Charset = RtCharsetCreate(&ForegroundColor, &BackgroundColor);
    if(Charset == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create raster charset."));

        return FALSE;
    }

    Camera = CameraCreate(RsGlobal.maximumWidth, RsGlobal.maximumHeight, TRUE);
    if(Camera == NULL)
    {
        RsErrorMessage(RWSTRING("Cannot create camera."));

        return FALSE;
    }

    worldPath = RsPathnameCreate(RWSTRING("models/example.bsp"));

    /* This will place our camera in the world. Before we load any new worlds
     * we remove the camera, load the world, then add the camera again... */
    World = LoadWorld(worldPath, NULL, Camera);
    if(World == NULL)
    {
        RwBBox box = {{0, 0, 0}, {100, 100, 100}};

        World = RpWorldCreate(&box);
        if (NULL == World)
        {
            RsErrorMessage(RWSTRING("Cannot create world."));
            return FALSE;
        }
    }

    rwstrcpy(CurrentWorldPath, worldPath);
    RsPathnameDestroy(worldPath);

    if(!InitializeMenu())
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
    /* Clean up lighting */
    gResetLighting = TRUE;
    LightScene(Camera);

#ifdef RWMETRICS
    RsMetricsClose();
#endif

    MenuClose();

    ResetScene(World, Camera);
    World = (RpWorld *)NULL;

    CameraDestroy(Camera);
    Camera = (RwCamera *)NULL;

    if(Charset)
    {
        RwRasterDestroy(Charset);
        Charset = (RtCharset *)NULL;
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
#if (!defined(SKY2_DRVMODEL_H))
    /* It is important that we attach this, otherwise a world
     * saved on a PC will lose KL values required when the world
     * is loaded on PS2 - resulting in blurry textures.
     * Note: you shouldn't add this plugin for SKY2 since the driver
     *       adds it automatically and adding it twice causes a crash. */
    if(!RpMipmapKLPluginAttach())
    {
        return(FALSE);
    }
#endif /* (!defined(SKY2_DRVMODEL_H)) */

    /* Attach world plug-in... */
    if(!RpWorldPluginAttach())
    {
        return(FALSE);
    }

    /* Attach collision plug-in */
    if(!RpCollisionPluginAttach())
    {
        return(FALSE);
    }

    /* Attach PVS plugin */
    if (!RpPVSPluginAttach())
    {
        return(FALSE);
    }
    
    /* Attach random plug-in... */
    if(!RpRandomPluginAttach())
    {
        return(FALSE);
    }
    
    /* Attach lightmap plug-in... */
    if(!RpLtMapPluginAttach())
    {
        return(FALSE);
    }

    /* For Sky builds register the lightmap PDS pipes */
#if (defined(SKY2_DRVMODEL_H))
    RpLtMapPipesAttach();
#endif /* (defined(SKY2_DRVMODEL_H)) */

#ifdef RWLOGO
    /* Attach logo plug-in... */
    if(!RpLogoPluginAttach())
    {
        return(FALSE);
    }
#endif

    return(TRUE);
}

/*
 *****************************************************************************
 */
static RwBool
FileLoad(RwChar *filename)
{
    RwChar *path = (RwChar *)NULL;
    RwBool loaded = FALSE;
    RwChar message[256];

    path = RsPathnameCreate(filename);

    if( rwstrstr(path, RWSTRING(".bsp")) || rwstrstr(path, RWSTRING(".BSP")) )
    {
        RpWorld *newWorld;

        newWorld = LoadWorld(path, World, Camera);
        if(NULL != newWorld)
        {
            World = newWorld;
            rwstrcpy(message, RsGlobal.appName);
            rwstrcat(message, RWSTRING(" - "));
            rwstrcat(message, path);
            RsWindowSetText(message);
            loaded = TRUE;
        }
    }
    else
    if( rwstrstr(path, RWSTRING(".dff")) || rwstrstr(path, RWSTRING(".DFF")) )
    {
        RpClump *newClump;

        /* As of RenderWare Graphics 3.3, these clumps can contain lights! :) */
        newClump = LoadClump(path);
        if(NULL != newClump)
        {
            RpWorldAddClump(World, newClump);
            loaded = TRUE;
        }
    }
    else
    {
        RsSprintf(message,
            RWSTRING("The file %s is not a BSP or DFF file."), path);
        RsErrorMessage(message);

        RsPathnameDestroy(path);

        return FALSE;
    }

    if(FALSE == loaded)
    {
        RsSprintf(message, RWSTRING("The file %s could not be loaded."), path);

        RsErrorMessage(message);
        return FALSE;
    }

    rwstrcpy(CurrentWorldPath, path);
    RsPathnameDestroy(path);

    return TRUE;
}

/*
 *****************************************************************************
 */
static void
DisplayOnScreenInfo(void)
{
    RwChar caption[256];

    if (gCreatingLightMaps)
    {
        RsSprintf(caption, RWSTRING("Calculating lightmap UVs..."));
        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSMIDDLE);
    }
    else if(gLightingWorld)
    {
        RsSprintf(caption,
                  RWSTRING("Lighting world... %.2f%% complete."),
                  100.0f*gLightingProgress);
        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSMIDDLE);
    }

    if(gDisplayFPS)
    {
        RsSprintf(caption, RWSTRING("FPS: %03d"), FramesPerSecond);
        RsCharsetPrint(Charset, caption, 0, 0, rsPRINTPOSTOPRIGHT);
    }

    return;
}


/*
 *****************************************************************************
 */
static RpWorldSector *
wireframeSectorCB(RpWorldSector *sector)
{
    RwUInt32 numVertices, numTriangles;

    numVertices = sector->numVertices;
    numTriangles = sector->numTriangles;
    if ((1) &&
        (NOWIREFRAME != gDrawWireframe) )
    {
        static RwImVertexIndex cubeInds[12][2] = {{0, 1}, {1, 3},
                                                  {3, 2}, {2, 0},
                                                  {0, 4}, {1, 5},
                                                  {3, 7}, {2, 6},
                                                  {4, 5}, {5, 7},
                                                  {7, 6}, {6, 4}};

        RwIm3DVertex *vertices;
        RwImVertexIndex *indices;
        RwV3d infSup[2];
        RwUInt32 i, j;

        if (WIREBOXES == gDrawWireframe)
        {
            numVertices  = 0;
            numTriangles = 0;
        }

        vertices = (RwIm3DVertex *)
            RwMalloc((8 + numVertices)*sizeof(RwIm3DVertex),
                     rwID_NAOBJECT);
        indices = (RwImVertexIndex *)
            RwMalloc((24 + 6*numTriangles)*sizeof(RwImVertexIndex),
                     rwID_NAOBJECT);
        RSASSERT(NULL != vertices);
        RSASSERT(NULL != indices);

        for (i = 0;i < numVertices;i++)
        {
            RwIm3DVertexSetRGBA(&(vertices[i]), 255, 255, 255, 255);
            RwIm3DVertexSetPos(&(vertices[i]),
                               sector->vertices[i].x,
                               sector->vertices[i].y,
                               sector->vertices[i].z);
        }
        infSup[0] = RpWorldSectorGetBBox(sector)->inf;
        infSup[1] = RpWorldSectorGetBBox(sector)->sup;
        for (i = 0;i < 8;i++)
        {
            /* Set up verts for the sector's BBox */
            RwIm3DVertexSetRGBA(&(vertices[i + numVertices]),
                255*((i&1)>>0), 255*((i&2)>>1), 255*((i&4)>>2), 255);
            RwIm3DVertexSetPos(&(vertices[i + numVertices]),
               infSup[((i&1)>>0)].x,
               infSup[((i&2)>>1)].y,
               infSup[((i&4)>>2)].z);
        }

        j = 0;
        for (i = 0;i < numTriangles;i++)
        {
            indices[j++] = sector->triangles[i].vertIndex[0];
            indices[j++] = sector->triangles[i].vertIndex[1];
            indices[j++] = sector->triangles[i].vertIndex[1];
            indices[j++] = sector->triangles[i].vertIndex[2];
            indices[j++] = sector->triangles[i].vertIndex[2];
            indices[j++] = sector->triangles[i].vertIndex[0];
        }
        for (i = 0;i < 24;i++)
        {
            /* Set up indices for the sector's BBox */
            indices[j++] = numVertices + cubeInds[i >> 1][i&1];
        }

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

        RwIm3DTransform(vertices, numVertices + 8, (RwMatrix *)NULL, rwIM3D_ALLOPAQUE);
        {
            RwIm3DRenderIndexedPrimitive(
                rwPRIMTYPELINELIST, indices, 24 + 6*numTriangles);
            RwIm3DEnd();
        }

        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);

        RwFree(indices);
        RwFree(vertices);
    }

    /* If uncommented, this will draw sector vertex normals in red/blue */
#if (0)
    if ((NOWIREFRAME != gDrawWireframe) &&
        (WIREBOXES != gDrawWireframe) &&
        (NULL != sector->normals) )
    {
        RwIm3DVertex *vertices;
        RwUInt32 i, j;

        vertices = (RwIm3DVertex *)
            RwMalloc(2*numVertices*sizeof(RwIm3DVertex), rwID_NAOBJECT);
        RSASSERT(NULL != vertices);

        j = 0;
        for (i = 0;i < numVertices;i++)
        {
            RwRGBA cols[2] = {{255,0,0,255},{0,0,255,255}};
            RwV3d normal;

            RwIm3DVertexSetRGBA(&(vertices[j]),
                cols[i&1].red, cols[i&1].green, cols[i&1].blue, 255);
            RwIm3DVertexSetPos(&(vertices[j]),
                               sector->vertices[j].x,
                               sector->vertices[j].y,
                               sector->vertices[j].z);
            j++;
            RwIm3DVertexSetRGBA(&(vertices[j]),
                cols[~i&1].red, cols[~i&1].green, cols[~i&1].blue, 255);
            RPV3DFROMVERTEXNORMAL(normal, sector->normals[j]);
            RwV3dScale(&normal, &normal, 35);
            RwIm3DVertexSetPos(&(vertices[j]),
                               sector->vertices[j - 1].x + normal.x,
                               sector->vertices[j - 1].y + normal.y,
                               sector->vertices[j - 1].z + normal.z);
            j++;
        }

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);

        RwIm3DTransform(vertices, 2*numVertices, (RwMatrix *)NULL, rwIM3D_ALLOPAQUE);
        {
            RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);
            RwIm3DEnd();
        }

        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);

        RwFree(vertices);
    }
#endif /* (0) */

    return(sector);
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
            RwUInt32 intCol = RWRGBALONG(BackgroundColor.red,
                                         BackgroundColor.green,
                                         BackgroundColor.blue,
                                         BackgroundColor.alpha);
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)TRUE);
            RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void *)rwFOGTYPELINEAR);
            RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void *)intCol);

#if (defined(SKY2_DRVMODEL_H))
            {
                RwReal farFogPlane = RwCameraGetFarClipPlane(Camera);
                RpSkyRenderStateSet(rpSKYRENDERSTATEFARFOGPLANE,
                                    (void *)&farFogPlane);

                /* Most platforms back-face cull automatically, not so PS2 */
                RwRenderStateSet(rwRENDERSTATECULLMODE, (void *)rwCULLMODECULLBACK);

                /* Necessary on PS2 given that the camera will be inside a
                 * world and that worlds generally contain large polygons */
                RpSkySelectTrueTSClipper(TRUE);
                RpSkySelectTrueTLClipper(TRUE);
            }
#endif /* (defined(SKY2_DRVMODEL_H)) */

            RpPVSHook(World);
            RpPVSSetViewPosition(World,
                RwMatrixGetPos(RwFrameGetLTM(RwCameraGetFrame(Camera))));
            RpWorldRender(World);
            RpPVSUnhook(World);

            if (NOWIREFRAME != gDrawWireframe)
            {
                RpWorldSetSectorRenderCallBack(World, wireframeSectorCB);
                RpWorldRender(World);
                RpWorldSetSectorRenderCallBack(World, (RpWorldSectorCallBackRender)NULL);
            }

            if (FALSE != gDrawLights)
            {
                DisplayLights(Camera);
            }

            /* Fog and the menu tend not to get along... */
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void *)FALSE);

            DisplayOnScreenInfo();
        }

        MenuRender(Camera, NULL);

#ifdef RWMETRICS
        RsMetricsRender();
#endif

        RwCameraEndUpdate(Camera);
    }

    /* Display camera's raster... */
    RsCameraShowRaster(Camera);

    FrameCounter++;

    return;
}

/*
 ******************************************************************************
 */
static RwBool
CameraUpdate(RwReal deltaTime)
{
    if ((CameraSpeed    != 0.0f) || (CameraPitchRate   != 0.0f) ||
        (CameraTurnRate != 0.0f) || (CameraStrafeSpeed != 0.0f) )
    {
        RwFrame  *frame = RwCameraGetFrame(Camera);
        RwMatrix *m = RwFrameGetMatrix(frame);
        RwV3d    *right = RwMatrixGetRight(m);
        RwV3d     pos, invPos, yAxis = {0, 1, 0};

        pos = *RwMatrixGetPos(m);

        /* Move camera to origin for rotation. */
        RwV3dNegate(&invPos, &pos);
        RwFrameTranslate(frame, &invPos, rwCOMBINEPOSTCONCAT);

        /* Rotate camera */
        RwFrameRotate(frame, right, CameraPitchRate * deltaTime,
                      rwCOMBINEPOSTCONCAT);
        RwFrameRotate(frame, &yAxis, CameraTurnRate * deltaTime,
                      rwCOMBINEPOSTCONCAT);
        /* Move to new position */
        RwV3dIncrementScaled(&pos, RwMatrixGetAt(m),
                             CameraSpeed * deltaTime);
        RwV3dIncrementScaled(&pos, RwMatrixGetRight(m),
                             CameraStrafeSpeed * deltaTime);

        RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
Idle(void)
{
    static RwBool   firstCall = TRUE;
    static RwUInt32 lastFrameTime;
    static RwUInt32 lastFPSUpdate;

    RwUInt32 thisTime;
    RwReal   deltaTime;

    if(firstCall)
    {
        lastFrameTime   = RsTimer();
        lastFPSUpdate   = lastFrameTime;

        firstCall = FALSE;
    }

    thisTime = RsTimer();
    deltaTime = (thisTime - lastFrameTime) * 0.001f;

    /* Has a second elapsed since we last updated the FPS? */
    if( thisTime > (1000 + lastFPSUpdate) )
    {
        /* Capture the frame counter... */
        FramesPerSecond = (RwInt32)
            (FrameCounter*(1000.0f / (thisTime - lastFPSUpdate)));

        /* ...and reset. */
        FrameCounter = 0;

        lastFPSUpdate = thisTime;
    }

    /* Update camera */
    CameraUpdate(deltaTime);

    /* Update our dynamic lights */
    LightsUpdate(deltaTime);

    Render();

    CreateLightMaps();
    ClearLightMaps();
    LightScene(Camera);

    lastFrameTime = thisTime;

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
            static RwBool startup = TRUE;
            CameraSize(Camera, (RwRect *)param,
                DEFAULT_VIEWWINDOW, DEFAULT_ASPECTRATIO);
            if (FALSE != startup)
            {
                CameraInitPosition(Camera, World);
                startup = FALSE;
            }

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

        case rsFILELOAD:
        {
            return FileLoad((RwChar *)param) ? rsEVENTPROCESSED : rsEVENTERROR;
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
