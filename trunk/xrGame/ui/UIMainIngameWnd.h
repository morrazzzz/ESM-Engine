#pragma once

#include "UIProgressBar.h"
#include "UIGameLog.h"

#include "../alife_space.h"

#include "UIMotionIcon.h"
#include "../hudsound.h"
class					CUIPdaMsgListItem;
class					CLAItem;
class					CUIZoneMap;
class					CUIArtefactPanel;
class					CUIScrollView;
struct					GAME_NEWS_DATA;
class CActor;
class					CMissile;
class					CInventoryItem;
class					CUIHudStatesWnd;

class CUIMainIngameWnd: public CUIWindow  
{
public:
	CUIMainIngameWnd();
	virtual ~CUIMainIngameWnd();

	virtual void Init();
	virtual void Draw();
	virtual void Update();

	bool KeyboardIngameWnd(int dik);
protected:
	
	CUIStatic*			UIStaticDiskIO;
	CUITextWnd*			UIStaticQuickHelp;
	CUIMotionIcon*		UIMotionIcon;
	CUIZoneMap*			UIZoneMap;

	CUIHudStatesWnd*	m_ui_hud_states;

	//иконка, показывающая количество активных PDA
	CUIStatic			UIPdaOnline;
public:
	CUIHudStatesWnd*	get_hud_states() { return m_ui_hud_states; } //temp
protected:

	// 5 статиков для отображения иконок:
	// - сломанного оружия
	// - радиации
	// - ранения
	// - голода
	// - усталости
	CUIStatic*			UIWeaponJammedIcon;
	CUIStatic*			UIRadiaitionIcon;
	CUIStatic*			UIWoundIcon;
	CUIStatic*			UIStarvationIcon;
	CUIStatic*			UIPsyHealthIcon;
	CUIStatic*			UIInvincibleIcon;

	CUIScrollView*		m_UIIcons;
public:
	
	// Енумы соответсвующие предупреждающим иконкам 
	enum EWarningIcons
	{
		ewiWeaponJammed,
		ewiRadiation,
		ewiWound,
		ewiStarvation,
		ewiPsyHealth,
		ewiInvincible,
//		ewiSleep,
	};

	// Задаем цвет соответствующей иконке
	void				SetWarningIconColor				(EWarningIcons icon, const u32 cl);
	void				TurnOffWarningIcon				(EWarningIcons icon);

	// Пороги изменения цвета индикаторов, загружаемые из system.ltx
	typedef				xr_map<EWarningIcons, xr_vector<float> >	Thresholds;
	typedef				Thresholds::iterator						Thresholds_it;
	Thresholds			m_Thresholds;

	// Енум перечисления возможных мигающих иконок
	enum EFlashingIcons
	{
		efiPdaTask	= 0,
		efiMail
	};
	
	void				SetFlashIconState_				(EFlashingIcons type, bool enable);

	void				AnimateContacts					(bool b_snd);
	HUD_SOUND_ITEM		m_contactSnd;

	void				ReceiveNews						(GAME_NEWS_DATA* news);
	void				UpdateMainIndicators			(const CActor* actor);
protected:
//	void				UpdateQuickSlots				();
	void				SetWarningIconColorUI			(CUIStatic* s, const u32 cl);
	void				InitFlashingIcons				(CUIXml* node);
	void				DestroyFlashingIcons			();
	void				UpdateFlashingIcons				();
//	void				UpdateActiveItemInfo			();

//	void				SetAmmoIcon						(const shared_str& seсt_name);

	// first - иконка, second - анимация
	DEF_MAP				(FlashingIcons, EFlashingIcons, CUIStatic*);
	FlashingIcons		m_FlashingIcons;

//	CMissile*			m_pGrenade;
//	CInventoryItem*		m_pItem;

	// Отображение подсказок при наведении прицела на объект
	void				RenderQuickInfos(CActor* actor);

public:
	CUIMotionIcon*		MotionIcon							(){return UIMotionIcon;}
	void				OnConnected							();
	void				reset_ui							();
protected:
	CInventoryItem*		m_pPickUpItem;
	CUIStatic*			UIPickUpItemIcon;

	float				m_iPickUpItemIconX;
	float				m_iPickUpItemIconY;
	float				m_iPickUpItemIconWidth;
	float				m_iPickUpItemIconHeight;

	void				UpdatePickUpItem();
public:
	void				SetPickUpItem	(CInventoryItem* PickUpItem);
};