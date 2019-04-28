#include "button.h"
#include "skeleton.h"
#include "menu.h"


ButtonLookUpStruct *ButtonLookUp = NULL;

RwInt32 DownButtonID  = 3;
RwInt32 UpButtonID    = 2;
RwInt32 LeftButtonID  = 4;
RwInt32 RightButtonID = 1;

RwInt32 SelectButtonID = -1;
RwInt32 CancelButtonID = -1;

static RwBool displauBLUI = FALSE;
static RwBool menuAdded = FALSE;

struct ButtonByLabelPacket
{
    RwInt32 buttonID;
    RwUInt32 animButtonState;
};
typedef struct ButtonByLabelPacket ButtonByLabelPacket;


struct ButtonLink
{
    RwLLLink link;
    BtnStruct button;
};
typedef struct ButtonLink ButtonLink;



static Rt2dMaestro*
CheckAnimCB(Rt2dMaestro *maestro, Rt2dAnim *anim __RWUNUSED__,
            Rt2dAnimProps *props __RWUNUSED__, void *pData)
{
    
    Rt2dMessage message;

    // Trigger button using it's label id
    message.messageType = rt2dMESSAGETYPEBUTTONBYLABEL;
    message.index = -1;  // No animation index needed as Maestro will find the right one
    message.intParam1 = ((ButtonByLabelPacket *)pData)->buttonID; // button label index
    message.intParam2 = ((ButtonByLabelPacket *)pData)->animButtonState;
    
    //Post message
    Rt2dMaestroPostMessages(maestro, &message, 1);
   
    return maestro;
}


RwInt32
GetButtonID(RwInt32 index)
{
    if(ButtonLookUp && index>-1)
    {
        return ButtonLookUp->buttons[index].id;
    }

    return -1;
}

static void
PressButton(Rt2dMaestro *maestro,RwInt32 buttonID,RwUInt32 animButtonState)
{
    if(buttonID>-1)
    {
        ButtonByLabelPacket packet;
        packet.buttonID        = buttonID;
        packet.animButtonState = animButtonState;
        Rt2dMaestroForAllVisibleAnimations(maestro,CheckAnimCB,(void*)&packet);
    }
}

void
PressButtonUp(Rt2dMaestro *maestro,RwInt32 buttonID)
{
    PressButton(maestro, buttonID, rt2dANIMBUTTONSTATEOVERDOWNTOOVERUP);
}

void
PressButtonDown(Rt2dMaestro *maestro,RwInt32 buttonID)
{
    PressButton(maestro, buttonID, rt2dANIMBUTTONSTATEOVERUPTOOVERDOWN);
}

void
CreateLookUpForAllButtons(Rt2dMaestro *maestro)
{
    if(maestro)
    {
        RwInt32 count = 0;
        const RwChar *name;
        RwInt32 nameID = 0;
        Rt2dStringLabelType type;
        
        Rt2dStringLabel *strLab = (Rt2dStringLabel*)1;
        
        RwLinkList *listlist = RwMalloc(sizeof(RwLinkList), rwID_NAOBJECT);
        rwLinkListInitialize(listlist);
        
        while(strLab)
        {
            strLab = Rt2dMaestroGetStringLabelByIndex(maestro,nameID);
            
            if(strLab)
            {
                name = Rt2dMaestroGetStringLabelName(maestro,strLab);
                type = Rt2dStringLabelGetStringLabelType(strLab);
                
                if(type == rt2dANIMLABELTYPEBUTTON)
                {
                    ButtonLink *btnLink = RwMalloc(sizeof(ButtonLink),
                                                   rwID_NAOBJECT);
                    
                    btnLink->button.name = name;
                    btnLink->button.id = nameID;
                    
                    rwLinkListAddLLLink(listlist, &btnLink->link);
                    
                    count++;
                }
                nameID++;
            }
        }
        
        if(count)
        {
            RwLLLink *curr = rwLinkListGetFirstLLLink(listlist);
            RwLLLink *term = rwLinkListGetTerminator(listlist);
            
            RwInt32 index = 0;

            CleanButtonLookUp();
            ButtonLookUp = RwMalloc(sizeof(ButtonLookUpStruct),
                                    rwID_NAOBJECT);
            
            ButtonLookUp->buttons = RwMalloc(sizeof(BtnStruct)*count,
                                             rwID_NAOBJECT);
            ButtonLookUp->count = count;
            
            while( curr != term )
            {
                ButtonLink *listlink = rwLLLinkGetData(curr, ButtonLink, link);
                
                ButtonLookUp->buttons[index++] = listlink->button;
                
                curr = rwLLLinkGetNext(curr);
                
                rwLinkListRemoveLLLink(&listlink->link);
                
                RwFree(listlink);
            }
        }
        
        RwFree(listlist);
    }
}



void
CreateMenuForButtonLookUp(void)
{
    static RwChar displayBLUIlabel[] = RWSTRING("Display ButtonLookUp Indices");
    static RwChar upLabel[] = RWSTRING("Up button use ID from lookup index:");
    static RwChar downLabel[] = RWSTRING("Down button use ID from lookup index:");
    static RwChar leftLabel[] = RWSTRING("Left button use ID from lookup index:");
    static RwChar rightLabel[] = RWSTRING("Right button use ID from lookup index:");
    static RwChar selectLabel[] = RWSTRING("Select button use ID from lookup index:");
    static RwChar cancelLabel[] = RWSTRING("Cancel button use ID from lookup index:");

    MenuAddSeparator();
    MenuAddEntryBool(displayBLUIlabel, &displauBLUI, NULL);

    MenuAddEntryInt(upLabel, &UpButtonID, NULL, -1, -1, 1, NULL);
    MenuAddEntryInt(downLabel, &DownButtonID, NULL, -1, -1, 1, NULL);
    MenuAddEntryInt(leftLabel, &LeftButtonID, NULL, -1, -1, 1, NULL);
    MenuAddEntryInt(rightLabel, &RightButtonID, NULL, -1, -1, 1, NULL);
    MenuAddEntryInt(selectLabel, &SelectButtonID, NULL, -1, -1, 1, NULL);
    MenuAddEntryInt(cancelLabel, &CancelButtonID, NULL, -1, -1, 1, NULL);
    MenuAddSeparator();

    menuAdded = TRUE;
}

void
UpdateMenuForButtonLookUp(void)
{
    if(menuAdded)
    {
        if(ButtonLookUp)
        {
            MenuSetRangeInt(&UpButtonID,-1,ButtonLookUp->count-1,1,NULL);
            MenuSetRangeInt(&DownButtonID,-1,ButtonLookUp->count-1,1,NULL);
            MenuSetRangeInt(&LeftButtonID,-1,ButtonLookUp->count-1,1,NULL);
            MenuSetRangeInt(&RightButtonID,-1,ButtonLookUp->count-1,1,NULL);
            MenuSetRangeInt(&SelectButtonID,-1,ButtonLookUp->count-1,1,NULL);
            MenuSetRangeInt(&CancelButtonID,-1,ButtonLookUp->count-1,1,NULL);
        }
        else
        {
            MenuSetRangeInt(&UpButtonID,-1,-1,1,NULL);
            MenuSetRangeInt(&DownButtonID,-1,-1,1,NULL);
            MenuSetRangeInt(&LeftButtonID,-1,-1,1,NULL);
            MenuSetRangeInt(&RightButtonID,-1,-1,1,NULL);
            MenuSetRangeInt(&SelectButtonID,-1,-1,1,NULL);
            MenuSetRangeInt(&CancelButtonID,-1,-1,1,NULL);
        }
    }
}

void
CleanButtonLookUp(void)
{
    if(ButtonLookUp)
    {
        RwInt32 n;
        for(n=0;n<ButtonLookUp->count;n++)
        {
            ButtonLookUp->buttons[n].name = NULL;
        }
        RwFree(ButtonLookUp->buttons);
        ButtonLookUp->buttons = NULL;
        RwFree(ButtonLookUp);
    }

    ButtonLookUp = NULL;
}



static void
PrintButton(RwInt32 id, const RwChar *key,
            RwInt32 x, RwInt32 y,
            RtCharset *charset, RwChar *buffer)
{
    if(id>-1)
    {
        RsSprintf(buffer,
            RWSTRING("%s is button %s Maestro ID %i"),key,
            ButtonLookUp->buttons[id].name,
            ButtonLookUp->buttons[id].id);

        RsCharsetPrint(charset, buffer, x, y, rsPRINTPOSTOPRIGHT);
    }
}


void
DisplayButtonLookUpIndices(RtCharset *charset)
{
    if(displauBLUI && charset)
    {
        RwChar caption[128];

        if(ButtonLookUp)
        {
            PrintButton(UpButtonID,    RWSTRING("Up"),    0,2,charset,caption);
            PrintButton(DownButtonID,  RWSTRING("Down"),  0,3,charset,caption);
            PrintButton(LeftButtonID,  RWSTRING("Left"),  0,4,charset,caption);
            PrintButton(RightButtonID, RWSTRING("Right"), 0,5,charset,caption);
            PrintButton(SelectButtonID,RWSTRING("Select"),0,6,charset,caption);
            PrintButton(CancelButtonID,RWSTRING("Cancel"),0,7,charset,caption);
        }
        else
        {
            RsSprintf(caption,RWSTRING("Button LookUp not created: No buttons?"));

            RsCharsetPrint(charset, caption, 10, -1, rsPRINTPOSTOPLEFT);
        }   
    }
}
