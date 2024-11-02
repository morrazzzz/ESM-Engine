#include "pch_script.h"
#include "uigamesp.h"
#include "actor.h"
#include "../xr_3da/xr_input.h"

#include "game_cl_Single.h"
#include "ui/UIPdaAux.h"
#include "xr_level_controller.h"
#include "actorcondition.h"
#include "../xr_3da/xr_ioconsole.h"
#include "object_broker.h"
#include "GameTaskManager.h"
#include "GameTask.h"

#include "ui/UIInventoryWnd.h"
#include "ui/UITradeWnd.h"
#include "ui/UIPdaWnd.h"
#include "ui/UITalkWnd.h"
#include "ui/UICarBodyWnd.h"
#include "ui/UIMessageBox.h"

CUIGameSP::CUIGameSP()
{
	m_game			= NULL;
	
	TalkMenu		= xr_new<CUITalkWnd>		();
	UICarBodyMenu	= xr_new<CUICarBodyWnd>		();
	UIChangeLevelWnd= xr_new<CChangeLevelWnd>		();
}

CUIGameSP::~CUIGameSP() 
{
	delete_data(TalkMenu);
	delete_data(UICarBodyMenu);
	delete_data(UIChangeLevelWnd);
}

void CUIGameSP::HideShownDialogs()
{
	CUIDialogWnd* mir = TopInputReceiver();
	if (mir && mir == m_InventoryMenu || mir == m_PdaMenu || mir == TalkMenu || mir == UICarBodyMenu)
		mir->HideDialog();

}

void CUIGameSP::SetClGame (game_cl_GameState* g)
{
	inherited::SetClGame				(g);
	m_game = smart_cast<game_cl_Single*>(g);
	R_ASSERT							(m_game);
}

void CUIGameSP::OnFrame()
{
	inherited::OnFrame();

	if (Device.Paused())	return;

	if (m_game_objective)
	{
		bool b_remove = false;
		int dik = get_action_dik(kSCORES, 0);
		if (dik && !pInput->iGetAsyncKeyState(dik))
			b_remove = true;

		dik = get_action_dik(kSCORES, 1);
		if (!b_remove && dik && !pInput->iGetAsyncKeyState(dik))
			b_remove = true;

		if (b_remove)
		{
			RemoveCustomStatic("main_task");
			m_game_objective = NULL;
		}
	}
}
void hud_adjust_mode_keyb(int dik);
void hud_draw_adjust_mode();

bool CUIGameSP::IR_UIOnKeyboardPress(int dik)
{
	if(inherited::IR_UIOnKeyboardPress(dik)) return true;
	if( Device.Paused()		) return false;

	CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
	if(!pActor)								
		return false;

	if(!pActor->g_Alive())	
		return false;

	hud_adjust_mode_keyb(dik);

	switch ( get_binded_action(dik) )
	{
	case kINVENTORY: 
		if (!TopInputReceiver() || TopInputReceiver() == m_InventoryMenu) {
			ShowHideInventoryMenu();
			return true;
		}
		break;
	case kACTIVE_JOBS:
		if (!TopInputReceiver() || TopInputReceiver() == m_PdaMenu) {
			ShowHidePdaMenu(eptQuests);
			return true;
		}
		break;

	case kMAP:
		if (!TopInputReceiver() || TopInputReceiver() == m_PdaMenu)
		{
			ShowHidePdaMenu(eptMap);
			return true;
		}
		break;

	case kCONTACTS:
		if (!TopInputReceiver() || TopInputReceiver() == m_PdaMenu) {
			ShowHidePdaMenu(eptContacts);
			return true;
		}
		break;

	case kSCORES:
		{
		    m_game_objective = AddCustomStatic("main_task", true);
			SGameTaskObjective* o	= pActor->GameTaskManager().ActiveObjective();
			m_game_objective->m_static->SetTextST(o ? *o->description : "st_no_active_task");
		}break;
	}
	return false;
}

void CUIGameSP::Render()
{
	inherited::Render();
	hud_draw_adjust_mode();
}

void CUIGameSP::StartTalk()
{
	RemoveCustomStatic		("main_task");


	TalkMenu->ShowDialog(true);
}

void CUIGameSP::StartCarBody(CInventoryOwner* pOurInv, CInventoryOwner* pOthers)
{
	if(TopInputReceiver())		
		return;

	UICarBodyMenu->InitCarBody		(pOurInv,  pOthers);
	UICarBodyMenu->ShowDialog(true);
}
void CUIGameSP::StartCarBody(CInventoryOwner* pOurInv, CInventoryBox* pBox)
{
	if(TopInputReceiver())		
		return;
	UICarBodyMenu->InitCarBody		(pOurInv,  pBox);
	UICarBodyMenu->ShowDialog(true);
}

void CUIGameSP::ReInitShownUI() 
{ 
 	if (m_InventoryMenu->IsShown())
		m_InventoryMenu->InitInventory_delayed();
	else if(UICarBodyMenu->IsShown())
		UICarBodyMenu->UpdateLists_delayed();
	
};


extern ENGINE_API BOOL bShowPauseString;
void CUIGameSP::ChangeLevel				(GameGraph::_GRAPH_ID game_vert_id, u32 level_vert_id, Fvector pos, Fvector ang, Fvector pos2, Fvector ang2, bool b)
{
	if (!TopInputReceiver() || TopInputReceiver() != UIChangeLevelWnd)
	{
		UIChangeLevelWnd->m_game_vertex_id = game_vert_id;
		UIChangeLevelWnd->m_level_vertex_id = level_vert_id;
		UIChangeLevelWnd->m_position = pos;
		UIChangeLevelWnd->m_angles = ang;
		UIChangeLevelWnd->m_position_cancel = pos2;
		UIChangeLevelWnd->m_angles_cancel = ang2;
		UIChangeLevelWnd->m_b_position_cancel = b;

		UIChangeLevelWnd->ShowDialog(true);
	}
}

CChangeLevelWnd::CChangeLevelWnd		()
{
	m_messageBox			= xr_new<CUIMessageBox>();	m_messageBox->SetAutoDelete(true);
	AttachChild				(m_messageBox);
	m_messageBox->Init		("message_box_change_level");
	SetWndPos				(m_messageBox->GetWndPos());
	m_messageBox->SetWndPos	(0.0f,0.0f);
	SetWndSize				(m_messageBox->GetWndSize());
}
void CChangeLevelWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if(pWnd==m_messageBox){
		if(msg==MESSAGE_BOX_YES_CLICKED){
			OnOk									();
		}else
		if(msg==MESSAGE_BOX_NO_CLICKED){
			OnCancel								();
		}
	}else
		inherited::SendMessage(pWnd, msg, pData);
}

void CChangeLevelWnd::OnOk()
{
	HideDialog();
	NET_Packet								p;
	p.w_begin								(M_CHANGE_LEVEL);
	p.w										(&m_game_vertex_id,sizeof(m_game_vertex_id));
	p.w										(&m_level_vertex_id,sizeof(m_level_vertex_id));
	p.w_vec3								(m_position);
	p.w_vec3								(m_angles);

	Level().Send							(p,net_flags(TRUE));
}

void CChangeLevelWnd::OnCancel()
{
	HideDialog();
	if(m_b_position_cancel){
		Actor()->MoveActor(m_position_cancel, m_angles_cancel);
	}
}

bool CChangeLevelWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if(keyboard_action==WINDOW_KEY_PRESSED)
	{
		if(is_binded(kQUIT, dik) )
			OnCancel		();
		return true;
	}
	return inherited::OnKeyboardAction(dik, keyboard_action);
}

bool g_block_pause	= false;
void CChangeLevelWnd::Show()
{
	g_block_pause							= true;
	Device.Pause							(TRUE, TRUE, TRUE, "CChangeLevelWnd_show");
	bShowPauseString						= FALSE;
}

void CChangeLevelWnd::Hide()
{
	g_block_pause							= false;
	Device.Pause							(FALSE, TRUE, TRUE, "CChangeLevelWnd_hide");
}

