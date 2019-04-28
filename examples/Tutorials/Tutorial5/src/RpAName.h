/**********************************************************************
 *
 * File : RpAName.h
 *
 **********************************************************************
 *
 * This file is a product of Criterion Software Ltd.
 *
 * This file is provided as is with no warranties of any kind and is
 * provided without any obligation on Criterion Software Ltd. or
 * Canon Inc. to assist in its use or modification.
 *
 * Criterion Software Ltd. will not, under any
 * circumstances, be liable for any lost revenue or other damages arising
 * from the use of this file.
 *
 * Copyright (c) 1999 Criterion Software Ltd.
 * All Rights Reserved.
 *
 * RenderWare is a trademark of Canon Inc.
 *
 ************************************************************************/

/* Function prototypes */
#ifndef RPANAME_H
#define RPANAME_H

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RwInt32 RpAtomicNameInitialize(void);

extern const char *RpAtomicNameSetName(RpAtomic *atom, const char *name);

extern const char *RpAtomicNameGetName(RpAtomic *atom);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* RPANAME_H */
