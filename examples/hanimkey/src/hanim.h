
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
 * hanim.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To demonstrate the usage of custom RpHAnimAnimation 
 *          keyframe interpolation schemes.
 * *****************************************************************************/

#ifndef HANIM4_H
#define HANIM4_H

#include "rwcore.h"
#include "rpworld.h"
#include "rtanim.h"

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

extern  RwBool  HAnimOpen(RpWorld *world);
extern  void    HAnimClose(void);

extern  void    HAnimMenuOpen(void);

extern  void    HAnimUpdate(RwReal deltaTime);
extern  void    HAnimRender(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* HANIM4_H */
