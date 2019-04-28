
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
 * vshader.h
 *
 * Copyright (C) 1999 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose : RenderWare 3.0 demo.
 *
 ****************************************************************************/

#ifndef VSHADER_H
#define VSHADER_H

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

#include "rwcore.h"
#include "rpworld.h"

extern RwBool VShaderOpen(void);
extern void VShaderClose(void);
extern void VShaderClumpSetPipeline(RpClump *clump);
extern void VShaderUpdate(RwReal delta);
extern void SetRotation(RwReal deltaX, RwReal deltaY);
extern void CameraTranslate(RwReal xDelta, RwReal zDelta);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* VSHADER_H */