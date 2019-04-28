
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
 * clmpcntl.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj
 * Reviewed by:
 *
 * Purpose: Handles rotation and translation of the clump.
 *
 ****************************************************************************/

#ifndef CLMPCNTL_H
#define CLMPCNTL_H

#include "rwcore.h"

extern RwBool ClumpRotate;
extern RwBool ClumpTranslate;

extern RwReal ClumpTranslateDeltaZ;
extern RwV3d ClumpRotateDelta;

extern RwBool ClumpDirectTranslate;
extern RwBool ClumpDirectRotate;

extern RwBool ClumpPick;

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern void ClumpControlDirectTranslateZ(RwReal deltaZ);
extern void ClumpControlDirectRotate(RwReal deltaX, RwReal deltaY);
extern void ClumpControlUpdate(RwReal delta);
extern void ClumpControlReset(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* CLMPCNTL_H */

