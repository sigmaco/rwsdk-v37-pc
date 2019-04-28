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
 * normalmapgen.c
 *
 * Copyright (C) 2002 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose : RenderWare 3.4 example.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rtnormmap.h"

#include "normalmapgen.h"

#ifdef XBOX_DRVMODEL_H
#define FAST_MODE   /* Lower mipmap quality but much faster to generate them on the Xbox devkit */
#endif

RwTexture *
NormalMapTextureSpaceCreateFromTexture(RwTexture *texture, RwReal bumpness)
{
    RwRaster *raster;
    RwTexture *normalmap = NULL;

    raster = RwTextureGetRaster(texture);
    if (raster != NULL)
    {
        RwUInt32 width, height;
        RwImage *image;
        RwBool clamp;
        RwUInt32 rasterFlags;
        RwRaster *rasternormalmap;

        width = RwRasterGetWidth(raster);
        height = RwRasterGetHeight(raster);

	    image = RwImageCreate(width, height, 32);
	    RwImageAllocatePixels(image);
	    RwImageSetFromRaster(image, raster);

        clamp = (RwTextureGetAddressingU(texture) == rwTEXTUREADDRESSCLAMP ||
                 RwTextureGetAddressingV(texture) == rwTEXTUREADDRESSCLAMP);

        rasterFlags = (RwRasterGetType(raster) |
#ifdef FAST_MODE
                       (RwRasterGetFormat(raster) & (rwRASTERFORMATMIPMAP | rwRASTERFORMATAUTOMIPMAP)) |
#else
                       (RwRasterGetFormat(raster) & rwRASTERFORMATMIPMAP) |
#endif
                       rwRASTERFORMAT888);

        rasternormalmap = NormalMapTextureSpaceCreateFromImage(image, rasterFlags, clamp, bumpness);

        /* Create texture */
        normalmap = RwTextureCreate(rasternormalmap);

        RwTextureSetFilterMode(normalmap, RwTextureGetFilterMode(texture));
        RwTextureSetAddressingU(normalmap, RwTextureGetAddressingU(texture));
        RwTextureSetAddressingV(normalmap, RwTextureGetAddressingV(texture));

        RwImageDestroy(image);
    }

    return normalmap;
}

RwRaster *
NormalMapTextureSpaceCreateFromImage(RwImage *image, RwUInt32 rasterFlags, RwBool clamp, RwReal bumpness)
{
    RwInt32 width, height;
    RwRaster *rasternormalmap;

    width = RwImageGetWidth(image);
    height = RwImageGetHeight(image);

    rasternormalmap = RwRasterCreate(width, height, 32, rasterFlags);

    if (rasternormalmap != NULL)
    {
        RwImage *imagenormalmap = NULL;

#ifdef FAST_MODE
        imagenormalmap = RtNormMapCreateFromImage(image, clamp, bumpness);

        if (imagenormalmap != NULL)
        {
            RwRasterSetFromImage(rasternormalmap, imagenormalmap);

            RwImageDestroy(imagenormalmap);
        }
#else
        RwUInt32 n, nummipmaps;
        
        width = RwRasterGetWidth(rasternormalmap);
        height = RwRasterGetHeight(rasternormalmap);
        nummipmaps = RwRasterGetNumLevels(rasternormalmap);

        for (n = 0; n < nummipmaps; n++)
        {
            RwRasterLock(rasternormalmap, n, rwRASTERLOCKWRITE | rwRASTERLOCKNOFETCH);

            if (RwImageGetWidth(image) != width ||
                RwImageGetHeight(image) != height)
            {
                RwImage *imagemipmap;

                imagemipmap = RwImageCreateResample(image, width, height);
                if (imagemipmap != NULL)
                {
                    imagenormalmap = RtNormMapCreateFromImage(imagemipmap, clamp, bumpness);

                    RwImageDestroy(imagemipmap);
                }
            }
            else
            {
                imagenormalmap = RtNormMapCreateFromImage(image, clamp, bumpness);
            }

            if (imagenormalmap != NULL)
            {
                RwRasterSetFromImage(rasternormalmap, imagenormalmap);

                RwImageDestroy(imagenormalmap);

                imagenormalmap = NULL;
            }

            RwRasterUnlock(rasternormalmap);

            width /= 2;
            if (width == 0)
            {
                width = 1;
            }

            height /= 2;
            if (height == 0)
            {
                height = 1;
            }

            bumpness *= 0.5f;
        }
#endif
    }

    return rasternormalmap;
}
