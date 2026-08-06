#include "stdafx.h"
#include "UIBtnHint.h"
#include "UIFrameLineWnd.h"
#include "UIStatic.h"
#include "UIXmlInit.h"

CUIButtonHint*		g_btnHint = NULL; 



CUIButtonHint::CUIButtonHint	()
:m_ownerWnd(NULL),m_enabledOnFrame(false)
{
	Device.seqRender.Add		(this, REG_PRIORITY_LOW-1000);

	CUIXmlInit					xml_init;
	CUIXml						uiXml;
	bool xml_result				= uiXml.Init(CONFIG_PATH, UI_PATH, "hint_item.xml");
	R_ASSERT3					(xml_result, "xml file not found", "hint_item.xml");

	xml_init.InitWindow			(uiXml,"button_hint",0,this);
	
	m_border					= xr_new<CUIFrameLineWnd>();m_border->SetAutoDelete(true);
	AttachChild					(m_border);
	xml_init.InitFrameLine		(uiXml,"button_hint:frame_line",0,m_border);

	m_text						= xr_new<CUITextWnd>();
	m_text->SetAutoDelete		(true);
	AttachChild					(m_text);
	xml_init.InitTextWnd		(uiXml,"button_hint:description",0,m_text);
}

CUIButtonHint::~CUIButtonHint	()
{
	Device.seqRender.Remove		(this);
}

void CUIButtonHint::OnRender	()
{
	if(m_enabledOnFrame){
		m_text->Update		();
		m_border->Update	();
		m_border->SetTextureColor(color_rgba(255,255,255,color_get_A(m_text->GetTextColor())));
		Draw				();
		m_enabledOnFrame	= false;
	}
}

void CUIButtonHint::SetHintText	(CUIWindow* w, LPCSTR text)
{
	m_ownerWnd					= w;
	m_text->SetTextST			(text);

	m_text->AdjustHeightToText	();

	Fvector2					new_size;
	new_size.x					= GetWndSize().x;
	new_size.y					= m_text->GetWndSize().y+20.0f;

	SetWndSize					(new_size);

	m_text->ResetColorAnimation();
}
