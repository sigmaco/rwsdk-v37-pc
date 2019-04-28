#include "button.h"
#include "skeleton.h"
#include "menu.h"


typedef struct ButtonByLabelPacket ButtonByLabelPacket;
struct ButtonByLabelPacket
{
    RwInt32 buttonID;
    RwUInt32 animButtonState;
};

BtnStruct BtnList[MAX_BUTTONS];


static Rt2dMaestro* CheckAnimCB(Rt2dMaestro *maestro, Rt2dAnim *anim __RWUNUSED__, Rt2dAnimProps *props __RWUNUSED__, void *pData)
{
    
    Rt2dMessage message;

    /* Trigger button using it's label id */
    message.messageType = rt2dMESSAGETYPEBUTTONBYLABEL;
    message.index = -1;  /* No animation index needed as Maestro will find the right one */
    message.intParam1 = ((ButtonByLabelPacket *)pData)->buttonID; /* button label index */
    message.intParam2 = ((ButtonByLabelPacket *)pData)->animButtonState;
    
    /* Post message */
    Rt2dMaestroPostMessages(maestro, &message, 1);
   
    return maestro;
}


void PressButton(Rt2dMaestro *maestro,RwInt32 buttonNo, RwUInt32 animButtonState)
{
    ButtonByLabelPacket packet;
    if (maestro)
    {
        packet.buttonID        = BtnList[buttonNo].id;
        packet.animButtonState = animButtonState;
        Rt2dMaestroForAllVisibleAnimations(maestro,CheckAnimCB,(void*)&packet);
    }
}

static void
LookupButton(Rt2dMaestro *maestro, const RwChar *name, BtnStruct *button)
{
    RwInt32 index;
    Rt2dMaestroFindStringLabel(maestro, rt2dANIMLABELTYPEBUTTON, name, &index );

    button->id   = index;
    button->name = name;
}

void
CreateLookUpForAllButtons(Rt2dMaestro *maestro)
{
    LookupButton(maestro, RWSTRING("btnUp"),     &BtnList[UpButton]);
    LookupButton(maestro, RWSTRING("btnDown"),   &BtnList[DownButton]);
    LookupButton(maestro, RWSTRING("btnLeft"),   &BtnList[LeftButton]);
    LookupButton(maestro, RWSTRING("btnRight"),  &BtnList[RightButton]);
    LookupButton(maestro, RWSTRING("btnSelect"), &BtnList[SelectButton]);
    LookupButton(maestro, RWSTRING("btnCancel"), &BtnList[CancelButton]);
}
