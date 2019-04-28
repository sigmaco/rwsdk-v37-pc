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
 * subrast.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how sub-rasters can be used to render multiple
 *          views.
 *
 ****************************************************************************/

#ifndef SUBRAST_H
#define SUBRAST_H

#include "rwcore.h"
#include "rpworld.h"

#define DEFAULT_VIEWWINDOW (0.5f)

#ifdef WIDE_SCREEN
#define DEFAULT_ASPECTRATIO (16.0f/9.0f)
#else
#define DEFAULT_ASPECTRATIO (4.0f/3.0f)
#endif

#define NUMOFSUBCAMERAS (4)

extern RwCamera *Camera;
extern RwCamera *SubCameras[NUMOFSUBCAMERAS];

extern RwV3d XAxis;
extern RwV3d YAxis;


#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */


extern RwBool CreateCameras(RpWorld *world);
extern void DestroyCameras(RpWorld *world);

extern void UpdateSubRasters(RwCamera *mainCamera, 
                              RwInt32 mainWidth, RwInt32 mainHeight);

extern void RotateClump(RwReal xAngle, RwReal yAngle);

#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif  /* SUBRAST_H */
