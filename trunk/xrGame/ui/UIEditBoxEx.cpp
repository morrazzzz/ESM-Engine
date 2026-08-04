#include "stdafx.h"

#include "UIEditBoxEx.h"
#include "UIFrameWindow.h"



CUIEditBoxEx::CUIEditBoxEx()
{
	m_pFrameWindow = xr_new<CUIFrameWindow>();
	AttachChild(m_pFrameWindow);

	TextItemControl()->SetTextComplexMode( true );
}

CUIEditBoxEx::~CUIEditBoxEx()
{
	xr_delete(m_pFrameWindow);
}	

void CUIEditBoxEx::Init(float x, float y, float width, float height)
{
	Fvector2 size{ width, height };

	m_pFrameWindow->SetWndSize(size);
	m_pFrameWindow->SetWndPos(Fvector2().set(0, 0));
	CUICustomEdit::Init(x,y,width,height);
}

void CUIEditBoxEx::InitTexture(const char* texture){
	m_pFrameWindow->InitTexture(texture);
}