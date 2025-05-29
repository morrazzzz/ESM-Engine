#include "pch_script.h"
#include "UIGameCustom.h"
#include "hudmanager.h"
#include "ui/UIMultiTextStatic.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIInventoryWnd.h"
#include "ui/UIMessagesWindow.h"
#include "ui/UIXmlInit.h"
#include "object_broker.h"
#include "string_table.h"

#include "actor.h"
#include "inventory.h"
#include "HudItem.h"

struct predicate_remove_stat {
	bool	operator() (SDrawStaticStruct& s) {
		return ( !s.IsActual() );
	}
};

CUIGameCustom::CUIGameCustom()
{
	m_msgs_xml = nullptr;
	WpnScopeXml = nullptr;
	m_InventoryMenu = nullptr;
	m_PdaMenu = nullptr;
	//m_window = nullptr;
	UIMainIngameWnd = nullptr;
	m_pMessagesWnd = nullptr;

	ShowGameIndicators(true);
	ShowCrosshair(true);
}

bool g_b_ClearGameCaptions = false;

CUIGameCustom::~CUIGameCustom()
{
	delete_data				(m_custom_statics);
	g_b_ClearGameCaptions = false;
}

void CUIGameCustom::OnFrame() 
{
	CDialogHolder::OnFrame();
	st_vec::iterator it = m_custom_statics.begin();
	for(;it!=m_custom_statics.end();++it)
		(*it).Update();

	m_custom_statics.erase(
		std::remove_if(
			m_custom_statics.begin(),
			m_custom_statics.end(),
			predicate_remove_stat()
		),
		m_custom_statics.end()
	);
	
	if(g_b_ClearGameCaptions)
	{
		delete_data				(m_custom_statics);
		g_b_ClearGameCaptions	= false;
	}

	//update windows
	if( GameIndicatorsShown() && psHUD_Flags.is(HUD_DRAW|HUD_DRAW_RT) )
		UIMainIngameWnd->Update	();

	m_pMessagesWnd->Update();
}

void CUIGameCustom::Render()
{
	//GameCaptions()->Draw();
	st_vec::iterator it = m_custom_statics.begin();
	for(;it!=m_custom_statics.end();++it)
		(*it).Draw();

	CEntity* pEntity = static_cast<CEntity*>(Level().CurrentEntity());
	if (pEntity && pEntity->g_Alive() && psHUD_Flags.is(HUD_WEAPON | HUD_WEAPON_RT))
	{
		CActor* pActor = pEntity->cast_actor();
		if (pActor && pActor->HUDview())
		{
			PIItem item = pActor->inventory().ActiveItem();
			if (item && item->render_item_ui_query())
				item->render_item_ui();
		}

		if( GameIndicatorsShown() && psHUD_Flags.is(HUD_DRAW | HUD_DRAW_RT) )
			UIMainIngameWnd->Draw();
	}

	m_pMessagesWnd->Draw();

	DoRenderDialogs();
}

void CUIGameCustom::AddCustomMessage		(LPCSTR id, float x, float y, float font_size, CGameFont *pFont, u16 alignment, u32 color/* LPCSTR def_text*/ )
{
	GameCaptions()->addCustomMessage(id,x,y,font_size,pFont,(CGameFont::EAligment)alignment,color,"");
}

void CUIGameCustom::AddCustomMessage		(LPCSTR id, float x, float y, float font_size, CGameFont *pFont, u16 alignment, u32 color, /*LPCSTR def_text,*/ float flicker )
{
	AddCustomMessage(id,x,y,font_size, pFont, alignment, color);
	GameCaptions()->customizeMessage(id, CUITextBanner::tbsFlicker)->fPeriod = flicker;
}

void CUIGameCustom::CustomMessageOut(LPCSTR id, LPCSTR msg, u32 color)
{
	GameCaptions()->setCaption(id,msg,color,true);
}

void CUIGameCustom::RemoveCustomMessage		(LPCSTR id)
{
	GameCaptions()->removeCustomMessage(id);
}

SDrawStaticStruct* CUIGameCustom::AddCustomStatic			(LPCSTR id, bool bSingleInstance)
{
	if(bSingleInstance){
		st_vec::iterator it = std::find(m_custom_statics.begin(),m_custom_statics.end(), id);
		if(it!=m_custom_statics.end())
			return &(*it);
	}
	
	CUIXmlInit xml_init;
	m_custom_statics.push_back		(SDrawStaticStruct());
	SDrawStaticStruct& sss			= m_custom_statics.back();

	sss.m_static					= xr_new<CUIStatic>();
	sss.m_name						= id;
	xml_init.InitStatic				(*m_msgs_xml, id, 0, sss.m_static);
	float ttl						= m_msgs_xml->ReadAttribFlt(id, 0, "ttl", -1);
	if(ttl>0.0f)
		sss.m_endTime				= Device.fTimeGlobal + ttl;

	return &sss;
}

SDrawStaticStruct* CUIGameCustom::GetCustomStatic		(LPCSTR id)
{
	st_vec::iterator it = std::find(m_custom_statics.begin(),m_custom_statics.end(), id);
	if(it!=m_custom_statics.end()){
		return &(*it);
	}
	return NULL;
}

void CUIGameCustom::RemoveCustomStatic		(LPCSTR id)
{
	st_vec::iterator it = std::find(m_custom_statics.begin(),m_custom_statics.end(), id);
	if(it!=m_custom_statics.end()){
		xr_delete((*it).m_static);
		m_custom_statics.erase(it);
	}
}


#include "ui/UIGameTutorial.h"

void CUIGameCustom::ShowHideInventoryMenu() const
{
	if (!m_InventoryMenu->IsShown())
		m_InventoryMenu->ShowDialog(true);
	else
		m_InventoryMenu->HideDialog();
}

void CUIGameCustom::ShowHidePdaMenu(const EPdaTabs tab) const
{
	if (!m_PdaMenu->IsShown())
	{
		m_PdaMenu->SetActiveSubdialog(tab);
		m_PdaMenu->ShowDialog(true);
	}
	else
		m_PdaMenu->HideDialog();
}

void CUIGameCustom::UnLoad()
{
	xr_delete					(m_msgs_xml);
	delete WpnScopeXml;
	xr_delete					(m_InventoryMenu);
	xr_delete					(m_PdaMenu);
//	xr_delete					(m_window);
	xr_delete					(UIMainIngameWnd);
	xr_delete					(m_pMessagesWnd);
}

void CUIGameCustom::Load()
{
	if(g_pGameLevel)
	{
		R_ASSERT				(!m_msgs_xml);
		m_msgs_xml				= xr_new<CUIXml>();
		m_msgs_xml->Init		(CONFIG_PATH, UI_PATH, "ui_custom_msgs.xml");

		R_ASSERT(!WpnScopeXml);
		WpnScopeXml = new CUIXml();
		if (!WpnScopeXml->Init(CONFIG_PATH, UI_PATH, "scopes.xml"))
		{
			delete WpnScopeXml;
			WpnScopeXml = nullptr;
		}

		R_ASSERT				(!m_InventoryMenu);
		m_InventoryMenu		    = xr_new<CUIInventoryWnd>	();

		R_ASSERT				(!m_PdaMenu);
		m_PdaMenu				= xr_new<CUIPdaWnd>			();
		
#pragma todo("not used for single player?????")
		//R_ASSERT				(!m_window);
		//m_window				= xr_new<CUIWindow>			();

		R_ASSERT				(!UIMainIngameWnd);
		UIMainIngameWnd			= xr_new<CUIMainIngameWnd>	();
		UIMainIngameWnd->Init	();

		R_ASSERT				(!m_pMessagesWnd);
		m_pMessagesWnd			= xr_new<CUIMessagesWindow>();
	}
}

void CUIGameCustom::OnConnected()
{
	if (g_pGameLevel)
	{
		if (!UIMainIngameWnd)
			Load();

		UIMainIngameWnd->OnConnected();
	}
}

SDrawStaticStruct::SDrawStaticStruct	()
{
	m_static	= NULL;
	m_endTime	= -1.0f;	
}

void SDrawStaticStruct::destroy()
{
	delete_data(m_static);
}

bool SDrawStaticStruct::IsActual()
{
	if(m_endTime<0) return true;
	return Device.fTimeGlobal < m_endTime;
}

void SDrawStaticStruct::Draw()
{
	if(m_static)
		m_static->Draw();
}

void SDrawStaticStruct::Update()
{
	if(!IsActual())	
		delete_data(m_static);
	else
		m_static->Update();
}
