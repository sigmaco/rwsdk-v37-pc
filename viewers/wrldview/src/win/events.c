
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
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 * Reviewed by: John Irwin (with substantial edits).
 * 
 * Purpose: RenderWare3 BSP viewer.
 *
 ****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"

#include "pvsgen.h"
#include "movement.h"
#include "main.h"
#include "world.h"
#include "render.h"

static RwBool Ctrl = FALSE;
static RwBool Translate = FALSE;

RwBool CameraPointing = FALSE;
RwBool CameraTranslating = FALSE;
RwReal CameraTranslateDelta;



/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonDown(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    CameraPointing = TRUE;

    return rsEVENTPROCESSED;
}


static RsEventStatus
HandleLeftButtonUp(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    CameraPointing = FALSE;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonDown(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    Translate = TRUE; 

    return rsEVENTPROCESSED;
}


static RsEventStatus
HandleRightButtonUp(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    Translate = FALSE; 

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus 
HandleMouseMove(RsMouseStatus *mouseStatus)
{
    if( CameraPointing )
    {
        CameraPoint(-mouseStatus->delta.y * 0.1f,
            -mouseStatus->delta.x * 0.1f);
    }

    if( Translate )
    {
        TranslateCameraZ(-mouseStatus->delta.y * TranslateScaleFactor);
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
            CameraTranslateDelta = 1.0f * TranslateScaleFactor;

            CameraTranslating = TRUE;

            return rsEVENTPROCESSED;
        }

        case rsDOWN:
        {
            CameraTranslateDelta = -1.0f * TranslateScaleFactor;

            CameraTranslating = TRUE;

            return rsEVENTPROCESSED;
        }

        case rsTAB:
        {
            if( SingleSectorOn )
            {
                if( Ctrl )
                {
                    SelectPreviousWorldSector();
                }
                else
                {
                    SelectNextWorldSector();
                }
            }

            return rsEVENTPROCESSED;
        }

        case rsRCTRL:
        case rsLCTRL:
        {
            Ctrl = TRUE;

            return rsEVENTPROCESSED;
        }

        default:
        {
            return rsEVENTNOTPROCESSED;
        }
    }
}


static RsEventStatus 
HandleKeyUp(RsKeyStatus *keyStatus)
{
    switch( keyStatus->keyCharCode )
    {
        case rsUP:
        {
            CameraTranslating = FALSE;

            return rsEVENTPROCESSED;
        }

        case rsDOWN:
        {
            CameraTranslating = FALSE;

            return rsEVENTPROCESSED;
        }

        case rsRCTRL:
        case rsLCTRL:
        {
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
