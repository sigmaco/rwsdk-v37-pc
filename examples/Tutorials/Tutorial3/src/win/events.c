
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
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 * 
 * Purpose: RenderWare Graphics Tutorial Three (User Interaction 2)
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"

#include "utils.h"

/*
 *****************************************************************************
 */
/* HandleLeftButtonDown function */

static RsEventStatus
HandleLeftButtonDown(RsMouseStatus *mouseStatus)
{
    /* Left mouse button down event handling...  */


    if (mouseStatus->shift)
    {
        MouseMoveAction = MMPickAndRotateObject;
    }
    else
    {
        MouseMoveAction = MMPickAndTranslateObject;
    }

    return rsEVENTPROCESSED;
}

/* HandleLeftButtonUp Function */

static RsEventStatus
HandleLeftButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /* Left mouse button up event handling... */

    MouseMoveAction = MMNoOperation;

    return rsEVENTPROCESSED;
}



/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonDown(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /* Right mouse button down event handling... */

    return rsEVENTPROCESSED;
}

static RsEventStatus
HandleRightButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    /* Right mouse button up event handling... */

    return rsEVENTPROCESSED;
}

/*
 *****************************************************************************
 */
/* HandleMouseMove function */

static RsEventStatus 
HandleMouseMove(RsMouseStatus *mouseStatus)
{
    /* Mouse move event handling... */

    MousePos = mouseStatus->pos;

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
        case rsF1:
        {
            RpClump *clump = DffLoad("models/cube.dff");
            if (clump)
            {
                RwV3d v = {0.0f, 0.0f, 5.0f};

                RwFrameTranslate(RpClumpGetFrame(clump), &v, rwCOMBINEREPLACE);
                RpWorldAddClump(World, clump);
            }
                return rsEVENTPROCESSED;
        }
        case rsUP:
        {
            /*
             * CURSOR-UP...
             */
            return rsEVENTPROCESSED;
        }

        case rsDOWN:
        {
            /*
             * CURSOR-DOWN...
             */
            return rsEVENTPROCESSED;
        }

        case rsLEFT:
        {
            /*
             * CURSOR-LEFT...
             */
            return rsEVENTPROCESSED;
        }

        case rsRIGHT:
        {
            /*
             * CURSOR-RIGHT...
             */
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
static RsEventStatus 
HandleKeyUp(RsKeyStatus *keyStatus __RWUNUSED__)
{
    /*
     * Whatever you want or...
     */
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
