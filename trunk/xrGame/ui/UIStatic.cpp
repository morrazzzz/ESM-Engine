#include "stdafx.h"
#include "uistatic.h"
#include "UIXmlInit.h"
#include "UITextureMaster.h"
#include "uiframewindow.h"
#include "../HUDManager.h"
#include "../../xr_3da/LightAnimLibrary.h"
#include "uilines.h"
#include "../string_table.h"
#include "../ui_base.h"

#include "../Include/xrRender/UIRender.h"

const char * const	clDefault	= "default";
#define CREATE_LINES if (!m_pLines) {m_pLines = xr_new<CUILines>(); m_pLines->SetTextAlignment(CGameFont::alLeft);}
#define LA_CYCLIC			(1<<0)
#define LA_ONLYALPHA		(1<<1)
#define LA_TEXTCOLOR		(1<<2)
#define LA_TEXTURECOLOR		(1<<3)
//(1<<4) registered !!!
void lanim_cont::set_defaults()
{
	m_lanim					= NULL;	
	m_lanim_start_time		= -1.0f;
	m_lanim_delay_time		= 0.0f;
	m_lanimFlags.zero		();
}

CUIStatic:: CUIStatic()
{
	m_bTextureEnable		= true;
	m_bStretchTexture		= false;

	m_TextureOffset.set		(0.0f,0.0f);
	m_TextOffset.set		(0.0f,0.0f);

	m_pMask					= NULL;
	m_ElipsisPos			= eepNone;
	m_iElipsisIndent		= 0;

	m_ClipRect.set			(-1,-1,-1,-1);

	m_bCursorOverWindow		= false;
	m_bHeading				= false;
	m_fHeading				= 0.0f;
	m_lanim_clr.set_defaults	();
	m_lanim_xform.set_defaults	();

	m_pLines				= NULL;
	m_bEnableTextHighlighting = false;
}

CUIStatic::~ CUIStatic()
{
	xr_delete(m_pLines);
}

void CUIStatic::SetXformLightAnim(LPCSTR lanim, bool bCyclic)
{
	if(lanim && lanim[0]!=0)
		m_lanim_xform.m_lanim	= LALib.FindItem(lanim);
	else
		m_lanim_xform.m_lanim	= NULL;
	
	m_lanim_xform.m_lanimFlags.zero		();

	m_lanim_xform.m_lanimFlags.set		(LA_CYCLIC,			bCyclic);
}

void CUIStatic::SetClrLightAnim(LPCSTR lanim, bool bCyclic, bool bOnlyAlpha, bool bTextColor, bool bTextureColor)
{
	if(lanim && lanim[0]!=0)
		m_lanim_clr.m_lanim	= LALib.FindItem(lanim);
	else
		m_lanim_clr.m_lanim	= NULL;
	
	m_lanim_clr.m_lanimFlags.zero		();

	m_lanim_clr.m_lanimFlags.set		(LA_CYCLIC,			bCyclic);
	m_lanim_clr.m_lanimFlags.set		(LA_ONLYALPHA,		bOnlyAlpha);
	m_lanim_clr.m_lanimFlags.set		(LA_TEXTCOLOR,		bTextColor);
	m_lanim_clr.m_lanimFlags.set		(LA_TEXTURECOLOR,	bTextureColor);
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


void CUIStatic::DrawText(){
	if (m_pLines)
	{
		m_pLines->SetWndSize(m_wndSize);

		if(IsHighlightText() && xr_strlen(m_pLines->GetText())>0 && m_bEnableTextHighlighting)
			DrawHighlightedText();		
		else{
			Fvector2			p;
			GetAbsolutePos		(p);
			m_pLines->Draw		(p.x + m_TextOffset.x, p.y + m_TextOffset.y);
		}

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
	if (m_lanim_clr.m_lanim)
	{
		if(m_lanim_clr.m_lanim_start_time<0.0f)		ResetClrAnimation	();
		float t = Device.dwTimeContinual/1000.0f;

		if (t < m_lanim_clr.m_lanim_start_time)	// consider animation delay
			return;

		if(m_lanim_clr.m_lanimFlags.test(LA_CYCLIC) || t-m_lanim_clr.m_lanim_start_time < m_lanim_clr.m_lanim->Length_sec()){

			int frame;
			u32 clr					= m_lanim_clr.m_lanim->CalculateRGB(t-m_lanim_clr.m_lanim_start_time,frame);

			if(m_lanim_clr.m_lanimFlags.test(LA_TEXTURECOLOR))
				if(m_lanim_clr.m_lanimFlags.test(LA_ONLYALPHA))
					SetColor				(subst_alpha(GetColor(), color_get_A(clr)));
				else
					SetColor				(clr);

			if(m_lanim_clr.m_lanimFlags.test(LA_TEXTCOLOR))
				if(m_lanim_clr.m_lanimFlags.test(LA_ONLYALPHA))
					SetTextColor				(subst_alpha(GetTextColor(), color_get_A(clr)));
				else
					SetTextColor				(clr);
			
		}
	}
	
	if(m_lanim_xform.m_lanim)
	{
		if(m_lanim_xform.m_lanim_start_time<0.0f){
			ResetXformAnimation();
		}
		float t = Device.dwTimeContinual/1000.0f;

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
			_sz.set				(m_xxxRect.width()*f_scale, m_xxxRect.height()*f_scale );
			SetWndSize			(_sz);
		}else{
			EnableHeading_int	( !!m_lanim_xform.m_lanimFlags.test(1<<4) );
			SetWndSize			(Fvector2().set(m_xxxRect.width(),m_xxxRect.height()));
		}
	}
}

void CUIStatic::ResetXformAnimation()
{
	m_lanim_xform.m_lanim_start_time = Device.dwTimeContinual/1000.0f;
}

void CUIStatic::ResetClrAnimation()
{
	m_lanim_clr.m_lanim_start_time = Device.dwTimeContinual/1000.0f + m_lanim_clr.m_lanim_delay_time/1000.0f;
}

void CUIStatic::SetClrAnimDelay(float delay){
	m_lanim_clr.m_lanim_delay_time = delay;
}

bool CUIStatic::IsClrAnimStoped(){
	if (m_lanim_clr.m_lanimFlags.test(LA_CYCLIC) || m_lanim_clr.m_lanim_start_time<0.0f)
		return false;
	
	float t = Device.dwTimeContinual/1000.0f;
	if(t-m_lanim_clr.m_lanim_start_time < m_lanim_clr.m_lanim->Length_sec())
		return false;
	else 
		return true;
}

void CUIStatic::SetFont(CGameFont* pFont){
	CUIWindow::SetFont(pFont);
	CREATE_LINES;
	m_pLines->SetFont(pFont);
}

void CUIStatic::SetTextComplexMode(bool md){
	CREATE_LINES;
	m_pLines->SetTextComplexMode(md);
}

CGameFont* CUIStatic::GetFont(){
	CREATE_LINES;
	return m_pLines->GetFont();
}

void  CUIStatic::SetShader(const ui_shader& sh)
{
	m_UIStaticItem.SetShader(sh);
}

LPCSTR CUIStatic::GetText(){
	static const char empty = 0;
	if (m_pLines)
		return m_pLines->GetText();
	else
		return &empty;
}

void CUIStatic::SetTextColor(u32 color){
	CREATE_LINES;
	m_pLines->SetTextColor(color);
}

u32 CUIStatic::GetTextColor(){
	CREATE_LINES;
	return m_pLines->GetTextColor();
}

u32& CUIStatic::GetTextColorRef(){
	return m_pLines->GetTextColorRef();
}

void CUIStatic::SetText(LPCSTR str)
{
	if (!str ) 
		return;
	CREATE_LINES;
	m_pLines->SetText(str);
}

void CUIStatic::SetTextColor(u32 color, E4States state){
	m_dwTextColor[state] = color;
	m_bUseTextColor[state] = true;
}

void CUIStatic::SetMask(CUIFrameWindow *pMask)
{
	DetachChild(m_pMask);
	m_pMask = pMask;
	if (m_pMask){
		AttachChild			(m_pMask);
		Frect r				= GetWndRect();
		m_pMask->SetWidth	(r.right - r.left);
		m_pMask->SetHeight	(r.bottom - r.top);
	}
}

//CGameFont::EAligment CUIStatic::GetTextAlign(){
//	return m_pLines->GetTextAlignment();
//}

CGameFont::EAligment CUIStatic::GetTextAlignment(){
	return m_pLines->GetTextAlignment();
}

//void CUIStatic::SetTextAlign(CGameFont::EAligment align){
//	CREATE_LINES;
//	m_pLines->SetTextAlignment(align);
//}

void CUIStatic::SetTextAlignment(CGameFont::EAligment align){
	CREATE_LINES;
	m_pLines->SetTextAlignment(align);
	m_pLines->GetFont()->SetAligment((CGameFont::EAligment)align);
}

void CUIStatic::SetVTextAlignment(EVTextAlignment al){
	CREATE_LINES;
	m_pLines->SetVTextAlignment(al);
}

void CUIStatic::SetTextAlign_script(u32 align)
{
	m_pLines->SetTextAlignment((CGameFont::EAligment)align);
	m_pLines->GetFont()->SetAligment((CGameFont::EAligment)align);
}

u32 CUIStatic::GetTextAlign_script()
{
	return static_cast<u32>(m_pLines->GetTextAlignment());
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

void CUIStatic::AdjustHeightToText(){
	m_pLines->SetWidth		(GetWidth());
	m_pLines->ParseText();
	SetHeight				(m_pLines->GetVisibleHeight());
}

void CUIStatic::AdjustWidthToText()
{
	float _len		= m_pLines->GetFont()->SizeOf_(m_pLines->GetText());
	UI().ClientToScreenScaledWidth(_len);
	SetWidth		(_len);
}

void CUIStatic::RescaleRelative2Rect(const Frect& r){
	SetStretchTexture(true);
	Frect my_r = m_xxxRect;
	float h_rel = my_r.width()/r.width();
	float v_rel = my_r.height()/r.height();

	if (ui_core::is_widescreen())
		h_rel *= 3.0f / 4.0f;
	
	float w;
	float h;
	if (h_rel < v_rel){
		w = r.width()*h_rel;
		h = r.height()*h_rel;
	}
	else{
		w = r.width()*v_rel;
		h = r.height()*v_rel;
	}


	my_r.x1 += (m_xxxRect.width() - w)/2;
	my_r.y1 += (m_xxxRect.height() - h)/2;
	my_r.x2 = my_r.x1 + w;
	my_r.y2 = my_r.y1 + h;
	SetWndRect(my_r);
}

void CUIStatic::SetTextST				(LPCSTR str_id)
{
	SetText					(*CStringTable().translate(str_id));
}

void CUIStatic::DrawHighlightedText(){
	Frect				rect;
	GetAbsoluteRect		(rect);
	u32 def_col			= m_pLines->GetTextColor();
	m_pLines->SetTextColor(m_HighlightColor);
	m_pLines->Draw(	rect.left + 0 + m_TextOffset.x, rect.top - 0 + m_TextOffset.y);
	m_pLines->SetTextColor(def_col);
}

bool CUIStatic::IsHighlightText()
{
	return m_bCursorOverWindow;
}
