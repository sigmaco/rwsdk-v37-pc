
/****************************************************************************
 *
 * metrics.c
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

#if (defined(RWMETRICS))

/*--- Include files ---*/
#include <stdio.h>
#include "rwcore.h"
#include "skeleton.h"
#include "platform.h"
#include "vecfont.h"
#include "metrics.h"

static RsVectorFont *vecFont;
static RwIm2DVertex box[4];

/*
 *****************************************************************************
 */
void
RsMetricsRender(void)
{
    /* Get the stats, and print them */
    RwMetrics          *metrics = RwEngineGetMetrics();

    if (vecFont && metrics)
    {
        RwChar              message[200];
        RwV2d               pos;
        RwBlendFunction     srcBlend, destBlend;

        pos.x = RwIm2DVertexGetScreenX(&(box[0]));
        pos.y = RwIm2DVertexGetScreenY(&(box[0]));

        /* Leave a margin within the box */
        pos.x += vecFont->size.x;
        pos.y += vecFont->size.y;

        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) FALSE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) FALSE);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *) NULL);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *) TRUE);
        RwRenderStateGet(rwRENDERSTATESRCBLEND, (void *)&srcBlend);
        RwRenderStateGet(rwRENDERSTATEDESTBLEND, (void *)&destBlend);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);

        RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, box, 4);

        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,
                         (void *) FALSE);

        sprintf(message, "numTriangles = %08d", metrics->numTriangles);
        RsVecFontPrint(vecFont, &pos, message);
        pos.y += vecFont->size.y;

        sprintf(message, "numProcTriangles = %08d",
                metrics->numProcTriangles);
        RsVecFontPrint(vecFont, &pos, message);
        pos.y += vecFont->size.y;

        sprintf(message, "numVertices = %08d", metrics->numVertices);
        RsVecFontPrint(vecFont, &pos, message);
        pos.y += vecFont->size.y;

        sprintf(message, "numResourceAllocs = %08d",
                metrics->numResourceAllocs);
        RsVecFontPrint(vecFont, &pos, message);
        pos.y += vecFont->size.y;

        sprintf(message, "numTextureUploads = %08d",
                metrics->numTextureUploads);
        RsVecFontPrint(vecFont, &pos, message);
        pos.y += vecFont->size.y;

        sprintf(message, "sizeTextureUploads = %08d",
                metrics->sizeTextureUploads);
        RsVecFontPrint(vecFont, &pos, message);
        pos.y += vecFont->size.y;

        /* Now the device specific ones */
        psMetricsRender(vecFont, &pos, metrics);

        /* Feed the size back into the box, so we get it right next frame */
        RwIm2DVertexSetScreenY(&box[1], pos.y + vecFont->size.y*(rsPRINTMARGINTOP + 1));
        RwIm2DVertexSetScreenY(&box[3], pos.y + vecFont->size.y*(rsPRINTMARGINTOP + 1));

        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) TRUE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) TRUE);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,
                         (void *) FALSE);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)srcBlend);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)destBlend);
    }

    return;

}

/*
 *****************************************************************************
 */
void
RsMetricsClose(void)
{
    if (vecFont)
    {
        RsVecFontDestroy(vecFont);
    }

    RsVecFontClose();

    return;
}

/*
 *****************************************************************************
 */
void
RsMetricsOpen(const RwCamera * camera)
{
    RwV2d               size = { 10.0f, 10.0f };
    RwRGBA              color = { 255, 255, 255, 255 };

    if (RsVecFontOpen())
    {
        vecFont = RsVecFontCreate(camera, &color, &size);
    }

    /* Might want to grow this dynamically to contain the number of metric
     * lines pertaining to this platform.
     */
    RwIm2DVertexSetScreenX(&box[0], 0.0f);
    RwIm2DVertexSetScreenY(&box[0], 0.0f);
    RwIm2DVertexSetScreenZ(&box[0], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetRecipCameraZ(&box[0],
                                1.0f /
                                RwCameraGetNearClipPlane(camera));
    RwIm2DVertexSetU(&box[0], 0.0f, 1.0f);
    RwIm2DVertexSetV(&box[0], 0.0f, 1.0f);
    RwIm2DVertexSetIntRGBA(&box[0], 0, 0, 0, 128);

    RwIm2DVertexSetScreenX(&box[1], 0.0f);
    RwIm2DVertexSetScreenY(&box[1], 0.0f);
    RwIm2DVertexSetScreenZ(&box[1], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetRecipCameraZ(&box[1],
                                1.0f /
                                RwCameraGetNearClipPlane(camera));
    RwIm2DVertexSetU(&box[1], 0.0f, 1.0f);
    RwIm2DVertexSetV(&box[1], 0.0f, 1.0f);
    RwIm2DVertexSetIntRGBA(&box[1], 0, 0, 0, 128);

    RwIm2DVertexSetScreenX(&box[2],
        320.0f + rsPRINTMARGINLEFT*vecFont->size.x);
    RwIm2DVertexSetScreenY(&box[2], 0.0f);
    RwIm2DVertexSetScreenZ(&box[2], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetRecipCameraZ(&box[2],
                                1.0f /
                                RwCameraGetNearClipPlane(camera));
    RwIm2DVertexSetU(&box[2], 0.0f, 1.0f);
    RwIm2DVertexSetV(&box[2], 0.0f, 1.0f);
    RwIm2DVertexSetIntRGBA(&box[2], 0, 0, 0, 128);

    RwIm2DVertexSetScreenX(&box[3],
        320.0f + rsPRINTMARGINLEFT*vecFont->size.x);
    RwIm2DVertexSetScreenY(&box[3], 0.0f);
    RwIm2DVertexSetScreenZ(&box[3], RwIm2DGetNearScreenZ());
    RwIm2DVertexSetRecipCameraZ(&box[3],
                                1.0f /
                                RwCameraGetNearClipPlane(camera));
    RwIm2DVertexSetU(&box[3], 0.0f, 1.0f);
    RwIm2DVertexSetV(&box[3], 0.0f, 1.0f);
    RwIm2DVertexSetIntRGBA(&box[3], 0, 0, 0, 128);

    return;
}
#endif /* (defined(RWMETRICS)) */

/*
 *****************************************************************************
 */
