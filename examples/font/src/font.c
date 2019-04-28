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
 * Copyright (c) 1999 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * font.c
 *
 * Copyright (C) 1999 Criterion Technologies.
 *
 * Original author: Colin Ho
 *
 * Purpose: Illustrate the basic usage of font reading and display.
 *
 ****************************************************************************/

#include <stdio.h>
#include <math.h>

#include "rwcore.h"
#include "rt2d.h"
#include "rpcriter.h"

#include "skeleton.h"
#include "menu.h"
#include "camera.h"

#ifdef RWMETRICS
#include "metrics.h"
#endif

#ifdef RWMOUSE
#include "mouse.h"
#endif

#ifdef RWLOGO
#include "rplogo.h"
#endif

#include "main.h"
#include "font.h"

/*
 *****************************************************************************
 */
Rt2dBrush   *Brush;

Rt2dFont    *Term;

RwInt32     FontIndex;
Rt2dFont    *Font[NUMFONT];
RwInt32     *FontCharSet[NUMFONT];
RwInt32     FontCharSetCount[NUMFONT];

RwChar      FontName[NUMFONT][20] =
    { RWSTRING(("tsa16")),
      RWSTRING(("unicode-met1")),
      RWSTRING(("unicode-met2")),
      RWSTRING(("fbigtxt")),
      RWSTRING(("helv")),
      RWSTRING(("hiragana"))
    };

static RwV2d TranslateXStep;
static RwV2d TranslateYStep;
static RwV2d RotateOrigin;
static RwV2d Position;

/*
 *****************************************************************************
 */
static RwTexture *
TextureFilterMode( RwTexture *tex, void * data)
{
    RwTextureFilterMode mode = (RwTextureFilterMode) data;

    RwTextureSetFilterMode(tex, mode);

    return tex;
}

/*
 *****************************************************************************
 */
void
Render2D(RwReal dTime __RWUNUSED__)
{
    Rt2dPath    *path;
    Rt2dBBox    bbox, winbbox;
    RwV2d       anchor;
    RwReal      fontHeight, termHeight, cellHeight, cellWidth;
    RwInt32     i;
    RwRGBA col[] = {
        {255,   0,   0, 255}, /* 0: Red */
        {255, 128,   0, 255}, /* 1: Orange */
        {  0, 128, 160, 255}, /* 2: Dark cyan */
        {255, 255, 255, 255}, /* 3: White */
        {  0,   0,  32, 255}, /* 4: Dark blue */
        {  0,   0, 160, 255}, /* 5: Deep blue */
        {  0, 255,   0, 255}, /* 6: Green */
        {255, 255,   0, 255}, /* 7: Yellow */
        {255,   0, 255,   0}  /* 8: Magenta */
    };
    RwChar      codeStr[10];

    /*
     * Display a background.
     */
    Rt2dCTMPush();
    Rt2dCTMSetIdentity();
    {
        path = Rt2dPathCreate();

        Rt2dDeviceGetClippath(path);

        Rt2dPathGetBBox(path, &winbbox);

        Rt2dBrushSetRGBA(Brush, &col[5], &col[5], &col[4], &col[4]);

        Rt2dPathFill(path, Brush);
    }
    Rt2dCTMPop();

    /* Does this font exist ? */
    if ((Font[FontIndex]) && (Term))
    {
        /* Query the height for the Term font. */
        termHeight = Rt2dFontGetHeight(Term);

        /* Query the height of the font about to be displayed.
        * If the font is outline, then set a constant for the height.
        */
        if (Rt2dFontIsOutline(Font[FontIndex]))
            fontHeight = 0.1f;
        else
            fontHeight = Rt2dFontGetHeight(Font[FontIndex]);

        /*
        * Find the size of the cell for displaying font's character and its
        * code.
        */
        cellHeight = (fontHeight + termHeight) * 1.25f;

        rwsprintf(codeStr, "000000");
        cellWidth = Rt2dFontGetStringWidth(Term, (const RwChar *) codeStr, termHeight);
        if (cellWidth < (fontHeight * 1.0f))
            cellWidth = fontHeight * 1.0f;

        /*
        * Font's character set.
        */
        Rt2dCTMPush();
        {
            /* Set the color for the text. */
            Rt2dBrushSetRGBA(Brush, &col[3], &col[3], &col[3], &col[3]);
            Rt2dBrushSetWidth(Brush, 0.025f);

            anchor.x = 0.0f;
            anchor.y = winbbox.h;

            /*
            * Character set.
            */
            bbox.x = 0.0f;
            bbox.y = anchor.y;
            bbox.w = cellWidth * 2.0f;

        /*
            * Check if it is a unicode font, if so, then the string must be
            * double byte. Apart from this, methods to display the string are the
            * same.
            */
            if (Rt2dFontIsUnicode(Font[FontIndex]))
            {
                RwUInt16        tmpStr[2];

                /*
                * The font is unicode.
                */

                tmpStr[1] = (RwUInt16) 0;

                for (i = 0; i < FontCharSetCount[FontIndex]; i++)
                {
                    /* Display a single font's character. */
                    bbox.y -= fontHeight;
                    bbox.h = fontHeight;
                    tmpStr[0] = (RwUInt16) FontCharSet[FontIndex][i];
                    Rt2dFontFlow(Font[FontIndex],
                        (RwChar *) &tmpStr[0], fontHeight, &bbox, rt2dJUSTIFYCENTER, Brush);

                    /* Display the char's unicode using the Term font. */
                    bbox.y -= termHeight;
                    bbox.h = termHeight;
                    rwsprintf(codeStr, "%04.4x", FontCharSet[FontIndex][i]);
                    Rt2dFontFlow(Term, (RwChar *) codeStr, termHeight, &bbox, rt2dJUSTIFYCENTER, Brush);

                    /* Move the next position */
                    bbox.x += cellWidth;
                    bbox.y = anchor.y;

                    /* Do we need to move to a new line */
                    if ((bbox.x + cellWidth) > winbbox.w)
                    {
                        bbox.x = 0.0f;
                        bbox.y -= cellHeight;
                        anchor.y -= cellHeight;
                    }
                }
            }
            else
            {
                RwChar              tmpStr[2];

                /*
                *  The font is ASCII.
                */

                tmpStr[1] = (RwChar) 0;

                for (i = 0; i < FontCharSetCount[FontIndex]; i++)
                {
                    /* Display a single font's character. */
                    bbox.y -= fontHeight;
                    bbox.h = fontHeight;
                    tmpStr[0] = (RwChar) FontCharSet[FontIndex][i];
                    Rt2dFontFlow(Font[FontIndex], (RwChar *) &tmpStr[0],
                        fontHeight, &bbox, rt2dJUSTIFYCENTER, Brush);

                    /* Display the char's ASCII code using the Term font. */
                    bbox.y -= termHeight;
                    bbox.h = termHeight;
                    rwsprintf(codeStr, "%04.4x", FontCharSet[FontIndex][i]);
                    Rt2dFontFlow(Term, codeStr, termHeight, &bbox, rt2dJUSTIFYCENTER, Brush);

                    /* Move to the next drawing position. */
                    bbox.x += cellWidth;
                    bbox.y = anchor.y;

                    /* Do we need to move to a new line */
                    if ((bbox.x + cellWidth) > winbbox.w)
                    {
                        bbox.x = 0.0f;
                        bbox.y -= cellHeight;
                        anchor.y -= cellHeight;
                    }
                }
            }
        }
        Rt2dCTMPop();
    }

    /*
     * Put a border around the screen of exactly 5 pixels
     */
    Rt2dCTMPush();
    Rt2dCTMSetIdentity();
    {
        RwV2d xstep, ystep, origin;

        Rt2dDeviceGetStep(&xstep, &ystep, &origin);

        Rt2dPathInset(path, RwV2dLength(&xstep) * 2.5f);

        Rt2dBrushSetWidth(Brush, RwV2dLength(&xstep) * 5.0f);
        Rt2dBrushSetRGBA(Brush, &col[0], &col[0], &col[7], &col[7]);

        Rt2dPathStroke(path, Brush);

        Rt2dPathDestroy(path);
    }
    Rt2dCTMPop();

    return;
}

/*
 *****************************************************************************
 */
RwBool
Initialize2D(void)
{
    RwChar              tempStr[256];
    RwUInt32            i, *charSet;
    RwChar              *path;
    RwTexDictionary     *texDict;

    Rt2dOpen(Camera);

    charSet = NULL;

    /* Set up the path where the fonts are stord. */
    rwsprintf(tempStr, RWSTRING("fonts%c"),
              RsPathGetSeparator() );
    path = RsPathnameCreate(tempStr);

    Rt2dFontSetPath(path);
    RsPathnameDestroy(path);

    for (i = 0; i < NUMFONT; i++)
    {
        /* Read in the font, either ASCII or Unicode. */
        Font[i] = Rt2dFontRead(FontName[i]);

        if (Font[i])
        {
            /* Query the number characters in the font. */
            FontCharSetCount[i] = _rt2dFontGetCharacterSetCount(Font[i]);

            /* Create a buffer to store all available characters in the font. */
            FontCharSet[i] = RwMalloc(FontCharSetCount[i] * sizeof(RwUInt32),
                                    rwID_NAOBJECT);
            _rt2dFontGetCharacterSet(Font[i], FontCharSet[i]);
        }
        else
        {
            RsSprintf(tempStr, RWSTRING("The font %s could not be loaded."), FontName[i]);

            RsErrorMessage(tempStr);
        }
    }

    /* Change the filtermode in the font's texture. */
    texDict = Rt2dFontTexDictionaryGet();

    RwTexDictionaryForAllTextures(texDict,
        TextureFilterMode, (void *) rwFILTERLINEAR);

#if (defined (RWTARGET_sky) || defined (RWTARGET_sky2))  /* On PS2 */
    Rt2dDeviceSetFlat(0.5f);
#else
    Rt2dDeviceSetFlat(1.0f);
#endif

    /* Create a global brush. */
    Brush = Rt2dBrushCreate();
    FontIndex = 0;

    /* We use the first font for general info display. */
    Term = Font[0];
    if (Term == NULL)
    {
        RsSprintf(tempStr, RWSTRING("No terminal fonts defined."));

        RsErrorMessage(tempStr);
    }

    /* Push the drawing layer behind the front clip plane. */
    Rt2dDeviceSetLayerDepth(RwCameraGetNearClipPlane(Camera) * (RwReal) 1.01);

    return TRUE;
}

/*
 *****************************************************************************
 */
void
Terminate2D(void)
{
    RwInt32 i;

    /* Destroy the fonts. */

    for (i = 0; i < NUMFONT; i++)
    {
        if (Font[i])
            Rt2dFontDestroy(Font[i]);
        Font[i] = NULL;

        if (FontCharSet[i])
            RwFree(FontCharSet[i]);
        FontCharSet[i] = NULL;
    }

    Term = NULL;

    if (Brush)
        Rt2dBrushDestroy(Brush);
    Brush = NULL;

    Rt2dClose();

    return;

}

/*
 *****************************************************************************
 */
void
PagePositionSet(RwReal x, RwReal y)
{
    Position.x = x;
    Position.y = y;

    return;
}

/*
 *****************************************************************************
 */
void
PageTranslateInit(void)
{
    RwV2d temp;

    Rt2dDeviceGetStep(&TranslateXStep, &TranslateYStep, &temp);

    return;
}

/*
 *****************************************************************************
 */
void
PageTranslate(RwReal x, RwReal y)
{
    Rt2dCTMTranslate( x * TranslateXStep.x,  x * TranslateXStep.y);
    Rt2dCTMTranslate(-y * TranslateYStep.x, -y * TranslateYStep.y);

    return;
}

/*
 *****************************************************************************
 */
void
PageRotateInit(RwReal x, RwReal y)
{
    RwV2d xStep, yStep, origin;

    Rt2dDeviceGetStep(&xStep, &yStep, &origin);
    RwV2dScale(&xStep, &xStep, x);
    RwV2dScale(&yStep, &yStep, y);
    RwV2dAdd(&RotateOrigin, &xStep, &yStep);
    RwV2dAdd(&RotateOrigin, &RotateOrigin, &origin);

    return;
}

/*
 *****************************************************************************
 */
void
PageRotate(RwReal x)
{
    Rt2dCTMTranslate(RotateOrigin.x, RotateOrigin.y);
    Rt2dCTMRotate(x);
    Rt2dCTMTranslate(-RotateOrigin.x, -RotateOrigin.y);

    return;
}

