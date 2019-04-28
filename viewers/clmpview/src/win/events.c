
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
 * Original author: Alexandre Hadjdaj
 * Reviewed by:
 *
 * Purpose: Event handler for clump viewer
 *
 ****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"

#include "main.h"

#include "clmpcntl.h"
#include "clmppick.h"
#include "clmpview.h"

static RwInt32 NumRotationKeys = 0;
static RwInt32 NumTranslationKeys = 0;

/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonDown(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    if( Clump )
    {
        /*
         * Left mouse button down event handling...
         */
        ClumpDirectTranslate = TRUE;

        return rsEVENTPROCESSED;
    }

    return rsEVENTNOTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /*
     * Left mouse button up event handling...
     */
    ClumpDirectTranslate = FALSE;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonDown(RsMouseStatus *mouseStatus)
{
    if( Clump )
    {
        /*
         * Left mouse button down event handling...
         */
        if( ClumpPick )
        {
            AtomicSelect((RwInt32)mouseStatus->pos.x, 
                (RwInt32)mouseStatus->pos.y);
        }
        else
        {
            ClumpDirectRotate = TRUE;
        }
    }

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonUp(RsMouseStatus * mouseStatus __RWUNUSED__)
{
    /*
     * Left mouse button up event handling...
     */
    ClumpDirectRotate = FALSE;

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
    if( ClumpDirectRotate )
    {
        ClumpControlDirectRotate(mouseStatus->delta.y * 0.5f, 
            mouseStatus->delta.x * 0.5f );
    }

    if( ClumpDirectTranslate )
    {
        ClumpControlDirectTranslateZ(-mouseStatus->delta.y * 0.05f);
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

    return rsEVENTNOTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleKeyDown(RsKeyStatus *keyStatus)
{
    switch( keyStatus->keyCharCode )
    {
        case rsTAB:
        {
            if( Clump )
            {
                SelectNextAtomic();
            }

            return rsEVENTPROCESSED;
        }

        case rsPGUP:
        {
            /*
             * PAGE-UP...
             */
            ClumpTranslate = TRUE;
            ClumpTranslateDeltaZ = -1.0f;
            NumTranslationKeys++;
            return rsEVENTPROCESSED;
        }

        case rsPGDN:
        {
            /*
             * PAGE-DOWN...
             */
            ClumpTranslate = TRUE;
            ClumpTranslateDeltaZ = 1.0f;
            NumTranslationKeys++;
            return rsEVENTPROCESSED;
        }

        case rsLEFT:
        {
            /*
             * CURSOR-LEFT...
             */
            ClumpRotate = TRUE;
            ClumpRotateDelta.y = -1.0f;
            NumRotationKeys++;
            return rsEVENTPROCESSED;
        }

        case rsRIGHT:
        {
            /*
             * CURSOR-RIGHT...
             */
            ClumpRotate = TRUE;
            ClumpRotateDelta.y = 1.0f;
            NumRotationKeys++;
            return rsEVENTPROCESSED;
        }

        case rsUP:
        {
            /*
             * CURSOR-UP...
             */
            ClumpRotate = TRUE;
            ClumpRotateDelta.x = 1.0f;
            NumRotationKeys++;
            return rsEVENTPROCESSED;
        }

        case rsDOWN:
        {
            /*
             * CURSOR-DOWN...
             */
            ClumpRotate = TRUE;
            ClumpRotateDelta.x = -1.0f;
            NumRotationKeys++;
            return rsEVENTPROCESSED;
        }

        case rsLCTRL:
        case rsRCTRL:
        {
            /*
             * CONTROL KEY
             */
             ClumpPick = TRUE;
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
HandleKeyUp(RsKeyStatus *keyStatus)
{
    /*
     * Whatever you want or...
     */
    switch( keyStatus->keyCharCode )
    {
        case rsLEFT:
        case rsRIGHT:
        case rsUP:
        case rsDOWN:
        {
            /*
             * CURSOR-RIGHT or CURSOR-LEFT...
             * CURSOR-DOWN or CURSOR-UP...
             */
            NumRotationKeys--;
            if ( NumRotationKeys <= 0 )
            {
                ClumpRotate = FALSE;
                NumRotationKeys = 0;
                ClumpRotateDelta.x = ClumpRotateDelta.y = 0.0f;
            }
            return rsEVENTPROCESSED;
        }

        case rsPGUP:
        case rsPGDN:
        {
            /*
             * PAGE-DOWN or PAGE-UP..
             */
            NumTranslationKeys--;
            if( NumTranslationKeys == 0 )
            {
                ClumpTranslate = FALSE;
                ClumpTranslateDeltaZ = 0.0f;
            }
            return rsEVENTPROCESSED;
        }

        case rsLCTRL:
        case rsRCTRL:
        {
            /*
             * CONTROL KEY
             */
             ClumpPick = FALSE;
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

    return rsEVENTNOTPROCESSED;

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
