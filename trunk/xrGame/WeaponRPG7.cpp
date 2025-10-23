#include "stdafx.h"
#include "weaponrpg7.h"
#include "xrserver_objects_alife_items.h"
#include "explosiverocket.h"
#include "entity.h"
#include "player_hud.h"
#include "ai_object_location.h"

CWeaponRPG7::CWeaponRPG7()
{
}

CWeaponRPG7::~CWeaponRPG7() 
{
}

void CWeaponRPG7::Load	(LPCSTR section)
{
	inherited::Load			(section);
	CRocketLauncher::Load	(section);

	m_zoom_params.m_fScopeZoomFactor	= pSettings->r_float	(section,"max_zoom_factor");
	m_sGrenadeBoneName		= pSettings->r_string	(section,"grenade_bone");

	m_sRocketSection		= pSettings->r_string	(section,"rocket_class");
}

bool CWeaponRPG7::AllowBore()
{
	return inherited::AllowBore() && 0!=iAmmoElapsed;
}

void CWeaponRPG7::FireTrace(const Fvector& P, const Fvector& D)
{
	inherited::FireTrace	(P, D);
	UpdateMissileVisibility	();
}

void CWeaponRPG7::on_a_hud_attach()
{
	inherited::on_a_hud_attach		();
	UpdateMissileVisibility			();
}

void CWeaponRPG7::UpdateMissileVisibility()
{
	bool vis_hud,vis_weap;
	vis_hud		= (!!iAmmoElapsed || GetState()==eReload);
	vis_weap	= !!iAmmoElapsed;

	if(GetHUDmode())
	{
		HudItemData()->set_bone_visible(m_sGrenadeBoneName.c_str(), vis_hud, TRUE);
	}

	IKinematics* pWeaponVisual	= smart_cast<IKinematics*>(Visual()); 
	VERIFY						(pWeaponVisual);
	pWeaponVisual->LL_SetBoneVisible(pWeaponVisual->LL_BoneID(m_sGrenadeBoneName.c_str()), vis_weap, TRUE);
}

BOOL CWeaponRPG7::net_Spawn(CSE_Abstract* DC) 
{
	BOOL l_res = inherited::net_Spawn(DC);

	UpdateMissileVisibility();
	if(iAmmoElapsed && !getCurrentRocket())
	{
		CSE_Abstract* object = Level().spawn_item(m_sRocketSection.c_str(), Position(), ai_location().level_vertex_id(), ID(), true);
		R_ASSERT(object);

		CSE_ALifeObject* alife_object = object->cast_alife_object();
		R_ASSERT(alife_object);
		alife_object->m_flags.set(CSE_ALifeObject::flCanSave, false);

		NET_Packet			P;
		object->Spawn_Write(P, TRUE);
		Level().Send(P);
		F_entity_Destroy(object);
	}

	return l_res;
}

void CWeaponRPG7::OnStateSwitch(u32 S) 
{
	inherited::OnStateSwitch(S);
	UpdateMissileVisibility();
}

void CWeaponRPG7::UnloadMagazine(bool spawn_ammo)
{
	inherited::UnloadMagazine	(spawn_ammo);
	UpdateMissileVisibility		();
}

void CWeaponRPG7::ReloadMagazine() 
{
	inherited::ReloadMagazine();

	if (iAmmoElapsed && !getRocketCount())
	{
		CSE_Abstract* object = Level().spawn_item(m_sRocketSection.c_str(), Position(), ai_location().level_vertex_id(), ID(), true);
		R_ASSERT(object);

		CSE_ALifeObject* alife_object = object->cast_alife_object();
		R_ASSERT(alife_object);
		alife_object->m_flags.set(CSE_ALifeObject::flCanSave, false);

		NET_Packet			P;
		object->Spawn_Write(P, TRUE);
		Level().Send(P);
		F_entity_Destroy(object);
	}
}
void CWeaponRPG7::SwitchState(u32 S) 
{
	inherited::SwitchState(S);
}

void CWeaponRPG7::FireStart()
{
	inherited::FireStart();
}

void CWeaponRPG7::switch2_Fire	()
{
	m_iShotNum = 0;
	m_bFireSingleShot = true;
	bWorking = false;

	if(GetState() == eFire	&& getRocketCount()) 
	{
		Fvector p1, /*d1, p*/ d;
		//Fvector p2, d2, d; 
		p1.set								(get_LastFP()); 
		d.set								(get_LastFD());
		//p = p1;
		//d = d1;
		CEntity* E = smart_cast<CEntity*>	(H_Parent());
		if(E)
		{
/*
			E->g_fireParams				(this, p2,d2);
			p = p2;
			d = d2;

			if(IsHudModeNow())
			{
				Fvector		p0;
				float dist	= HUD().GetCurrentRayQuery().range;
				p0.mul		(d2,dist);
				p0.add		(p1);
				p			= p1;
				d.sub		(p0,p1);
				d.normalize_safe();
			}
*/
			E->g_fireParams(this, p1, d);
		}

		Fmatrix								launch_matrix;
		launch_matrix.identity				();
		launch_matrix.k.set					(d);
		Fvector::generate_orthonormal_basis(launch_matrix.k,
											launch_matrix.j, launch_matrix.i);
		launch_matrix.c.set					(p1);

		d.normalize							();
		d.mul								(m_fLaunchSpeed);

		CRocketLauncher::LaunchRocket		(launch_matrix, d, zero_vel);

		CExplosiveRocket* pGrenade			= smart_cast<CExplosiveRocket*>(getCurrentRocket());
		VERIFY								(pGrenade);
		pGrenade->SetInitiator				(H_Parent()->ID());

		CRocketLauncher::DetachRocket(getCurrentRocket(), true);
		UpdateMissileVisibility();
	}
}

void CWeaponRPG7::ObjectTakeItem(CGameObject* object)
{
	CRocketLauncher::AttachRocket(object, this);
}

void CWeaponRPG7::ObjectRejectItem(CGameObject* object, bool just_before_destroy)
{
	if (!just_before_destroy)
		return;

	CRocketLauncher::DetachRocket(object, false);
}