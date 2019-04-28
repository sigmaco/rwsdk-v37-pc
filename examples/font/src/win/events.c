
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
 * events.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: RenderWare Graphics camera example.
 *          can be calculated.
 ****************************************************************************/


#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"

#include "main.h"
#include "font.h"


static RwBool Ctrl;
static RwBool Alt;
static RwBool Shift;

enum _MMMode
{
    MMNoAction,
    MMTranslate,
    MMRotate,

    _MMMODEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
typedef enum _MMMode MMMode;

static MMMode MouseMoveMode = MMNoAction;


/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonDown(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    PageTranslateInit();

    MouseMoveMode = MMTranslate;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonUp(RsMouseStatus *mouseStatus )
{
    MouseMoveMode = MMNoAction;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonDown(RsMouseStatus *mouseStatus )
{
    PageRotateInit(mouseStatus->pos.x, WinHeight - mouseStatus->pos.y);

    MouseMoveMode = MMRotate;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    MouseMoveMode = MMNoAction;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleMouseMove(RsMouseStatus *mouseStatus)
{
    switch (MouseMoveMode)
    {
        case MMTranslate:
        {
            PageTranslate(mouseStatus->delta.x,
                          mouseStatus->delta.y);
            break;
        }

        case MMRotate:
        {
            PageRotate(mouseStatus->delta.x);
            break;
        }

        default:
            break;

    }

    PagePositionSet(mouseStatus->pos.x, mouseStatus->pos.y);

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
MouseHandler(RsEvent event, void *param)
{
    /*
     * Let the menu system have a look-in first...
     */
    if( MenuMouseHandler(event, param) == rsEVENTPROCESSED )
    {
        return rsEVENTPROCESSED;
    }

    /*
     * ...then the application events, if necessary...
     */
    switch( event )
    {
        case rsLEFTBUTTONDOWN:
        {
            return HandleLeftButtonDown((RsMouseStatus *)param);
        }

        case rsLEFTBUTTONUP:
        {
            return HandleLeftButtonUp((RsMouseStatus *)param);
        }

        case rsRIGHTBUTTONDOWN:
        {
            return HandleRightButtonDown((RsMouseStatus *)param);
        }

        case rsRIGHTBUTTONUP:
        {
            return HandleRightButtonUp((RsMouseStatus *)param);
        }

        case rsMOUSEMOVE:
        {
            return HandleMouseMove((RsMouseStatus *)param);
        }

        default:
        {
            return rsEVENTNOTPROCESSED;
        }
    }
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleKeyDown(RsKeyStatus *keyStatus)
{
    switch( keyStatus->keyCharCode )
    {
        case rsLCTRL:
        case rsRCTRL:
        {
            Ctrl = TRUE;

            return rsEVENTPROCESSED;
        }

        case rsLALT:
        case rsRALT:
        {
            Alt = TRUE;

            return rsEVENTPROCESSED;
        }

        case rsLSHIFT:
        case rsRSHIFT:
        {
            Shift = TRUE;

            return rsEVENTPROCESSED;
        }

        default:
        {
            return rsEVENTNOTPROCESSED;
        }
    }
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleKeyUp(RsKeyStatus *keyStatus)
{
    switch( keyStatus->keyCharCode )
    {
        case rsLCTRL:
        case rsRCTRL:
        {
            Ctrl = FALSE;

            return rsEVENTPROCESSED;
        }

        case rsLALT:
        case rsRALT:
        {
            Alt = FALSE;

            return rsEVENTPROCESSED;
        }

        case rsLSHIFT:
        case rsRSHIFT:
        {
            Shift = FALSE;

            return rsEVENTPROCESSED;
        }

        default:
        {
            return rsEVENTNOTPROCESSED;
        }
	}
}


/*
 *****************************************************************************
 */
static RsEventStatus
KeyboardHandler(RsEvent event, void *param)
{
    /*
     * Let the menu system have a look-in first...
     */
    if( MenuKeyboardHandler(event, param) == rsEVENTPROCESSED )
    {
        return rsEVENTPROCESSED;
    }

    /*
     * ...then the application events, if necessary...
     */
    switch( event )
    {
        case rsKEYDOWN:
        {
            return HandleKeyDown((RsKeyStatus *)param);
        }

        case rsKEYUP:
        {
            return HandleKeyUp((RsKeyStatus *)param);
        }

        default:
        {
            return rsEVENTNOTPROCESSED;
        }
    }
}


/*
 *****************************************************************************
 */
RwBool
AttachInputDevices(void)
{
    RsInputDeviceAttach(rsKEYBOARD, KeyboardHandler);

    RsInputDeviceAttach(rsMOUSE, MouseHandler);

    return TRUE;
}

/*
 *****************************************************************************
 */
