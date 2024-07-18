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

	CEntity* pEntity = smart_cast<CEntity*>(Level().CurrentEntity());
	if (pEntity)
	{
		CActor* pActor = smart_cast<CActor*>(pEntity);
		if (pActor)
		{
			PIItem item = pActor->inventory().ActiveItem();
			if (item && pActor->HUDview() && smart_cast<CHudItem*>(item))
				(smart_cast<CHudItem*>(item))->OnDrawUI();
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
		
		Init					(0);
		Init					(1);
		Init					(2);
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

CMapListHelper	gMapListHelper;
xr_token		game_types[];

void CMapListHelper::Load()
{
	string_path					fn;
	FS.update_path				(fn, "$game_config$", "mp\\map_list.ltx");
	CInifile map_list_cfg		(fn);

	//read weathers set
	CInifile::Sect w			= map_list_cfg.r_section("weather");
	CInifile::SectCIt wi		= w.Data.begin();
	CInifile::SectCIt wi_e		= w.Data.end();
	for( ;wi!=wi_e; ++wi)
	{
		m_weathers.resize		(m_weathers.size()+1);
		SGameWeathers& gw		= m_weathers.back();
		gw.m_weather_name		= (*wi).first;
		gw.m_start_time			= (*wi).second;
	}

	//read original maps from config
	CInifile::RootIt it			= map_list_cfg.sections().begin();
	CInifile::RootIt it_e		= map_list_cfg.sections().end();
	for( ;it!=it_e; ++it)
	{
		m_storage.resize		(m_storage.size()+1);
		SGameTypeMaps&	Itm		= m_storage.back();
		Itm.m_game_type_name	= (*it)->Name;
		Itm.m_game_type_id		= (EGameTypes)get_token_id(game_types, Itm.m_game_type_name.c_str() );

		CInifile::SectCIt sit	= (*it)->Data.begin();
		CInifile::SectCIt sit_e	= (*it)->Data.end();
		
		for( ;sit!=sit_e; ++sit)
		{
			Itm.m_map_names.push_back	((*sit).first);
		}
	}
	// scan for additional maps
	FS_FileSet			fset;
	FS.file_list		(fset,"$game_levels$",FS_ListFiles,"*level.ltx");

	FS_FileSetIt fit	= fset.begin();
	FS_FileSetIt fit_e	= fset.end();

	for( ;fit!=fit_e; ++fit)
	{
		string_path					map_cfg_fn;
		FS.update_path				(map_cfg_fn, "$game_levels$", (*fit).name.c_str());
		CInifile	map_ini			(map_cfg_fn);

		if(map_ini.section_exist("map_usage"))
		{
			CInifile::Sect S			= map_ini.r_section("map_usage");
			CInifile::SectCIt si		= S.Data.begin();
			CInifile::SectCIt si_e		= S.Data.end();
			for( ;si!=si_e; ++si)
			{
				const shared_str& game_type = (*si).first;
				SGameTypeMaps* M			= GetMapListInt(game_type);
				if(!M)
				{
					Msg						("--unknown game type-%s",game_type.c_str());
					m_storage.resize		(m_storage.size()+1);
					SGameTypeMaps&	Itm		= m_storage.back();
					Itm.m_game_type_name	= game_type;
					Itm.m_game_type_id		= (EGameTypes)get_token_id(game_types, game_type.c_str() );
					M						= &m_storage.back();
				}
				shared_str _map_name			= (*fit).name.substr(0,(*fit).name.find('\\')).c_str();
				
				if(M->m_map_names.end()==std::find(M->m_map_names.begin(),M->m_map_names.end(),_map_name))
					M->m_map_names.push_back	(_map_name);
			}			
		}
	}

	R_ASSERT2	(m_storage.size(), "unable to fill map list");
	R_ASSERT2	(m_weathers.size(), "unable to fill weathers list");
}

const SGameTypeMaps& CMapListHelper::GetMapListFor(const shared_str& game_type)
{
	if( !m_storage.size() )
		Load		();

	return *GetMapListInt(game_type);
}

SGameTypeMaps* CMapListHelper::GetMapListInt(const shared_str& game_type)
{

	TSTORAGE_CIT it		= m_storage.begin();
	TSTORAGE_CIT it_e	= m_storage.end();
	for( ;it!=it_e; ++it)
	{
		if(game_type==(*it).m_game_type_name )
			return &(*it);
	}
	return NULL;
}

const SGameTypeMaps& CMapListHelper::GetMapListFor(const EGameTypes game_id)
{
	if( !m_storage.size() )
	{
		Load		();
		R_ASSERT2	(m_storage.size(), "unable to fill map list");
	}
	TSTORAGE_CIT it		= m_storage.begin();
	TSTORAGE_CIT it_e	= m_storage.end();
	for( ;it!=it_e; ++it)
	{
		if(game_id==(*it).m_game_type_id )
			return (*it);
	}
	return m_storage[0];
}

const GAME_WEATHERS& CMapListHelper::GetGameWeathers() 
{
	if(!m_weathers.size())
		Load();

	return m_weathers;
}