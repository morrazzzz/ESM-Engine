#include "stdafx.h"
#include "HUDManager.h"
#include "hudtarget.h"

#include "actor.h"
#include "MainMenu.h"
#include "../xr_3da/igame_level.h"
#include "clsid_game.h"
#include "GamePersistent.h"
#include "UIGameCustom.h"
#include "game_cl_base.h"

extern CUIGameCustom*	CurrentGameUI()	{return HUD().GetGameUI();}

CFontManager::CFontManager()
{
	Device.seqDeviceReset.Add(this,REG_PRIORITY_HIGH);

	m_all_fonts.push_back(&pFontMedium				);// used cpp
	m_all_fonts.push_back(&pFontDI					);// used cpp
	m_all_fonts.push_back(&pFontArial14				);// used xml
	m_all_fonts.push_back(&pFontGraffiti19Russian	);
	m_all_fonts.push_back(&pFontGraffiti22Russian	);
	m_all_fonts.push_back(&pFontLetterica16Russian	);
	m_all_fonts.push_back(&pFontLetterica18Russian	);
	m_all_fonts.push_back(&pFontGraffiti32Russian	);
	m_all_fonts.push_back(&pFontGraffiti50Russian	);
	m_all_fonts.push_back(&pFontLetterica25			);
	m_all_fonts.push_back(&pFontStat				);

	FONTS_VEC_IT it		= m_all_fonts.begin();
	FONTS_VEC_IT it_e	= m_all_fonts.end();
	for(;it!=it_e;++it)
		(**it) = NULL;

	InitializeFonts();

}

void CFontManager::InitializeFonts()
{

	InitializeFont(pFontMedium				,"hud_font_medium"				);
	InitializeFont(pFontDI					,"hud_font_di",					CGameFont::fsGradient|CGameFont::fsDeviceIndependent);
	InitializeFont(pFontArial14				,"ui_font_arial_14"				);
	InitializeFont(pFontGraffiti19Russian	,"ui_font_graffiti19_russian"	);
	InitializeFont(pFontGraffiti22Russian	,"ui_font_graffiti22_russian"	);
	InitializeFont(pFontLetterica16Russian	,"ui_font_letterica16_russian"	);
	InitializeFont(pFontLetterica18Russian	,"ui_font_letterica18_russian"	);
	InitializeFont(pFontGraffiti32Russian	,"ui_font_graff_32"				);
	InitializeFont(pFontGraffiti50Russian	,"ui_font_graff_50"				);
	InitializeFont(pFontLetterica25			,"ui_font_letter_25"			);
	InitializeFont(pFontStat				,"stat_font",					CGameFont::fsDeviceIndependent);

}

LPCSTR CFontManager::GetFontTexName (LPCSTR section)
{
	constexpr LPCSTR tex_names[]={"texture800","texture","texture1600"};
	int def_idx		= 1;//default 1024x768
	int idx			= def_idx;

	u32 h = Device.dwHeight;

	if(h<=600)		idx = 0;
	else if(h<1024)	idx = 1;
	else 			idx = 2;

	while(idx>=0)
	{
		if (pSettings->line_exist(section,tex_names[idx]))
			return pSettings->r_string(section,tex_names[idx]);

		--idx;
	}

	return pSettings->r_string(section,tex_names[def_idx]);
}

void CFontManager::InitializeFont(CGameFont*& F, LPCSTR section, u32 flags)
{
	LPCSTR font_tex_name = GetFontTexName(section);
	R_ASSERT(font_tex_name);

	if(!F)
		F = xr_new<CGameFont> ("font", font_tex_name, flags);
	else
		F->Initialize("font",font_tex_name);

#ifdef DEBUG
	F->m_font_name = section;
#endif
	if (pSettings->line_exist(section,"size")){
		float sz = pSettings->r_float(section,"size");
		if (flags&CGameFont::fsDeviceIndependent)	F->SetHeightI(sz);
		else										F->SetHeight(sz);
	}
	if (pSettings->line_exist(section,"interval"))
	F->SetInterval(pSettings->r_fvector2(section,"interval"));

}

CFontManager::~CFontManager()
{
	Device.seqDeviceReset.Remove(this);
	for (auto& it : m_all_fonts)
		xr_delete(*it);
}

void CFontManager::Render()
{
	for(auto& it: m_all_fonts)
		(*it)->OnRender();
}
void CFontManager::OnDeviceReset()
{
	InitializeFonts();
}

//--------------------------------------------------------------------
CHUDManager::CHUDManager()
{ 
	b_online = false;
	pUIGame = nullptr;
	m_pHUDTarget = xr_new<CHUDTarget>();
}
//--------------------------------------------------------------------
CHUDManager::~CHUDManager()
{
	OnDisconnected();

	if (pUIGame)
		pUIGame->UnLoad();

	xr_delete(pUIGame);
	xr_delete(m_pHUDTarget);
}

//--------------------------------------------------------------------

void CHUDManager::OnFrame()
{
	if (!b_online)					
		return;

	if (!b_online)
		return;

	if (pUIGame)
		pUIGame->OnFrame();

	m_pHUDTarget->CursorOnFrame();
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------

ENGINE_API extern float psHUD_FOV;

bool CHUDManager::NeedRenderHUD(CObject* object)
{
	if (!psHUD_Flags.is(HUD_WEAPON | HUD_WEAPON_RT))
		return false;

	if (!pUIGame)
		return false;

	if (!object)	
		return false;
	CActor* A = smart_cast<CActor*>(object);

	if (!A || !A->HUDview())							
		return false;

	return true;
}

void CHUDManager::Render_First(CObject* object)
{
	// only shadow 
	::Render->set_Invisible(TRUE);
	object->renderable_Render();
	::Render->set_Invisible(FALSE);
}

void CHUDManager::Render_Last(CObject* object)
{
	if (object->CLS_ID == CLSID_CAR)
		return;

	// hud itself
	object->OnHUDDraw(this);
}
extern void draw_wnds_rects();
extern ENGINE_API BOOL bShowPauseString;
//��������� ��������� ����������
#include "string_table.h"
void  CHUDManager::RenderUI()
{
	if(!b_online)					return;

	BOOL bAlready					= FALSE;
	if (true)
	{
		HitMarker.Render			();
		if (pUIGame)
			pUIGame->Render();

		UI().RenderFont();
	}

	if (psHUD_Flags.is(HUD_CROSSHAIR|HUD_CROSSHAIR_RT|HUD_CROSSHAIR_RT2) && !bAlready)	
		m_pHUDTarget->Render();

	draw_wnds_rects		();

	if( Device.Paused() && bShowPauseString){
		CGameFont* pFont	= UI().Font().pFontGraffiti50Russian;
		pFont->SetColor		(0x80FF0000	);
		LPCSTR _str			= CStringTable().translate("st_game_paused").c_str();
		
		Fvector2			_pos;
		_pos.set			(UI_BASE_WIDTH/2.0f, UI_BASE_HEIGHT/2.0f);
		UI().ClientToScreenScaled(_pos);
		pFont->SetAligment	(CGameFont::alCenter);
		pFont->Out			(_pos.x, _pos.y, _str);
		pFont->OnRender		();
	}

}

void CHUDManager::OnEvent(EVENT E, u64 P1, u64 P2)
{
}

collide::rq_result&	CHUDManager::GetCurrentRayQuery	() 
{
	return m_pHUDTarget->RQ;
}

void CHUDManager::SetCrosshairDisp	(float dispf, float disps)
{	
	m_pHUDTarget->HUDCrosshair.SetDispersion(psHUD_Flags.test(HUD_CROSSHAIR_DYNAMIC) ? dispf : disps);
}

void  CHUDManager::ShowCrosshair	(bool show)
{
	m_pHUDTarget->m_bShowCrosshair = show;
}


void CHUDManager::Hit(int idx, float power, const Fvector& dir)	
{
	HitMarker.Hit(idx, dir);
}

void CHUDManager::SetHitmarkType		(LPCSTR tex_name)
{
	HitMarker.InitShader				(tex_name);
}

// ------------------------------------------------------------------------------------

#include "ui\UIMainInGameWnd.h"
void CHUDManager::Load()
{
	if (!pUIGame)
	{
		pUIGame				= Game().createGameUI();
	} else
	{
		pUIGame->SetClGame	(&Game());
	}
}

void CHUDManager::OnScreenResolutionChanged()
{
	pUIGame->HideShownDialogs();

	//xr_delete(pWpnScopeXml);

	pUIGame->UnLoad();
	pUIGame->Load();

	pUIGame->OnConnected();
}

void CHUDManager::OnDisconnected()
{
    b_online				= false;
	if(pUIGame)
		Device.seqFrame.Remove	(pUIGame);
}

void CHUDManager::OnConnected()
{
	if(b_online)			return;
	b_online				= true;
	if(pUIGame)
		Device.seqFrame.Add	(pUIGame,REG_PRIORITY_LOW-1000);
}

void CHUDManager::net_Relcase	(CObject *object)
{
	VERIFY						(m_pHUDTarget);
	m_pHUDTarget->net_Relcase	(object);
}

CDialogHolder* CurrentDialogHolder()
{
	if (MainMenu()->IsActive())
		return MainMenu();
	else
		return HUD().GetGameUI();
}


bool   CHUDManager::RenderActiveItemUIQuery()
{
	return 0;
}

void   CHUDManager::RenderActiveItemUI()
{
}

//restore????
/*
#include "player_hud.h"
bool   CHUDManager::RenderActiveItemUIQuery()
{
	if (!psHUD_Flags.is(HUD_DRAW_RT2))
		return false;

	if (!psHUD_Flags.is(HUD_WEAPON|HUD_WEAPON_RT|HUD_WEAPON_RT2))return false;

	if(!need_render_hud())			return false;

	return (g_player_hud && g_player_hud->render_item_ui_query() );
}

void   CHUDManager::RenderActiveItemUI()
{
	if (!psHUD_Flags.is(HUD_DRAW_RT2))
		return;

	g_player_hud->render_item_ui		();
}
*/
