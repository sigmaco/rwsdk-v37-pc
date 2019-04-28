
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
 * Copyright (c) 2001 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * events.c (win)
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Matt Thorman
 * Reviewed by: 
 * 
 * Purpose: crowd example
 *
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"
#include "camera.h"
#include "menu.h"
#include "events.h"

#include "main.h"

static RwBool       LeftButtonDown = FALSE;
static RwBool       Forwards = FALSE;
static RwBool       Backwards = FALSE;
static RwBool       StrafeLeft = FALSE;
static RwBool       StrafeRight = FALSE;

#define CAMERASPEED (0.5f*MAXCAMERASPEED)
#define CAMERAROTSCL  (0.1f*MAXCAMERAROTSPEED)

/*
 *****************************************************************************
 */
static              RsEventStatus
HandleLeftButtonDown(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    /*
     * Left mouse button down event handling...
     */
    LeftButtonDown = TRUE;

    return rsEVENTPROCESSED;
}

static              RsEventStatus
HandleLeftButtonUp(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    /*
     * Left mouse button up event handling...
     */
    LeftButtonDown = FALSE;
    AddDecal();

    return rsEVENTPROCESSED;
}

/*
 *****************************************************************************
 */
static              RsEventStatus
HandleRightButtonDown(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    /*
     * Right mouse button down event handling...
     */
    return rsEVENTPROCESSED;
}

static              RsEventStatus
HandleRightButtonUp(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    /*
     * Right mouse button up event handling...
     */
    return rsEVENTPROCESSED;
}

/*
 *****************************************************************************
 */
static              RsEventStatus
HandleMouseMove(RsMouseStatus * mouseStatus)
{
    RwV2d               pos;

    CameraTurnRate = -CAMERAROTSCL * mouseStatus->delta.x;
    CameraPitchRate = CAMERAROTSCL * mouseStatus->delta.y;

    /* centre the mouse */
    pos.x = (RwReal) (RsGlobal.maximumWidth) * 0.5f;
    pos.y = (RwReal) (RsGlobal.maximumHeight) * 0.5f;
    RsMouseSetPos(&pos);

    return rsEVENTPROCESSED;
}

/*
 *****************************************************************************
 */
static              RsEventStatus
MouseHandler(RsEvent event, void *param)
{
    /*
     * Let the menu system have a look-in first...
     */
    if (MenuMouseHandler(event, param) == rsEVENTPROCESSED)
    {
        return rsEVENTPROCESSED;
    }

    /*
     * ...then the application events, if necessary...
     */
    switch (event)
    {
        case rsLEFTBUTTONDOWN:
            {
                return HandleLeftButtonDown((RsMouseStatus *) param);
            }

        case rsLEFTBUTTONUP:
            {
                return HandleLeftButtonUp((RsMouseStatus *) param);
            }

        case rsRIGHTBUTTONDOWN:
            {
                return HandleRightButtonDown((RsMouseStatus *) param);
            }

        case rsRIGHTBUTTONUP:
            {
                return HandleRightButtonUp((RsMouseStatus *) param);
            }

        case rsMOUSEMOVE:
            {
                return HandleMouseMove((RsMouseStatus *) param);
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
static              RsEventStatus
HandleKeyDown(RsKeyStatus * keyStatus)
{
    switch (keyStatus->keyCharCode)
    {
        case rsUP:
            {
                /*
                 * CURSOR-UP...
                 */
                CameraSpeed = CAMERASPEED;
                Forwards = TRUE;

                return rsEVENTPROCESSED;
            }

        case rsDOWN:
            {
                /*
                 * CURSOR-DOWN...
                 */
                CameraSpeed = -CAMERASPEED;
                Backwards = TRUE;

                return rsEVENTPROCESSED;
            }

        case rsLEFT:
            {
                /*
                 * CURSOR-LEFT...
                 */
                CameraStrafeSpeed = CAMERASPEED;
                StrafeLeft = TRUE;
                return rsEVENTPROCESSED;
            }

        case rsRIGHT:
            {
                /*
                 * CURSOR-RIGHT...
                 */
                CameraStrafeSpeed = -CAMERASPEED;
                StrafeRight = TRUE;
                return rsEVENTPROCESSED;
            }

        case rsPGUP:
            {
                /*
                 * PAGE-UP...
                 */
                return rsEVENTPROCESSED;
            }

        case rsPGDN:
            {
                /*
                 * PAGE-DOWN...
                 */
                return rsEVENTPROCESSED;
            }

        case rsLCTRL:
        case rsRCTRL:
            {
                /*
                 * LEFT-CTRL or RIGHT-CTRL...
                 */
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
static              RsEventStatus
HandleKeyUp(RsKeyStatus * keyStatus)
{
    switch (keyStatus->keyCharCode)
    {
        case rsUP:
            {
                /*
                 * CURSOR-UP...
                 */
                Forwards = FALSE;
                CameraSpeed = Backwards ? -CAMERASPEED : 0.0f;

                return rsEVENTPROCESSED;
            }

        case rsDOWN:
            {
                /*
                 * CURSOR-DOWN...
                 */
                Backwards = FALSE;
                CameraSpeed = Forwards ? CAMERASPEED : 0.0f;

                return rsEVENTPROCESSED;
            }

        case rsLEFT:
            {
                /*
                 * CURSOR-LEFT...
                 */
                StrafeLeft = FALSE;
                CameraStrafeSpeed = StrafeRight ? -CAMERASPEED : 0.0f;
                return rsEVENTPROCESSED;
            }

        case rsRIGHT:
            {
                /*
                 * CURSOR-RIGHT...
                 */
                StrafeRight = FALSE;
                CameraStrafeSpeed = StrafeLeft ? CAMERASPEED : 0.0f;
                return rsEVENTPROCESSED;
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
static              RsEventStatus
KeyboardHandler(RsEvent event, void *param)
{
    /*
     * Let the menu system have a look-in first...
     */
    if (MenuKeyboardHandler(event, param) == rsEVENTPROCESSED)
    {
        return rsEVENTPROCESSED;
    }

    /*
     * ...then the application events, if necessary...
     */
    switch (event)
    {
        case rsKEYDOWN:
            {
                return HandleKeyDown((RsKeyStatus *) param);
            }

        case rsKEYUP:
            {
                return HandleKeyUp((RsKeyStatus *) param);
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
