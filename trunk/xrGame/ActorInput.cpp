#include "stdafx.h"
#include <dinput.h>
#include "Actor.h"
#include "Torch.h"
#include "../xr_3da/CameraBase.h"
#ifdef DEBUG
#include "PHDebug.h"
#endif
#include "Car.h"
#include "HudManager.h"
#include "UIGameSP.h"
#include "inventory.h"
#include "level.h"
#include "game_cl_base.h"
#include "xr_level_controller.h"
#include "UsableScriptObject.h"
#include "clsid_game.h"
#include "actorcondition.h"
#include "Actor_Flags.h"
#include "actor_input_handler.h"
#include "string_table.h"
#include "UI/UIStatic.h"
#include "CharacterPhysicsSupport.h"
#include "InventoryBox.h"
#include "player_hud.h"
#include "HudItem.h"
extern int g_bHudAdjustMode;

void CActor::IR_OnKeyboardPress(int cmd)
{
	if (g_bHudAdjustMode)
		return;

	if (Remote())		return;

//	if (conditions().IsSleeping())	return;
	if (IsTalking())	return;
	if (m_input_external_handler && !m_input_external_handler->authorized(cmd))	return;
	
	switch (cmd)
	{
	case kWPN_FIRE:
		{
			mstate_wishful &=~mcSprint;
			//-----------------------------
			if (OnServer())
			{
				NET_Packet P;
				P.w_begin(M_PLAYER_FIRE); 
				P.w_u16(ID());
				u_EventSend(P);
			}
		}break;
	default:
		{
		}break;
	}

	if (!g_Alive()) 
		return;

	if(m_holder && kUSE != cmd)
	{
		m_holder->OnKeyboardPress			(cmd);
		if(m_holder->allowWeapon() && inventory().Action(cmd, CMD_START))		return;
		return;
	}else
		if(inventory().Action(cmd, CMD_START))					return;

	switch(cmd){
	case kJUMP:		
		{
			mstate_wishful |= mcJump;
			{
//				NET_Packet	P;
//				u_EventGen(P, GE_ACTOR_JUMPING, ID());
//				u_EventSend(P);
			}
		}break;
	case kSPRINT_TOGGLE:	
		{
		if (psActorFlags.test(AF_SPRINT_TOGGLE))
		{

			if (psActorFlags.test(AF_WALK_TOGGLE)) mstate_wishful &= ~mcAccel;
			if (psActorFlags.test(AF_CROUCH_TOGGLE)) mstate_wishful &= ~mcCrouch;
			mstate_wishful ^= mcSprint;
		}
		}
		break;
	case kCROUCH:
	{
		if (psActorFlags.test(AF_CROUCH_TOGGLE))
			mstate_wishful ^= mcCrouch;
	}
	break;
	case kACCEL:
	{
		if (psActorFlags.test(AF_WALK_TOGGLE))
			mstate_wishful ^= mcAccel;
	}
	break;
	case kCAM_1:	cam_Set			(eacFirstEye);				break;
	case kCAM_2:	cam_Set			(eacLookAt);				break;
	case kCAM_3:	cam_Set			(eacFreeLook);				break;
	case kNIGHT_VISION:
		{
			const xr_vector<CAttachableItem*>& all = CAttachmentOwner::attached_objects();
			xr_vector<CAttachableItem*>::const_iterator it = all.begin();
			xr_vector<CAttachableItem*>::const_iterator it_e = all.end();
			for(;it!=it_e;++it){
				CTorch* torch = smart_cast<CTorch*>(*it);
				if (torch){		
					torch->SwitchNightVision();
					break;
				}
			}
		}break;
	case kTORCH:{ 
		const xr_vector<CAttachableItem*>& all = CAttachmentOwner::attached_objects();
		xr_vector<CAttachableItem*>::const_iterator it = all.begin();
		xr_vector<CAttachableItem*>::const_iterator it_e = all.end();
		for(;it!=it_e;++it){
				CTorch* torch = smart_cast<CTorch*>(*it);
				if (torch){		
					torch->Switch();
					break;
				}
		}
		}break;
	case kWPN_1:	
	case kWPN_2:	
	case kWPN_3:	
	case kWPN_4:	
	case kWPN_5:	
	case kWPN_6:	
	case kWPN_RELOAD:
		//Weapons->ActivateWeaponID	(cmd-kWPN_1);			
		break;
	case kUSE:
		ActorUse();
		break;
	case kDROP:
		b_DropActivated			= TRUE;
		f_DropPower				= 0;
		break;
	case kNEXT_SLOT:
		{
			OnNextWeaponSlot();
		}break;
	case kPREV_SLOT:
		{
			OnPrevWeaponSlot();
		}break;

	case kUSE_BANDAGE:
	case kUSE_MEDKIT:
		{
			if(IsGameTypeSingle())
			{
				PIItem itm = inventory().item((cmd==kUSE_BANDAGE)?  CLSID_IITEM_BANDAGE:CLSID_IITEM_MEDKIT );	
				if(itm)
				{
					inventory().Eat				(itm);
					SDrawStaticStruct* _s		= CurrentGameUI()->AddCustomStatic("item_used", true);
					_s->m_endTime				= Device.fTimeGlobal+3.0f;// 3sec
					string1024					str;
					strconcat					(sizeof(str),str,*CStringTable().translate("st_item_used"),": ", itm->Name());
					_s->wnd()->SetText			(str);
				}
			}
		}break;
	}
}
void CActor::IR_OnMouseWheel(int direction)
{
	if (g_bHudAdjustMode)
	{
		g_player_hud->tune(Ivector().set(0, 0, direction));
		return;
	}

	if(inventory().Action( (direction>0)? kWPN_ZOOM_DEC:kWPN_ZOOM_INC , CMD_START)) return;


	if (direction>0)
		OnNextWeaponSlot				();
	else
		OnPrevWeaponSlot				();
}
void CActor::IR_OnKeyboardRelease(int cmd)
{
	if (g_bHudAdjustMode)
		return;

	if (Remote())		return;

//	if (conditions().IsSleeping())	return;
	if (m_input_external_handler && !m_input_external_handler->authorized(cmd))	return;

	if (g_Alive())	
	{
		if (cmd == kUSE) 
			PickupModeOff();

		if(m_holder)
		{
			m_holder->OnKeyboardRelease(cmd);
			
			if(m_holder->allowWeapon() && inventory().Action(cmd, CMD_STOP))		return;
			return;
		}else
			if(inventory().Action(cmd, CMD_STOP))		return;



		switch(cmd)
		{
		case kJUMP:		mstate_wishful &=~mcJump;		break;
		case kDROP:		if(GAME_PHASE_INPROGRESS == Game().Phase()) g_PerformDrop();				break;
		}
	}
}

void CActor::IR_OnKeyboardHold(int cmd)
{
	if (g_bHudAdjustMode)
		return;

	if (Remote() || !g_Alive())					return;
//	if (conditions().IsSleeping())				return;
	if (m_input_external_handler && !m_input_external_handler->authorized(cmd))	return;
	if (IsTalking())							return;

	if(m_holder)
	{
		m_holder->OnKeyboardHold(cmd);
		return;
	}

	float LookFactor = GetLookFactor();
	switch(cmd)
	{
	case kUP:
	case kDOWN: 
		cam_Active()->Move( (cmd==kUP) ? kDOWN : kUP, 0, LookFactor);									break;
	case kCAM_ZOOM_IN: 
	case kCAM_ZOOM_OUT: 
		cam_Active()->Move(cmd);												break;
	case kLEFT:
	case kRIGHT:
		if (eacFreeLook!=cam_active) cam_Active()->Move(cmd, 0, LookFactor);	break;

	case kL_STRAFE:	mstate_wishful |= mcLStrafe;								break;
	case kR_STRAFE:	mstate_wishful |= mcRStrafe;								break;
	case kL_LOOKOUT:mstate_wishful |= mcLLookout;								break;
	case kR_LOOKOUT:mstate_wishful |= mcRLookout;								break;
	case kFWD:		mstate_wishful |= mcFwd;									break;
	case kBACK:		mstate_wishful |= mcBack;									break;
	case kCROUCH:
	{
		if (!psActorFlags.test(AF_CROUCH_TOGGLE))
			mstate_wishful |= mcCrouch;
	}
	break;
	case kACCEL:
	{
		if (!psActorFlags.test(AF_WALK_TOGGLE))
			mstate_wishful |= mcAccel;
	}
	break;
	case kSPRINT_TOGGLE:
	{
		if (!psActorFlags.test(AF_SPRINT_TOGGLE))
		{
			if (psActorFlags.test(AF_WALK_TOGGLE)) mstate_wishful &= ~mcAccel;
			if (psActorFlags.test(AF_CROUCH_TOGGLE)) mstate_wishful &= ~mcCrouch;
			mstate_wishful |= mcSprint;
		}
	}
	break;
	}
}

void CActor::IR_OnMouseMove(int dx, int dy)
{
	if (g_bHudAdjustMode)
	{
		g_player_hud->tune(Ivector().set(dx, dy, 0));
		return;
	}

	PIItem iitem = inventory().ActiveItem();
	if (iitem && iitem->cast_hud_item())
		iitem->cast_hud_item()->ResetSubStateTime();

	if (Remote())		return;
//	if (conditions().IsSleeping())	return;

	if(m_holder) 
	{
		m_holder->OnMouseMove(dx,dy);
		return;
	}

	float LookFactor = GetLookFactor();

	CCameraBase* C	= cameras	[cam_active];
	float scale		= (C->f_fov/g_fov)*psMouseSens * psMouseSensScale/50.f  / LookFactor;
	if (dx){
		float d = float(dx)*scale;
		cam_Active()->Move((d<0)?kLEFT:kRIGHT, _abs(d));
	}
	if (dy){
		float d = ((psMouseInvert.test(1))?-1:1)*float(dy)*scale*3.f/4.f;
		cam_Active()->Move((d>0)?kUP:kDOWN, _abs(d));
	}
}
#include "HudItem.h"
bool CActor::use_Holder				(CHolderCustom* holder)
{

	if(m_holder){
		bool b = false;
		CGameObject* holderGO			= smart_cast<CGameObject*>(m_holder);
		
		if(smart_cast<CCar*>(holderGO))
			b = use_Vehicle(0);
		else
			if (holderGO->CLS_ID==CLSID_OBJECT_W_MOUNTED ||
				holderGO->CLS_ID==CLSID_OBJECT_W_STATMGUN)
				b = use_MountedWeapon(0);

		if(inventory().ActiveItem()){
			CHudItem* hi = smart_cast<CHudItem*>(inventory().ActiveItem());
			if(hi) hi->OnAnimationEnd(hi->GetState());
		}

		return b;
	}else{
		bool b = false;
		CGameObject* holderGO			= smart_cast<CGameObject*>(holder);
		if(smart_cast<CCar*>(holder))
			b = use_Vehicle(holder);

		if (holderGO->CLS_ID==CLSID_OBJECT_W_MOUNTED ||
			holderGO->CLS_ID==CLSID_OBJECT_W_STATMGUN)
			b = use_MountedWeapon(holder);
		
		if(b){//used succesfully
			// switch off torch...
			CAttachableItem *I = CAttachmentOwner::attachedItem(CLSID_DEVICE_TORCH);
			if (I){
				CTorch* torch = smart_cast<CTorch*>(I);
				if (torch) torch->Switch(false);
			}
		}

		if(inventory().ActiveItem()){
			CHudItem* hi = smart_cast<CHudItem*>(inventory().ActiveItem());
			if(hi) hi->OnAnimationEnd(hi->GetState());
		}

		return b;
	}
}

void CActor::ActorUse()
{
	//mstate_real = 0;
	PickupModeOn();

		
	if (m_holder)
	{
		CGameObject*	GO			= smart_cast<CGameObject*>(m_holder);
		NET_Packet		P;
		CGameObject::u_EventGen		(P, GEG_PLAYER_DETACH_HOLDER, ID());
		P.w_u32						(GO->ID());
		CGameObject::u_EventSend	(P);
		return;
	}
				
	if(character_physics_support()->movement()->PHCapture())
		character_physics_support()->movement()->PHReleaseObject();

	

	if(m_pUsableObject)m_pUsableObject->use(this);
	
	if(m_pInvBoxWeLookingAt && m_pInvBoxWeLookingAt->nonscript_usable())
	{
		if (CurrentGameUI())
		{
			CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(CurrentGameUI());
			pGameSP->StartCarBody(this, m_pInvBoxWeLookingAt);
		}
		return;
	}

	if(!m_pUsableObject||m_pUsableObject->nonscript_usable())
	{
		if (m_pPersonWeLookingAt)
		{
			CEntityAlive* pEntityAliveWeLookingAt =
				smart_cast<CEntityAlive*>(m_pPersonWeLookingAt);

			VERIFY(pEntityAliveWeLookingAt);

			if (pEntityAliveWeLookingAt->g_Alive())
			{
				TryToTalk();
			}
			//обыск трупа
			else if (!Level().IR_GetKeyState(DIK_LSHIFT) && CurrentGameUI())
			{
				//только если находимся в режиме single
				CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(CurrentGameUI());
				pGameSP->StartCarBody(this, m_pPersonWeLookingAt);
			}
		}

		collide::rq_result& RQ = HUD().GetCurrentRayQuery();
		CPhysicsShellHolder* object = smart_cast<CPhysicsShellHolder*>(RQ.O);
		u16 element = BI_NONE;
		if(object) 
			element = (u16)RQ.element;

		if(object && Level().IR_GetKeyState(DIK_LSHIFT))
		{
			bool b_allow = !!pSettings->line_exist("ph_capture_visuals",object->cNameVisual());
			if(b_allow && !character_physics_support()->movement()->PHCapture())
			{
				character_physics_support()->movement()->PHCaptureObject(object,element);

			}

		}
		else
		{
			if (object && smart_cast<CHolderCustom*>(object))
			{
					NET_Packet		P;
					CGameObject::u_EventGen		(P, GEG_PLAYER_ATTACH_HOLDER, ID());
					P.w_u32						(object->ID());
					CGameObject::u_EventSend	(P);
					return;
			}

		}
	}


}
BOOL CActor::HUDview				( )const 
{ 
	return IsFocused()&&(cam_active==eacFirstEye)&&
		((!m_holder) || (m_holder && m_holder->allowWeapon() && m_holder->HUDView() ) ); 
}

//void CActor::IR_OnMousePress(int btn)
static	u32 SlotsToCheck [] = {
		KNIFE_SLOT		,		// 0
		PISTOL_SLOT		,		// 1
		RIFLE_SLOT		,		// 2
		GRENADE_SLOT	,		// 3
		APPARATUS_SLOT	,		// 4
		ARTEFACT_SLOT	,		// 10
};

void	CActor::OnNextWeaponSlot()
{
	u32 ActiveSlot = inventory().GetActiveSlot();
	if (ActiveSlot == NO_ACTIVE_SLOT) 
		ActiveSlot = inventory().GetPrevActiveSlot();

	if (ActiveSlot == NO_ACTIVE_SLOT) 
		ActiveSlot = KNIFE_SLOT;
	
	u32 NumSlotsToCheck = sizeof(SlotsToCheck)/sizeof(u32);	
	u32 CurSlot = 0;
	for (; CurSlot<NumSlotsToCheck; CurSlot++)
	{
		if (SlotsToCheck[CurSlot] == ActiveSlot) break;
	};
	if (CurSlot >= NumSlotsToCheck) return;
	for (u32 i=CurSlot+1; i<NumSlotsToCheck; i++)
	{
		if (inventory().ItemFromSlot(SlotsToCheck[i]))
		{
			if (SlotsToCheck[i] == ARTEFACT_SLOT) 
			{
				IR_OnKeyboardPress(kARTEFACT);
			}
			else
				IR_OnKeyboardPress(kWPN_1+(i-KNIFE_SLOT));
			return;
		}
	}
};

void	CActor::OnPrevWeaponSlot()
{
	u32 ActiveSlot = inventory().GetActiveSlot();
	if (ActiveSlot == NO_ACTIVE_SLOT) 
		ActiveSlot = inventory().GetPrevActiveSlot();

	if (ActiveSlot == NO_ACTIVE_SLOT) 
		ActiveSlot = KNIFE_SLOT;

	u32 NumSlotsToCheck = sizeof(SlotsToCheck)/sizeof(u32);	
	u32 CurSlot = 0;
	for (; CurSlot<NumSlotsToCheck; CurSlot++)
	{
		if (SlotsToCheck[CurSlot] == ActiveSlot) break;
	};
	if (CurSlot >= NumSlotsToCheck) return;
	for (s32 i=s32(CurSlot-1); i>=0; i--)
	{
		if (inventory().ItemFromSlot(SlotsToCheck[i]))
		{
			if (SlotsToCheck[i] == ARTEFACT_SLOT) 
			{
				IR_OnKeyboardPress(kARTEFACT);
			}
			else
				IR_OnKeyboardPress(kWPN_1+(i-KNIFE_SLOT));
			return;
		}
	}
};

float	CActor::GetLookFactor()
{
	if (m_input_external_handler) 
		return m_input_external_handler->mouse_scale_factor();

	
	float factor	= 1.f;

	PIItem pItem	= inventory().ActiveItem();

	if (pItem)
		factor *= pItem->GetControlInertionFactor();

	VERIFY(!fis_zero(factor));

	return factor;
}

void CActor::set_input_external_handler(CActorInputHandler *handler) 
{
	// clear state
	if (handler) 
		mstate_wishful			= 0;

	// release fire button
	if (handler)
		IR_OnKeyboardRelease	(kWPN_FIRE);

	// set handler
	m_input_external_handler	= handler;
}



