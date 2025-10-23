#include "stdafx.h"
#include "alife_space.h"
#include "hit.h"
//#include "ode_include.h"
#include "..\xr_3da\bone.h"
#include "net_utils.h"
#include "xrMessages.h"
#include "Level.h"
#include "../xrPhysics/mathutils.h"
#include "GameObject.h"

xr_vector<SHit> vectorHits;

void AddHitObject(float powerHit, const Fvector& dirHit, u16 newObjectWhoGetHit, u16 newIDObjectHitted, u16 newObjectWeapon,
	u16 boneIDHit, const Fvector& pInBonePaceHit, float impulseHit, ALife::EHitType typeHit, float apHit, bool aimBulletHit)
{
	vectorHits.emplace_back(powerHit, dirHit, Level().Objects.net_Find(newIDObjectHitted), Level().Objects.net_Find(newObjectWeapon), newObjectWhoGetHit, boneIDHit, pInBonePaceHit, impulseHit, typeHit, apHit, aimBulletHit);
}

void UpdateHitObjects()
{
	if (vectorHits.empty())
		return;

	CTimer T; T.Start();

	for (u32 i = 0; i < vectorHits.size(); i++)
	{
		CGameObject* object = static_cast<CGameObject*>(Level().Objects.net_Find(vectorHits[i].objectWhoGetHit));

		if (!object)
			continue;

		object->Hit(&vectorHits[i]);
	}

	vectorHits.clear();

	Msg("##PROCESS HITS OBJECTS: [%fms]", T.GetElapsed_sec() * 1000.f);
}

SHit::SHit(float powerHit, const Fvector& dirHit, CObject* newObjectHitted, CObject* newObjectWeapon, u16 newObjectWhoGetHit, u16 boneIDHit,
	const Fvector& pInBonePaceHit, float impulseHit, ALife::EHitType typeHit, float apHit, bool aimBulletHit)
{
	power = powerHit;
	dir = dirHit;
	objectHitted = newObjectHitted;
	objectWhoGetHit = newObjectWhoGetHit;
	objectWeapon = newObjectWeapon;
	boneID = boneIDHit;
	p_in_bone_space = pInBonePaceHit;
	impulse = impulseHit;
	hit_type = typeHit;
	ap = apHit;
	aim_bullet = aimBulletHit;
}

void SHit::invalidate()
{
}

bool SHit::is_valide() const
{
	return hit_type!=ALife::eHitTypeMax;
}

#ifdef DEBUG
void SHit::_dump()
{
	Msg("SHit::_dump()---begin");
	Log("power=",power);
	Log("impulse=",impulse);
	Log("dir=",dir);
//	Log("whoID=",whoID);
//	Log("weaponID=",weaponID);
	Log("element=",boneID);
	Log("p_in_bone_space=",p_in_bone_space);
	Log("hit_type=",(int)hit_type);
	Log("ap=",ap);
	Msg("SHit::_dump()---end");
}
#endif
