
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
 * Original author: Alexandre Hadjadj
 *
 * Purpose: 2d Viewer UI.
 *
 ****************************************************************************/

#include "rwcore.h"
   
#include "rt2d.h"   
#include "rt2danim.h"


#include "skeleton.h"
#include "menu.h"
#include "events.h"
#include "view.h"
#include "button.h"

RwV2d CursorPos = {100.0f, 100.0f};

RwBool CmdTranslate = FALSE;
RwBool CmdRotate = FALSE;
/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonDown(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    Rt2dMessage     message;
    RwV2d temp;

    /* 
     * Translation Init
     */
     CmdTranslate = TRUE; 

    Rt2dDeviceGetStep(&TranslateXStep, &TranslateYStep, &temp); 

    if (Maestro)
    {
        message.messageType = rt2dMESSAGETYPEMOUSEBUTTONSTATE;
        message.index = -1;
        message.intParam1 = (RwInt32)TRUE;

        Rt2dMaestroPostMessages(Maestro, &message, 1);
    }

    return rsEVENTPROCESSED;
}

static RsEventStatus
HandleLeftButtonUp(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    Rt2dMessage     message;
    /*
     * Left mouse button up event handling...
     */
    CmdTranslate = FALSE;

    if (Maestro)
    {
        message.messageType = rt2dMESSAGETYPEMOUSEBUTTONSTATE;
        message.index = -1;
        message.intParam1 = (RwInt32)FALSE;

        Rt2dMaestroPostMessages(Maestro, &message, 1);
    }

    return rsEVENTPROCESSED;
}



/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonDown(RsMouseStatus *mouseStatus)
{
    RwV2d xStep, yStep, origin;
    /*
     * Right mouse button down event handling...
     */
    
    CmdRotate = TRUE;

    Rt2dDeviceGetStep(&xStep, &yStep, &origin);
    RwV2dScale(&xStep, &xStep, mouseStatus->pos.x);
    RwV2dScale(&yStep, &yStep, WinHeight - mouseStatus->pos.y);
    RwV2dAdd(&RotateOrigin, &xStep, &yStep);
    RwV2dAdd(&RotateOrigin, &RotateOrigin, &origin);
    
    return rsEVENTPROCESSED;
}

static RsEventStatus
HandleRightButtonUp(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    /*
     * Right mouse button up event handling...
     */
    CmdRotate = FALSE;
    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus 
HandleMouseMove(RsMouseStatus *mouseStatus)
{
    Rt2dMessage         message;
    
    
    Position = mouseStatus->pos;
    
    /*
     * Mouse move event handling...
     */
    if(CmdTranslate)
    {
        Rt2dCTMTranslate(
                                mouseStatus->delta.x * TranslateXStep.x,  
                                mouseStatus->delta.x * TranslateXStep.y);
        Rt2dCTMTranslate(
            -mouseStatus->delta.y * TranslateYStep.x, 
            -mouseStatus->delta.y * TranslateYStep.y);

        ViewChanged = TRUE;
    }

    if(CmdRotate)
    {
        Rt2dCTMTranslate(RotateOrigin.x, RotateOrigin.y);
        Rt2dCTMRotate(mouseStatus->delta.x);
        Rt2dCTMTranslate(-RotateOrigin.x, -RotateOrigin.y);

        ViewChanged = TRUE;
    }

    message.messageType = rt2dMESSAGETYPEMOUSEMOVETO;
    message.index = 0;
    
    message.intParam1 = (RwInt32)mouseStatus->pos.x;
    message.intParam2 = (RwInt32)mouseStatus->pos.y;
  
    /* Post the message and process. */
    if (Maestro)
    {   
        Rt2dMaestroPostMessages(Maestro, &message, 1);
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

            if(MENUMODE != MenuGetStatus())
                PressButtonDown(Maestro,GetButtonID(UpButtonID));

            return rsEVENTPROCESSED;
        }

        case rsDOWN:
        {
            /*
             * CURSOR-DOWN...
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonDown(Maestro,GetButtonID(DownButtonID));
                        
            return rsEVENTPROCESSED;
        }

        case rsLEFT:
        {
            /*
             * CURSOR-LEFT...
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonDown(Maestro,GetButtonID(LeftButtonID));
            
            return rsEVENTPROCESSED;
        }

        case rsRIGHT:
        {
            /*
             * CURSOR-RIGHT...
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonDown(Maestro,GetButtonID(RightButtonID));

            return rsEVENTPROCESSED;
        }

        case rsENTER:
        {
            /*
             * ENTER
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonDown(Maestro,GetButtonID(SelectButtonID));

            return rsEVENTPROCESSED;
        }

        case rsBACKSP:
        {
            /*
             * ENTER
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonDown(Maestro,GetButtonID(CancelButtonID));

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

        case rsLSHIFT:
        case rsRSHIFT:
        {
            /*
             * LEFT-SHIFT or RIGHT-SHIFT...
             */
            CmdZoomIn = TRUE;

            return rsEVENTPROCESSED;
        }

        case rsLCTRL:
        case rsRCTRL:
        {
            /*
             * LEFT-CTRL or RIGHT-CTRL...
             */
            CmdZoomOut = TRUE;

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
            /*
             * CURSOR-UP...
             */

            if(MENUMODE != MenuGetStatus())
                PressButtonUp(Maestro,GetButtonID(UpButtonID));

            return rsEVENTPROCESSED;
        }

        case rsDOWN:
        {
            /*
             * CURSOR-DOWN...
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonUp(Maestro,GetButtonID(DownButtonID));
                        
            return rsEVENTPROCESSED;
        }

        case rsLEFT:
        {
            /*
             * CURSOR-LEFT...
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonUp(Maestro,GetButtonID(LeftButtonID));
            
            return rsEVENTPROCESSED;
        }

        case rsRIGHT:
        {
            /*
             * CURSOR-RIGHT...
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonUp(Maestro,GetButtonID(RightButtonID));

            return rsEVENTPROCESSED;
        }

        case rsENTER:
        {
            /*
             * ENTER
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonUp(Maestro,GetButtonID(SelectButtonID));

            return rsEVENTPROCESSED;
        }

        case rsBACKSP:
        {
            /*
             * ENTER
             */
            if(MENUMODE != MenuGetStatus())
                PressButtonUp(Maestro,GetButtonID(CancelButtonID));

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

        case rsLSHIFT:
        case rsRSHIFT:
        {
            /*
             * LEFT-SHIFT or RIGHT-SHIFT...
             */
            CmdZoomIn = FALSE;

            return rsEVENTPROCESSED;
        }

        case rsLCTRL:
        case rsRCTRL:
        {
            /*
             * LEFT-CTRL or RIGHT-CTRL...
             */
            CmdZoomOut = FALSE;

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
