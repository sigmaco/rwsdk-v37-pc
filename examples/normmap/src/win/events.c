
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
 * events.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Starting point for any new RenderWare demo using the demo 
 * skeleton.
 *
 ****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"

#include "main.h"

static RwBool MainLightRotateActive = FALSE;
static RwBool ClumpsRotateActive = FALSE;

static RwBool ClumpsMoveKeyActive = FALSE;

static RwBool ClumpsTranslateActive = FALSE;
static RwBool ClumpsZoomActive = FALSE;

RwBool CameraZooming = FALSE;
RwReal CameraZoomDelta = 0.0f;

RwBool CameraTranslating = FALSE;
RwReal CameraTranslateDelta = 0.0f;

/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonDown(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /*
     * Left mouse button down event handling...
     */
    MainLightRotateActive = TRUE;
    ClumpsZoomActive = TRUE;

    return rsEVENTPROCESSED;
}

static RsEventStatus
HandleLeftButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /*
     * Left mouse button up event handling...
     */
    MainLightRotateActive = FALSE;
    ClumpsZoomActive = FALSE;

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
    ClumpsTranslateActive = TRUE;
    ClumpsRotateActive = TRUE;

    return rsEVENTPROCESSED;
}

static RsEventStatus
HandleRightButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /*
     * Right mouse button up event handling...
     */
    ClumpsTranslateActive = FALSE;
    ClumpsRotateActive = FALSE;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus 
HandleMouseMove(RsMouseStatus *mouseStatus)
{
    /*
     * Mouse move event handling...
     */
    if (ClumpsMoveKeyActive)
    {
        if (ClumpsTranslateActive)
        {
            ClumpsTranslate(mouseStatus->delta.x, mouseStatus->delta.y);
        }

        if (ClumpsZoomActive)
        {
            ClumpsZoom(mouseStatus->delta.y);
        }
    }
    else
    {
        if (MainLightRotateActive)
        {
            MainLightRotate(mouseStatus->pos.x, mouseStatus->pos.y);
        }

        if (ClumpsRotateActive)
        {
            ClumpsRotate(mouseStatus->delta.x, mouseStatus->delta.y);
        }
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
            CameraZoomDelta = -1.0f;

            CameraZooming = TRUE;

            return rsEVENTPROCESSED;
        }

        case rsDOWN:
        {
            CameraZoomDelta = 1.0f;

            CameraZooming = TRUE;

            return rsEVENTPROCESSED;
        }

        case rsLEFT:
        {
            /*
             * CURSOR-LEFT...
             */
            CameraTranslateDelta = -1.0f;

            CameraTranslating = TRUE;

            return rsEVENTPROCESSED;
        }

        case rsRIGHT:
        {
            /*
             * CURSOR-RIGHT...
             */
            CameraTranslateDelta = 1.0f;

            CameraTranslating = TRUE;

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
            ClumpsMoveKeyActive = TRUE;

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
    /*
     * Whatever you want or...
     */
    switch( keyStatus->keyCharCode )
    {
        case rsUP:
        {
            CameraZooming = FALSE;

            return rsEVENTPROCESSED;
        }

        case rsDOWN:
        {
            CameraZooming = FALSE;

            return rsEVENTPROCESSED;
        }

        case rsLEFT:
        {
            CameraTranslating = FALSE;

            return rsEVENTPROCESSED;
        }

        case rsRIGHT:
        {
            CameraTranslating = FALSE;

            return rsEVENTPROCESSED;
        }

        case rsLCTRL:
        case rsRCTRL:
        {
            /*
             * CONTROL KEY
             */
             ClumpsMoveKeyActive = FALSE;

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
