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
 * view.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjdaj.
 *
 * Purpose: 2d Viewer base file.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rt2d.h"
#include "rt2danim.h"

#include "skeleton.h"

#include "view.h"

#include "menu.h"

Rt2dObject *MainScene = NULL;

Rt2dObject *Scene = NULL;
Rt2dAnimProps *Props = NULL;
Rt2dAnim *Anim = NULL;

Rt2dMaestro *Maestro = NULL;
Rt2dObject *MaestroScene = NULL;
Rt2dObject *MaestroBBox = NULL;

RwBool ObjectLoaded = FALSE;
RwBool ObjectToLoad = FALSE;
RwChar OTLPath[256];



RwV2d Position = { 0.0f, 0.0f };

RwInt32 WinHeight = 480;
RwInt32 WinWidth = 640 ;

RwV2d TranslateXStep;
RwV2d TranslateYStep;
RwV2d RotateOrigin;

RwBool CmdZoomIn = FALSE;
RwBool CmdZoomOut = FALSE;
RwBool ViewChanged = FALSE;
RwBool justLoaded = FALSE;

static RwReal AnimSpeed = 1.0f;
static RwBool AnimRunning = TRUE;
static RwBool AnimLoop = FALSE;
static RwBool AnimInterpolate = FALSE;
static RwBool AnimDispMsg = TRUE;

static RwReal MaestroSpeed = 1.0f;
static RwBool MaestroRunning = TRUE;
static RwBool MaestroInterpolate = FALSE;
static RwBool MaestroShowBBox = FALSE;

#define DUMP_MAESTRO_MESSAGE
#define DUMP_MOUSE_MESSAGEx



/*
 ****************************************************************************
 */
static RwBool
AnimSpeedCB(RwBool justCheck)
{
    if(justCheck)
    {
        return(NULL != Anim);
    }

    Rt2dAnimSetDeltaTimeScale(Anim, AnimSpeed);

    return TRUE ;

}

static RwBool
AnimStopCB(RwBool justCheck)
{
    if(justCheck)
    {
        if(NULL != Anim)
        {
            return (AnimRunning);
        }
        else
        {
            return( FALSE );
        }
    }

    AnimRunning = FALSE;

    return TRUE;

}

static RwBool
AnimPlayCB(RwBool justCheck)
{
    if(justCheck)
    {
        if(NULL != Anim)
        {
            return (!AnimRunning);
        }
        else
        {
            return( FALSE );
        }
    }

    AnimRunning = TRUE;

    return TRUE;

}

static RwBool
AnimLoopCB(RwBool justCheck)
{
    if(justCheck)
    {
        return(NULL != Anim);
    }

    if(AnimLoop)
    {
        Rt2dAnimSetOnEndReachedCallBack(Rt2dAnimOnEndReachedCallBackLoop);
    }
    else
    {
        Rt2dAnimSetOnEndReachedCallBack(Rt2dAnimOnEndReachedCallBackStop);
    }

    return TRUE;
}

static RwBool
AnimDispMsgCB(RwBool justCheck)
{
    if(justCheck)
    {
        return(TRUE);
    }

    return TRUE;
}

static RwBool
AnimRestartCB(RwBool justCheck)
{
    if(justCheck)
    {
        return(NULL != Anim && AnimRunning == TRUE);
    }

    if (Anim)
    {
        Rt2dAnimReset(Anim , Props);
    }

    return TRUE;
}

static RwBool
AnimInterpolateCB(RwBool justCheck)
{
    if(justCheck)
    {
        return(NULL != Anim);
    }

    Rt2dAnimSetInterpolate(Anim, AnimInterpolate);

    return TRUE;
}

static void
AnimMenuCreate(void)
{
    static RwChar AnimInterpolateLabel[32];
    static RwChar AnimSpeedLabel[32];
    static RwChar AnimPlayLabel[32];
    static RwChar AnimStopLabel[32];
    static RwChar AnimRestartLabel[32];
    static RwChar AnimLoopLabel[32];
    static RwChar AnimDispMsgLabel[32];

    rwstrcpy(AnimInterpolateLabel,RWSTRING("Use interpolation_I"));
    rwstrcpy(AnimSpeedLabel,RWSTRING("Animation Speed"));
    rwstrcpy(AnimPlayLabel,RWSTRING("Play Animation_P"));
    rwstrcpy(AnimStopLabel,RWSTRING("Stop Animation_S"));
    rwstrcpy(AnimRestartLabel,RWSTRING("Restart Animation_A"));
    rwstrcpy(AnimLoopLabel,RWSTRING("Loop Animation_L"));
    rwstrcpy(AnimDispMsgLabel, RWSTRING("Display Message_d"));

    if( NULL != Anim )
    {
        MenuAddEntryBool(AnimInterpolateLabel,
                         &AnimInterpolate,
                         AnimInterpolateCB);

        MenuAddEntryReal(AnimSpeedLabel,
                         &AnimSpeed,
                         AnimSpeedCB,
                         0.0f, 20.0f, 0.1f);

        MenuAddEntryTrigger(AnimPlayLabel, AnimPlayCB);

        MenuAddEntryTrigger(AnimStopLabel, AnimStopCB);

        MenuAddEntryTrigger(AnimRestartLabel, AnimRestartCB);

        MenuAddEntryBool(AnimLoopLabel,
                         &AnimLoop,
                         AnimLoopCB);

        MenuAddEntryBool(AnimDispMsgLabel,
                         &AnimDispMsg,
                         AnimDispMsgCB);

    }
}

static void
AnimMenuDestroy(void)
{
    MenuRemoveEntry(&AnimSpeed);

    MenuRemoveEntry(AnimPlayCB);

    MenuRemoveEntry(AnimStopCB);

    MenuRemoveEntry(AnimRestartCB);

    MenuRemoveEntry(&AnimLoop);

    MenuRemoveEntry(&AnimInterpolate);
}


/*
 ****************************************************************************
 */
static void
MessageDumper(Rt2dMessage *message)
{
    RwChar str[256];
    switch(message->messageType)
    {
        case rt2dMESSAGETYPEPLAY:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPEPLAY"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
        }
        break;
        case rt2dMESSAGETYPESTOP:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPESTOP"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

        }
        break;
        case rt2dMESSAGETYPENEXTFRAME:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPENEXTFRAME"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            RsSprintf(str,RWSTRING("  param1 %d"),message->intParam1);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
            RsSprintf(str,RWSTRING("  param2 %d"),message->intParam2);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
        }
        break;
        case rt2dMESSAGETYPEPREVFRAME:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPEPREVFRAME"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            RsSprintf(str,RWSTRING("  param1 %d"),message->intParam1);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
            RsSprintf(str,RWSTRING("  param2 %d"),message->intParam2);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
        }
        break;
        case rt2dMESSAGETYPEGOTOFRAME:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPEGOTOFRAME"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            RsSprintf(str,RWSTRING("  frame %d"),message->intParam1);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
        }
        break;
        case rt2dMESSAGETYPEGOTOLABEL:
        {
            Rt2dStringLabel *stringLabel;
            const RwChar *labelName;

            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPEGOTOLABEL"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
            stringLabel = Rt2dMaestroGetStringLabelByIndex(Maestro, (RwInt32)message->intParam1);
            labelName = Rt2dMaestroGetStringLabelName(
									Maestro,
									stringLabel);

            RsSprintf(str,RWSTRING("  frame name %s"),labelName);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

        }
        break;
        case rt2dMESSAGETYPEGETURL:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPEGETURL"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            RsSprintf(str,RWSTRING("  param1 %d"),message->intParam1);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
            RsSprintf(str,RWSTRING("  param2 %d"),message->intParam2);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
        }
        break;
        case rt2dMESSAGETYPEDOACTION:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPEDOACTION"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            RsSprintf(str,RWSTRING("  param1 %d"),message->intParam1);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
            RsSprintf(str,RWSTRING("  param2 %d"),message->intParam2);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
        }
        break;
        case rt2dMESSAGETYPEFOREIGN:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPEFOREIGN"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            RsSprintf(str,RWSTRING("  param1 %d"),message->intParam1);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
            RsSprintf(str,RWSTRING("  param2 %d"),message->intParam2);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
        }
        break;
        case rt2dMESSAGETYPEMOUSEMOVETO:
        {
#ifdef DUMP_MOUSE_MESSAGE
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPEMOUSEMOVETO"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            RsSprintf(str,RWSTRING("  Move To %d,%d"),message->intParam1,message->intParam2);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
#endif
        }
        break;
        case rt2dMESSAGETYPEMOUSEBUTTONSTATE:
        {
#ifdef DUMP_MOUSE_MESSAGE
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPEMOUSEBUTTONSTATE"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            if( message->intParam1 == (RwInt32)TRUE)
            {
                RsSprintf(str,RWSTRING("  Button Down"));
            }
            else
            {
                RsSprintf(str,RWSTRING("  Button Up"));
            }
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),str);
#endif
        }
        break;
        case rt2dMESSAGETYPESPECIALTELLTARGET:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPESPECIALTELLTARGET"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            RsSprintf(str,RWSTRING("  param1 %d"),message->intParam1);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
            RsSprintf(str,RWSTRING("  param2 %d"),message->intParam2);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
        }
        break;
        case rt2dMESSAGETYPENULL:
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("rt2dMESSAGETYPENULL"));
        default:
        {
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           RWSTRING("Unknown message type"));

            RsSprintf(str,RWSTRING("  anim %d"),message->index);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);

            RsSprintf(str,RWSTRING("  param1 %d"),message->intParam1);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
            RsSprintf(str,RWSTRING("  param2 %d"),message->intParam2);
            RwDebugSendMessage(rwDEBUGMESSAGE, RWSTRING("Message :"),
                           str);
        }
        break;
    }

}
static Rt2dMessage *
ViewerMessageHandler(Rt2dMaestro *maestro, Rt2dMessage *message)
{
    switch(message->messageType)
    {
    case rt2dMESSAGETYPESTOP:
        MaestroRunning = FALSE;
        break;
    case rt2dMESSAGETYPEPLAY:
        MaestroRunning = TRUE;
        break;
    default:
        break;
    }

    if (FALSE != AnimDispMsg)
        MessageDumper(message);

    return Rt2dMessageHandlerDefaultCallBack(maestro,message);

}

static RwBool
MaestroStopCB(RwBool justCheck)
{
    Rt2dMessage     message;

    if(justCheck)
    {
        if(NULL != Maestro)
        {
            return (MaestroRunning);
        }
        else
        {
            return( FALSE );
        }
    }

    message.messageType = rt2dMESSAGETYPESTOP;
    message.index = 0;
    message.intParam1 = 0;
    message.intParam2 = 0;

    /* Post the message and process. */
    Rt2dMaestroPostMessages(Maestro, &message, 1);

    return TRUE;

}

static RwBool
MaestroPlayCB(RwBool justCheck)
{
    Rt2dMessage     message;

    if(justCheck)
    {
        if(NULL != Maestro)
        {
            return (!MaestroRunning);
        }
        else
        {
            return( FALSE );
        }
    }

    message.messageType = rt2dMESSAGETYPEPLAY;
    message.index = 0;
    message.intParam1 = 0;
    message.intParam2 = 0;

    /* Post the message and process. */
    Rt2dMaestroPostMessages(Maestro, &message, 1);

    return TRUE;

}

static Rt2dMaestro *
AllAnimSetInterpolateCB(Rt2dMaestro *maestro,
						Rt2dAnim *anim,
						Rt2dAnimProps *props __RWUNUSED__,
						void * pData)
{
    Rt2dAnimSetInterpolate(anim, (RwBool)pData);

    return maestro;
}

static RwBool
MaestroInterpolateCB(RwBool justCheck)
{
    if(justCheck)
    {
        return(NULL != Maestro);
    }

    Rt2dMaestroForAllAnimations(Maestro,
                         AllAnimSetInterpolateCB, (void*)MaestroInterpolate);

    return TRUE;
}

static void
MaestroMenuCreate(void)
{
    static RwChar MaestroSpeedLabel[32];
    static RwChar MaestroStopLabel[32];
    static RwChar MaestroPlayLabel[32];
    static RwChar MaestroInterpolateLabel[32];
    static RwChar MaestroShowBBoxLabel[32];
    static RwChar MaestroDispMsgLabel[32];

    rwstrcpy(MaestroInterpolateLabel, RWSTRING("Use Interpolation_I"));
    rwstrcpy(MaestroSpeedLabel, RWSTRING("Maestro Speed"));
    rwstrcpy(MaestroShowBBoxLabel, RWSTRING("Render Maestro Bounding Box_B"));
    rwstrcpy(MaestroPlayLabel, RWSTRING("Play Maestro_P"));
    rwstrcpy(MaestroStopLabel, RWSTRING("Stop Maestro_S"));
    rwstrcpy(MaestroDispMsgLabel, RWSTRING("Display Messages"));

    MenuAddEntryBool(MaestroInterpolateLabel,
                     &MaestroInterpolate,
                     MaestroInterpolateCB);

    MenuAddEntryReal(MaestroSpeedLabel,
                     &MaestroSpeed,
                     NULL,
                     0.0f, 20.0f, 0.1f);

    MenuAddEntryBool(MaestroShowBBoxLabel,
                     &MaestroShowBBox,
                     NULL);

    MenuAddEntryTrigger(MaestroPlayLabel, MaestroPlayCB);

    MenuAddEntryTrigger(MaestroStopLabel, MaestroStopCB);

    MenuAddEntryBool(MaestroDispMsgLabel,
                     &AnimDispMsg,
                     AnimDispMsgCB);

}

static void
MaestroMenuDestroy(void)
{
    MenuRemoveEntry(&MaestroInterpolate);

    MenuRemoveEntry(&MaestroShowBBox);

    MenuRemoveEntry(&MaestroSpeed);

    MenuRemoveEntry(MaestroPlayCB);

    MenuRemoveEntry(MaestroStopCB);

    MenuRemoveEntry(AnimDispMsgCB);
}


/*
 ****************************************************************************
 */
static void
ViewerClean(void)
{
    if( Maestro )
    {
        MaestroMenuDestroy();
        Rt2dMaestroDestroy(Maestro);
        Maestro = NULL;
        MaestroScene = NULL;
        Rt2dShapeDestroy(MaestroBBox);
        MaestroBBox = NULL;
        MaestroRunning = FALSE;
        MaestroShowBBox = FALSE;
        MaestroInterpolate = FALSE;
    }

    if( MainScene )
    {
        Rt2dSceneDestroy(MainScene);
        MainScene = NULL;
    }

    if( Anim )
    {
        AnimMenuDestroy();

        AnimSpeed = 1.0f;

        AnimRunning = TRUE;

        AnimLoop = FALSE;
        Rt2dAnimSetOnEndReachedCallBack(Rt2dAnimOnEndReachedCallBackStop);

        AnimInterpolate = FALSE;
        Rt2dAnimSetInterpolate(Anim, FALSE);

        Rt2dAnimDestroy(Anim, Props);

        Anim = NULL;

    }

    if( Props )
    {
        Rt2dAnimPropsDestroy(Props);
        Props = NULL;
    }

    return;
}


/*
 ****************************************************************************
 */
static Rt2dAnim *
LoadAnim(RwChar *fPath, Rt2dAnimProps *props)
{
    RwStream *stream = NULL;
    Rt2dAnim *anim = NULL;

    /*
     * Open a stream connected to the disk file...
     */
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, fPath);
    if( stream )
    {
        /*
         * Find a clump chunk in the stream...
         */
        if( !RwStreamFindChunk(stream, rwID_2DANIM,
                               (RwUInt32 *)NULL, (RwUInt32 *)NULL) )
        {
            RsErrorMessage(RWSTRING("Cannot open stream to read Anim."));
            RwStreamClose(stream, NULL);
            return (Rt2dAnim *)NULL;
        }

        /*
         * Read the clump chunk...
         */
        anim = Rt2dAnimStreamRead(stream,props);

        if(anim == NULL)
        {
            RsErrorMessage(RWSTRING("Cannot Read Anim file."));
        }

        RwStreamClose(stream, NULL);
    }

    return anim;
}


/*
 ****************************************************************************
 */
void
PreLoadObject(RwChar *filename)
{
    ObjectToLoad = TRUE;
    if( filename[0] != 0 )
    {
        rwstrcpy(OTLPath, filename);
    }
}

/*
 ****************************************************************************
 */
RwBool
LoadObject(RwChar *filename)
{
    RwStream *stream = NULL;
    Rt2dObject *object = NULL;
    Rt2dMaestro *maestro = NULL;
    RwChar *fPath;
    RwChar texturesPath[256];
    RwInt32 i;

    fPath = RsPathnameCreate(filename);
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, fPath);

    rwstrcpy(texturesPath,fPath);
    i = rwstrlen(texturesPath)-1;
    while( i >= 0 )
    {
        if( texturesPath[i] == '.' )
        {
            break;
        }

        i--;
    }

    while( i >= 0 )
    {
        if( texturesPath[i] == RsPathGetSeparator() )
        {
            texturesPath[i+1] = '\0';

            break;
        }

        i--;
    }

    /*
     * Set texture path
     */
    RwImageSetPath(texturesPath);

    RsPathnameDestroy(fPath);

	/*
     * Open a stream connected to the disk file...
     */
    if( stream )
    {
        RwChunkHeaderInfo chunkHeaderInfo;

        if (!RwStreamReadChunkHeaderInfo (  stream,
                                       &chunkHeaderInfo ))
        {
            return FALSE;
        }

        /* Type dependant streaming */
        switch (chunkHeaderInfo.type)
        {
            case rwID_2DSCENE:
            {
                object = Rt2dSceneStreamRead(stream);
                break;
            }

            case rwID_2DSHAPE:
            {
                object = Rt2dShapeStreamRead(stream);
                break;
            }

            case rwID_2DOBJECTSTRING:
            {
                object = Rt2dObjectStringStreamRead(stream);
                break;
            }
            case rwID_2DPICKREGION:
            {
                RsErrorMessage(RWSTRING("2dPickRegion not supported yet."));
                break;
            }
            case rwID_2DMAESTRO:
            {
                maestro = Rt2dMaestroStreamRead(NULL,stream);
                break;
            }
            case rwID_2DOBJECT:
            default:
            {
                RsErrorMessage(RWSTRING("Unrecognized type."));
                RwStreamClose(stream, NULL);
                return FALSE;
            }
        }

        RwStreamClose(stream, NULL);

        ViewerClean();

        /* Actually Loaded Something... */
        if( NULL != object || NULL != maestro )
        {

            /* Loaded an object*/
            if( object )
            {
                /* Create the manipulating scene or MainScene */
                MainScene = Rt2dSceneCreate();

                /* Base translation/Scale */
                /* DUMMY Should be replaced by an autocalc system */
                Rt2dObjectMTMScale(object, 0.002f, 0.002f);
                Rt2dObjectMTMTranslate(object, 100.0f, 400.0f);


                /* Add freshly loaded object to the scene */
                Rt2dSceneAddChild(MainScene, object);

                /* Unlock the scene */
                Rt2dSceneUnlock(MainScene);

                object = Rt2dSceneGetChildByIndex(MainScene,0);

                if(Rt2dObjectIsScene(object))
                {
                    RwChar animName[256];

                    rwstrcpy(animName,filename);

                    animName[rwstrlen(animName)-3] = 'a';
                    animName[rwstrlen(animName)-2] = 'n';
                    animName[rwstrlen(animName)-1] = 'm';

                    Scene = object;
                    Props =  Rt2dAnimPropsCreate(Scene);

                    Anim = LoadAnim(animName, Props);

                    if(Anim)
                    {
                        Rt2dAnimSetDeltaTimeScale(Anim, 1.0f);

                        Rt2dAnimReset(Anim , Props);

                        AnimMenuCreate();
                    }

                }
            }
            else /* it's a maestro */
            {
                Rt2dBBox *bbox;
                RwReal scaleH,scaleW,scale;
                RwReal scaledWinHeight,scaledWinWidth;
                RwV2d xStep, yStep, origin;
                Rt2dPath *path;
                Rt2dBrush *strokeBrush;
                RwRGBA Red      = {255,   0,   0, 255}; /* 0: Red */


                Rt2dDeviceGetStep(&xStep, &yStep, &origin);

                Maestro = maestro;
                MaestroScene = Rt2dMaestroGetScene(Maestro);

                bbox = Rt2dMaestroGetBBox(Maestro);

                scaledWinHeight = WinHeight*yStep.y + WinHeight*yStep.x;
                scaledWinWidth = WinWidth*xStep.x + WinWidth*xStep.y;

                scaleH = (RwReal)(scaledWinHeight)/(bbox->h);
                scaleW = (RwReal)(scaledWinWidth)/(bbox->w);

                scale = RwRealMin2(scaleH,scaleW);

                Rt2dObjectMTMScale(
                    MaestroScene, scale, scale);
                Rt2dObjectMTMTranslate(MaestroScene,
                                        (WinWidth*xStep.x/scale-bbox->w)/2.0f,
                                        bbox->h+(WinHeight*yStep.y/scale-bbox->h)/2.0f);


                Rt2dMaestroSetMessageHandler(Maestro, ViewerMessageHandler);
                MaestroMenuCreate();

                MaestroBBox = Rt2dShapeCreate();
                path = Rt2dPathCreate();
                Rt2dPathRect(path, bbox->x, bbox->y, bbox->w, bbox->h);

                Rt2dPathUnlock(path);

                strokeBrush = Rt2dBrushCreate();
                Rt2dBrushSetRGBA(strokeBrush, &Red, &Red, &Red, &Red);
                Rt2dBrushSetWidth(strokeBrush, 2.000f);

                Rt2dShapeAddNode(MaestroBBox, 0, path, strokeBrush);
                Rt2dBrushDestroy(strokeBrush);

                Rt2dObjectSetDepth(MaestroBBox,1);
                Rt2dObjectMTMScale(MaestroBBox, scale, scale);
                Rt2dObjectMTMTranslate(MaestroBBox,
                                    (WinWidth*xStep.x/scale-bbox->w)/2.0f,
                                    (WinHeight*yStep.y/scale-bbox->h)/2.0f);

                Rt2dSceneUpdateLTM(MaestroScene);
            }

            /* Reset the view */
            ViewChanged = TRUE;
            Rt2dCTMSetIdentity();

            /* Warn the App */
            ObjectLoaded = TRUE;

            /* Timer restart */
            justLoaded = TRUE;

            return TRUE;
        }
        else
        {
            RsErrorMessage(RWSTRING("2dViewer failed to load object"));
        }
    }


    return FALSE;
}


/*
 ****************************************************************************
 */
RwBool
CreateViewer(RwCamera *camera)
{
    RwChar *fPath;

    /* Open the Tool kit */
    Rt2dOpen(camera);

    /* Open the animation tool kit */
    Rt2dAnimOpen();

    /* Create the Main Global Scene */
    MainScene = Rt2dSceneCreate();

    Rt2dDeviceSetFlat(32.0f);

    /* Set the font path */
    fPath = RsPathnameCreate(RWSTRING("font/"));
    Rt2dFontSetPath(fPath);
    RsPathnameDestroy(fPath);

    return TRUE;
}


/*
 ****************************************************************************
 */
void
DestroyViewer(void)
{
    ViewerClean();

    Rt2dAnimClose();

    Rt2dClose();

    return;
}



/*
 ****************************************************************************
 */
void
InputUpdateViewer(void)
{
    if( ObjectToLoad )
    {
        ObjectToLoad = FALSE;
        LoadObject(OTLPath);
    }

    if (CmdZoomIn)
    {
        RwV2d xStep, yStep, origin;

        Rt2dDeviceGetStep(&xStep, &yStep, &origin);

        RwV2dScale(&xStep, &xStep, Position.x);
        RwV2dScale(&yStep, &yStep, WinHeight - Position.y);

        Rt2dCTMTranslate(origin.x, origin.y);
        Rt2dCTMTranslate(xStep.x, xStep.y);
        Rt2dCTMTranslate(yStep.x, yStep.y);

        Rt2dCTMScale((RwReal)(1.03), (RwReal)(1.03));

        Rt2dCTMTranslate(-yStep.x, -yStep.y);
        Rt2dCTMTranslate(-xStep.x, -xStep.y);
        Rt2dCTMTranslate(-origin.x, -origin.y);

        ViewChanged = TRUE;

    }

    if (CmdZoomOut)
    {
        RwV2d xStep, yStep, origin;

        Rt2dDeviceGetStep(&xStep, &yStep, &origin);

        RwV2dScale(&xStep, &xStep, Position.x);
        RwV2dScale(&yStep, &yStep, WinHeight - Position.y);

        Rt2dCTMTranslate(origin.x, origin.y);
        Rt2dCTMTranslate(xStep.x, xStep.y);
        Rt2dCTMTranslate(yStep.x, yStep.y);

        Rt2dCTMScale(
                        (RwReal)(1.0) / (RwReal)(1.03),
                        (RwReal)(1.0) / (RwReal)(1.03));

        Rt2dCTMTranslate(-yStep.x, -yStep.y);
        Rt2dCTMTranslate(-xStep.x, -xStep.y);
        Rt2dCTMTranslate(-origin.x, -origin.y);

        ViewChanged = TRUE;
    }

#if ((defined(SKY2_DRVMODEL_H)) || (defined(GCN_DRVMODEL_H)) || (defined(XBOX_DRVMODEL_H)))

#ifdef RWMOUSE
    UpdateMouseCursor();
#endif /* RWMOUSE */

#endif  /* ((defined(SKY2_DRVMODEL_H)) || (defined(GCN_DRVMODEL_H)) || (defined(XBOX_DRVMODEL_H))) */

}


/*
 ****************************************************************************
 */
void
UpdateViewer(RwReal deltaTime)
{
    if( Anim && AnimRunning == TRUE)
    {
        Rt2dAnimAddDeltaTime(Anim, Props, deltaTime);

        Rt2dAnimTimeUpdate(Anim, Props);
    }

/*    deltaTime = 1.0f/120.0f;*/

    if( Maestro )
    {
        Rt2dMaestroAddDeltaTime(Maestro,  deltaTime * MaestroSpeed);
        Rt2dMaestroUpdateAnimations(Maestro);

        Rt2dMaestroProcessMessages(Maestro);
    }


    return;
}

/*
 ****************************************************************************
 */
void
RenderViewer(RwCamera *camera __RWUNUSED__)
{
    if(ObjectLoaded)
    {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) FALSE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) FALSE);
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *) rwSHADEMODEGOURAUD);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *) rwFILTERLINEAR);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *) TRUE);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE );


        Rt2dCTMPush();

        /* As the CTM contains the view manipulations (zoom/translation)
         *  we need to force the object to recalculate the LTM
         */
        if( MainScene )
        {
            if( ViewChanged )
            {
                Rt2dObjectMTMChanged(MainScene);
                ViewChanged = FALSE;
            }

            Rt2dSceneRender(MainScene);
        }

        if( Maestro )
        {
            if( ViewChanged )
            {
                Rt2dObjectMTMChanged(MaestroScene);
                Rt2dObjectMTMChanged(MaestroBBox);
                ViewChanged = FALSE;
            }

            Rt2dMaestroRender(Maestro);
            if( MaestroShowBBox )
            {
                Rt2dShapeRender(MaestroBBox);
            }

        }

        Rt2dCTMPop();
    }

    return;
}
