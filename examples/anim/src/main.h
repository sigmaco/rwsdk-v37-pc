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
 ****************************************************************************/

/****************************************************************************
 *
 * main.h
 *
 * Copyright (C) 2002 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: This Example demonstrates the use of the rtanim toolkit.
 *
 ****************************************************************************/

#ifndef MAIN_H
#define MAIN_H

#include "rwcore.h"
#include "rpworld.h"
#include "rtanim.h"

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern void
CameraTranslateZ(RwReal zDelta);

extern void
CameraLook(RwReal deltaX, RwReal deltaY);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif
