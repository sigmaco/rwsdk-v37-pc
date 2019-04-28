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
 * viewer.h
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics camera example.
 *          can be calculated.
 ****************************************************************************/


#ifndef VIEWER_H
#define VIEWER_H

#include "rwcore.h"
#include "rpworld.h"

#define VIEWERFARCLIPPLANEMAX   (20.0f)
#define VIEWERNEARCLIPPLANEMIN  (0.1f)
#define VIEWERNEARCLIPPLANESTEP (0.1f)
#define VIEWERFARCLIPPLANESTEP  (0.1f)

#define VIEWERFARCLIPPLANEMIN  (VIEWERNEARCLIPPLANEMIN+VIEWERFARCLIPPLANESTEP)
#define VIEWERNEARCLIPPLANEMAX (VIEWERFARCLIPPLANEMAX-VIEWERNEARCLIPPLANESTEP)



#ifdef    __cplusplus
extern "C"
{
#endif  /* __cplusplus */


extern RwCamera *ViewerCreate(RpWorld *world);
extern RpWorld *ViewerDestroy(RwCamera *camera, RpWorld *world);

extern RwCamera *ViewerSize(RwCamera *camera, RwRect *rect, 
                            RwReal viewWindow, RwReal aspectRatio);
extern RwCamera *ViewerMove(RwCamera *camera, RwV3d *offset);
extern RwCamera *ViewerRotate(RwCamera *camera, RwReal deltaX, RwReal deltaY);
extern RwCamera *ViewerTranslate(RwCamera *camera, RwReal deltaX, RwReal deltaY);

extern RwCamera *ViewerSetPosition(RwCamera *camera, RwV3d *position);


#ifdef    __cplusplus
}
#endif  /* __cplusplus */

#endif /* VIEWER_H */
