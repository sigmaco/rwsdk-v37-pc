
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
 * lightmap.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example to demonstrate the functions of light maps as
   as they are provided in RpLightmap and RtLightmap.
 *
*****************************************************************************/

#ifndef LIGHTMAP_H
#define LIGHTMAP_H

#include "rwcore.h"
#include "rpworld.h"
#include "rpmipkl.h"


extern RwBool   gCreatingLightMaps;
extern RwBool   gCreatedLightMaps;
extern RwBool   gClearingLightMaps;
extern RwBool   gLightingWorld;
extern RwBool   gUseAreaLights;
extern RwBool   gUseRpLights;
extern RwBool   gUseDynamicLights;
extern RwUInt32 gNumDynamicLights;
extern RwReal   gLightingProgress;
extern RwBool   gResetLighting;
extern RwUInt32 gRenderStyle;
extern RwBool   gPointSampling;
extern RwBool   gPreLighting;
extern RwChar   gPlatformString[];
extern RwTexDictionary *gBaseTexDict;
extern RwChar  *gBaseTexDictPath;
extern RwUInt32 gNumTeapots;
extern RpClump *gTeapots[];
extern RwChar  *CurrentTeapotPath;
extern RwUInt32 gLumCalcIndex;
extern RwUInt32 gTexDictIndex;

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RpWorld *LoadWorld(RwChar *file, RpWorld *oldWorld, RwCamera *camera);
extern RpClump *LoadClump(const RwChar *file);

extern RwBool   CreateLightMaps(void);
extern RwBool   ClearLightMaps(void);
extern RwBool   DestroyLightMaps(void);

extern RwBool   LightScene(RwCamera *camera);

extern RwBool   SaveLightMapTexDict(RwChar *path);
extern RwBool   SaveLightMapImages(RwChar *path);
extern RwBool   MakeDarkMaps(void);
extern RwBool   ProcessBaseTexture(void);
extern RwBool   SaveScene(void);

extern void     CameraInitPosition(RwCamera *camera, RpWorld *world);
extern RwBool   DisplayLights(RwCamera *camera);
extern RwBool   ToggleDynamicLights(RpWorld *world);
extern RwBool   LightsUpdate(RwReal deltaTime);
extern void     ResetScene(RpWorld *World, RwCamera *camera);

#if (defined(GCN_DRVMODEL_H) || defined(NULLGCN_DRVMODEL_H))
extern void      DetectLightMaps(RpWorld *world);
extern RpAtomic *GCNAtomicSetupCB(RpAtomic *atomic, void __RWUNUSED__ *data);
#endif /* (defined(GCN_DRVMODEL_H) || defined(NULLGCN_DRVMODEL_H)) */

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif /* LIGHTMAP_H */
