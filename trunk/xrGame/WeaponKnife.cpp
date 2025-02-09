#include "stdafx.h"

#include "WeaponKnife.h"
#include "Entity.h"
#include "Actor.h"
#include "level.h"
#include "xr_level_controller.h"
#include "../xr_3da/GameMtlLib.h"
#include "level_bullet_manager.h"
#include "ai_sounds.h"
#include "game_cl_single.h"

#define KNIFE_MATERIAL_NAME "objects\\knife"

CWeaponKnife::CWeaponKnife()
{
	SetState				( eHidden );
	SetNextState			( eHidden );
	knife_material_idx		= (u16)-1;
}
CWeaponKnife::~CWeaponKnife()
{
}

void CWeaponKnife::Load	(LPCSTR section)
{
	// verify class
	inherited::Load		(section);

	fWallmarkSize = pSettings->r_float(section,"wm_size");
	m_sounds.LoadSound(section,"snd_shoot"		, "sndShot"		, false, SOUND_TYPE_WEAPON_SHOOTING		);

	knife_material_idx =  GMLib.GetMaterialIdx(KNIFE_MATERIAL_NAME);
}

void CWeaponKnife::OnStateSwitch	(u32 S)
{
	inherited::OnStateSwitch(S);
	switch (S)
	{
	case eIdle:
		switch2_Idle	();
		break;
	case eShowing:
		switch2_Showing	();
		break;
	case eHiding:
		switch2_Hiding	();
		break;
	case eHidden:
		switch2_Hidden	();
		break;
	case eFire:
		{
			//-------------------------------------------
			m_eHitType		= m_eHitType_1;
			//fHitPower		= fHitPower_1;
			if (ParentIsActor())
			{
				if (GameID() == GAME_SINGLE)
				{
					fCurrentHit		= fvHitPower_1[g_SingleGameDifficulty];
				}
				else
				{
					fCurrentHit		= fvHitPower_1[egdMaster];
				}
			}
			else
			{
				fCurrentHit		= fvHitPower_1[egdMaster];
			}
			fHitImpulse		= fHitImpulse_1;
			//-------------------------------------------
			switch2_Attacking	(S);
		}break;
	case eFire2:
		{
			//-------------------------------------------
			m_eHitType		= m_eHitType_2;
			//fHitPower		= fHitPower_2;
			if (ParentIsActor())
			{
				if (GameID() == GAME_SINGLE)
				{
					fCurrentHit		= fvHitPower_2[g_SingleGameDifficulty];
				}
				else
				{
					fCurrentHit		= fvHitPower_2[egdMaster];
				}
			}
			else
			{
				fCurrentHit		= fvHitPower_2[egdMaster];
			}
			fHitImpulse		= fHitImpulse_2;
			//-------------------------------------------
			switch2_Attacking	(S);
		}break;
	}
}
	

void CWeaponKnife::KnifeStrike(const Fvector& pos, const Fvector& dir)
{
	CCartridge						cartridge; 
	cartridge.m_buckShot			= 1;				
	cartridge.m_impair				= 1;
	cartridge.m_kDisp				= 1;
	cartridge.m_kHit				= 1;
	cartridge.m_kImpulse			= 1;
	cartridge.m_kPierce				= 1;
	cartridge.m_flags.set			(CCartridge::cfTracer, FALSE);
	cartridge.m_flags.set			(CCartridge::cfRicochet, FALSE);
	cartridge.fWallmarkSize			= fWallmarkSize;
	cartridge.bullet_material_idx	= knife_material_idx;

	while(m_magazine.size() < 2)	m_magazine.push_back(cartridge);
	iAmmoElapsed					= m_magazine.size();
	bool SendHit					= SendHitAllowed(H_Parent());

	PlaySound						("sndShot",pos);

	Level().BulletManager().AddBullet(	pos, 
										dir, 
										m_fStartBulletSpeed, 
										fCurrentHit, 
										fHitImpulse, 
										H_Parent()->ID(), 
										ID(), 
										m_eHitType, 
										fireDistance, 
										cartridge, 
										SendHit);
}

void CWeaponKnife::OnMotionMark(u32 state)
{
	inherited::OnMotionMark(state);
		/*
	if (state == eFire)
	{
		m_hit_dist		=	m_Hit1Distance;
		m_splash_dir	=	m_Hit1SpashDir;
		m_splash_radius	=	m_Hit1SplashRadius;
		m_hits_count	=	m_Splash1HitsCount;
		m_perv_hits_count = m_Splash1PerVictimsHCount;
	} else if (state == eFire2)
	{
		m_hit_dist		=	m_Hit2Distance;
		m_splash_dir	=	m_Hit2SpashDir;
		m_splash_radius	=	m_Hit2SplashRadius;
		m_hits_count	=	m_Splash2HitsCount;
		m_perv_hits_count = 0;
	} else
	{
		return;
	}
	*/

	if (state != eFire && state != eFire2)
		return;

	Fvector	p1, d; 
	p1.set			(get_LastFP()); 
	d.set			(get_LastFD());
	//fireDistance	= m_hit_dist + m_splash_radius;

	if(H_Parent())
	{
		smart_cast<CEntity*>(H_Parent())->g_fireParams(this, p1,d);
		KnifeStrike(p1,d);
	}
}

void CWeaponKnife::OnAnimationEnd(u32 state)
{
	switch (state)
	{
	case eHiding:	SwitchState(eHidden);	break;
	case eFire: 
	case eFire2: 	SwitchState(eIdle);		break;

	case eShowing:
	case eIdle:		SwitchState(eIdle);		break;	

	default:		inherited::OnAnimationEnd(state);
	}
}

void CWeaponKnife::state_Attacking	(float)
{
}

void CWeaponKnife::switch2_Attacking	(u32 state)
{
	if(IsPending())	return;

	if(state==eFire)
		PlayHUDMotion("anim_shoot1_start", "anm_attack", false, state);
	else //eFire2
		PlayHUDMotion("anim_shoot2_start", "anm_attack2", false, state);

	SetPending			(TRUE);
}

void CWeaponKnife::switch2_Idle	()
{
	VERIFY(GetState()==eIdle);

	PlayAnimIdle		();
	SetPending			(FALSE);
}

void CWeaponKnife::switch2_Hiding	()
{
	FireEnd					();
	VERIFY(GetState()==eHiding);
	PlayHUDMotion("anim_hide", "anm_hide", true, GetState());
}

void CWeaponKnife::switch2_Hidden()
{
	signal_HideComplete		();
	SetPending				(FALSE);
}

void CWeaponKnife::switch2_Showing	()
{
	VERIFY(GetState()==eShowing);
	PlayHUDMotion("anim_draw", "anm_show", false, GetState());
}


void CWeaponKnife::FireStart()
{	
	inherited::FireStart();
	SwitchState			(eFire);
}

void CWeaponKnife::Fire2Start () 
{
	SwitchState(eFire2);
}


bool CWeaponKnife::Action(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;
	switch(cmd) 
	{

		case kWPN_ZOOM : 
			if(flags&CMD_START) 
				Fire2Start			();

			return true;
	}
	return false;
}

void CWeaponKnife::LoadFireParams(LPCSTR section)
{
	inherited::LoadFireParams(section);

	string256			full_name;
	string32			buffer;
	shared_str			s_sHitPower_2;
	//fHitPower_1		= fHitPower;
	fvHitPower_1		= fvHitPower;
	fHitImpulse_1		= fHitImpulse;
	m_eHitType_1		= ALife::g_tfString2HitType(pSettings->r_string(section, "hit_type"));

	//fHitPower_2			= pSettings->r_float	(section,strconcat(full_name, prefix, "hit_power_2"));
	s_sHitPower_2			= pSettings->r_string_wb	(section, "hit_power_2" );
	//s_sHitPowerCritical_2	= pSettings->r_string_wb	(section, "hit_power_critical_2" );
	
	fvHitPower_2[egdMaster]			= (float)atof(_GetItem(*s_sHitPower_2,0,buffer));//первый параметр - это хит для уровня игры мастер
	//fvHitPowerCritical_2[egdMaster]	= (float)atof(_GetItem(*s_sHitPowerCritical_2,0,buffer));//первый параметр - это хит для уровня игры мастер

	fvHitPower_2[egdNovice] = fvHitPower_2[egdStalker] = fvHitPower_2[egdVeteran] = fvHitPower_2[egdMaster];//изначально параметры для других уровней сложности такие же
	//fvHitPowerCritical_2[egdNovice] = fvHitPowerCritical_2[egdStalker] = fvHitPowerCritical_2[egdVeteran] = fvHitPowerCritical_2[egdMaster];//изначально параметры для других уровней сложности такие же

	int num_game_diff_param=_GetItemCount(*s_sHitPower_2);//узнаём колличество параметров для хитов
	if (num_game_diff_param>1)//если задан второй параметр хита
	{
		fvHitPower_2[egdVeteran]	= (float)atof(_GetItem(*s_sHitPower_2,1,buffer));//то вычитываем его для уровня ветерана
	}
	if (num_game_diff_param>2)//если задан третий параметр хита
	{
		fvHitPower_2[egdStalker]	= (float)atof(_GetItem(*s_sHitPower_2,2,buffer));//то вычитываем его для уровня сталкера
	}
	if (num_game_diff_param>3)//если задан четвёртый параметр хита
	{
		fvHitPower_2[egdNovice]	= (float)atof(_GetItem(*s_sHitPower_2,3,buffer));//то вычитываем его для уровня новичка
	}

	fHitImpulse_2		= pSettings->r_float	(section, "hit_impulse_2" );
	m_eHitType_2		= ALife::g_tfString2HitType(pSettings->r_string(section, "hit_type_2"));
}

void CWeaponKnife::GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count)
{
	str_name		= NameShort();
	str_count		= "";
	icon_sect_name	= *cNameSect();
}
