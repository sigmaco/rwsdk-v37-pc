
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
 * main.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: A viewer capable of displaying clump's - including bones, skin, 
 *          and animation support
 *
 ****************************************************************************/

#ifndef MAIN_H
#define MAIN_H

#include "rwcore.h"
#include "rpworld.h"
#include "rtcharse.h"

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

typedef enum
{
    RENDERSOLID = 0,
    RENDERWIRE,
    RENDERSKEL,
    RENDERWIRESKEL,
    RENDERWIRESOLID,
    RENDERSOLIDSKEL,
    RENDERTRISTRIP,
    RENDERTRISTRIPS,
    RENDERMESHES,
    RENDERALL,

    NUMRENDERMODES,

    RENDERMODEENUMFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
}
RenderModeEnum;

extern RwInt32 RenderMode;
extern RwInt32 NumTriStripAngles;
extern RwReal NormalsScaleFactor;
extern RwBool NormalsOn;

extern RwBool SpinOn;

extern RtCharset *Charset;

#endif /* MAIN_H */

