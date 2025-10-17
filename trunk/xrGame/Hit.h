#pragma once

struct SHit
{
	SHit() = default;
	SHit(float powerHit, const Fvector& dirHit, CObject* newObjectHitted, CObject* newObjectWeapon, u16 newObjectWhoGetHit,
		u16 boneIDHit, const Fvector& pInBonePaceHit, float impulseHit, ALife::EHitType typeHit, float apHit, bool aimBulletHit = false);

	bool				is_valide()		const;
	void				invalidate();
	IC	float				damage()		const { VERIFY(is_valide()); return power; }
	IC	const Fvector& direction()		const { VERIFY(is_valide()); return dir; }
	IC	CObject* initiator()		const { VERIFY(is_valide()); return objectHitted; }
	IC			u16			bone()		const { VERIFY(is_valide()); return boneID; }
	IC	const Fvector& bone_space_position()		const { VERIFY(is_valide()); return p_in_bone_space; }
	IC			float		phys_impulse()		const { VERIFY(is_valide()); return impulse; }
	IC	ALife::EHitType		type()		const { VERIFY(is_valide()); return hit_type; }

	float power;
	Fvector	dir;
	CObject* objectHitted{};
	CObject* objectWeapon{};
	u16 objectWhoGetHit{};
	//	u16 whoID;
	// 	u16	weaponID;
	u16	boneID;
	Fvector p_in_bone_space;
	float impulse;
	ALife::EHitType	hit_type;
	float ap;
	bool aim_bullet;
#ifdef DEBUG
	void				_dump();
#endif
};

extern xr_vector<SHit> vectorHits;
extern void AddHitObject(float powerHit, const Fvector& dirHit, u16 newObjectWhoGetHit, u16 newObjectHitted, u16 newObjectWeapon,
	u16 boneIDHit, const Fvector& pInBonePaceHit, float impulseHit, ALife::EHitType typeHit, float apHit, bool aimBulletHit = false);
extern void __stdcall UpdateHitObjects();