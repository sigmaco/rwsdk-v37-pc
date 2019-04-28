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
 * Copyright (c) 2000 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * texadrss.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics texture addressing example.
 *
 ****************************************************************************/

#ifndef TEXADRSS_H
#define TEXADRSS_H

#include "rwcore.h"
#include "rpworld.h"

#define NUMMODES (5)

typedef struct _TextureAddressInfo
{
    RwTextureAddressMode mode[NUMMODES];
    const RwChar *strings[NUMMODES];
    RwUInt8 number;
}
TextureAddressInfo;

typedef struct _TextureAddressAllInfo
{
    TextureAddressInfo both;
    TextureAddressInfo u;
    TextureAddressInfo v;
}
TextureAddressAllInfo;

typedef enum 
{
    TextureAddressNone = 0,
    TextureAddressBoth,
    TextureAddressU,
    TextureAddressV,

    TEXTUREADDRESSTYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
}
TextureAddressType;

extern RpClump *Clump;
extern RwCamera *Camera;

extern RwV3d Xaxis;
extern RwV3d Yaxis;
extern RwV3d Zaxis;

extern TextureAddressAllInfo AllModeInfo;

#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern RpClump *ClumpCreate(RpWorld *world);
extern void ClumpRotate(RpClump *clump, RwCamera *camera, 
                        RwReal xAngle, RwReal yAngle);
extern void ClumpTranslateZ(RpClump *clump, RwCamera *camera, RwReal zDelta);

extern void ClumpSetTextureAddressMode(RpClump *clump, 
                TextureAddressType texAdrssType, RwTextureAddressMode modeIndex);

extern void QueryTextureAddressAllInfo(void);
extern RwTextureAddressMode QueryTextureAddressMode(RpClump *clump, 
                TextureAddressType texAdrssType);

extern RwUInt32 SetModeIndex(TextureAddressInfo *texAdrssInfo, 
                RwTextureAddressMode texAdrssMode);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* TEXADRSS_H */
