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
 * Purpose: To illustrate the frame hierarchy of a series of atomics 
 * in a clump.
 *****************************************************************************/

#include "rwcore.h"

#include "skeleton.h"
#include "menu.h"
#include "events.h"

#include "main.h"
#include "frame.h"

static RwBool Ctrl = FALSE;
static RwBool RotateFrame = FALSE;


/*
 *****************************************************************************
 */
static RsEventStatus
HandleLeftButtonDown(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    RotateFrame = TRUE;

    return rsEVENTPROCESSED;
}


static RsEventStatus
HandleLeftButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    RotateFrame = FALSE;

    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus
HandleRightButtonDown(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    return rsEVENTPROCESSED;
}


static RsEventStatus
HandleRightButtonUp(RsMouseStatus *mouseStatus __RWUNUSED__)
{
    return rsEVENTPROCESSED;
}


/*
 *****************************************************************************
 */
static RsEventStatus 
HandleMouseMove(RsMouseStatus *mouseStatus)
{
    if( RotateFrame )
    {
        FrameRotate(mouseStatus->delta.x, -mouseStatus->delta.y);
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
static RwFrame *
FindFirstChild(RwFrame *frame, void *data)
{
    /*
     * Find the first atomic that is attached to a child frame...
     */
    RwFrameForAllObjects(frame, GetFirstAtomic, data);
    
    return NULL;
}
  
   
/*
 *****************************************************************************
 */
static RsEventStatus 
HandleKeyDown(RsKeyStatus *keyStatus)
{
    RwFrame *frame;

    switch( keyStatus->keyCharCode )
    {
        case rsTAB:
        {
            if( Ctrl )
            {
                /*
                 * Select the previous frame on the same level...
                 */
                SelectedAtomic = 
                    NextAndPreviousAtomic[GetAtomicIndex(SelectedAtomic)].previous;
            }
            else
            {
                /*
                 * Select the next frame on the same level...
                 */
                SelectedAtomic = 
                    NextAndPreviousAtomic[GetAtomicIndex(SelectedAtomic)].next;
            }

            return rsEVENTPROCESSED;
        }
        
        /*
         * Select the current frame's parent...
         */
        case rsPGUP:
        {
            frame = RwFrameGetParent(RpAtomicGetFrame(SelectedAtomic));
            if( frame )
            {
                /*
                 * Find the atomic that is attached to the parent frame...
                 */
                RwFrameForAllObjects(frame, GetFirstAtomic, 
                    (void *)&SelectedAtomic);
            }

            return rsEVENTPROCESSED;
        }
        
        /*
         * Select the current frame's first child...
         */
        case rsPGDN:
        {
            RwFrameForAllChildren(RpAtomicGetFrame(SelectedAtomic), 
                FindFirstChild, (void *)&SelectedAtomic);
        
            return rsEVENTPROCESSED;
        }

        case rsLCTRL:
        case rsRCTRL:
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


/*
 *****************************************************************************
 */
static RsEventStatus 
HandleKeyUp(RsKeyStatus *keyStatus)
{
    switch( keyStatus->keyCharCode )
    {
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
