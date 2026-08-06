#pragma once

#include "UIWindow.h"
#include "../alife_space.h"
#include "../inventory_space.h"
#include "../actor_defs.h"

class CUIStatic;
class CUITextWnd;
class CUIProgressBar;
class CUIProgressShape;
class CUIXml;
class CUIArtefactPanel;
class CUICarPanel;
class CActor;


class CUIHudStatesWnd : public CUIWindow
{
private:
	typedef CUIWindow						inherited;
	CUIStatic*			UIStaticHealth;
	CUIStatic*			UIStaticArmor;

	CUITextWnd*			UIWeaponSignAmmo;
	CUIStatic*			UIWeaponBack;

	CUICarPanel*		m_UICarPanel;
	CUIArtefactPanel*	m_artefactPanel;

	CUIStatic*			m_ui_weapon_icon;
	Frect				m_ui_weapon_icon_rect;

	CUIProgressBar*		m_ui_health_bar;
	CUIProgressBar*		m_ui_armor_bar;

	//morrazzzz: CoPMerge: Port me!!!
	//II_BriefInfo		m_item_info;
public:
					CUIHudStatesWnd		();
	virtual			~CUIHudStatesWnd	();

			void	InitFromXml			( CUIXml& xml, LPCSTR path );
	virtual void	Update				();
//	virtual void	Draw				();

			void	on_connected		();
			void	reset_ui			();
			void	UpdateHealth		( CActor* actor );
			void	SetAmmoIcon			( const shared_str& sect_name );
			void	UpdateActiveItemInfo( CActor* actor );

			inline CUIArtefactPanel& UIArtefactPanel() { return *m_artefactPanel; }
			inline CUICarPanel& UICarPaner() { return *m_UICarPanel; }

}; // class CUIHudStatesWnd
