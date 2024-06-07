#pragma once

#include "script_export_space.h"
#include "object_interfaces.h"
#include "UIDialogHolder.h"
#include "ui/UIPdaWnd.h"
#include "ui/UIMainIngameWnd.h"
#include "../xr_3da/CustomHUD.h"

// refs
class CUI;
class CTeamBaseZone;
class game_cl_GameState;
class CUIDialogWnd;
class CUICaption;
class CUIStatic;
class CUIWindow;
class CUIXml;
class CUIInventoryWnd;
class CUIMessagesWindow;

struct SDrawStaticStruct :public IPureDestroyableObject{
	SDrawStaticStruct	();
	virtual	void	destroy			();
	CUIStatic*		m_static;
	float			m_endTime;
	shared_str		m_name;
	void			Draw();
	void			Update();
	CUIStatic*		wnd()		{return m_static;}
	bool			IsActual();
	bool operator ==(LPCSTR str){
		return (m_name == str);
	}
};


typedef xr_vector<SDrawStaticStruct>	st_vec;
#include "game_base_space.h"
struct SGameTypeMaps
{
	shared_str				m_game_type_name;
	EGameTypes				m_game_type_id;
	xr_vector<shared_str>	m_map_names;
};

struct SGameWeathers
{
	shared_str				m_weather_name;
	shared_str				m_start_time;
};
typedef xr_vector<SGameWeathers>					GAME_WEATHERS;
typedef xr_vector<SGameWeathers>::iterator			GAME_WEATHERS_IT;
typedef xr_vector<SGameWeathers>::const_iterator	GAME_WEATHERS_CIT;

class CMapListHelper
{
	typedef xr_vector<SGameTypeMaps>	TSTORAGE;
	typedef TSTORAGE::iterator			TSTORAGE_IT;
	typedef TSTORAGE::iterator			TSTORAGE_CIT;
	TSTORAGE							m_storage;
	GAME_WEATHERS						m_weathers;

	void						Load			();
	SGameTypeMaps*				GetMapListInt	(const shared_str& game_type);
public:
	const SGameTypeMaps&		GetMapListFor	(const shared_str& game_type);
	const SGameTypeMaps&		GetMapListFor	(const EGameTypes game_id);
	const GAME_WEATHERS&		GetGameWeathers	();
};

extern CMapListHelper	gMapListHelper;

class CUIGameCustom :public DLL_Pure, public CDialogHolder
{
protected:
	CUIXml*				m_msgs_xml;
	CUICaption*			GameCaptions			() {return m_pgameCaptions;}
	CUICaption*			m_pgameCaptions;
	st_vec m_custom_statics;

	CUIInventoryWnd* m_InventoryMenu;
	CUIPdaWnd* m_PdaMenu;

	bool				m_bShowGameIndicators;

public:
	CUIMainIngameWnd*		UIMainIngameWnd;
	CUIMessagesWindow*		m_pMessagesWnd;

	virtual void		SetClGame				(game_cl_GameState* g){};

						CUIGameCustom			();
	virtual				~CUIGameCustom			();

	virtual	void		Init					(int stage)	{};
	
	virtual void		Render					();
	virtual void _BCL	OnFrame					();

	IC CUIInventoryWnd&	InventoryMenu			() const { return *m_InventoryMenu; }
	IC CUIPdaWnd&		PdaMenu					() const { return *m_PdaMenu;   }
	void ShowHideInventoryMenu() const;
	void ShowHidePdaMenu(const EPdaTabs tab) const;
	  
    void				ShowGameIndicators		(bool b)			{ m_bShowGameIndicators = b;};
	bool				GameIndicatorsShown		()					{return m_bShowGameIndicators;};
	void				ShowCrosshair			(bool b)			{psHUD_Flags.set			(HUD_CROSSHAIR_RT, b);}
	bool				CrosshairShown			()					{return !!psHUD_Flags.test	(HUD_CROSSHAIR_RT);}

	virtual void		ReInitShownUI			() = 0;
	virtual void		HideShownDialogs		(){};

			void		AddCustomMessage		(LPCSTR id, float x, float y, float font_size, CGameFont *pFont, u16 alignment, u32 color);
			void		AddCustomMessage		(LPCSTR id, float x, float y, float font_size, CGameFont *pFont, u16 alignment, u32 color/*, LPCSTR def_text*/, float flicker );
			void		CustomMessageOut		(LPCSTR id, LPCSTR msg, u32 color);
			void		RemoveCustomMessage		(LPCSTR id);

			SDrawStaticStruct*	AddCustomStatic		(LPCSTR id, bool bSingleInstance);
			SDrawStaticStruct*	GetCustomStatic		(LPCSTR id);
			void				RemoveCustomStatic	(LPCSTR id);

    virtual void		UnLoad					();
	void				Load					();

	void				OnConnected             ();

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CUIGameCustom)
#undef script_type_list
#define script_type_list save_type_list(CUIGameCustom)
