#include "stdafx.h"
#include "actor_mp_server.h"

CSE_ActorMP::CSE_ActorMP		(LPCSTR section) : 
	inherited				(section)
{
	m_ready_to_update		= false;
}

BOOL CSE_ActorMP::Net_Relevant	()
{
	if (fHealth<=0) return (false);
	return (inherited::Net_Relevant());
}
