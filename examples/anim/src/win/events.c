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
 * Copyright (c) 2002 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * events.c
 *
 * Copyright (C) 2002 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: This Example demonstrates the use of the rtanim plugin to load
 * and render models, and shows how to apply "inks" and "paints".
 *
 ****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "main.h"

/*
 *****************************************************************************
 */

static RwBool spin = FALSE, zoom = FALSE;

static RsEventStatus
HandleLeftButtonDown(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /*
     * Left mouse button down event handling...
     */

    spin = TRUE;

    return rsEVENTPROCESSED;
}

static RsEventStatus
HandleLeftButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /*
     * Left mouse button up event handling...
     */
    spin = FALSE;

    return rsEVENTPROCESSED;
}



/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonDown(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /*
     * Right mouse button down event handling...
     */
    zoom = TRUE;

    return rsEVENTPROCESSED;
}

static RsEventStatus
HandleRightButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /*
     * Right mouse button up event handling...
     */

    zoom = FALSE;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus 
HandleMouseMove(RsMouseStatus *param)
{
    /*
     * Mouse move event handling...
     */
    if (spin)
    {
        CameraLook(param->delta.x, param->delta.y);
    }

    if (zoom)
    {
        CameraTranslateZ(10.0f * param->delta.y);
    }

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
        case rsUP:
        {
            /*
             * CURSOR-UP...
             */
            return rsEVENTNOTPROCESSED;
        }

        case rsDOWN:
        {
            /*
             * CURSOR-DOWN...
             */
            return rsEVENTNOTPROCESSED;
        }

        case rsLEFT:
        {
            /*
             * CURSOR-LEFT...
             */
            return rsEVENTNOTPROCESSED;
        }

        case rsRIGHT:
        {
            /*
             * CURSOR-RIGHT...
             */
            return rsEVENTNOTPROCESSED;
        }

        case rsPGUP:
        {
            /*
             * PAGE-UP...
             */
            return rsEVENTNOTPROCESSED;
        }

        case rsPGDN:
        {
            /*
             * PAGE-DOWN...
             */
            return rsEVENTNOTPROCESSED;
        }

        case rsLCTRL:
        case rsRCTRL:
        {
            /*
             * LEFT-CTRL or RIGHT-CTRL...
             */
            return rsEVENTNOTPROCESSED;
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
HandleKeyUp(RsKeyStatus *keyStatus __RWUNUSED__)
{
    /*
     * Whatever you want or...
     */
    switch( keyStatus->keyCharCode )
    {
        case rsLCTRL:
        case rsRCTRL:
        {
            /*
             * LEFT-CTRL or RIGHT-CTRL...
             */
            return rsEVENTNOTPROCESSED;
        }

        default:
        {
            return rsEVENTNOTPROCESSED;
        }
    }
    return rsEVENTNOTPROCESSED;
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
