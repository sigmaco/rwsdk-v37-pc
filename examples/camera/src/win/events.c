
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
#include "viewer.h"
#include "camexamp.h"

static RwBool RotateClump = FALSE;
static RwBool TranslateClump = FALSE;

static RwBool RotateCamera = FALSE;
static RwBool TranslateCamera = FALSE;

static RwBool ViewXWindow = FALSE;
static RwBool ViewYWindow = FALSE;

static RwBool ViewXOffset = FALSE;
static RwBool ViewYOffset = FALSE;

static RwBool Ctrl;
static RwBool Alt;
static RwBool Shift;



/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonDown(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    if( Ctrl )
    {
        RotateCamera = TRUE;
    }
    else if( Shift )
    {
        ViewXWindow = TRUE;
    }
    else if( Alt)
    {
        ViewXOffset = TRUE;
    }
    else
    {
        RotateClump = TRUE;
    }

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    RotateCamera = FALSE;
    ViewXWindow = FALSE;
    ViewXOffset = FALSE;
    RotateClump = FALSE;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonDown(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    if( Ctrl )
    {
        TranslateCamera = TRUE;
    }
    else if( Shift )
    {
        ViewYWindow = TRUE;
    }
    else if( Alt )
    {
        ViewYOffset = TRUE;
    }
    else
    {
        TranslateClump = TRUE;
    }

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    TranslateCamera = FALSE;
    ViewYWindow = FALSE;
    ViewYOffset = FALSE;
    TranslateClump = FALSE;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus 
HandleMouseMove(RsMouseStatus *mouseStatus)
{
    if( RotateClump )
    {
        /*
         * Rotate the clump by the given delta angles...
         */
        RwReal deltaX;
        RwReal deltaY;

        deltaX = mouseStatus->delta.x;
        deltaY = -mouseStatus->delta.y;

        ClumpRotate(Clump, GetMainCamera(), deltaX, deltaY);
    }

    if( TranslateClump )
    {
        /*
         * Translate the clump by the given delta...
         */
        RwReal deltaX;
        RwReal deltaZ;

        deltaX = -mouseStatus->delta.x * 0.01f;
        deltaZ = -mouseStatus->delta.y * 0.1f;

        ClumpTranslate(Clump, GetMainCamera(), deltaX, deltaZ);
    }

    if( RotateCamera )
    {
        /*
         * Rotate the camera by the given delta angles...
         */
        RwReal deltaX;
        RwReal deltaY;

        deltaX = -mouseStatus->delta.x * 0.1f;
        deltaY = mouseStatus->delta.y * 0.1f;

        ViewerRotate(GetSubCamera(), deltaX, deltaY);
    }

    if( TranslateCamera )
    {
        /*
         * Translate the camera by the given delta...
         */
        RwReal deltaX;
        RwReal deltaY;

        deltaX = -mouseStatus->delta.x * 0.01f;
        deltaY = -mouseStatus->delta.y * 0.01f;
        
        ViewerTranslate(GetSubCamera(), deltaX, deltaY);
    }

    if( ViewXWindow )
    {
        /*
         * Change the view-window horizontally...
         */
        RwReal delta;

        delta = -mouseStatus->delta.y * 0.01f;
        
        ChangeViewWindow(delta, 0.0f);
    }


    if( ViewYWindow )
    {
        /*
         * Change the view-window vertically...
         */
        RwReal delta;

        delta = -mouseStatus->delta.y * 0.01f;
        
        ChangeViewWindow(0.0f, delta);
    }

    if( ViewXOffset )
    {
        /*
         * Change the camera X-offset...
         */
        RwReal delta;

        delta = -mouseStatus->delta.y * 0.01f;
        
        ChangeViewOffset(delta, 0.0f);
    }

    if( ViewYOffset )
    {
        /*
         * Change the camera Y-offset...
         */
        RwReal delta;

        delta = -mouseStatus->delta.y * 0.01f;
        
        ChangeViewOffset(0.0f, delta);
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
