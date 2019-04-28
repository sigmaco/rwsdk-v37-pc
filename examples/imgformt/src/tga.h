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
 * tga.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Demonstrates how different image file formats can be registered 
 *          and used with RenderWare.
 *
*****************************************************************************/

#ifndef TGA_H
#define TGA_H

#include "rwcore.h"

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwImage *ImageReadTGA(const RwChar *imageName);
extern RwImage *ImageWriteTGA(RwImage *image, const RwChar *imageName);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


#endif /* TGA_H */
