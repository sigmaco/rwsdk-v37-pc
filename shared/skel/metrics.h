
/****************************************************************************
 *
 * metrics.h
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
 * Copyright (c) 1999, 2000 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

#if (!defined(_METRICS_H))
#define _METRICS_H

#include "rwcore.h"


#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern void RsMetricsOpen(const RwCamera *camera);
extern void RsMetricsClose(void);
extern void RsMetricsRender(void);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */


#endif /* (!defined(_METRICS_H) */

