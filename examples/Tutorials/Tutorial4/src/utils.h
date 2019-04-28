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

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* UTILS */
