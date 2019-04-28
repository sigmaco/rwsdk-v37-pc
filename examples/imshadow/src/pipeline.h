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
 * pipeline.h
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example of parallel-projection shadow rendering using
 *          multi-texturing.
 *
*****************************************************************************/

#ifndef PIPELINE_H
#define PIPELINE_H

extern RxPipeline *
ProjectionPipelineCreate(void);

extern void
ProjectionPipelineBegin(RwTexture *shadowTexture,
                        RwCamera *camera,
                        RwCamera *shadowCamera,
                        RwReal shadowStrength,
                        RwReal shadowZoneRadius);

extern void
ProjectionPipelineEnd(void);

extern RxPipeline *
ShadowPipelineCreate(void);

#endif /* PIPELINE_H */
