#ifndef UTILS
#define UTILS

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
extern RpWorld *RwsLoadWorld(RwChar *filename);
extern RpWorld *BspLoad(RwChar *filename);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* UTILS */
