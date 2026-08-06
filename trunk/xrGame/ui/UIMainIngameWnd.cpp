#include "stdafx.h"

#include "UIMainIngameWnd.h"
#include "UIMessagesWindow.h"
#include "../UIZoneMap.h"


#include <dinput.h>
#include "../actor.h"
#include "../HUDManager.h"
#include "../PDA.h"
#include "../character_info.h"
#include "../inventory.h"
#include "../UIGameSP.h"
#include "../weaponmagazined.h"
#include "../missile.h"
#include "../Grenade.h"
#include "../xrServer_objects_ALife.h"
#include "../alife_simulator.h"
#include "../alife_object_registry.h"
#include "Actor_Flags.h"
#include "../level.h"
#include "../seniority_hierarchy_holder.h"

#include "../date_time.h"
#include "../xrServer_Objects_ALife_Monsters.h"
#include "../../xr_3da/LightAnimLibrary.h"

#include "UIInventoryUtilities.h"
#include "UIHelper.h"
#include <functional>

#include "UIXmlInit.h"
#include "UIPdaMsgListItem.h"
#include "../alife_registry_wrappers.h"
#include "../actorcondition.h"

#include "../string_table.h"
#include "../clsid_game.h"
#include "../../xr_3da/xr_input.h"

#include "UIScrollView.h"
#include "map_hint.h"
#include "UIColorAnimatorWrapper.h"
#include "../game_news.h"
#include "UIHudStatesWnd.h"

#ifdef DEBUG
#include "../../Include/xrRender/Kinematics.h"
#include "../debug_renderer.h"
#endif


using namespace InventoryUtilities;

const u32	g_clWhite					= 0xffffffff;

#define		DEFAULT_MAP_SCALE			1.f

#define		C_SIZE						0.025f
#define		NEAR_LIM					0.5f

#define		SHOW_INFO_SPEED				0.5f
#define		HIDE_INFO_SPEED				10.f
#define		C_ON_ENEMY					D3DCOLOR_XRGB(0xff,0,0)
#define		C_DEFAULT					D3DCOLOR_XRGB(0xff,0xff,0xff)

#define				MAININGAME_XML				"maingame.xml"

CUIMainIngameWnd::CUIMainIngameWnd()
{
	UIZoneMap					= xr_new<CUIZoneMap>();
	m_pPickUpItem				= NULL;
}

#include "UIProgressShape.h"
extern CUIProgressShape* g_MissileForceShape;

CUIMainIngameWnd::~CUIMainIngameWnd()
{
	DestroyFlashingIcons		();
	xr_delete					(UIZoneMap);
	HUD_SOUND_ITEM::DestroySound(m_contactSnd);
	xr_delete					(g_MissileForceShape);

	xr_delete(UIInvincibleIcon);
	xr_delete(UIWoundIcon);
	xr_delete(UIRadiaitionIcon);
	xr_delete(UIWeaponJammedIcon);
	xr_delete(UIPsyHealthIcon);
	xr_delete(UIStarvationIcon);
}

void CUIMainIngameWnd::Init()
{
	CUIXml						uiXml;
	uiXml.Init					(CONFIG_PATH, UI_PATH, MAININGAME_XML);
	
	CUIXmlInit					xml_init;
	CUIWindow::Init				(0,0, UI_BASE_WIDTH, UI_BASE_HEIGHT);

	Enable(false);

	UIPickUpItemIcon = UIHelper::CreateStatic(uiXml, "pick_up_item", this);
	UIPickUpItemIcon->SetShader(GetEquipmentIconsShader());

	m_iPickUpItemIconWidth = UIPickUpItemIcon->GetWidth();
	m_iPickUpItemIconHeight = UIPickUpItemIcon->GetHeight();
	m_iPickUpItemIconX = UIPickUpItemIcon->GetWndRect().left;
	m_iPickUpItemIconY = UIPickUpItemIcon->GetWndRect().top;

	//индикаторы 
	UIZoneMap->Init				();
	UIZoneMap->SetScale			(DEFAULT_MAP_SCALE);

	xml_init.InitStatic					(uiXml, "static_pda_online", 0, &UIPdaOnline);
	UIZoneMap->Background().AttachChild	(&UIPdaOnline);

	// Подсказки, которые возникают при наведении прицела на объект
	UIStaticQuickHelp			= UIHelper::CreateTextWnd(uiXml, "quick_info", this);

	uiXml.SetLocalRoot			(uiXml.GetRoot());

	m_UIIcons					= xr_new<CUIScrollView>(); m_UIIcons->SetAutoDelete(true);
	xml_init.InitScrollView		(uiXml, "icons_scroll_view", 0, m_UIIcons);
	AttachChild					(m_UIIcons);

	// Загружаем иконки 
	UIStarvationIcon = UIHelper::CreateStatic(uiXml, "starvation_static", nullptr);
	UIStarvationIcon->Show(false);

	UIPsyHealthIcon = UIHelper::CreateStatic(uiXml, "psy_health_static", nullptr);
	UIPsyHealthIcon->Show(false);

	UIWeaponJammedIcon = UIHelper::CreateStatic(uiXml, "weapon_jammed_static", nullptr);
	UIWeaponJammedIcon->Show(false);

	UIRadiaitionIcon = UIHelper::CreateStatic(uiXml, "radiation_static", nullptr);
	UIRadiaitionIcon->Show(false);

	UIWoundIcon = UIHelper::CreateStatic(uiXml, "wound_static", nullptr);
	UIWoundIcon->Show(false);

	UIInvincibleIcon = UIHelper::CreateStatic(uiXml, "invincible_static", nullptr);
	UIInvincibleIcon->Show(false);
	
	shared_str warningStrings[6] = 
	{	
		"jammed",
		"radiation",
		"wounds",
		"starvation",
		"fatigue",
		"invincible"
	};

	// Загружаем пороговые значения для индикаторов
	EWarningIcons j = ewiWeaponJammed;
	while (j < ewiInvincible)
	{
		// Читаем данные порогов для каждого индикатора
		shared_str cfgRecord = pSettings->r_string("main_ingame_indicators_thresholds", *warningStrings[static_cast<int>(j)]);
		u32 count = _GetItemCount(*cfgRecord);

		char	singleThreshold[8];
		float	f = 0;
		for (u32 k = 0; k < count; ++k)
		{
			_GetItem(*cfgRecord, k, singleThreshold);
			sscanf(singleThreshold, "%f", &f);

			m_Thresholds[j].push_back(f);
		}

		j = static_cast<EWarningIcons>(j + 1);
	}


	// Flashing icons initialize
	uiXml.SetLocalRoot						(uiXml.NavigateToNode("flashing_icons"));
	InitFlashingIcons						(&uiXml);

	uiXml.SetLocalRoot						(uiXml.GetRoot());

	UIMotionIcon = xr_new<CUIMotionIcon>(); UIMotionIcon->SetAutoDelete(true);
	AttachChild(UIMotionIcon);
	UIMotionIcon->Init();

	UIStaticDiskIO = UIHelper::CreateStatic(uiXml, "disk_io", this);

	m_ui_hud_states = xr_new<CUIHudStatesWnd>();
	m_ui_hud_states->SetAutoDelete(true);
	AttachChild(m_ui_hud_states);
	m_ui_hud_states->InitFromXml(uiXml, "hud_states");

	HUD_SOUND_ITEM::LoadSound("maingame_ui", "snd_new_contact", m_contactSnd, SOUND_TYPE_IDLE);
}

float UIStaticDiskIO_start_time = 0.0f;
void CUIMainIngameWnd::Draw()
{
	// show IO icon
	bool IOActive	= (FS.dwOpenCounter>0);
	if	(IOActive)	UIStaticDiskIO_start_time = Device.fTimeGlobal;

	if ((UIStaticDiskIO_start_time+1.0f) < Device.fTimeGlobal)	UIStaticDiskIO->Show(false); 
	else {
		u32		alpha			= clampr(iFloor(255.f*(1.f-(Device.fTimeGlobal-UIStaticDiskIO_start_time)/1.f)),0,255);
		UIStaticDiskIO->Show		( true  ); 
		UIStaticDiskIO->SetTextureColor(color_rgba(255,255,255,alpha));
	}
	FS.dwOpenCounter = 0;

	if (!Level().CurrentViewEntity())
		return;

	CActor* pActor = static_cast<CGameObject*>(Level().CurrentViewEntity())->cast_actor();
	if (!pActor || !pActor->g_Alive()) return;

	UIMotionIcon->SetNoise((s16)(0xffff&iFloor(pActor->m_snd_noise*100)));
	CUIWindow::Draw				();
	UIZoneMap->Render			();			

	RenderQuickInfos			(pActor);		
}

void CUIMainIngameWnd::Update()
{
	CUIWindow::Update();

	if (!Level().CurrentViewEntity())
		return;

	CActor* pActor = static_cast<CGameObject*>(Level().CurrentViewEntity())->cast_actor();
	if (!pActor)
		return;

	UIMotionIcon->SetPower(pActor->conditions().GetPower() * 100.0f);

	UIZoneMap->UpdateRadar(Device.vCameraPosition);
	float h, p;
	Device.vCameraDirection.getHP(h, p);
	UIZoneMap->SetHeading(-h);

	UpdatePickUpItem();

	if (!(Device.dwFrame % 30))
	{
		string256				text_str;
		CPda* _pda = pActor->GetPDA();
		u32 _cn = 0;
		if (_pda && 0 != (_cn = _pda->ActiveContactsNum()))
		{
			sprintf_s(text_str, "%d", _cn);
			UIPdaOnline.SetText(text_str);
		}
		else
		{
			UIPdaOnline.SetText("");
		}
	}

	if (Device.dwFrame % 10)
		return;

	bool b_God = GodMode();
	if(b_God)
		SetWarningIconColor	(ewiInvincible,0xffffffff);
	else
		SetWarningIconColor	(ewiInvincible,0x00ffffff);

	UpdateMainIndicators(pActor);
}

bool CUIMainIngameWnd::KeyboardIngameWnd(int dik)
{
	switch (dik)
	{
	case SDL_SCANCODE_KP_MINUS:
		if (pInput->GetModState(SDL_KMOD_SHIFT))
			UIZoneMap->ZoomOut();
		else
			CurrentGameUI()->ShowGameIndicators(false);

		return true;
	case SDL_SCANCODE_KP_PLUS:
		if (pInput->GetModState(SDL_KMOD_SHIFT))
			UIZoneMap->ZoomIn();
		else
			CurrentGameUI()->ShowGameIndicators(true);

		return true;
	}

	return false;
}


void CUIMainIngameWnd::RenderQuickInfos(CActor* actor)
{
	if (!actor)
		return;

	static CGameObject *pObject			= NULL;
	LPCSTR actor_action					= actor->GetDefaultActionForObject();
	UIStaticQuickHelp->Show				(NULL!=actor_action);

	if(NULL!=actor_action)
	{
		if(stricmp(actor_action,UIStaticQuickHelp->GetText()))
			UIStaticQuickHelp->SetTextST				(actor_action);
	}

	if(pObject!=actor->ObjectWeLookingAt())
	{
		UIStaticQuickHelp->SetTextST				(actor_action?actor_action:" ");
		UIStaticQuickHelp->ResetColorAnimation	();
		pObject	= actor->ObjectWeLookingAt	();
	}
}

void CUIMainIngameWnd::ReceiveNews(GAME_NEWS_DATA* news)
{
	VERIFY(news->texture_name.size());

	CurrentGameUI()->m_pMessagesWnd->AddIconedPdaMessage(news);
//	CurrentGameUI()->UpdatePda();
}

void CUIMainIngameWnd::SetWarningIconColorUI(CUIStatic* s, const u32 cl)
{
	int bOn = (cl>>24);
	bool bIsShown = s->IsShown();

	if ( bOn )
	{
		s->SetTextureColor( cl );
	}

	if(bOn&&!bIsShown){
		m_UIIcons->AddWindow	(s, false);
		s->Show					(true);
	}

	if(!bOn&&bIsShown){
		m_UIIcons->RemoveWindow	(s);
		s->Show					(false);
	}
}

void CUIMainIngameWnd::SetWarningIconColor(EWarningIcons icon, const u32 cl)
{
	// Задаем цвет требуемой иконки
	switch(icon)
	{
	case ewiWeaponJammed:
		SetWarningIconColorUI(UIWeaponJammedIcon, cl);
		break;
	case ewiRadiation:
		SetWarningIconColorUI(UIRadiaitionIcon, cl);
		break;
	case ewiWound:
		SetWarningIconColorUI(UIWoundIcon, cl);
		break;
	case ewiStarvation:
		SetWarningIconColorUI(UIStarvationIcon, cl);
		break;	
	case ewiPsyHealth:
		SetWarningIconColorUI(UIPsyHealthIcon, cl);
		break;
	case ewiInvincible:
		SetWarningIconColorUI(UIInvincibleIcon, cl);
		break;
	default:
		R_ASSERT(!"Unknown warning icon type");
		break;
	}
}

void CUIMainIngameWnd::TurnOffWarningIcon(EWarningIcons icon)
{
	SetWarningIconColor(icon, 0x00ffffff);
}


void CUIMainIngameWnd::SetFlashIconState_(EFlashingIcons type, bool enable)
{
	// Включаем анимацию требуемой иконки
	FlashingIcons_it icon = m_FlashingIcons.find(type);
	R_ASSERT2(icon != m_FlashingIcons.end(), "Flashing icon with this type not existed");
	icon->second->Show(enable);
}

void CUIMainIngameWnd::InitFlashingIcons(CUIXml* node)
{
	const char * const flashingIconNodeName = "flashing_icon";
	int staticsCount = node->GetNodesNum("", 0, flashingIconNodeName);

	CUIXmlInit xml_init;
	CUIStatic *pIcon = NULL;
	// Пробегаемся по всем нодам и инициализируем из них статики
	for (int i = 0; i < staticsCount; ++i)
	{
		pIcon = xr_new<CUIStatic>();
		xml_init.InitStatic(*node, flashingIconNodeName, i, pIcon);
		shared_str iconType = node->ReadAttrib(flashingIconNodeName, i, "type", "none");

		// Теперь запоминаем иконку и ее тип
		EFlashingIcons type = efiPdaTask;

		if		(iconType == "pda")		type = efiPdaTask;
		else if (iconType == "mail")	type = efiMail;
		else	R_ASSERT(!"Unknown type of mainingame flashing icon");

		R_ASSERT2(m_FlashingIcons.find(type) == m_FlashingIcons.end(), "Flashing icon with this type already exists");

		CUIStatic* &val	= m_FlashingIcons[type];
		val			= pIcon;

		AttachChild(pIcon);
		pIcon->Show(false);
	}
}

void CUIMainIngameWnd::DestroyFlashingIcons()
{
	for (FlashingIcons_it it = m_FlashingIcons.begin(); it != m_FlashingIcons.end(); ++it)
	{
		DetachChild(it->second);
		xr_delete(it->second);
	}

	m_FlashingIcons.clear();
}

void CUIMainIngameWnd::UpdateFlashingIcons()
{
	for (FlashingIcons_it it = m_FlashingIcons.begin(); it != m_FlashingIcons.end(); ++it)
	{
		it->second->Update();
	}
}

void CUIMainIngameWnd::AnimateContacts(bool b_snd)
{
	UIPdaOnline.ResetColorAnimation();

	if(b_snd)
		HUD_SOUND_ITEM::PlaySound	(m_contactSnd, Fvector().set(0,0,0), 0, true );
}


void CUIMainIngameWnd::SetPickUpItem	(CInventoryItem* PickUpItem)
{
	m_pPickUpItem = PickUpItem;
};

void CUIMainIngameWnd::UpdatePickUpItem	()
{
	if (!m_pPickUpItem || !Level().CurrentViewEntity() || Level().CurrentViewEntity()->CLS_ID != CLSID_OBJECT_ACTOR) 
	{
		UIPickUpItemIcon->Show(false);
		return;
	};


	shared_str sect_name	= m_pPickUpItem->object().cNameSect();

	//properties used by inventory menu
	int m_iGridWidth	= pSettings->r_u32(sect_name, "inv_grid_width");
	int m_iGridHeight	= pSettings->r_u32(sect_name, "inv_grid_height");

	int m_iXPos			= pSettings->r_u32(sect_name, "inv_grid_x");
	int m_iYPos			= pSettings->r_u32(sect_name, "inv_grid_y");

	float scale_x = m_iPickUpItemIconWidth/
		float(m_iGridWidth*INV_GRID_WIDTH);

	float scale_y = m_iPickUpItemIconHeight/
		float(m_iGridHeight*INV_GRID_HEIGHT);

	scale_x = (scale_x>1) ? 1.0f : scale_x;
	scale_y = (scale_y>1) ? 1.0f : scale_y;

	float scale = scale_x<scale_y?scale_x:scale_y;

	Frect texture_rect;
	texture_rect.lt.set(m_iXPos*INV_GRID_WIDTH, m_iYPos*INV_GRID_HEIGHT);
	texture_rect.rb.set(m_iGridWidth*INV_GRID_WIDTH, m_iGridHeight*INV_GRID_HEIGHT);
	texture_rect.rb.add(texture_rect.lt);
	UIPickUpItemIcon->GetStaticItem()->SetTextureRect(texture_rect);
	UIPickUpItemIcon->SetStretchTexture(true);


	UIPickUpItemIcon->SetWidth(m_iGridWidth*INV_GRID_WIDTH*scale*UI().get_current_kx());
	UIPickUpItemIcon->SetHeight(m_iGridHeight*INV_GRID_HEIGHT*scale);

	UIPickUpItemIcon->SetWndPos(Fvector2().set(	m_iPickUpItemIconX+(m_iPickUpItemIconWidth-UIPickUpItemIcon->GetWidth())/2.0f,
												m_iPickUpItemIconY+(m_iPickUpItemIconHeight-UIPickUpItemIcon->GetHeight())/2.0f) );

	UIPickUpItemIcon->SetTextureColor(color_rgba(255,255,255,192));
	UIPickUpItemIcon->Show(true);
};

void CUIMainIngameWnd::OnConnected()
{
	UIZoneMap->SetupCurrentMap();
}

void CUIMainIngameWnd::reset_ui()
{
	m_pPickUpItem					= NULL;
	UIMotionIcon->ResetVisibility	();
}

void CUIMainIngameWnd::UpdateMainIndicators(const CActor* actor)
{
	if (!actor)
		return;

	EWarningIcons i = ewiWeaponJammed;

	while (i < ewiInvincible)
	{
		float value = 0;
		switch (i)
		{
			//radiation
		case ewiRadiation:
			value = actor->conditions().GetRadiation();
			break;
		case ewiWound:
			value = actor->conditions().BleedingSpeed();
			break;
		case ewiWeaponJammed:
		{
			if (!actor->inventory().ActiveItem())
				break;
				
			CWeapon* item = actor->inventory().ActiveItem()->cast_weapon();
			if (item)
				value = 1 - item->GetConditionToShow();
		}
			break;
		case ewiStarvation:
			value = 1 - actor->conditions().GetSatiety();
			break;
		case ewiPsyHealth:
			value = 1 - actor->conditions().GetPsyHealth();
			break;
		default:
			R_ASSERT(!"Unknown type of warning icon");
		}

		xr_vector<float>::reverse_iterator	rit;

		// Сначала проверяем на точное соответсвие
		rit = std::find(m_Thresholds[i].rbegin(), m_Thresholds[i].rend(), value);

		// Если его нет, то берем последнее меньшее значение ()
		if (rit == m_Thresholds[i].rend())
			rit = std::find_if(m_Thresholds[i].rbegin(), m_Thresholds[i].rend(), std::bind(std::less<float>(), std::placeholders::_1, value));

		// Минимальное и максимальное значения границы
		float min = m_Thresholds[i].front();
		float max = m_Thresholds[i].back();

		if (rit != m_Thresholds[i].rend()) {
			float v = *rit;
			SetWarningIconColor(i, color_argb(0xFF, clampr<u32>(static_cast<u32>(255 * ((v - min) / (max - min) * 2)), 0, 255),
				clampr<u32>(static_cast<u32>(255 * (2.0f - (v - min) / (max - min) * 2)), 0, 255),
				0));
		}
		else
			TurnOffWarningIcon(i);

		i = (EWarningIcons)(i + 1);
	}
}