#include "stdafx.h"
#include "xrServer.h"
#include "script_process.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "script_engine.h"
#include "script_engine_space.h"
#include "level.h"
#include "ai_space.h"
#include "../xr_3da/XR_IOConsole.h"

ENGINE_API	bool g_dedicated_server;

CSE_Abstract*		game_sv_GameState::get_entity_from_eid		(u16 id)
{
	return				m_server->ID_to_entity(id);
}

game_sv_GameState::game_sv_GameState()
{
	VERIFY(g_pGameLevel);
	m_server					= Level().Server;
}

game_sv_GameState::~game_sv_GameState()
{
}

shared_str game_sv_GameState::level_name		(const shared_str &server_options) const
{
	string64			l_name = "";
	VERIFY				(_GetItemCount(*server_options,'/'));
	return				(_GetItem(*server_options,0,l_name,'/'));
}
