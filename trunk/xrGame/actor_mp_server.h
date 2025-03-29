#pragma once

#include "xrServer_Objects_ALife_Monsters.h"
#include "actor_mp_state.h"

class CSE_ActorMP : public CSE_ALifeCreatureActor {
private:
	typedef CSE_ALifeCreatureActor	inherited;

private:
	actor_mp_state_holder	m_state_holder;
	bool					m_ready_to_update;

private:
			void			fill_state		(actor_mp_state &state);

public:
							CSE_ActorMP		(LPCSTR		section);
	virtual BOOL			Net_Relevant	();
};
