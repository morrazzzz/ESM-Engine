#include "stdafx.h"
#include "actor_mp_server.h"

CSE_ActorMP::CSE_ActorMP		(LPCSTR section) : 
	inherited				(section)
{
	m_ready_to_update		= false;
}
