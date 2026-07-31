// UICheckButton.cpp: класс кнопки, имеющей 2 состояния:
// с галочкой и без
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\uicheckbutton.h"
#include "../HUDManager.h"
#include "UILines.h"
#include "../../xr_3da/xr_input.h"
#include "UIXmlInit.h"

CUICheckButton::CUICheckButton(void)
{	
	SetTextAlignment(CGameFont::alLeft);
	m_bCheckMode = true;
	m_pDependControl = NULL;
}

CUICheckButton::~CUICheckButton(void)
{
}

void CUICheckButton::SetDependControl(CUIWindow* pWnd){
	m_pDependControl = pWnd;
}

void CUICheckButton::Update(){
	CUI3tButton::Update();

	if (m_pDependControl)
		m_pDependControl->Enable(GetCheck());
}


void CUICheckButton::SetCurrentValue(){
	SetCheck(GetOptBoolValue());
}

void CUICheckButton::SaveValue(){
	CUIOptionsItem::SaveValue();
	SaveOptBoolValue(GetCheck());
}

bool CUICheckButton::IsChanged(){
	return b_backup_val != GetCheck();
}

void CUICheckButton::Init(Fvector2 pos, Fvector2 size, LPCSTR texture_name)
{
	InitButton(pos, size);
	InitTexture(texture_name);
	m_pLines->SetWndPos(pos);
	m_pLines->SetWndSize(Fvector2().set(size.x, m_background.GetE()->GetStaticItem()->GetSize().y));
}

void CUICheckButton::InitTexture(LPCSTR texture_name)
{
	CUI3tButton::InitTexture(texture_name);
	Frect r = m_background.GetE()->GetStaticItem()->GetTextureRect();
	CUI3tButton::SetTextX(GetTextX() + r.width());
}

void CUICheckButton::SeveBackUpValue()
{
	b_backup_val = GetCheck();
}

void CUICheckButton::Undo()
{
	SetCheck		(b_backup_val);
	SaveValue		();
}

void CUICheckButton::OnFocusLost()
{
	if (m_eButtonState == BUTTON_PUSHED && pInput->GetPressedMouseKey(MOUSE_LEFT_BUTTON))
		return;

	inherited::OnFocusLost();
}

void CUICheckButton::OnFocusReceive()
{
	inherited::OnFocusReceive();
}

void CUICheckButton::Show(bool status)
{
	inherited::Show(status);
}
