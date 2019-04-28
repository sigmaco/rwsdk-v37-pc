/***************************************************************************
 *                                                                         *
 * Module  : button.h                                                      *
 *                                                                         *
 **************************************************************************/

#ifndef BUTTON_H
#define BUTTON_H

/****************************************************************************
 Includes
 */
#include "rwcore.h"
#include "rt2d.h"
#include "rt2danim.h"
#include "rtcharse.h"

#ifdef    __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

struct BtnStruct
{
    const RwChar *name;
    RwInt32 id;
};
typedef struct BtnStruct BtnStruct;

struct ButtonLookUpStruct
{
    BtnStruct *buttons;
    RwInt32 count;
};
typedef struct ButtonLookUpStruct ButtonLookUpStruct;

extern ButtonLookUpStruct *ButtonLookUp;

extern RwInt32 DownButtonID;
extern RwInt32 UpButtonID;
extern RwInt32 LeftButtonID;
extern RwInt32 RightButtonID;

extern RwInt32 SelectButtonID;
extern RwInt32 CancelButtonID;


extern void
PressButtonUp(Rt2dMaestro *maestro,RwInt32 buttonID);

extern void
PressButtonDown(Rt2dMaestro *maestro,RwInt32 buttonID);

extern void
CreateLookUpForAllButtons(Rt2dMaestro *maestro);

extern void
CreateMenuForButtonLookUp(void);

extern void
UpdateMenuForButtonLookUp(void);

extern void
CleanButtonLookUp(void);

extern void
DisplayButtonLookUpIndices(RtCharset *charset);

extern RwInt32
GetButtonID(RwInt32 index);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* BUTTON_H */
