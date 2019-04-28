
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
 * texadrss.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics texture addressing example.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"

#include "texadrss.h"
 

typedef struct _TextureAddressChangeInfo
{
    RwTextureAddressMode textureAddressMode;
    TextureAddressType textureAddressType;
} 
TextureAddressChangeInfo;

static RwTextureAddressMode TextureAddressModes[] =
{
    rwTEXTUREADDRESSNATEXTUREADDRESS,
    rwTEXTUREADDRESSWRAP,
    rwTEXTUREADDRESSMIRROR,
    rwTEXTUREADDRESSCLAMP,
    rwTEXTUREADDRESSBORDER
};

static const RwChar *TextureAddressModeStrings[] = 
{
    RWSTRING("(Not applicable)"),
    RWSTRING("rwTEXTUREADDRESSWRAP"),
    RWSTRING("rwTEXTUREADDRESSMIRROR"),
    RWSTRING("rwTEXTUREADDRESSCLAMP"),
    RWSTRING("rwTEXTUREADDRESSBORDER")
};


TextureAddressAllInfo AllModeInfo;


/*
 *****************************************************************************
 */
static void 
TextureAddressGetInfo(RwRenderState renderState, TextureAddressInfo *info)
{
    /*
     * Using 
     * RwRenderStateSet(RwRenderState renderState,(void *)texAdrssMode)
     * we can determine the texture addressing modes available...
     */
    RwUInt8 i;
    RwInt32 numModes;

    numModes = sizeof(TextureAddressModes) / sizeof(*TextureAddressModes);
    
    for(i=0, info->number=0; i<numModes; i++)
    {
        if( TextureAddressModes[i] == rwTEXTUREADDRESSNATEXTUREADDRESS ||
            RwRenderStateSet(renderState, (void *)TextureAddressModes[i]) )
        {
            info->mode[info->number] = TextureAddressModes[i];

            info->strings[info->number] = TextureAddressModeStrings[i];

            info->number++;
        }
    }

    return;
}


/*
 *****************************************************************************
 */
void
QueryTextureAddressAllInfo(void)
{
    /*
     * We need to decide which texture addressing modes are available 
     * before we try setting them.
     *
     * We query this information by calling:
     *    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, ...);
     *    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSU, ...);
     *    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESSV, ...);
     * 
     * For all the different mode types:
     *   rwTEXTUREADDRESSWRAP
     *   rwTEXTUREADDRESSMIRROR
     *   rwTEXTUREADDRESSCLAMP
     *   rwTEXTUREADDRESSBORDER
     *
     * Now we start by querying the RenderStateSet options of
     *   ...rwRENDERSTATETEXTUREADDRESS... and then 
     *   ...rwRENDERSTATETEXTUREADDRESSU... and then finally
     *   ...rwRENDERSTATETEXTUREADDRESSV. 
     */
    TextureAddressGetInfo(rwRENDERSTATETEXTUREADDRESS, &AllModeInfo.both);
    TextureAddressGetInfo(rwRENDERSTATETEXTUREADDRESSU, &AllModeInfo.u);
    TextureAddressGetInfo(rwRENDERSTATETEXTUREADDRESSV, &AllModeInfo.v);

    /* 
     * Return the rwRENDERSTATETEXTUREADDRESS to a default...
     */
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS,
        (void*)rwTEXTUREADDRESSWRAP);

    return;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialSetTextureAddressMode(RpMaterial *material, void *data)
{
    TextureAddressChangeInfo *texAdrssChangeInfo = 
        (TextureAddressChangeInfo *)data;
    RwTexture * const texture = 
        RpMaterialGetTexture(material);
    const RwTextureAddressMode textureAddressMode =
        texAdrssChangeInfo->textureAddressMode;
    RwTexture * checkTexture;

    switch( texAdrssChangeInfo->textureAddressType )
    {
        
        case TextureAddressBoth:
        {
            checkTexture = 
                RwTextureSetAddressing(texture, textureAddressMode );

            break;
        }

        case TextureAddressU:
        {
            checkTexture = 
                RwTextureSetAddressingU(texture, textureAddressMode );

            break;
        }

        case TextureAddressV:
        {
            checkTexture = 
                RwTextureSetAddressingV(texture, textureAddressMode );

            break;
        }

        default:
        {
            break;
        }
    }

    return material;
}


static RpAtomic *
AtomicSetTextureAddressMode(RpAtomic *atomic, void *data)
{
    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
        MaterialSetTextureAddressMode, data);

    return atomic;
}


void 
ClumpSetTextureAddressMode(RpClump *clump, TextureAddressType texAdrssType,
                           RwTextureAddressMode modeIndex)
{
    /*
     * We call RpClumpForAllAtomics requesting a call to 
     * AtomicSetTextureAddressMode per atomic. This in tern calls
     * RpGeometryForAllMaterials requesting a call to 
     * MaterialSetTextureAddressMode per material. This then
     * finally sets the texture addressing by calling 
     *   RwTextureSetAddressing or
     *   RwTextureSetAddressingU or
     *   RwTextureSetAddressingV 
     * dependent on the initial TexAdrssType.
     *
     * Possible texture address types are:
     *   TextureAddressBoth
     *   TextureAddressU
     *   TextureAddressV
     *
     * Possible texture address modes are:
     *   rwTEXTUREADDRESSWRAP
     *   rwTEXTUREADDRESSMIRROR
     *   rwTEXTUREADDRESSCLAMP
     *   rwTEXTUREADDRESSBORDER
     *
     */
    static TextureAddressChangeInfo texAdrssChangeInfo;

    texAdrssChangeInfo.textureAddressType = texAdrssType;
    texAdrssChangeInfo.textureAddressMode = modeIndex;

    RpClumpForAllAtomics(clump, 
        AtomicSetTextureAddressMode, (void *)&texAdrssChangeInfo);

    return;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialGetTextureAddressMode(RpMaterial *material, void *data)
{
    TextureAddressChangeInfo *texAdrssChangeInfo = 
        (TextureAddressChangeInfo *)data;
    RwTexture * const texture = RpMaterialGetTexture(material);
    
    switch( texAdrssChangeInfo->textureAddressType )
    {
        case TextureAddressBoth:
        {
            texAdrssChangeInfo->textureAddressMode = 
                RwTextureGetAddressing(texture);

            break;
        }

        case TextureAddressU:
        {
            texAdrssChangeInfo->textureAddressMode = 
                RwTextureGetAddressingU(texture);

            break;
        }

        case TextureAddressV:
        {
            texAdrssChangeInfo->textureAddressMode = 
                RwTextureGetAddressingV(texture);

            break;
        }

        default:
        {
            break;
        }
    }

    return material;
}


static RpAtomic *
AtomicGetTextureAddressMode(RpAtomic *atomic, void *data)
{
    RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic),
        MaterialGetTextureAddressMode, data);

    return atomic;
}


RwTextureAddressMode 
QueryTextureAddressMode(RpClump *clump, TextureAddressType texAdrssType)
{
    /*
     * After a call to:
     *   RwTextureSetAddressing   or
     *   RwTextureSetAddressingU  or
     *   RwTextureSetAddressingV
     * it's possible another TextureAddressType has had its 
     * RwTextureAddressMode changed - or even become invalid.
     * We therefore use: 
     *   RwTextureGetAddressing   and
     *   RwTextureGetAddressingU  and
     *   RwTextureGetAddressingV
     * to query the updated RwTextureAddressModes.
     */
    static TextureAddressChangeInfo texAdrssChangeInfo;

    texAdrssChangeInfo.textureAddressType = texAdrssType;

    RpClumpForAllAtomics(clump, 
        AtomicGetTextureAddressMode, (void *)&texAdrssChangeInfo);

    return texAdrssChangeInfo.textureAddressMode;
}


/*
 *****************************************************************************
 */
RwUInt32 
SetModeIndex(TextureAddressInfo *texAdrssInfo, RwTextureAddressMode texAdrssMode)
{
    /*
     * We search through the texAdrssInfo->mode array until we find the
     * RwTextureAddressMode we are looking for. Then we return the index...
     */
    RwUInt32 i;

    for(i=0; i<texAdrssInfo->number; i++)
    {
        if( texAdrssInfo->mode[i] == texAdrssMode )
        {
            return i;
        }
    }

    return 0; /* Hopefully shouldn't get here */
}

/*
 *****************************************************************************
 */
