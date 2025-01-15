#include "stdafx.h"
#include <dinput.h>
#include "HUDmanager.h"
#include "../xr_3da/xr_ioconsole.h"
#include "level.h"
#include "xr_level_controller.h"
#include "game_cl_base.h"

#include "ui/UIDialogWnd.h"
#include "UIGameCustom.h"
#include "../xr_3da/xr_input.h"
#include "saved_game_wrapper.h"
#include "Include/xrRender/DebugRender.h"

#ifdef DEBUG
#include "Actor.h"
#include "Inventory.h"
#include "HudItem.h"
#include "clsid_game.h"
#endif

#ifdef DEBUG
	extern void try_change_current_entity();
	extern void restore_actor();
#endif

bool g_bDisableAllInput = false;
extern	float	g_fTimeFactor;

#define CURRENT_ENTITY()	(game?((GameID() == GAME_SINGLE) ? CurrentEntity() : CurrentControlEntity()):NULL)

void CLevel::IR_OnMouseWheel(int direction)
{
	if (g_bDisableAllInput) return;

	if (CurrentGameUI()->IR_UIOnMouseWheel(direction)) return;
	if (Device.Paused()) return;

	if (game && Game().IR_OnMouseWheel(direction)) return;

	if (CURRENT_ENTITY()) {
		IInputReceiver* IR = smart_cast<IInputReceiver*>	(smart_cast<CGameObject*>(CURRENT_ENTITY()));
		if (IR)				IR->IR_OnMouseWheel(direction);
	}
}

static int mouse_button_2_key []	=	{MOUSE_1,MOUSE_2,MOUSE_3};

void CLevel::IR_OnMousePress(int btn)
{	IR_OnKeyboardPress(mouse_button_2_key[btn]);}
void CLevel::IR_OnMouseRelease(int btn)
{	IR_OnKeyboardRelease(mouse_button_2_key[btn]);}
void CLevel::IR_OnMouseHold(int btn)
{	IR_OnKeyboardHold(mouse_button_2_key[btn]);}

void CLevel::IR_OnMouseMove(int dx, int dy)
{
	if (g_bDisableAllInput)						return;
	if (CurrentGameUI()->IR_UIOnMouseMove(dx, dy))		return;
	if (Device.Paused())							return;
	if (CURRENT_ENTITY()) {
		IInputReceiver* IR = smart_cast<IInputReceiver*>	(smart_cast<CGameObject*>(CURRENT_ENTITY()));
		if (IR)				IR->IR_OnMouseMove(dx, dy);

	}
}

class		vtune_		{
	BOOL	enabled_	;
public:
			vtune_	()		{
		enabled_	= FALSE;
	}
	void	enable	()	{ if (!enabled_)	{ 
		Engine.External.tune_resume	();	enabled_=TRUE;	
		Msg	("vtune : enabled");
	}}
	void	disable	()	{ if (enabled_)		{ 
		Engine.External.tune_pause	();	enabled_=FALSE; 
		Msg	("vtune : disabled");
	}}
}	vtune	;

// ��������� ������� ������
extern bool g_block_pause;

void CLevel::IR_OnKeyboardPress(int key)
{
	bool b_ui_exist = CurrentGameUI();

	//.	if (DIK_F10 == key)		vtune.enable();
	//.	if (DIK_F11 == key)		vtune.disable();

	EGameActions _curr = get_binded_action(key);
	switch (_curr)
	{

	case kSCREENSHOT:
		Render->Screenshot();
		return;
		break;

	case kCONSOLE:
		Console->Show();
		return;
		break;

	case kQUIT: {
		if (b_ui_exist && CurrentGameUI()->TopInputReceiver()) {
			if (CurrentGameUI()->IR_UIOnKeyboardPress(key))	return;//special case for mp and main_menu
			CurrentGameUI()->TopInputReceiver()->HideDialog();
		}
		else
			Console->Execute("main_menu");
		return;
	}break;

	case kPAUSE:
		if (!g_block_pause)
		{
			if (IsGameTypeSingle())
			{
				Device.Pause(!Device.Paused(), TRUE, TRUE, "li_pause_key");
			}
		}
		return;
		break;

	};

	if (g_bDisableAllInput)	return;
	if (!b_ui_exist)			return;

	if (b_ui_exist && CurrentGameUI()->IR_UIOnKeyboardPress(key)) return;

	if (Device.Paused())		return;

	if (game && Game().IR_OnKeyboardPress(key)) return;

	if (_curr == kQUICK_SAVE && IsGameTypeSingle())
	{
		Console->Execute("save");
		return;
	}
	if (_curr == kQUICK_LOAD && IsGameTypeSingle())
	{
#ifdef DEBUG
		FS.get_path("$game_config$")->m_Flags.set(FS_Path::flNeedRescan, TRUE);
		FS.get_path("$game_scripts$")->m_Flags.set(FS_Path::flNeedRescan, TRUE);
		FS.rescan_pathes();
#endif // DEBUG
		string_path					saved_game, command;
		strconcat(sizeof(saved_game), saved_game, Core.UserName, "_", "quicksave");
		if (!CSavedGameWrapper::valid_saved_game(saved_game))
			return;

		strconcat(sizeof(command), command, "load ", saved_game);
		Console->Execute(command);
		return;
	}

#if defined(DEBUG) || defined(OPTICK_ENABLE)
	switch (key) {
	case DIK_NUMPAD0:
	{
		Console->Hide();
		Console->Execute("optick_capture");
	}
	break;
#ifdef DEBUG
	case DIK_NUMPAD5:
	{
		Console->Hide();
		Console->Execute("demo_record 1");
	}
	break;
	case DIK_RETURN:
		bDebug = !bDebug;
		return;

	case DIK_BACK:
		if (GameID() == GAME_SINGLE)
			DRender->NextSceneMode();
		//HW.Caps.SceneMode			= (HW.Caps.SceneMode+1)%3;
		return;

	case DIK_F4: {
		if (pInput->iGetAsyncKeyState(DIK_LALT))
			break;

		if (pInput->iGetAsyncKeyState(DIK_RALT))
			break;

		bool bOk = false;
		u32 i = 0, j, n = Objects.o_count();
		if (pCurrentEntity)
			for (; i < n; ++i)
				if (Objects.o_get_by_iterator(i) == pCurrentEntity)
					break;
		if (i < n) {
			j = i;
			bOk = false;
			for (++i; i < n; ++i) {
				CEntityAlive* tpEntityAlive = smart_cast<CEntityAlive*>(Objects.o_get_by_iterator(i));
				if (tpEntityAlive) {
					bOk = true;
					break;
				}
			}
			if (!bOk)
				for (i = 0; i < j; ++i) {
					CEntityAlive* tpEntityAlive = smart_cast<CEntityAlive*>(Objects.o_get_by_iterator(i));
					if (tpEntityAlive) {
						bOk = true;
						break;
					}
				}
			if (bOk) {
				CObject* tpObject = CurrentEntity();
				CObject* __I = Objects.o_get_by_iterator(i);
				CObject** I = &__I;

				SetEntity(*I);
				if (tpObject != *I)
				{
					CActor* pActor = smart_cast<CActor*> (tpObject);
					if (pActor)
						pActor->inventory().Items_SetCurrentEntityHud(false);
				}
				if (tpObject)
				{
					Engine.Sheduler.Unregister(tpObject);
					Engine.Sheduler.Register(tpObject, TRUE);
				};
				Engine.Sheduler.Unregister(*I);
				Engine.Sheduler.Register(*I, TRUE);

				CActor* pActor = smart_cast<CActor*> (*I);
				if (pActor)
				{
					pActor->inventory().Items_SetCurrentEntityHud(true);

					CHudItem* pHudItem = smart_cast<CHudItem*>(pActor->inventory().ActiveItem());
					if (pHudItem)
					{
						pHudItem->OnStateSwitch(pHudItem->GetState());
					}
				}
			}
		}
		return;
	}
	case MOUSE_1: {
		if (GameID() != GAME_SINGLE)
			break;
		if (pInput->iGetAsyncKeyState(DIK_LALT)) {
			if (CurrentEntity()->CLS_ID == CLSID_OBJECT_ACTOR)
				try_change_current_entity();
			else
				restore_actor();
			return;
		}
		break;
	}
#endif
	}
#endif // MASTER_GOLD

	if (bindConsoleCmds.execute(key))
		return;

	if (b_ui_exist && CurrentGameUI()->TopInputReceiver())return;
	if (CURRENT_ENTITY()) {
		IInputReceiver* IR = smart_cast<IInputReceiver*>	(smart_cast<CGameObject*>(CURRENT_ENTITY()));
		if (IR)				IR->IR_OnKeyboardPress(get_binded_action(key));
	}
}

void CLevel::IR_OnKeyboardRelease(int key)
{
	if (g_bDisableAllInput) return;
	if (CurrentGameUI() && CurrentGameUI()->IR_UIOnKeyboardRelease(key)) return;
	if (Device.Paused()) return;
	if (game && Game().OnKeyboardRelease(get_binded_action(key))) return;

	if (CURRENT_ENTITY()) {
		IInputReceiver* IR = smart_cast<IInputReceiver*>	(smart_cast<CGameObject*>(CURRENT_ENTITY()));
		if (IR)				IR->IR_OnKeyboardRelease(get_binded_action(key));
	}
}

void CLevel::IR_OnKeyboardHold(int key)
{
	if (g_bDisableAllInput) return;

	if (CurrentGameUI() && CurrentGameUI()->IR_UIOnKeyboardHold(key)) return;
	if (Device.Paused()) return;
	if (CURRENT_ENTITY()) {
		IInputReceiver* IR = smart_cast<IInputReceiver*>	(smart_cast<CGameObject*>(CURRENT_ENTITY()));
		if (IR)				IR->IR_OnKeyboardHold(get_binded_action(key));
	}
}

void CLevel::IR_OnMouseStop( int /**axis/**/, int /**value/**/)
{
}

void CLevel::IR_OnActivate()
{
	if(!pInput) return;
	int i;
	for (i = 0; i < CInput::COUNT_KB_BUTTONS; i++ )
	{
		if(IR_GetKeyState(i))
		{

			EGameActions action = get_binded_action(i);
			switch (action){
			case kFWD			:
			case kBACK			:
			case kL_STRAFE		:
			case kR_STRAFE		:
			case kLEFT			:
			case kRIGHT			:
			case kUP			:
			case kDOWN			:
			case kCROUCH		:
			case kACCEL			:
			case kL_LOOKOUT		:
			case kR_LOOKOUT		:	
			case kWPN_FIRE		:
				{
					IR_OnKeyboardPress	(i);
				}break;
			};
		};
	}
}