
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
 * spline.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 *                                                                         
 * Purpose: RenderWare3 BSP viewer.
 *                         
 ****************************************************************************/

#ifndef SPLINE_H
#define SPLINE_H

#include "rwcore.h"
#include "rpworld.h"
#include "rpspline.h"

extern RpSpline *WayPointSpline;

extern RwReal SplinePos;

extern RwBool SplineOn;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */ 

extern void RenderSpline(void);
extern RwBool LoadSpline(RwChar *splinePath, RpWorld *world);
extern void DestroySpline(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */    

#endif  /* SPLINE_H */
