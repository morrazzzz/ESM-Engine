// UIDialogWnd.cpp: класс простого диалога, нужен для стандартного запуска
// разным менюшек путем вызова виртуальных Show() И Hide()
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "uidialogwnd.h"
#include "../hudmanager.h"
#include "../xr_level_controller.h"
#include "../../xr_3da/xr_ioconsole.h"
#include "../level.h"
#include "../GameObject.h"

CUIDialogWnd:: CUIDialogWnd()
{
	m_pHolder		= NULL;
	m_bWorkInPause	= false;
	Hide			();
}

CUIDialogWnd::~ CUIDialogWnd()
{
}

void CUIDialogWnd::Show()
{
	inherited::Enable(true);
	inherited::Show(true);

	ResetAll();
}


void CUIDialogWnd::Hide()
{

	inherited::Enable(false);
	inherited::Show(false);
	
}

bool CUIDialogWnd::OnKeyboardHold(int dik)
{
	if(!IR_process()) return false;
	return inherited::OnKeyboardHold(dik);
}

bool CUIDialogWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if(!IR_process()) return false;
	if (inherited::OnKeyboardAction(dik, keyboard_action) )
		return true;
	return false;
}

bool CUIDialogWnd::IR_process()
{
	if(!IsEnabled())					return false;

	if (GetHolder()->IgnorePause())		return true;

	if(Device.Paused()&&!WorkInPause())	return false;
	return true;
}

CDialogHolder* CurrentDialogHolder();

void CUIDialogWnd::ShowDialog(bool bDoHideIndicators)
{
	if (!IsShown())
		CurrentDialogHolder()->StartDialog(this, bDoHideIndicators);
}

void CUIDialogWnd::HideDialog()
{
	R_ASSERT2(IsShown(), "dialog already hidden");
	GetHolder()->StopDialog(this);
}

void CUIDialogWnd::Update(){
	CUIWindow::Update();
}
