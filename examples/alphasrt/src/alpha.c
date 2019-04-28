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
 * alpha.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: An example to illustrate how to sort a collection of atomics 
 *          with alpha components, such that they are rendered in the 
 *          correct back-to-front order.
 *                         
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"

#include "alpha.h"
#include "main.h"

typedef struct sortAtomic SortAtomic;
struct sortAtomic
{
    RpAtomic *atomic;
    RwReal depth;
    RwChar id;

    SortAtomic *next;
};

typedef struct alphaAtomic AlphaAtomic;
struct alphaAtomic
{
    RpAtomic *atomic;
    RwChar id;

    AlphaAtomic *next;
};

static SortAtomic *AlphaSortedAtomics = NULL;
static AlphaAtomic *AlphaAtomics = NULL;
static RwInt32 NumAlphaAtomics = 0;

static RpAtomicCallBackRender DefaultAtomicRenderCallback;

RwChar *RenderOrderString;



/*
 *****************************************************************************
 */
static RwChar
ColorGetID(const RwRGBA *color)
{
    RwChar name;

    if( color->red && color->green && color->blue )
    {
        name = 'W';
    }
    else if( color->red && color->green )
    {
        name = 'Y';
    }
    else if( color->red && color->blue )
    {
        name = 'M';
    }
    else if( color->green && color->blue )
    {
        name = 'C';
    }
    else if( color->red )
    {
        name = 'R';
    }
    else if( color->green )
    {
        name = 'G';
    }
    else if( color->blue )
    {
        name = 'B';
    }
    else
    {
        name = '*';
    }

    return name;
}


/*
 *****************************************************************************
 */
static RwBool
AlphaTestPalettizedTexture(RwRaster *raster)
{
    RwImage *image = NULL;
    RwInt32 width, height, depth;

    /*
     * Create a device-independent image from the texture's 
     * device-dependent raster...
     */

    width = RwRasterGetWidth(raster);
    height = RwRasterGetHeight(raster);
    depth = RwRasterGetDepth(raster);

    image = RwImageCreate(width, height, depth);
    if( image )
    {
        RwRGBA *palette = NULL;
        RwInt32 paletteSize, i;

        RwImageAllocatePixels(image);
        RwImageSetFromRaster(image, raster);

        /*
         * Run through the palette entries looking for an alpha
         * value less than 255...
         */
        palette = RwImageGetPalette(image);
        
        paletteSize = 1 << depth;

        for(i=0; i<paletteSize; i++)
        {
            if( palette->alpha < 255 )
            {
                /*
                 * One of the palette alphas is less than 255 so we
                 * can stop checking the remaining entries...
                 */
                RwImageDestroy(image);

                return TRUE;
            }

            palette++;
        }

        RwImageDestroy(image);   
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
AlphaTestTrueColorTexture(RwRaster *raster)
{
    RwImage *image;
    RwInt32 width, height;

    /*
     * Create a device-independent image from the texture's 
     * device-dependent raster...
     */

    width = RwRasterGetWidth(raster);
    height = RwRasterGetHeight(raster);

    image = RwImageCreate(width, height, 32);
    if( image )
    {
        RwInt32 x, y;
        RwUInt8 *imagePixels;
        RwUInt32 *currentPixel;

        RwImageAllocatePixels(image);
        RwImageSetFromRaster(image, raster);
        imagePixels = RwImageGetPixels(image);

        /*
         * Run through the pixel data looking for an alpha
         * value less than 255...
         */
        for(y=0; y<height; y++)
        {
            currentPixel = (RwUInt32 *)imagePixels;

            for(x=0; x<width; x++)
            {
                RwUInt32 pixel;
                RwUInt8 alpha;

                /*
                 * Pixel format for RwImage:
                 * Red: bits 0 - 7
                 * Green: bits 8 - 15
                 * Blue: bits 16 - 23
                 * Alpha: bits 24 - 31
                 */
                pixel = *currentPixel;
                alpha = (RwUInt8)((pixel >> 24) & 0xFF);

                if( alpha < 255 )
                {
                    /*
                     * One of the image's pixel alpha value is less than
                     * 255 so we can stop checking the remaining pixels...
                     */
                    RwImageDestroy(image);

                    return TRUE;
                }
    
                currentPixel++;
            }

            /*
             * Jump to the beginning of the next pixel row...
             */
            imagePixels += RwImageGetStride(image);
        }

        RwImageDestroy(image);   
    }
    
    return FALSE;
}


/*
 *****************************************************************************
 */
static RwBool
TextureTestAlpha(RwTexture *texture)
{
    RwRaster *raster;
    RwInt32 rasterFormat;

    raster = RwTextureGetRaster(texture);
    rasterFormat = RwRasterGetFormat(raster);

    /*
     * First check if the texture has an alpha channel...
     */
    if( (rasterFormat & rwRASTERFORMATPIXELFORMATMASK) == rwRASTERFORMAT8888 ||
        (rasterFormat & rwRASTERFORMATPIXELFORMATMASK) == rwRASTERFORMAT4444 ||
        (rasterFormat & rwRASTERFORMATPIXELFORMATMASK) == rwRASTERFORMAT1555 )
    {
        /*
         * ...then if the texture is palettized or true color...
         */
        if( (rasterFormat & rwRASTERFORMATPAL4) ||
            (rasterFormat & rwRASTERFORMATPAL8) )
        {
            return AlphaTestPalettizedTexture(raster);
        }
        else
        {
            return AlphaTestTrueColorTexture(raster);
        }
    }

    /*
     * Texture has no alpha channel...
     */
    return FALSE;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialTestAlpha(RpMaterial *material, void *data)
{
    const RwRGBA *color;
    RwBool hasAlpha = FALSE;

    /*
     * First check the material's color for alpha less than 255...
     */
    color = RpMaterialGetColor(material);

    if( color->alpha < 255 )
    {
        hasAlpha = TRUE;
    }
    
    if( !hasAlpha )
    {
        RwTexture *texture;

        /*
         * The material's color's not transparent so check the 
         * material's texture...
         */
        texture = RpMaterialGetTexture(material);
        if( texture )
        {
            hasAlpha = TextureTestAlpha(texture);
        }
    }

    if( hasAlpha )
    {
        /*
         * Add atomic to list of alpha atomics...
         */
        AlphaAtomic *entry;

        entry = (AlphaAtomic *)RwMalloc(sizeof(AlphaAtomic),
                                        rwID_NAOBJECT);

        entry->atomic = (RpAtomic *)data;
        entry->id = ColorGetID(color);
        entry->next = AlphaAtomics;

        AlphaAtomics = entry;

        NumAlphaAtomics++;

        /*
         * We now know there are some alpha components
         * in the current atomic, so stop looking...
         */
        return NULL;
    }
    else
    {
        /*
         * ...otherwise keep going...
         */
        return material;
    }
}


/*
 *****************************************************************************
 */
static RwBool
GeometryPreLightsTestAlpha(RpGeometry *geometry)
{
    if( RpGeometryGetFlags(geometry) & rpGEOMETRYPRELIT )
    {
        RwRGBA *color;
        RwInt32 i;

        color = RpGeometryGetPreLightColors(geometry);

        i = RpGeometryGetNumVertices(geometry)-1;

        while( i >= 0 )
        {
            if( color[i].alpha < 255 )
            {
                return TRUE;
            }

            i--;
        }
    }

    return FALSE;
}


/*
 *****************************************************************************
 */
static RpMaterial *
MaterialGetColorID(RpMaterial *material, void *data)
{
    *(RwChar *)data = ColorGetID(RpMaterialGetColor(material));

    return NULL;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicTestAlpha(RpAtomic *atomic, 
                void *data __RWUNUSED__)
{
    RpGeometry *geometry;

    geometry = RpAtomicGetGeometry(atomic);
    if( geometry )
    {
        /*
         * Test the geometry's prelight colors for alphas less than 255...
         */
        if( GeometryPreLightsTestAlpha(geometry) )
        {
            RwChar id;
            AlphaAtomic *entry;

            /*
             * Get the material's color ID...
             */
            RpGeometryForAllMaterials(geometry, MaterialGetColorID, (void *)&id);

            /*
             * Add atomic to list of alpha atomics...
             */
            entry = (AlphaAtomic *)RwMalloc(sizeof(AlphaAtomic),
                                            rwID_NAOBJECT);

            entry->atomic = atomic;
            entry->id = id;
            entry->next = AlphaAtomics;

            AlphaAtomics = entry;

            NumAlphaAtomics++;
        }
        else
        {
             /*
              * No prelight alphas found, test the geometry materials....
              */
            RpGeometryForAllMaterials(geometry, MaterialTestAlpha, (void *)atomic);
        }
    }

    return atomic;
}


/*
 *****************************************************************************
 */
void 
ClumpGetAlphaAtomics(RpClump *clump)
{
    /*
     * Cache a list of atomics that all have alpha components...
     */
    RpClumpForAllAtomics(clump, AtomicTestAlpha, NULL);

    /*
     * Allocate enough memory for the render-order string "A-B-C-..."...
     */
    RenderOrderString = 
        (RwChar *)RwMalloc(2 * NumAlphaAtomics * sizeof(RwChar) + 1,
                           rwID_NAOBJECT);

    RenderOrderString[0] = '\0';

    return;
}


/*
 *****************************************************************************
 */
void 
DestroyAlphaAtomicsList(void)
{
    AlphaAtomic *entry = AlphaAtomics;
    AlphaAtomic *temp;

    while( entry )
    {
        temp = entry->next;

        RwFree(entry);

        entry = temp;
    }

    RwFree(RenderOrderString);

    return;
}


/*
 *****************************************************************************
 */
static RwReal
AtomicGetCameraDistance(RpAtomic *atomic)
{
    RwFrame *frame;
    RwV3d *camPos, atomicPos, temp;
    RwSphere *atomicBSphere;
    RwReal distance2;

    /*
     * Atomic's bounding-sphere world-space position...
     */
    atomicBSphere = RpAtomicGetBoundingSphere(atomic);
    RwV3dTransformPoint(&atomicPos, &atomicBSphere->center, 
        RwFrameGetLTM(RpAtomicGetFrame(atomic)));

    /*
     * ...camera position...
     */
    frame = RwCameraGetFrame(RwCameraGetCurrentCamera());
    camPos = RwMatrixGetPos(RwFrameGetLTM(frame));

    /*
     * ...vector from camera to atomic...
     */
    RwV3dSub(&temp, &atomicPos, camPos);

    /*
     * Squared distance...
     */
    distance2 = RwV3dDotProduct(&temp, &temp);

    return distance2;
}


/*
 *****************************************************************************
 */
static void
AtomicAddToSortedList(SortAtomic *entry)
{
    SortAtomic *cur;

    /*
     * If current list is empty, start a new list...
     */
    if( !AlphaSortedAtomics )
    {
       AlphaSortedAtomics = entry;

       return;
    }

    /*
     * ...otherwise find correct place in list...
     */
    for(cur=AlphaSortedAtomics; cur; cur=cur->next)
    {
        if( entry->depth >= cur->depth )
        {
            /*
             * Add to start of list... 
             */
            entry->next = AlphaSortedAtomics;
            AlphaSortedAtomics = entry;
            
            break;
        }

        if( cur->next == NULL )
        {
           /*
            * Add to end of list... 
            */
           cur->next = entry;

           break;
        }

        if( entry->depth < cur->depth && entry->depth >= cur->next->depth )
        {
            /*
             * Insert into list...
             */
            entry->next = cur->next;
            cur->next = entry;

            break;
        }
    }

    return;
}


/*
 *****************************************************************************
 */
static RpAtomic *
AlphaSortedAtomicRenderCallback(RpAtomic *atomic)
{
    AlphaAtomic *entry = AlphaAtomics;
    RwBool hasAlpha = FALSE;
    RwChar id = '\0';

    /*
     * Is this atomic a transparent one?...
     */
    while( entry )
    {
        if( atomic == entry->atomic )
        {
            id = entry->id;

            hasAlpha = TRUE;

            break;
        }

        entry = entry->next;
    }

    if( hasAlpha )
    {
        /*
         * Add atomic to deferred list of alpha atomics...
         */
        SortAtomic *entry;

        entry = (SortAtomic *)RwMalloc(sizeof(SortAtomic),
                                       rwID_NAOBJECT);

        entry->atomic = atomic;
        entry->depth = AtomicGetCameraDistance(atomic);
        entry->id = id;
        entry->next = NULL;

        AtomicAddToSortedList(entry);
    }
    else
    {
        /*
         * Atomic has no alpha components, render it now...
         */
        DefaultAtomicRenderCallback(atomic);
    }

    return atomic;
}


/*
 *****************************************************************************
 */
static RpAtomic *
NonAlphaSortedAtomicRenderCallback(RpAtomic *atomic)
{
    AlphaAtomic *entry = AlphaAtomics;
    RwBool hasAlpha = FALSE;
    RwChar id = '\0';

    /*
     * We still look for transparent atomics so that we can
     * display the render order when sorting is disabled...
     */
    while( entry )
    {
        if( atomic == entry->atomic )
        {
            id = entry->id;

            hasAlpha = TRUE;

            break;
        }

        entry = entry->next;
    }

    if( hasAlpha )
    {
        RwChar temp[8];

        /*
         * Add atomic to render order string...
         */
        RsSprintf(temp, RWSTRING("%c-"), id);
        rwstrcat(RenderOrderString, temp);
    }

    /*
     * Render it...
     */
    return DefaultAtomicRenderCallback(atomic);
}


/*
 *****************************************************************************
 */
static RpAtomic *
AtomicSetRenderCallback(RpAtomic *atomic, void *data)
{
    RwBool sort = *(RwBool *)data;

    if( !DefaultAtomicRenderCallback )
    {
        /* 
         * First time through - save default callback...
         */
        DefaultAtomicRenderCallback = RpAtomicGetRenderCallBack(atomic);
    }

    if( sort )
    {
        /*
         * Defer and sort the rendering of transparent atomics...
         */
        RpAtomicSetRenderCallBack(atomic, AlphaSortedAtomicRenderCallback);
    }
    else
    {
        /*
         * Render all atomics without deferral...
         */
        RpAtomicSetRenderCallBack(atomic, NonAlphaSortedAtomicRenderCallback);
    }

    return atomic;
}


void 
ClumpSetAtomicRenderCallback(RpClump *clump, RwBool alphaSort)
{
    RpClumpForAllAtomics(clump, AtomicSetRenderCallback, (void *)&alphaSort);

    return;
}


/*
 *****************************************************************************
 */
void
RenderAlphaSortedAtomics(void)
{
    /* 
     * Render list of deferred transparent atomics. 
     * Once rendered entries are removed...
     */
    while( AlphaSortedAtomics )
    {
        SortAtomic *temp;
        RwChar tempStr[8];

        /*
         * Render atomic...
         */
        DefaultAtomicRenderCallback(AlphaSortedAtomics->atomic);

        /*
         * Add atomic to render order string...
         */
        RsSprintf(tempStr, RWSTRING("%c-"), AlphaSortedAtomics->id);
        rwstrcat(RenderOrderString, tempStr);

        /*
         * Remove the atomic from the list...
         */
        temp = AlphaSortedAtomics;
        AlphaSortedAtomics = AlphaSortedAtomics->next;
        RwFree(temp);
    }

    return;
}

/*
 *****************************************************************************
 */
