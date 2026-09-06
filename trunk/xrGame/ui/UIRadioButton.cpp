//////////////////////////////////////////////////////////////////////
// UIRadioButton.cpp: класс кнопки, имеющей 2 состояния
// и работающей в группе с такими же кнопками
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include ".\uiradiobutton.h"
#include "UILines.h"


void CUIRadioButton::Init(float x, float y, float width, float height)
{
	TextItemControl();
	CUI3tButton::InitTexture("ui_radio");
	Fvector2 sz = m_background->Get(S_Enabled)->GetStaticItem()->GetSize();
	TextItemControl()->m_TextOffset.x = sz.x;

    CUI3tButton::Init(x,y, width, sz.y - 5.0f);

	TextItemControl()->m_wndPos.set(x, y);
	TextItemControl()->m_wndSize.set(Fvector2().set(width, m_background->Get(S_Enabled)->GetStaticItem()->GetSize().y));
}

void CUIRadioButton::InitTexture(LPCSTR tex_name){
	// do nothing
}
