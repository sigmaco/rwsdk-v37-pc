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
 * gcond.h
 *
 * Copyright (C) 2003 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate how to use geometry conditioning for textured scenes
 * with discontiguous UV coordinates.
 *
 ****************************************************************************/

#ifndef GCOND_H
#define GCOND_H

#include "rwcore.h"
#include "rpworld.h"

/*
 * Number of squares along the edge of the surface...
 */
#define NumEdges 16


#ifdef    __cplusplus
extern "C"
{
#endif	/* __cplusplus */

extern RpWorld *CreateWorld(RwBool geometryConditioning, RwBool wrap, RwInt32 uvLimit);


#ifdef    __cplusplus
}
#endif	/* __cplusplus */

#endif	/* GCOND_H */
