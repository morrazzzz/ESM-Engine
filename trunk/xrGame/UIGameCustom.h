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

class CUIGameCustom: public CDialogHolder
{
protected:
	CUIXml* m_msgs_xml;
	CUICaption*			GameCaptions			() {return m_pgameCaptions;}
	CUICaption*			m_pgameCaptions;
	st_vec m_custom_statics;

	CUIInventoryWnd* m_InventoryMenu;
	CUIPdaWnd* m_PdaMenu;

	bool				m_bShowGameIndicators;

public:
	CUIXml* WpnScopeXml;
	CUIMainIngameWnd*		UIMainIngameWnd;
	CUIMessagesWindow*		m_pMessagesWnd;

						CUIGameCustom			();
	virtual				~CUIGameCustom			();
	
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
