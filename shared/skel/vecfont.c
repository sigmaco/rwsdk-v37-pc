
/****************************************************************************
 *
 * vecfont.c
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
 * Copyright (c) 1999, 2000 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

#include "rwcore.h"

#include "skeleton.h"

#include "vecfont.h"

typedef struct charLine charLine;
struct charLine
{
    RwUInt8             startX;
    RwUInt8             startY;
    RwUInt8             endX;
    RwUInt8             endY;
};

static RwUInt32     charLength[256];
static charLine    *charMaps[256];

#define VECFONTSCALAR (1.0f / 192.0f)

/*
 *****************************************************************************
 */
static void
assignLineList(RwUInt32 target, charLine * lineList)
{
    charLength[target] = 0;
    charMaps[target] = lineList;
    while ((lineList->startX | lineList->startY |
            lineList->endX | lineList->endY) != 0)
    {
        charLength[target]++;
        lineList++;
    }
}

/*
 *****************************************************************************
 */
void
RsVecFontPrint(RsVectorFont * vecFont, const RwV2d * pos,
               const RwChar * string)
{
    RwUInt32 i, numLines;
    RwV2d    localPos;

    localPos = *pos;

    /* Adjust for screen margins */
    localPos.x += vecFont->size.x*rsPRINTMARGINLEFT;
    localPos.y += vecFont->size.x*rsPRINTMARGINTOP;

    /* Make sure that the line vertex buffer is big enough */
    i = numLines = 0;
    while (string[i])
    {
        numLines += charLength[(RwUInt32) string[i]];
        i++;
    }

    /* Will it fit? */
    if (numLines > vecFont->lineVertBufferSize)
    {
        RwUInt32            newSize =
            2 * sizeof(RwIm2DVertex) * numLines;
        RwIm2DVertex       *newLineVertBuffer;

        if (vecFont->lineVertBuffer)
        {
            newLineVertBuffer = (RwIm2DVertex *)
                RwRealloc(vecFont->lineVertBuffer, newSize, rwID_NAOBJECT);
        }
        else
        {
            newLineVertBuffer = (RwIm2DVertex *) RwMalloc(newSize,
                                                          rwID_NAOBJECT);
        }

        if (newLineVertBuffer)
        {
            /* Got through and set all uv, recip z, screen z, etc */
            for (i = vecFont->lineVertBufferSize * 2; i < numLines * 2;
                 i++)
            {
                RwIm2DVertexSetScreenZ(&newLineVertBuffer[i],
                                       RwIm2DGetNearScreenZ());
                RwIm2DVertexSetRecipCameraZ(&newLineVertBuffer[i],
                                            vecFont->recipZ);
                RwIm2DVertexSetU(&newLineVertBuffer[i], 0.0f, 1.0f);
                RwIm2DVertexSetV(&newLineVertBuffer[i], 0.0f, 1.0f);
                RwIm2DVertexSetIntRGBA(&newLineVertBuffer[i],
                                       vecFont->color.red,
                                       vecFont->color.green,
                                       vecFont->color.blue,
                                       vecFont->color.alpha);
            }
            vecFont->lineVertBuffer = newLineVertBuffer;
            vecFont->lineVertBufferSize = numLines;
        }
    }

    /* Will it fit now */
    if (numLines <= vecFont->lineVertBufferSize)
    {
        RwIm2DVertex       *curLineVerts = vecFont->lineVertBuffer;

        i = 0;
        while (string[i])
        {
            RwUInt32            numCharLines =
                charLength[(RwUInt32) string[i]];
            charLine           *curLine =
                charMaps[(RwUInt32) string[i]];

            while (numCharLines--)
            {
                RwLine              line;

                line.start.x = (localPos.x +
                                ((RwReal) curLine->startX *
                                 VECFONTSCALAR*vecFont->size.x));
                line.start.y =
                    (localPos.y +
                     ((RwReal) (128 - curLine->startY) *
                      VECFONTSCALAR*vecFont->size.y));
                line.end.x =
                    (localPos.x +
                     ((RwReal) curLine->endX *
                     VECFONTSCALAR*vecFont->size.x));
                line.end.y =
                    (localPos.y +
                     ((RwReal) (128 - curLine->endY) *
                      VECFONTSCALAR*vecFont->size.y));

                RwIm2DVertexSetScreenX(&curLineVerts[0], line.start.x);
                RwIm2DVertexSetScreenY(&curLineVerts[0], line.start.y);
                RwIm2DVertexSetScreenX(&curLineVerts[1], line.end.x);
                RwIm2DVertexSetScreenY(&curLineVerts[1], line.end.y);

                curLineVerts += 2;
                curLine++;
            }

            localPos.x += vecFont->size.x;
            i++;
        }

        /* We don't need to corrupt too much render state here */
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

        RwIm2DRenderPrimitive(rwPRIMTYPELINELIST,
                              vecFont->lineVertBuffer, numLines * 2);
    }
}

/*
 *****************************************************************************
 */
RwBool
RsVecFontDestroy(RsVectorFont * vecFont)
{
    if (vecFont)
    {
        if (vecFont->lineVertBuffer)
        {
            RwFree(vecFont->lineVertBuffer);
        }
        RwFree(vecFont);
    }

    return (TRUE);
}

/*
 *****************************************************************************
 */
RsVectorFont       *
RsVecFontCreate(const RwCamera * camera, const RwRGBA * color,
                const RwV2d * size)
{
    RsVectorFont       *vecFont;

    vecFont = (RsVectorFont *) RwMalloc(sizeof(RsVectorFont), rwID_NAOBJECT);
    if (vecFont)
    {
        vecFont->size = *size;
        RwRGBAAssign(&vecFont->color, color);
        vecFont->recipZ = 1.0f / RwCameraGetNearClipPlane(camera);

        vecFont->lineVertBuffer = NULL;
        vecFont->lineVertBufferSize = 0;

        return (vecFont);
    }

    return (NULL);
}

/*
 *****************************************************************************
 */
void
RsVecFontClose(void)
{
    /* Nothing to do */
}

/*
 *****************************************************************************
 */
RwBool
RsVecFontOpen(void)
{
    static charLine     symbolMaps1[15][16] = { {{64, 0, 64, 16},
                                                 {64, 32, 64, 128}}, /* ! */
    {{32, 96, 32, 128},
     {96, 96, 96, 128}},       /* " */
    {{32, 0, 32, 128},
     {96, 0, 96, 128},
     {0, 32, 128, 32},
     {0, 96, 128, 96}},        /* # */
    {{0, 0, 128, 0},
     {128, 0, 128, 64},
     {128, 64, 0, 64},
     {0, 64, 0, 128},
     {0, 128, 128, 128},
     {64, 0, 64, 128}},        /* $ */
    {{0, 0, 128, 128},
     {0, 112, 16, 128},
     {112, 0, 128, 16}},       /* % */
    {{0, 0, 0, 0}},            /* & - yeah,   right */
    {{64, 96, 64, 128}},       /* ' */
    {{96, 128, 64, 96},
     {64, 96, 64, 32},
     {64, 32, 96, 0}},         /* ( */
    {{32, 128, 64, 96},
     {64, 96, 64, 32},
     {64, 32, 32, 0}},         /* ) */
    {{64, 0, 64, 128},
     {0, 64, 128, 64},
     {32, 32, 96, 96},
     {32, 96, 96, 32}},        /* * */
    {{64, 32, 64, 96},
     {32, 64, 96, 32}},        /* + */
    {{64, 0, 96, 32}},         /* , */
    {{32, 64, 96, 64}},        /* - */
    {{64, 0, 64, 32}},         /* . */
    {{0, 0, 128, 128}}
    };                         /* / */
    static charLine     symbolMaps2[7][16] = { {{64, 0, 64, 16},
                                                {64, 128, 64, 112}}, /* : */
    {{32, 0, 64, 32},
     {64, 128, 64, 112}},      /* ; */
    {{128, 128, 0, 64},
     {0, 64, 128, 0}},         /* < */
    {{0, 32, 128, 32},
     {0, 96, 128, 96}},        /* =
                                */
    {{0, 128, 128, 64},
     {128, 64, 0, 0}},         /* > */
    {{0, 0, 0, 0}},            /* ? - yeah, right */
    {{0, 0, 0, 0}}
    };                         /* @ - yeah, right */
    static charLine     digitMaps[10][16] = { {{0, 0, 128, 0},
                                               {128, 0, 128, 128},
                                               {128, 128, 0, 128},
                                               {0, 128, 0, 0}}, /* 0 */
    {{128, 0, 128, 128}},      /* 1 */
    {{0, 128, 128, 128},
     {128, 128, 128, 64},
     {128, 64, 0, 64},
     {0, 64, 0, 0},
     {0, 0, 128, 0}},          /* 2 */
    {{0, 128, 128, 128},
     {128, 128, 128, 0},
     {128, 0, 0, 0},
     {128, 64, 0, 64}},        /* 3 */
    {{0, 128, 0, 64},
     {0, 64, 128, 64},
     {128, 128, 128, 0}},      /* 4 */
    {{128, 128, 0, 128},
     {0, 128, 0, 64},
     {0, 64, 128, 64},
     {128, 64, 128, 0},
     {128, 0, 0, 0}},          /* 5 */
    {{128, 128, 0, 128},
     {0, 128, 0, 0},
     {0, 0, 128, 0},
     {128, 0, 128, 64},
     {128, 64, 0, 64}},        /* 6 */
    {{0, 128, 128, 128},
     {128, 128, 128, 0}},      /* 7 */
    {{0, 0, 128, 0},
     {128, 0, 128, 128},
     {128, 128, 0, 128},
     {0, 128, 0, 0},
     {0, 64, 128, 64}},        /* 8 */
    {{0, 0, 128, 0},
     {128, 0, 128, 128},
     {128, 128, 0, 128},
     {0, 128, 0, 64}, {0, 64, 128, 64}}
    };                         /* 9 */
    static charLine     alphaMaps[26][16] = { {{0, 0, 64, 128},
                                               {64, 128, 128, 0},
                                               {32, 64, 96, 64}}, /* a */
    {{0, 128, 0, 0},
     {0, 0, 128, 0},
     {128, 0, 128, 64},
     {128, 64, 0, 64}},        /* b */
    {{128, 128, 0, 128},
     {0, 128, 0, 0},
     {0, 0, 128, 0}},          /* c */
    {{0, 0, 0, 128},
     {0, 128, 128, 64},
     {128, 64, 128, 0},
     {128, 0, 0, 0}},          /* d */
    {{128, 128, 0, 128},
     {0, 128, 0, 0},
     {0, 0, 128, 0},
     {0, 64, 64, 64}},         /* e */
    {{128, 128, 0, 128},
     {0, 128, 0, 0},
     {0, 64, 64, 64}},         /* f */
    {{128, 128, 0, 128},
     {0, 128, 0, 0},
     {0, 0, 128, 0},
     {128, 0, 128, 64}},       /* g */
    {{0, 128, 0, 0},
     {0, 64, 128, 64},
     {128, 128, 128, 0}},      /* h */
    {{64, 0, 64, 128}},        /* i */
    {{0, 0, 128, 0},
     {128, 0, 128, 128}},      /* j */
    {{0, 0, 0, 128},
     {128, 0, 0, 64},
     {0, 64, 128, 128}},       /* k */
    {{0, 0, 0, 128},
     {0, 0, 128, 0}},          /* l */
    {{0, 0, 0, 128},
     {0, 128, 64, 0},
     {64, 0, 128, 128},
     {128, 128, 128, 0}},      /* m */
    {{0, 0, 0, 128},
     {0, 128, 128, 128},
     {128, 128, 128, 0}},      /* n */
    {{0, 0, 128, 0},
     {128, 0, 128, 128},
     {128, 128, 0, 128},
     {0, 128, 0, 0}},          /* o */
    {{0, 0, 0, 128},
     {0, 128, 128, 128},
     {128, 128, 128, 64},
     {128, 64, 0, 64}},        /* p */
    {{128, 0, 128, 128},
     {128, 128, 0, 128},
     {0, 128, 0, 64},
     {0, 64, 128, 64}},        /* q */
    {{0, 0, 0, 128},
     {0, 128, 128, 128},
     {128, 128, 128, 64},
     {128, 64, 0, 64},
     {0, 64, 128, 0}},         /* r */
    {{0, 0, 128, 0},
     {128, 0, 128, 64},
     {128, 64, 0, 64},
     {0, 64, 0, 128},
     {0, 128, 128, 128}},      /* s */
    {{0, 128, 128, 128},
     {64, 128, 64, 0}},        /* t */
    {{0, 128, 0, 0},
     {0, 0, 128, 0},
     {128, 0, 128, 128}},      /* u */
    {{0, 128, 64, 0},
     {64, 0, 128, 128}},       /* v */
    {{0, 128, 0, 0},
     {0, 0, 64, 128},
     {64, 128, 128, 0},
     {128, 0, 128, 128}},      /* w */
    {{0, 128, 128, 0},
     {0, 0, 128, 128}},        /* x */
    {{0, 0, 128, 0},
     {128, 0, 128, 128},
     {0, 128, 0, 64},
     {0, 64, 128, 64}},        /* y */
    {{128, 0, 0, 0},
     {0, 0, 128, 128},
     {128, 128, 0, 128}}
    };                         /* z */
    RwUInt32            i;

    for (i = 0; i < 256; i++)
    {
        charLength[i] = 0;
    }

    for (i = 0; i < 15; i++)
    {
        assignLineList('!' + i, symbolMaps1[i]);
    }

    for (i = 0; i < 10; i++)
    {
        assignLineList('0' + i, digitMaps[i]);
    }

    for (i = 0; i < 7; i++)
    {
        assignLineList(':' + i, symbolMaps2[i]);
    }

    for (i = 0; i < 26; i++)
    {
        assignLineList('a' + i, alphaMaps[i]);
        assignLineList('A' + i, alphaMaps[i]);
    }

    return (TRUE);
}

/*
 *****************************************************************************
 */
