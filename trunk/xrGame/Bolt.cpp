#include "stdafx.h"

#include "Actor.h"
#include "ai_sounds.h"
#include "bolt.h"
#include "../xrPhysics/PhysicsShell.h"

CBolt::CBolt() 
{
	m_weight					= .1f;
	m_slot						= BOLT_SLOT;
	m_flags.set					(Fruck, false);
	m_thrower_id				= static_cast<u16>(-1);
}

CBolt::~CBolt() {
	HUD_SOUND_ITEM::DestroySound(m_ThrowSnd);
}

void CBolt::Load(LPCSTR section) {
	inherited::Load(section);
	if(pSettings->line_exist(section, "snd_throw"))
		HUD_SOUND_ITEM::LoadSound(section, "snd_throw", m_ThrowSnd, SOUND_TYPE_WEAPON_SHOOTING);
}


void CBolt::OnH_A_Chield() 
{
	inherited::OnH_A_Chield();
	CObject* o= H_Parent()->H_Parent();
	if(o)SetInitiator(o->ID());
	
}

void CBolt::Throw() 
{
	if (const auto actor = smart_cast<CActor*>(H_Parent()))
		HUD_SOUND_ITEM::PlaySound(m_ThrowSnd, actor->Position(), nullptr, true);

	auto					*l_pBolt = smart_cast<CMissile*>(m_fake_missile);
	if(!l_pBolt)				return;
	l_pBolt->set_destroy_time	(m_dwDestroyTimeMax/static_cast<u32>(phTimefactor));
	inherited::Throw			();
	spawn_fake_missile			();
}

bool CBolt::Useful() const
{
	return false;
}

bool CBolt::Action(s32 cmd, u32 flags) 
{
	return inherited::Action(cmd, flags);
}

void CBolt::Destroy()
{
	inherited::Destroy();
}

void CBolt::activate_physic_shell	()
{
	inherited::activate_physic_shell	();
	m_pPhysicsShell->SetAirResistance	(.0001f);
}

void CBolt::SetInitiator			(u16 id)
{
	m_thrower_id=id;
}
u16	CBolt::Initiator				()
{
	return m_thrower_id;
}