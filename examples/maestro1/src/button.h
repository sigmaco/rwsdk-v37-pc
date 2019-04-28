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

#define MAX_BUTTONS 6

enum ButtonNo {
    DownButton=0,
    UpButton,
    LeftButton,
    RightButton,
    SelectButton,
    CancelButton,
    LastButton = MAX_BUTTONS
};

extern BtnStruct BtnList[MAX_BUTTONS];

extern void
PressButton(Rt2dMaestro *maestro,RwInt32 buttonID,RwUInt32 animButtonState);

extern void
CreateLookUpForAllButtons(Rt2dMaestro *maestro);

#ifdef    __cplusplus
}
#endif                          /* __cplusplus */

#endif /* BUTTON_H */
