
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
 * tga.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Demonstrates how different image file formats can be registered 
 *          and used with RenderWare.
 *
*****************************************************************************/

#include <string.h>

#include "rwcore.h"

#include "tga.h"

#define MAX_ID_LENGTH (255)

#define GETBYTE(p, off) ((RwUInt8 *)(p) + (off))

#ifdef rwLITTLEENDIAN
/*
 * We can just pull from memory...
 */
#define GETLITTLERWINT16(p)  (*(RwInt16 *)(GETBYTE(p, 0)))

#else /* rwLITTLEENDIAN */
/*
 * We have to shuffle the bits...
 */
#define GETLITTLERWINT16(p)  ((((RwInt16)(*GETBYTE(p, 0))))  | \
                                  (((RwInt16)(*GETBYTE(p, 1))) << 8))

#define GETBIGRWINT16(p)     ((((RwInt16)(*GETBYTE(p, 1))))  | \
                                  (((RwInt16)(*GETBYTE(p, 0))) << 8))
#endif /* rwLITTLEENDIAN */

#define GETPIXELDATA(dest, src, offset) \
    (dest[0] = src[offset+0], \
     dest[1] = src[offset+1], \
     dest[2] = src[offset+2])

/*
 * TGA image file format header...
 */
typedef struct TGAHeader TGAHeader;
struct TGAHeader
{
    RwUInt8             idLength;
    RwUInt8             colorMapType;
    RwUInt8             imageType;
    RwUInt16            firstEntryIndex;
    RwUInt16            colorMapLength;
    RwUInt8             colorMapEntrySize;
    RwUInt16            xOriginImage;
    RwUInt16            yOriginImage;
    RwUInt16            imageWidth;
    RwUInt16            imageHeight;
    RwUInt8             pixelDepth;
    RwUInt8             imageDescriptor;
};

typedef struct RLE24bitPacket RLE24bitPacket;
struct RLE24bitPacket
{
    RwUInt8             pixelCount;
    RwUInt8             pixelData[3];
};

/*
 *****************************************************************************
 */
static RwImage *
tgaImageReadRLECompressed24bitRGB(TGAHeader header, RwStream * stream)
{
    RwImage            *tgaImage;
    RwUInt8            *imagePixels;
    RwUInt8            *pixelData;
    RwInt32             imageSize;
    RwInt32             i, j;
    RLE24bitPacket      rlePacket;

    /*
     * Create an image...
     */
    imageSize = header.imageWidth * header.imageHeight;
    tgaImage = RwImageCreate((RwInt32) header.imageWidth,
                             (RwInt32) header.imageHeight, 32);

    /*
     * Allocate pixel space...
     */
    RwImageAllocatePixels(tgaImage);

    /*
     * Get pointer to image pixels...
     */
    imagePixels = RwImageGetPixels(tgaImage);

    /*
     * Read the pixel data from file...
     */
    pixelData = (RwUInt8 *) RwMalloc(imageSize * 3, rwID_NAOBJECT);

    i = 0;
    while (i < imageSize * 3)
    {
        RwStreamRead(stream, &rlePacket.pixelCount, 1);

        if (rlePacket.pixelCount & 128)
        {
            /*
             * Process single RLE packet...
             */
            RwStreamRead(stream, &rlePacket.pixelData, 3);

            for (j = 0; j < (rlePacket.pixelCount % 128 + 1); j++)
            {
                memcpy(pixelData + i, &rlePacket.pixelData, 3);

                i += 3;
            }
        }
        else
        {
            /*
             * Process block of RAW packets...
             */
            RwStreamRead(stream, pixelData + i,
                         (rlePacket.pixelCount + 1) * 3);

            i += (rlePacket.pixelCount + 1) * 3;
        }

    }

    /*
     * Convert data to 32-bit...
     */
    for (i = 0; i < imageSize; i++)
    {
        imagePixels[(i * 4) + 0] = pixelData[(i * 3) + 2];
        imagePixels[(i * 4) + 1] = pixelData[(i * 3) + 1];
        imagePixels[(i * 4) + 2] = pixelData[(i * 3) + 0];
        imagePixels[(i * 4) + 3] = 255;
    }

    RwFree(pixelData);

    return tgaImage;
}

/*
 *****************************************************************************
 */
static RwImage *
tgaImageReadUncompressed24bitRGB(TGAHeader header, RwStream * stream)
{
    RwImage            *tgaImage;
    RwUInt8            *imagePixels;
    RwUInt8            *pixelData;
    RwInt32             imageSize;
    RwInt32             i;

    /*
     * Create an image...
     */
    imageSize = header.imageWidth * header.imageHeight;
    tgaImage = RwImageCreate((RwInt32) header.imageWidth,
                             (RwInt32) header.imageHeight, 32);

    /*
     * Allocate pixel space...
     */
    RwImageAllocatePixels(tgaImage);

    /*
     * Get pointer to image pixels...
     */
    imagePixels = RwImageGetPixels(tgaImage);

    /*
     * Read the pixel data from file...
     */
    pixelData = (RwUInt8 *) RwMalloc(imageSize * 3, rwID_NAOBJECT);
    RwStreamRead(stream, pixelData, imageSize * 3);

    /*
     * Convert data to 32-bit...
     */
    for (i = 0; i < imageSize; i++)
    {
        imagePixels[(i * 4) + 0] = pixelData[(i * 3) + 2];
        imagePixels[(i * 4) + 1] = pixelData[(i * 3) + 1];
        imagePixels[(i * 4) + 2] = pixelData[(i * 3) + 0];
        imagePixels[(i * 4) + 3] = 255;
    }

    RwFree(pixelData);

    return tgaImage;
}

/*
 *****************************************************************************
 */
RwImage *
ImageReadTGA(const RwChar * imageName)
{
    RwStream           *stream;

    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, imageName);

    if (stream)
    {
        TGAHeader           tgaHeader;
        RwImage            *image;

        /*
         * Read the header...
         */
        RwStreamRead(stream, &tgaHeader.idLength, 1);
        RwStreamRead(stream, &tgaHeader.colorMapType, 1);
        RwStreamRead(stream, &tgaHeader.imageType, 1);
        RwStreamRead(stream, &tgaHeader.firstEntryIndex, 2);
        RwStreamRead(stream, &tgaHeader.colorMapLength, 2);
        RwStreamRead(stream, &tgaHeader.colorMapEntrySize, 1);
        RwStreamRead(stream, &tgaHeader.xOriginImage, 2);
        RwStreamRead(stream, &tgaHeader.yOriginImage, 2);
        RwStreamRead(stream, &tgaHeader.imageWidth, 2);
        RwStreamRead(stream, &tgaHeader.imageHeight, 2);
        RwStreamRead(stream, &tgaHeader.pixelDepth, 1);
        RwStreamRead(stream, &tgaHeader.imageDescriptor, 1);

#ifndef rwLITTLEENDIAN
        /*
         * Those variables longer than 1 byte will be incorrect...
         */
        tgaHeader.firstEntryIndex =
            GETLITTLERWINT16(&tgaHeader.firstEntryIndex);
        tgaHeader.colorMapLength =
            GETLITTLERWINT16(&tgaHeader.colorMapLength);
        tgaHeader.xOriginImage =
            GETLITTLERWINT16(&tgaHeader.xOriginImage);
        tgaHeader.yOriginImage =
            GETLITTLERWINT16(&tgaHeader.yOriginImage);
        tgaHeader.imageWidth = GETLITTLERWINT16(&tgaHeader.imageWidth);
        tgaHeader.imageHeight =
            GETLITTLERWINT16(&tgaHeader.imageHeight);
#endif /* rwLITTLEENDIAN */

        /*
         * We only load 24-bit images...
         */
        if (tgaHeader.pixelDepth != 24)
        {
            RwStreamClose(stream, NULL);

            return (RwImage *)NULL;
        }

        switch (tgaHeader.imageType)
        {
            case 2:
                {
                    /*
                     * Uncompressed, RGB image...
                     */
                    image =
                        tgaImageReadUncompressed24bitRGB(tgaHeader,
                                                         stream);

                    break;
                }

            case 10:
                {
                    /*
                     * RLE encoded, RGB image...
                     */
                    image =
                        tgaImageReadRLECompressed24bitRGB(tgaHeader,
                                                          stream);

                    break;
                }

            default:
                {
                    image = (RwImage *)NULL;
                }
        }

        RwStreamClose(stream, NULL);

        return image;
    }

    return (RwImage *)NULL;
}

/*
 *****************************************************************************
 */
static RwImage *
tgaImageWriteUncompressed24bitRGB(RwImage * image, TGAHeader header,
                                  RwStream * stream)
{
    RwUInt8            *imagePixels;
    RwUInt8            *pixelData;
    RwInt32             imageSize;
    RwInt32             i;

    /*
     * Create an image...
     */
    imageSize = header.imageWidth * header.imageHeight;

    /*
     * Get pointer to image pixels...
     */
    imagePixels = RwImageGetPixels(image);

    /*
     * Create the pixel data...
     */
    pixelData = (RwUInt8 *) RwMalloc(imageSize * 3, rwID_NAOBJECT);

    /*
     * Convert data back to 24-bit...
     */
    for (i = 0; i < imageSize; i++)
    {
        pixelData[(i * 3) + 0] = imagePixels[(i * 4) + 2];
        pixelData[(i * 3) + 1] = imagePixels[(i * 4) + 1];
        pixelData[(i * 3) + 2] = imagePixels[(i * 4) + 0];
    }

    RwStreamWrite(stream, pixelData, imageSize * 3);

    RwFree(pixelData);

    return image;
}

#if (defined(TGAIMAGEWRITERLECOMPRESSED24BITRGB))

/*
 *****************************************************************************
 */
static RwImage *
tgaImageWriteRLECompressed24bitRGB(RwImage * image, TGAHeader header,
                                   RwStream * stream)
{
    RwUInt8            *imagePixels;
    RwUInt8            *pixelData;
    RwInt32             pixelDataSize = 0;
    RwInt32             imageSize;
    RwInt32             i;

    /*
     * Create an image...
     */
    imageSize = header.imageWidth * header.imageHeight;

    /*
     * Get pointer to image pixels...
     */
    imagePixels = RwImageGetPixels(image);

    /*
     * Create the pixel data:
     * - potentially there can be up to 4 bytes per pixel, a header(1) and data(3)
     * - the image is not analysed to determine whether there are any benefits to
     *   storing the data as RLE. Images with few areas of the same color don't
     *   benefit from RLE and advanced writers use RAW format for such images.
     */
    pixelData = (RwUInt8 *) RwMalloc(imageSize * 4, rwID_NAOBJECT);

    /*
     * Convert data to RLE 24-bit...
     */
    for (i = 0; i < imageSize; i++)
    {
        static RwUInt8      curPixelData[3];
        static RwUInt8      pixelCount = 0;
        static RwBool       newPixel = TRUE;

        /*
         * Compare the current pixel data to the current pixel we are counting...
         */
        if (newPixel)
        {
            /*
             * Initialize pixel data and counter...
             */
            pixelCount = 1;

            GETPIXELDATA(curPixelData, imagePixels, i * 4);

            newPixel = FALSE;
        }
        else if (imagePixels[(i * 4) + 2] == curPixelData[0] &&
                 imagePixels[(i * 4) + 1] == curPixelData[1] &&
                 imagePixels[(i * 4) + 0] == curPixelData[2])
        {
            /*
             * Count number of pixels that are the same...
             */
            pixelCount++;

            /*
             * The packet cannot cross scanlines...
             */
            if ((i + 1) % header.imageWidth == 0)
            {
                /*
                 * Write the header...
                 */
                pixelData[pixelDataSize] = (pixelCount - 1) | 128;

                /*
                 * Write the pixel data...
                 */
                pixelData[pixelDataSize + 1] = curPixelData[2];
                pixelData[pixelDataSize + 2] = curPixelData[1];
                pixelData[pixelDataSize + 3] = curPixelData[0];
                pixelDataSize += 4;

                newPixel = TRUE;
            }
        }
        else
        {
            /*
             * Pixel is different...
             */

            /*
             * Write the header...
             */
            pixelData[pixelDataSize] = (pixelCount - 1) | 128;

            /*
             * Write the pixel data...
             */
            pixelData[pixelDataSize + 1] = curPixelData[2];
            pixelData[pixelDataSize + 2] = curPixelData[1];
            pixelData[pixelDataSize + 3] = curPixelData[0];
            pixelDataSize += 4;

            /*
             * Record different pixel data...
             */
            pixelCount = 1;

            GETPIXELDATA(curPixelData, imagePixels, i * 4);
        }
    }

    RwStreamWrite(stream, pixelData, pixelDataSize);

    RwFree(pixelData);

    return image;
}

#endif /* (defined(TGAIMAGEWRITERLECOMPRESSED24BITRGB)) */

/*
 *****************************************************************************
 */
RwImage *
ImageWriteTGA(RwImage * image, const RwChar * imageName)
{
    RwStream           *stream;

    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, imageName);

    if (stream)
    {
        TGAHeader           tgaHeader;
        RwImage            *img;

        /*
         * Construct the header (we only write uncompressed image files)...
         */
        tgaHeader.idLength = (RwUInt8) 0;
        tgaHeader.colorMapType = (RwUInt8) 0;
        tgaHeader.imageType = (RwUInt8) 2;
        tgaHeader.firstEntryIndex = (RwUInt16) 0;
        tgaHeader.colorMapLength = (RwUInt16) 0;
        tgaHeader.colorMapEntrySize = (RwUInt8) 0;
        tgaHeader.xOriginImage = (RwUInt16) 0;
        tgaHeader.yOriginImage = (RwUInt16) 0;
        tgaHeader.imageWidth = (RwUInt16) RwImageGetWidth(image);
        tgaHeader.imageHeight = (RwUInt16) RwImageGetHeight(image);
        tgaHeader.pixelDepth = (RwUInt8) 24;
        tgaHeader.imageDescriptor = (RwUInt8) 0;

#ifndef rwLITTLEENDIAN
        /*
         * Those variables longer than 1 byte will be incorrect...
         */
        tgaHeader.firstEntryIndex =
            GETBIGRWINT16(&tgaHeader.firstEntryIndex);
        tgaHeader.colorMapLength =
            GETBIGRWINT16(&tgaHeader.colorMapLength);
        tgaHeader.xOriginImage = GETBIGRWINT16(&tgaHeader.xOriginImage);
        tgaHeader.yOriginImage = GETBIGRWINT16(&tgaHeader.yOriginImage);
        tgaHeader.imageWidth = GETBIGRWINT16(&tgaHeader.imageWidth);
        tgaHeader.imageHeight = GETBIGRWINT16(&tgaHeader.imageHeight);
#endif /* rwLITTLEENDIAN */

        /* Write the header */
        RwStreamWrite(stream, &tgaHeader.idLength, 1);
        RwStreamWrite(stream, &tgaHeader.colorMapType, 1);
        RwStreamWrite(stream, &tgaHeader.imageType, 1);
        RwStreamWrite(stream, &tgaHeader.firstEntryIndex, 2);
        RwStreamWrite(stream, &tgaHeader.colorMapLength, 2);
        RwStreamWrite(stream, &tgaHeader.colorMapEntrySize, 1);
        RwStreamWrite(stream, &tgaHeader.xOriginImage, 2);
        RwStreamWrite(stream, &tgaHeader.yOriginImage, 2);
        RwStreamWrite(stream, &tgaHeader.imageWidth, 2);
        RwStreamWrite(stream, &tgaHeader.imageHeight, 2);
        RwStreamWrite(stream, &tgaHeader.pixelDepth, 1);
        RwStreamWrite(stream, &tgaHeader.imageDescriptor, 1);

        /*
         * Uncompressed, RGB images...
         */
        img =
            tgaImageWriteUncompressed24bitRGB(image, tgaHeader, stream);

        RwStreamClose(stream, NULL);

        return img;
    }

    return (RwImage *)NULL;
}

/*
 *****************************************************************************
 */
