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

#include <string.h>

#include "rwcore.h"
#include "rt2d.h"
#include "rt2danim.h"

#include "skeleton.h"

#include "view.h"

#include "menu.h"

#include "button.h"

Rt2dObject *MainScene = NULL;

Rt2dObject *Scene = NULL;

Rt2dMaestro *Maestro = NULL;
Rt2dObject *MaestroScene = NULL;

RwBool ObjectLoaded = FALSE;
RwBool ObjectToLoad = FALSE;
RwChar OTLPath[256];


RwV2d Position = { 0.0f, 0.0f };

RwInt32 WinHeight = 480;
RwInt32 WinWidth = 640 ;

RwV2d TranslateXStep;
RwV2d TranslateYStep;
RwV2d RotateOrigin;

RwBool ViewChanged = FALSE;
RwBool justLoaded = FALSE;

static RwReal MaestroSpeed = 1.0f;
static RwBool MaestroRunning = TRUE;


static RwInt32 g_onNewGameSelected;
static RwInt32 g_onOptionsSelected;
static RwInt32 g_onLoadGameSelected;
static RwInt32 g_onGameOneSelected;
static RwInt32 g_onGameTwoSelected;
static RwInt32 g_onGameThreeSelected;
static RwInt32 g_onCharacterSelectChanged;
static RwInt32 g_onLetterSliderBtnRightPushed;
static RwInt32 g_onLetterSliderBtnLeftPushed;
static RwInt32 g_onLetterSliderBtnUpPushed;
static RwInt32 g_onLetterSliderBtnDownPushed;
static RwInt32 g_onLetterSliderSelectPushed;
static RwInt32 g_onMusicVolumeSliderChanged;
static RwInt32 g_onAutosaveOnOffChanged;
static RwInt32 g_onSoundFXVolumeSliderChanged;

static const RwChar *g_MarkerText=RWSTRING("Player1 name");
static RwUInt32 g_MaxPlayerNameLen = 12;
static RwUInt32 g_CurrentEditPos=0;
static RwChar *g_PlayerName=NULL;
static Rt2dAnim *g_PlayerNameEditSlider=NULL;

Rt2dFont *MessageFont = NULL;
Rt2dBrush *MessageBrush = NULL;
#define MESSAGE_BUFFER_SIZE 512
RwChar g_MessageBuffer[MESSAGE_BUFFER_SIZE];
RwChar *g_MessageHead, *g_MessageTail;
RwReal MessageBaseSpeed = 48.0f;

static void
RegisterGetURLEvent(const RwChar *name, RwInt32 *event)
{
    Rt2dMaestroFindStringLabel(Maestro, rt2dANIMLABELTYPEURL, name, event);
}

static void
RegisterEvents()
{
    RegisterGetURLEvent(RWSTRING("onNewGameSelected"), &g_onNewGameSelected);
    RegisterGetURLEvent(RWSTRING("onOptionsSelected"), &g_onOptionsSelected);
    RegisterGetURLEvent(RWSTRING("onLoadGameSelected"), &g_onLoadGameSelected);
    RegisterGetURLEvent(RWSTRING("onGameOneSelected"), &g_onGameOneSelected);
    RegisterGetURLEvent(RWSTRING("onGameTwoSelected"), &g_onGameTwoSelected);
    RegisterGetURLEvent(RWSTRING("onGameThreeSelected"), &g_onGameThreeSelected);
    RegisterGetURLEvent(RWSTRING("onCharacterSelectChanged"), &g_onCharacterSelectChanged);
    RegisterGetURLEvent(RWSTRING("onLetterSliderBtnRightPushed"), &g_onLetterSliderBtnRightPushed);
    RegisterGetURLEvent(RWSTRING("onLetterSliderBtnLeftPushed"), &g_onLetterSliderBtnLeftPushed);
    RegisterGetURLEvent(RWSTRING("onLetterSliderBtnUpPushed"), &g_onLetterSliderBtnUpPushed);
    RegisterGetURLEvent(RWSTRING("onLetterSliderBtnDownPushed"), &g_onLetterSliderBtnDownPushed);
    RegisterGetURLEvent(RWSTRING("onLetterSliderSelectPushed"), &g_onLetterSliderSelectPushed);
    RegisterGetURLEvent(RWSTRING("onMusicVolumeSliderChanged"), &g_onMusicVolumeSliderChanged);
    RegisterGetURLEvent(RWSTRING("onAutosaveOnOffChanged"), &g_onAutosaveOnOffChanged);
    RegisterGetURLEvent(RWSTRING("onSoundFXVolumeSliderChanged"), &g_onSoundFXVolumeSliderChanged);
}

/*
 ****************************************************************************
 *     MESSAGE PRINTING
 */

static void
SetupPrinting()
{
    RwRGBA MessageColor = {240, 240, 255, 255};
    MessageFont  = Rt2dFontRead(RWSTRING("t16"));
    MessageBrush = Rt2dBrushCreate();
    g_MessageHead = g_MessageBuffer;
    g_MessageTail = g_MessageBuffer;
    g_MessageBuffer[MESSAGE_BUFFER_SIZE-1] = 0;

    Rt2dBrushSetRGBA(MessageBrush, &MessageColor, &MessageColor, &MessageColor, &MessageColor);
}

static void
ShutdownPrinting()
{
    Rt2dFontDestroy(MessageFont);
    Rt2dBrushDestroy(MessageBrush);
}

static RwInt32
QueueSize()
{
    if (g_MessageTail>=g_MessageHead)
        return g_MessageTail-g_MessageHead;
    else
        return MESSAGE_BUFFER_SIZE-1-(g_MessageHead-g_MessageTail);
}

static RwBool
QueueFull()
{
    RwInt32 size=g_MessageTail-g_MessageHead;
    return (  (2-MESSAGE_BUFFER_SIZE==size)
           || (MESSAGE_BUFFER_SIZE-2==size)
           );
}

static void
QueueChars(const RwChar *message)
{
    RwInt32 len=strlen(message);
    const RwChar *pos=message;
    const RwChar *end=&message[len];

    if (len+QueueSize()>MESSAGE_BUFFER_SIZE-2)
        return;

    while ( pos!=end && !QueueFull())
    {
        *g_MessageTail++ = *pos++;
        if (g_MessageTail > &g_MessageBuffer[MESSAGE_BUFFER_SIZE-2])
            g_MessageTail = g_MessageBuffer;
    }

    *g_MessageTail='\0';
}

static void
QueueMessage(const RwChar *message)
{
    static RwChar spacer[]="         ";
    QueueChars(spacer);
    QueueChars(message);
}

static void
RenderCurrentMessage(RwReal deltaTime)
{
    RwReal width;
    static RwChar cBuff[] = " \0";
    static RwReal pos=0.0f;
    RwReal speed;
    RwV2d v;

    if (g_MessageTail==g_MessageHead)
        return;

    Rt2dCTMPush();
    Rt2dCTMSetIdentity();

    {
        Rt2dCTMScale(4.0f/WinHeight, 4.0f / WinHeight);

        cBuff[0] = g_MessageHead[0];
        width = Rt2dFontGetStringWidth(MessageFont, &cBuff[0], Rt2dFontGetHeight(MessageFont));

        if (QueueSize() > MESSAGE_BUFFER_SIZE * 0.75f)
        {
            speed = 160.0f;
        }
        else
        {
            speed = 40.0f;
        }
        pos+=speed * deltaTime;

        if (pos>width)
        {
            pos-=width;
            g_MessageHead++;
            if (g_MessageHead>&g_MessageBuffer[MESSAGE_BUFFER_SIZE-2])
                g_MessageHead = g_MessageBuffer;
        }

        width = Rt2dFontGetStringWidth(MessageFont, g_MessageHead, Rt2dFontGetHeight(MessageFont));

        v.x=-pos;
        v.y=5.0;
        Rt2dFontShow(MessageFont, g_MessageHead, Rt2dFontGetHeight(MessageFont), &v, MessageBrush);
        if (g_MessageHead>g_MessageTail)
        {       
            v.x = width-pos;

            Rt2dFontShow(MessageFont, g_MessageBuffer, Rt2dFontGetHeight(MessageFont), &v, MessageBrush);            
        }
    }
    
    Rt2dCTMPop();
}

/*
 ****************************************************************************
 */

static Rt2dObject *
LocatePlayerNameStringInScene(Rt2dObject *child, Rt2dObject *parent __RWUNUSED__, void *data)
{
    RwChar **playerName = (RwChar **)data;

    /* early out if we've already got the name */
    if (*playerName)
    {
        return child;
    }
    
    /* Otherwise search the scene for it */
    if (Rt2dObjectIsScene(child))
    {
        /* Here we go again */
        Rt2dSceneForAllChildren(child, LocatePlayerNameStringInScene, playerName);
    }
    else if (Rt2dObjectIsObjectString(child))
    {
        /* If it matches the marker text, we have a winner */
        if (0 == strcmp(Rt2dObjectStringGetText(child), g_MarkerText))
        {
            *playerName=Rt2dObjectStringGetText(child);
        }
    }

    return child;
}

static void
SetupPlayerNameEdit()
{
    Rt2dStringLabel *label;
    RwInt32 index;
    RwUInt32 i;

    /* Find the player name */
    g_PlayerName = NULL;
    LocatePlayerNameStringInScene(Rt2dMaestroGetScene(Maestro), NULL, &g_PlayerName);

    /* Blank it out */
    for (i=0; i<g_MaxPlayerNameLen; ++i)
    {
        g_PlayerName[i]=' ';
    }
    
    /* Get the edit slider animation */
    label = Rt2dMaestroFindStringLabel(
             Maestro, rt2dANIMLABELTYPEANIM, "/LetterSlider/", &index );
    g_PlayerNameEditSlider
        = Rt2dMaestroGetAnimationsByIndex(
             Maestro,
             (RwInt32)Rt2dStringLabelGetInternalData(label));
}


static void
DoPlayerNameEdit()
{
    RwInt32 i = Rt2dAnimGetPrevFrameIndex(g_PlayerNameEditSlider);

    if (i<26)
    {   
        /* a character */
        if (g_CurrentEditPos<g_MaxPlayerNameLen-1)
        {
            g_PlayerName[g_CurrentEditPos++] = 'A'+i;
        }
    }
    else if (26==i)
    {
        /* a space */
        if (g_CurrentEditPos<g_MaxPlayerNameLen-1)
        {
            g_PlayerName[g_CurrentEditPos++] = ' ';
        }
    }
    else if (27==i)
    {
        /* a backspace */
        if (g_CurrentEditPos)
        {
            g_CurrentEditPos--;
        }
        g_PlayerName[g_CurrentEditPos] = ' ';
    }

}

static void
NewScene()
{
   /* Well... nothing to do really. */
}

static void
HandleEvent(RwInt32 index)
{
    /* Note that this could all have been done with tables/callbacks and
     * Rt2dStringLabelSetUserData, removing the need for the else-if chain.
     * This would have been faster / better, but the else-if chain is clearer
     * for the purpose of demonstration
     */
    if (index==g_onNewGameSelected)
    {
        QueueMessage("GetURL(\"onNewGameSelected\")");
        NewScene();   /* On a new scene, do any general new-screen stuff */
    }
    else if (index==g_onOptionsSelected)
    {
        QueueMessage("GetURL(\"onOptionsSelected\")");
        NewScene();   /* On a new scene, do any general new-screen stuff */

    }
    else if (index==g_onLoadGameSelected)
    {
        QueueMessage("GetURL(\"onLoadGameSelected\")");
        NewScene();   /* On a new scene, do any general new-screen stuff */

    }
    else if (index==g_onGameOneSelected)
    {
        QueueMessage("GetURL(\"onGameOneSelected\")");
        NewScene();   /* On a new scene, do any general new-screen stuff */

    }
    else if (index==g_onGameTwoSelected)
    {
        QueueMessage("GetURL(\"onGameTwoSelected\")");
        NewScene();   /* On a new scene, do any general new-screen stuff */

    }
    else if (index==g_onGameThreeSelected)
    {
        QueueMessage("GetURL(\"onGameThreeSelected\")");
        NewScene();   /* On a new scene, do any general new-screen stuff */

    }
    else if (index==g_onCharacterSelectChanged)
    {
        QueueMessage("GetURL(\"onCharacterSelectChanged\")");
    }
    else if (index==g_onLetterSliderBtnRightPushed)
    {
        QueueMessage("GetURL(\"onLetterSliderBtnRightPushed\")");
    }
    else if (index==g_onLetterSliderBtnLeftPushed)
    {
        QueueMessage("GetURL(\"onLetterSliderBtnLeftPushed\")");
    }
    else if (index==g_onLetterSliderBtnUpPushed)
    {
        QueueMessage("GetURL(\"onLetterSliderBtnUpPushed\")");
    }
    else if (index==g_onLetterSliderBtnDownPushed)
    {
        QueueMessage("GetURL(\"onLetterSliderBtnDownPushed\")");
    }
    else if (index==g_onLetterSliderSelectPushed)
    {
        QueueMessage("GetURL(\"onLetterSliderSelectPushed\")");
        DoPlayerNameEdit();
    }
    else if (index==g_onMusicVolumeSliderChanged)
    {
        QueueMessage("GetURL(\"onMusicVolumeSliderChanged\")");
    }
    else if (index==g_onAutosaveOnOffChanged)
    {
        QueueMessage("GetURL(\"onAutosaveOnOffChanged\")");
    }
    else if (index==g_onSoundFXVolumeSliderChanged)
    {
        QueueMessage("GetURL(\"onSoundFXVolumeSliderChanged\")");
    }
}


/*
 ****************************************************************************
 */

static Rt2dMessage *
ViewerMessageHandler(Rt2dMaestro *maestro, Rt2dMessage *message)
{
    switch(message->messageType)
    {
        case rt2dMESSAGETYPEGETURL:
        {
            HandleEvent(message->intParam1);
        }
        break;
    default:
        break;
    }

    return Rt2dMessageHandlerDefaultCallBack(maestro,message);
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

static void
MaestroSetInterpolateAll(RwBool maestroInterpolate)
{
    Rt2dMaestroForAllAnimations(Maestro,
                         AllAnimSetInterpolateCB, (void*)(&maestroInterpolate));
}

/*
 ****************************************************************************
 */
static void
ViewerClean(void)
{
    if( Maestro )
    {
        Rt2dMaestroDestroy(Maestro);
        Maestro = NULL;
        MaestroScene = NULL;
        MaestroRunning = FALSE;
    }

    if( MainScene )
    {
        Rt2dSceneDestroy(MainScene);
        MainScene = NULL;
    }

    return;
}


/*
 ****************************************************************************
 */
void
PreLoadObject(const RwChar *filename)
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
LoadMaestro(RwChar *filename)
{
    RwStream *stream = NULL;
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
        RwUInt32 size, version;

        if (!RwStreamFindChunk(stream, rwID_2DMAESTRO, &size, &version))

        {
            return FALSE;
        }

        maestro = Rt2dMaestroStreamRead(NULL,stream);

        RwStreamClose(stream, NULL);

        if (!maestro)
        {
            RsErrorMessage(RWSTRING("Failed to load object"));
        }

        ViewerClean();

        if( maestro )
        {

            /* Loaded a Maestro*/
            Rt2dBBox *bbox;
            RwReal scaleH,scaleW,scale;
            RwReal scaledWinHeight,scaledWinWidth;
            RwV2d xStep, yStep, origin;

            Rt2dDeviceGetStep(&xStep, &yStep, &origin);

            Maestro = maestro;

            MaestroSetInterpolateAll(TRUE);
            
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
                                    WinHeight*yStep.y/scale);


            Rt2dMaestroSetMessageHandler(Maestro, ViewerMessageHandler);

            Rt2dSceneUpdateLTM(MaestroScene);

            /* Reset the view */
            ViewChanged = TRUE;
            Rt2dCTMSetIdentity();

            /* Warn the App */
            ObjectLoaded = TRUE;

            /* Timer restart */
            justLoaded = TRUE;

            return TRUE;
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

    SetupPrinting();

    return TRUE;
}


/*
 ****************************************************************************
 */
void
DestroyViewer(void)
{
    ViewerClean();

    ShutdownPrinting();

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
        LoadMaestro(OTLPath);
        /*
         * Link the buttons up
         */
        CreateLookUpForAllButtons(Maestro);
        /*
         * Register GetURL triggers
         */
        RegisterEvents();
        /*
         * Find the player name string
         */
        SetupPlayerNameEdit();

        QueueMessage("                                      Maestro1 Example. Use directional keys "
                     "and SELECT / CANCEL equivalents to navigate. These notes are generated "
                     "in-program and aren't part of the animation file.");
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
RenderViewer(RwCamera *camera __RWUNUSED__, RwReal deltaTime)
{
    if(ObjectLoaded)
    {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *) FALSE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *) FALSE);
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *) rwSHADEMODEGOURAUD);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *) rwFILTERLINEAR);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *) TRUE);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE );

        /* Make sure rasters of any format are going to work transparency-wise */
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *) rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *) rwBLENDINVSRCALPHA);

        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *) TRUE);
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
                ViewChanged = FALSE;
            }

            Rt2dMaestroRender(Maestro);

        }

        RenderCurrentMessage(deltaTime);    

        Rt2dCTMPop();
    }

    return;
}
