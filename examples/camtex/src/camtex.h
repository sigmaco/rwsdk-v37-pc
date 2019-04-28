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
 * camtex.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics camera texture example.
 ****************************************************************************/

#ifndef CAMTEX_H
#define CAMTEX_H

#include "rwcore.h"
#include "rpworld.h"

extern RwCamera *Camera;
extern RpClump *Clump;

extern RwBool SpinOn;

#ifdef    __cplusplus
extern "C"
{
#endif

extern void ClumpRotate(RpClump *clump, RwCamera *camera, 
                        RwReal xAngle, RwReal yAngle);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* CAMTEX_H */
