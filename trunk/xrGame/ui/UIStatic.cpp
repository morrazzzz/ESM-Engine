#include "stdafx.h"
#include "uistatic.h"
#include "UIXmlInit.h"
#include "UITextureMaster.h"
#include "../../xr_3da/LightAnimLibrary.h"
#include "uilines.h"
#include "../string_table.h"
#include "../ui_base.h"

#include "../Include/xrRender/UIRender.h"

void lanim_cont::set_defaults()
{
	m_lanim					= NULL;	
	m_lanim_start_time		= -1.0f;
	m_lanim_delay_time		= 0.0f;
	m_lanimFlags.zero		();
}
void lanim_cont_xf::set_defaults()
{
	lanim_cont::set_defaults();
	m_origSize.set			(0,0);
}

CUIStatic:: CUIStatic()
{
	m_bTextureEnable		= true;
	m_bStretchTexture		= false;

	m_TextureOffset.set		(0.0f,0.0f);
	m_ElipsisPos			= eepNone;
	m_iElipsisIndent		= 0;

	m_ClipRect.set			(-1,-1,-1,-1);

	m_bCursorOverWindow		= false;
	m_bHeading				= false;
	m_fHeading				= 0.0f;
	m_lanim_xform.set_defaults	();

	m_pTextControl = nullptr;
}

CUIStatic::~CUIStatic()
{
	xr_delete(m_pTextControl);
}

void CUIStatic::SetXformLightAnim(LPCSTR lanim, bool bCyclic)
{
	if(lanim && lanim[0]!=0)
		m_lanim_xform.m_lanim	= LALib.FindItem(lanim);
	else
		m_lanim_xform.m_lanim	= NULL;
	
	m_lanim_xform.m_lanimFlags.zero		();

	m_lanim_xform.m_lanimFlags.set		(LA_CYCLIC,			bCyclic);
	m_lanim_xform.m_origSize			= GetWndSize();
}

void CUIStatic::Init(LPCSTR tex_name, float x, float y, float width, float height)
{
	Init(x, y, width, height);
	InitTexture(tex_name);
}

void CUIStatic::InitEx(LPCSTR tex_name, LPCSTR sh_name, float x, float y, float width, float height)
{
	Init(x, y, width, height);
	InitTextureEx(tex_name, sh_name);	
}

void CUIStatic::Init(float x, float y, float width, float height){
	CUIWindow::Init(x,y,width,height);
	m_xxxRect.set(x,y,x+width,y+height);
}

void CUIStatic::InitTexture(LPCSTR texture){
	InitTextureEx(texture);
}

void CUIStatic::CreateShader(const char* tex, const char* sh){
	m_UIStaticItem.CreateShader(tex,sh);	
}

ui_shader& CUIStatic::GetShader(){
	return m_UIStaticItem.GetShader();
}


void CUIStatic::SetTextureColor(u32 color){
	m_UIStaticItem.SetTextureColor(color);
}

u32 CUIStatic::GetTextureColor() const{
	return m_UIStaticItem.GetTextureColor();
}

void CUIStatic::InitTextureEx(LPCSTR tex_name, LPCSTR sh_name)
{
	LPCSTR res_shname = UIRender->UpdateShaderName(tex_name, sh_name);
	CUITextureMaster::InitTexture	(tex_name, &m_UIStaticItem, res_shname);

	Fvector2 p						= GetWndPos();
	m_UIStaticItem.SetPos			(p.x, p.y);
}

void  CUIStatic::Draw()
{
	DrawTexture				();	
	inherited::Draw			();
	DrawText				();
}


void CUIStatic::DrawText()
{
	if (m_pTextControl)
	{
		if( !fsimilar(m_pTextControl->m_wndSize.x, m_wndSize.x) || !fsimilar(m_pTextControl->m_wndSize.y, m_wndSize.y))
		{
			m_pTextControl->m_wndSize		= m_wndSize;
			m_pTextControl->ParseText		(true);
		}

		Fvector2			p;
		GetAbsolutePos		(p);
		m_pTextControl->Draw(p.x, p.y);
	}
}

#include "../../Include/xrRender/UIShader.h"

void CUIStatic::DrawTexture()
{
	if(m_bTextureEnable && GetShader() && GetShader()->inited())
	{
		Frect			rect;
		GetAbsoluteRect	(rect);
		m_UIStaticItem.SetPos	(rect.left + m_TextureOffset.x, rect.top + m_TextureOffset.y);

		if(m_bStretchTexture)
		{
			if(Heading())
			{
				if( m_UIStaticItem.GetFixedLTWhileHeading() )
				{
					float t1,t2;
					t1			= rect.width();
					t2			= rect.height();
					rect.y2		= rect.y1 + t1;
					rect.x2		= rect.x1 + t2;
				}
			}
			m_UIStaticItem.SetSize(Fvector2().set(rect.width(), rect.height()));
		}else
		{
			Frect r={0.0f,0.0f,
				m_UIStaticItem.GetTextureRect().width(),
				m_UIStaticItem.GetTextureRect().height()};

			{	
				if(Heading())
				{
					float t1,t2;
					t1			= rect.width();
					t2			= rect.height();
					rect.y2		= rect.y1 + t1;
					rect.x2		= rect.x1 + t2;
				}

				m_UIStaticItem.SetSize(Fvector2().set(r.width(),r.height()));
			}
		}

		if( Heading() )
		{
			m_UIStaticItem.Render( GetHeading() );
		}else
			m_UIStaticItem.Render();
	}
}

void CUIStatic::Update()
{
	inherited::Update();
	//update light animation if defined
	UpdateColorAnimation();

	if(m_lanim_xform.m_lanim)
	{
		if(m_lanim_xform.m_lanim_start_time<0.0f)
			ResetXformAnimation();

		float t = Device.dwTimeGlobal/1000.0f;

		if(	m_lanim_xform.m_lanimFlags.test(LA_CYCLIC) || 
			t - m_lanim_xform.m_lanim_start_time < m_lanim_xform.m_lanim->Length_sec() )
		{
			int frame;
			u32 clr				= m_lanim_xform.m_lanim->CalculateRGB(t-m_lanim_xform.m_lanim_start_time,frame);
			
			EnableHeading_int	(true);
			float heading		= (PI_MUL_2/255.0f) * color_get_A(clr);
			SetHeading			(heading);

			float _value		= (float)color_get_R(clr);
			
			float f_scale		= _value / 64.0f;
			Fvector2 _sz;
			_sz.set				(m_lanim_xform.m_origSize.x*f_scale, m_lanim_xform.m_origSize.y*f_scale );
			SetWndSize			(_sz);
		}else
		{
			EnableHeading_int	( m_bHeading );
			SetWndSize			(m_lanim_xform.m_origSize);
		}
	}
}

void CUIStatic::SetFont(CGameFont* pFont){
	CUIWindow::SetFont(pFont);
	TextItemControl()->SetFont(pFont);
}

void CUIStatic::SetTextComplexMode(bool md){
	TextItemControl()->SetTextComplexMode(md);
}

CGameFont* CUIStatic::GetFont(){
	return TextItemControl()->GetFont();
}

void CUIStatic::ResetXformAnimation()
{
	m_lanim_xform.m_lanim_start_time = Device.dwTimeGlobal/1000.0f;
}

void  CUIStatic::SetShader(const ui_shader& sh)
{
	m_UIStaticItem.SetShader(sh);
}

CUILines* CUIStatic::TextItemControl()
{
	if (!m_pTextControl)
	{
		m_pTextControl = xr_new<CUILines>();
		m_pTextControl->SetTextAlignment(CGameFont::alLeft);
	}
	return m_pTextControl;
}

LPCSTR CUIStatic::GetText(){
	static const char empty = 0;
	if (m_pTextControl)
		return m_pTextControl->GetText();
	else
		return &empty;
}

void CUIStatic::SetTextColor(u32 color){
	TextItemControl()->SetTextColor(color);
}

u32 CUIStatic::GetTextColor(){
	return TextItemControl()->GetTextColor();
}

u32& CUIStatic::GetTextColorRef(){
	return m_pTextControl->GetTextColorRef();
}

void CUIStatic::SetText(LPCSTR str)
{
	if (!str ) 
		return;
	TextItemControl()->SetText(str);
}

void CUIStatic::AdjustHeightToText()
{
	if (!fsimilar(TextItemControl()->m_wndSize.x, GetWidth()))
	{
		TextItemControl()->m_wndSize.x = GetWidth();
		TextItemControl()->ParseText(true);
	}
	SetHeight(TextItemControl()->GetVisibleHeight());
}

void CUIStatic::AdjustWidthToText()
{
	if (!m_pTextControl)	return;
	float _len = m_pTextControl->GetFont()->SizeOf_(m_pTextControl->GetText());
	UI().ClientToScreenScaledWidth(_len);
	SetWidth(_len);
}

void CUIStatic::ColorAnimationSetTextureColor(u32 color, bool only_alpha)
{
	SetTextureColor((only_alpha) ? subst_alpha(GetTextureColor(), color) : color);
}

void CUIStatic::ColorAnimationSetTextColor(u32 color, bool only_alpha)
{
	TextItemControl()->SetTextColor((only_alpha) ? subst_alpha(TextItemControl()->GetTextColor(), color) : color);
}

void CUIStatic::SetTextColor(u32 color, E4States state){
	m_dwTextColor[state] = color;
	m_bUseTextColor[state] = true;
}

//CGameFont::EAligment CUIStatic::GetTextAlign(){
//	return m_pLines->GetTextAlignment();
//}

CGameFont::EAligment CUIStatic::GetTextAlignment(){
	return m_pTextControl->GetTextAlignment();
}

//void CUIStatic::SetTextAlign(CGameFont::EAligment align){
//	CREATE_LINES;
//	m_pLines->SetTextAlignment(align);
//}

void CUIStatic::SetTextAlignment(CGameFont::EAligment align){
	TextItemControl()->SetTextAlignment(align);
	TextItemControl()->GetFont()->SetAligment((CGameFont::EAligment)align);
}

void CUIStatic::SetVTextAlignment(EVTextAlignment al){
	TextItemControl()->SetVTextAlignment(al);
}

void CUIStatic::SetTextAlign_script(u32 align)
{
	m_pTextControl->SetTextAlignment((CGameFont::EAligment)align);
	m_pTextControl->GetFont()->SetAligment((CGameFont::EAligment)align);
}

u32 CUIStatic::GetTextAlign_script()
{
	return static_cast<u32>(m_pTextControl->GetTextAlignment());
}


void CUIStatic::Elipsis(const Frect &rect, EElipsisPosition elipsisPos)
{
#pragma todo("Satan->Satan : need adaptation")
	//if (eepNone == elipsisPos) return;

	//CUIStatic::Elipsis(m_sEdit, rect, elipsisPos, GetFont());

	//// Now paste elipsis
	//m_str = &m_sEdit.front();
	//str_len = m_sEdit.size();
	//buf_str.resize(str_len + 1);
}

void CUIStatic::SetElipsis(EElipsisPosition pos, int indent)
{
#pragma todo("Satan->Satan : need adaptation")
	m_ElipsisPos		= pos;
	m_iElipsisIndent	= indent;
}

void CUIStatic::OnFocusReceive()
{
	inherited::OnFocusReceive();
	if (GetMessageTarget())
        GetMessageTarget()->SendMessage(this, STATIC_FOCUS_RECEIVED, NULL);
}

void CUIStatic::OnFocusLost(){

	inherited::OnFocusLost();
	if (GetMessageTarget())
		GetMessageTarget()->SendMessage(this, STATIC_FOCUS_LOST, NULL);
}

void CUIStatic::SetTextST				(LPCSTR str_id)
{
	SetText					(*CStringTable().translate(str_id));
}

