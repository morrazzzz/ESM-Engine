#include "stdafx.h"
#include "UIHudStatesWnd.h"
#include "../Actor.h"
#include "../ActorCondition.h"
#include "../EntityCondition.h"
#include "../CustomOutfit.h"
//#include "../ActorHelmet.h"
#include "../inventory.h"
#include "../RadioactiveZone.h"
#include "UIStatic.h"
#include "UIProgressBar.h"
#include "UIProgressShape.h"
#include "UIXmlInit.h"
#include "UIHelper.h"
//#include "ui_arrow.h"
#include "UIInventoryUtilities.h"
#include "../CustomDetector.h"
#include "../ai/monsters/basemonster/base_monster.h"
#include "../PDA.h"
#include "../WeaponMagazinedWGrenade.h"
#include "UICarPanel.h"
#include "UIArtefactPanel.h"

CUIHudStatesWnd::CUIHudStatesWnd()
{
}

CUIHudStatesWnd::~CUIHudStatesWnd()
{
}

void CUIHudStatesWnd::reset_ui()
{
}

void CUIHudStatesWnd::InitFromXml( CUIXml& xml, LPCSTR path )
{
	CUIXmlInit::InitWindow( xml, path, 0, this );
	XML_NODE* stored_root = xml.GetLocalRoot();
	
	XML_NODE* new_root = xml.NavigateToNode( path, 0 );
	xml.SetLocalRoot( new_root );

	UIStaticHealth = UIHelper::CreateStatic(xml, "static_health", this);
	UIStaticArmor = UIHelper::CreateStatic(xml, "static_armor", this);
	UIWeaponBack = UIHelper::CreateStatic(xml, "static_weapon", this);

	UIWeaponSignAmmo = UIHelper::CreateTextWnd(xml, "static_ammo", UIWeaponBack);

	m_ui_health_bar = UIHelper::CreateProgressBar(xml, "progress_bar_health", UIStaticHealth);
	m_ui_armor_bar = UIHelper::CreateProgressBar(xml, "progress_bar_armor", UIStaticArmor);

	m_ui_weapon_icon			= UIHelper::CreateStatic( xml, "static_wpn_icon", UIWeaponBack);
	m_ui_weapon_icon->SetShader( InventoryUtilities::GetEquipmentIconsShader() );
//	m_ui_weapon_icon->Enable	( false );
	m_ui_weapon_icon_rect		= m_ui_weapon_icon->GetWndRect();

	m_UICarPanel = new CUICarPanel();
	AttachChild(m_UICarPanel);
	m_UICarPanel->SetAutoDelete(true);
	CUIXmlInit::InitWindow(xml, "car_panel", 0, m_UICarPanel);

	m_artefactPanel = new CUIArtefactPanel();
	AttachChild(m_artefactPanel);
	m_artefactPanel->SetAutoDelete(true);
	m_artefactPanel->InitFromXML(xml, "artefact_panel", 0);

	xml.SetLocalRoot( stored_root );
}

void CUIHudStatesWnd::on_connected()
{
}


void CUIHudStatesWnd::Update()
{
	if (!Level().CurrentViewEntity())
		return;

	CActor* actor = static_cast<CGameObject*>(Level().CurrentViewEntity())->cast_actor();
	if (!actor)
		return;

	UpdateHealth( actor );
	UpdateActiveItemInfo( actor );

	inherited::Update();
}

void CUIHudStatesWnd::UpdateHealth(CActor* actor)
{
	// Armor indicator stuff
	PIItem	pItem = actor->inventory().ItemFromSlot(OUTFIT_SLOT);
	if (pItem)
	{
		m_ui_armor_bar->Show(true);
		UIStaticArmor->Show(true);
		m_ui_armor_bar->SetProgressPos(pItem->GetCondition() * 100);
	}
	else
	{
		m_ui_armor_bar->Show(false);
		UIStaticArmor->Show(false);
	}
	m_ui_health_bar->SetProgressPos(actor->GetfHealth() * 100.0f);
}

void CUIHudStatesWnd::UpdateActiveItemInfo( CActor* actor )
{
	PIItem item = actor->inventory().ActiveItem();
	if (item)
	{
		xr_string					str_name;
		xr_string					icon_sect_name;
		xr_string					str_count;
		item->GetBriefInfo(str_name, icon_sect_name, str_count);

		UIWeaponSignAmmo->Show(true);
		UIWeaponBack->TextItemControl()->SetText(str_name.c_str());
		UIWeaponSignAmmo->SetText(str_count.c_str());
		SetAmmoIcon(icon_sect_name.c_str());
	}
	else
	{
		m_ui_weapon_icon->Show(false);
		UIWeaponSignAmmo->Show(false);
		UIWeaponBack->TextItemControl()->SetText("");
	}
}

void CUIHudStatesWnd::SetAmmoIcon(const shared_str& sect_name)
{
	if (!sect_name.size())
	{
		m_ui_weapon_icon->Show(false);
		return;
	}
	m_ui_weapon_icon->Show(true);

	//properties used by inventory menu
	float iGridWidth = pSettings->r_float(sect_name, "inv_grid_width");
	float iGridHeight = pSettings->r_float(sect_name, "inv_grid_height");

	float iXPos = pSettings->r_float(sect_name, "inv_grid_x");
	float iYPos = pSettings->r_float(sect_name, "inv_grid_y");

	Frect rect{ (iXPos * INV_GRID_WIDTH), (iYPos * INV_GRID_HEIGHT),
				(iGridWidth * INV_GRID_WIDTH), (iGridHeight * INV_GRID_HEIGHT) };
	rect.rb.add(rect.lt);

	m_ui_weapon_icon->GetUIStaticItem().SetTextureRect(rect);
	m_ui_weapon_icon->SetStretchTexture(true);

	// now perform only width scale for ammo, which (W)size >2
	// all others ammo (1x1, 1x2) will be not scaled (original picture)
	float w = ((iGridWidth > 2) ? 1.6f : iGridWidth) * INV_GRID_WIDTH * 0.9f;
	float h = INV_GRID_HEIGHT * 0.9f;//1 cell

	float x = m_ui_weapon_icon_rect.x1;
	if (iGridWidth < 2)
		x += (m_ui_weapon_icon_rect.width() - w) / 2.0f;

	m_ui_weapon_icon->SetWndPos(x, m_ui_weapon_icon_rect.y1);

	m_ui_weapon_icon->SetWidth(w * UI().get_current_kx());
	m_ui_weapon_icon->SetHeight(h);
}
