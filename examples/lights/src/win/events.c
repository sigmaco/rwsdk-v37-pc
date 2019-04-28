
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
 * Purpose: To illustrate the different lights that are available for use in
 *          RenderWare.
 *
 *****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"

#include "lights.h"

static RwBool RotateLight = FALSE;
static RwBool TranslateLightZ = FALSE;
static RwBool TranslateLightXY = FALSE;

static RwBool Ctrl = FALSE;


/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonDown(void)
{
    if( Ctrl )
    {
        TranslateLightXY = TRUE;
    }
    else
    {
        RotateLight = TRUE;
    }

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonUp(void)
{
    TranslateLightXY = FALSE;
    RotateLight = FALSE;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonDown(void)
{
    if( Ctrl )
    {
        TranslateLightZ = TRUE;
    }

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonUp(void)
{
    TranslateLightZ = FALSE;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleMouseMove(void *param)
{
    RsMouseStatus *mouseStatus;

    mouseStatus = (RsMouseStatus *)param;

    if( RotateLight )
    {
        RwReal xAngle, yAngle;

        xAngle = -mouseStatus->delta.x;
        yAngle = mouseStatus->delta.y;

        LightRotate(xAngle, yAngle);
    }

    if( TranslateLightZ )
    {
        RwReal yDelta;

        yDelta = -mouseStatus->delta.y * 0.1f;

        LightTranslateZ(yDelta);
    }

    if( TranslateLightXY )
    {
        RwReal xDelta, yDelta;

        xDelta = -mouseStatus->delta.x * 0.1f;
        yDelta = -mouseStatus->delta.y * 0.1f;

        LightTranslateXY(xDelta, yDelta);
    }

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
MouseHandler(RsEvent event, void *param)
{
    if( MenuMouseHandler(event, param) == rsEVENTPROCESSED )
    {
        return rsEVENTPROCESSED;
    }

    switch( event )
    {
        case rsLEFTBUTTONDOWN:
        {
            return HandleLeftButtonDown();
        }

        case rsLEFTBUTTONUP:
        {
            return HandleLeftButtonUp();
        }

        case rsRIGHTBUTTONDOWN:
        {
            return HandleRightButtonDown();
        }

        case rsRIGHTBUTTONUP:
        {
            return HandleRightButtonUp();
        }

        case rsMOUSEMOVE:
        {
            return HandleMouseMove(param);
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
            /*
             * LEFT-CTRL or RIGHT-CTRL...
             */
            Ctrl = TRUE;

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
            /*
             * LEFT-CTRL or RIGHT-CTRL...
             */
            Ctrl = FALSE;

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
