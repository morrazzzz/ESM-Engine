#include "stdafx.h"
#include "weaponBM16.h"

CWeaponBM16::~CWeaponBM16()
{
}

void CWeaponBM16::Load(LPCSTR section)
{
	inherited::Load(section);
	m_sounds.LoadSound(section, "snd_reload_1", "sndReload1", true, m_eSoundShot);

	AllowAnmReload1 = pSettings->line_exist(section, "anm_reload_1");
}

static LPCSTR const animsBoreBM16[]{ "anm_bore_0", "anm_bore_1", "anm_bore_2"};

void CWeaponBM16::SetAllowBoreAnm(LPCSTR section)
{
	for (size_t i = 0; i < 3; i++)
	{
		if (!pSettings->line_exist(section, animsBoreBM16[i]))
			return;
	}

	AllowBoreAnm = true;
}

void CWeaponBM16::PlayReloadSound()
{
	if(m_magazine.size()==1)	
		PlaySound	("sndReload1",get_LastFP());
	else						
		PlaySound	("sndReload",get_LastFP());
}

void CWeaponBM16::PlayAnimShoot()
{
	switch( m_magazine.size() )
	{
	case 1:
		PlayHUDMotion("anim_shoot_1", "anm_shot_1", false, GetState());
		break;
	case 2:
		PlayHUDMotion("anim_shoot", "anm_shot_2", false, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimShow()
{
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anim_draw", "anm_show_0", true, GetState());
		break;
	case 1:
		PlayHUDMotion("anim_draw", "anm_show_1", true, GetState());
		break;
	case 2:
		PlayHUDMotion("anim_draw", "anm_show_2", true, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimHide()
{
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anim_holster", "anm_hide_0", true, GetState());
		break;
	case 1:
		PlayHUDMotion("anim_holster", "anm_hide_1", true, GetState());
		break;
	case 2:
		PlayHUDMotion("anim_holster", "anm_hide_2", true, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimBore()
{
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anm_bore_0",true, GetState());
		break;
	case 1:
		PlayHUDMotion("anm_bore_1",true, GetState());
		break;
	case 2:
		PlayHUDMotion("anm_bore_2",true, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimReload()
{
	bool b_both = HaveCartridgeInInventory(2);

	VERIFY(GetState()==eReload);
	
	if (m_magazine.size() == 1 || !b_both)
	{
		if (AllowAnmReload1 && m_set_next_ammoType_on_reload == undefined_ammo_type ||
			m_ammoType == m_set_next_ammoType_on_reload)
			PlayHUDMotion("anm_reload_1", true, GetState());

		if (!AllowAnmReload1)
			PlayHUDMotion("anim_reload_1", true, GetState());
	}
	else
		PlayHUDMotion("anim_reload", "anm_reload_2", true, GetState());
}

void  CWeaponBM16::PlayAnimIdleMoving()
{
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anim_idle", "anm_idle_moving_0",true, GetState());
		break;
	case 1:
		PlayHUDMotion("anim_idle_1", "anm_idle_moving_1", true, GetState());
		break;
	case 2:
		PlayHUDMotion("anim_idle_2", "anm_idle_moving_2",true, GetState());
		break;
	}
}

void  CWeaponBM16::PlayAnimIdleSprint()
{
	switch( m_magazine.size() )
	{
	case 0:
		PlayHUDMotion("anim_idle_sprint", "anm_idle_sprint_0", true, GetState());
		break;
	case 1:
		PlayHUDMotion("anim_idle_sprint", "anm_idle_sprint_1", true, GetState());
		break;
	case 2:
		PlayHUDMotion("anim_idle_sprint", "anm_idle_sprint_2",true, GetState());
		break;
	}
}

void CWeaponBM16::PlayAnimIdle()
{
	if(TryPlayAnimIdle())	return;

	if(IsZoomed())
	{
		switch (m_magazine.size())
		{
		case 0:{
			PlayHUDMotion("anim_idle_aim", "anm_idle_aim_0", true, GetState());
		}break;
		case 1:{
			PlayHUDMotion("anim_zoomed_idle_1", "anm_idle_aim_1", true, GetState());
		}break;
		case 2:{
			PlayHUDMotion("anim_zoomedidle_2", "anm_idle_aim_2", true, GetState());
		}break;
		};
	}else{
		switch (m_magazine.size())
		{
		case 0:{
			PlayHUDMotion("anim_idle", "anm_idle_0", true, GetState());
		}break;
		case 1:{
			PlayHUDMotion("anim_idle_1", "anm_idle_1", true, GetState());
		}break;
		case 2:{
			PlayHUDMotion("anim_idle_2", "anm_idle_2", true, GetState());
		}break;
		};
	}
}
