#pragma once
#include "missile.h"
#include "../xrPhysics/DamageSource.h"
class CBolt :
	public CMissile,
	public IDamageSource
{
	using inherited = CMissile;
	u16	m_thrower_id;
	HUD_SOUND_ITEM m_ThrowSnd{};
public:
	CBolt(void);
	virtual ~CBolt(void);

	virtual void Load(LPCSTR section) override;
	virtual void OnH_A_Chield();
	
	virtual	void SetInitiator(u16 id);
	virtual	u16	 Initiator();

	virtual void Throw();
	virtual bool Action(s32 cmd, u32 flags);
	virtual bool Useful() const;
    virtual void Destroy();
    virtual void activate_physic_shell	();

	virtual BOOL UsedAI_Locations() {return FALSE;}
	virtual IDamageSource*	cast_IDamageSource			()	{return this;}
};
