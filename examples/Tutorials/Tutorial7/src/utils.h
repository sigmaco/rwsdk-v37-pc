#ifndef UTILS
#define UTILS

#include "rttoc.h"

typedef enum {
    MMNoOperation,
    MMPickAndTranslateObject,
    MMPickAndRotateObject,
    MMPickAndZTranslateObject,
    MMTranslateObject,
    MMRotateObject,
    MMZTranslateObject,
    MMRotateCameraStart,
    MMRotateCamera,
    MMPanCameraStart,
    MMPanCamera,
    MMDollyCameraStart,
    MMDollyCamera,

    MOUSEMOVEOPERATIONFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
} MouseMoveOperation;

extern RpWorld *World;
extern RwV2d MousePos;
extern MouseMoveOperation MouseMoveAction;

/* accept or decline TOC entry callback */

typedef RwBool (* AcceptTOCEntryCB)( RtTOCEntry *tocEntry );


/****************************************************************************
 Function prototypes
 */
#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

extern RpClump *DffLoad(RwChar *filename);
extern void AtomicGetBBox(RpAtomic *atom, RwBBox *box);
extern void DffSave(RpAtomic *atomic, char *filename);

extern RtTOC *
RwsLoadTOC( RwChar *filename );

extern RpWorld *
RwsLoadWorldFromTOC( RwChar *filename,
                     RtTOC *toc,
                     AcceptTOCEntryCB acceptTOCEntry );

extern RpWorld *RwsLoadWorld(RwChar *filename);
extern RpWorld *BspLoad(RwChar *filename);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* UTILS */
