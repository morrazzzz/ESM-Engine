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

void game_sv_GameState::Create					(shared_str &options)
{
	if (!g_dedicated_server)
	{
		// loading scripts
		ai().script_engine().remove_script_process(ScriptEngine::eScriptProcessorGame);
		string_path					S;
		FS.update_path				(S,"$game_config$","script.ltx");
		CInifile					*l_tpIniFile = xr_new<CInifile>(S);
		R_ASSERT					(l_tpIniFile);

		if( l_tpIniFile->section_exist( type_name() ) )
			if (l_tpIniFile->r_string(type_name(),"script"))
				ai().script_engine().add_script_process(ScriptEngine::eScriptProcessorGame,xr_new<CScriptProcess>("game",l_tpIniFile->r_string(type_name(),"script")));
			else
				ai().script_engine().add_script_process(ScriptEngine::eScriptProcessorGame,xr_new<CScriptProcess>("game",""));

		xr_delete					(l_tpIniFile);
	}
}

void game_sv_GameState::Update		()
{	
	if (!g_dedicated_server)
	{
		CScriptProcess				*script_process = ai().script_engine().script_process(ScriptEngine::eScriptProcessorGame);
		if (script_process)
			script_process->update	();
	}
}

game_sv_GameState::game_sv_GameState()
{
	VERIFY(g_pGameLevel);
	m_server					= Level().Server;
}

game_sv_GameState::~game_sv_GameState()
{
	if (!g_dedicated_server)
		ai().script_engine().remove_script_process(ScriptEngine::eScriptProcessorGame);
}

bool game_sv_GameState::change_level (NET_Packet &net_packet)
{
	return						(true);
}

void game_sv_GameState::save_game (NET_Packet &net_packet)
{
}

bool game_sv_GameState::load_game (NET_Packet &net_packet)
{
	return						(true);
}

void game_sv_GameState::reload_game (NET_Packet &net_packet)
{
}

void game_sv_GameState::teleport_object	(NET_Packet &packet, u16 id)
{
}

void game_sv_GameState::add_restriction	(NET_Packet &packet, u16 id)
{
}

void game_sv_GameState::remove_restriction(NET_Packet &packet, u16 id)
{
}

void game_sv_GameState::remove_all_restrictions	(NET_Packet &packet, u16 id)
{
}

shared_str game_sv_GameState::level_name		(const shared_str &server_options) const
{
	string64			l_name = "";
	VERIFY				(_GetItemCount(*server_options,'/'));
	return				(_GetItem(*server_options,0,l_name,'/'));
}

void game_sv_GameState::on_death	(CSE_Abstract *e_dest, CSE_Abstract *e_src)
{
	CSE_ALifeCreatureAbstract	*creature = smart_cast<CSE_ALifeCreatureAbstract*>(e_dest);
	if (!creature)
		return;

	VERIFY						(creature->m_killer_id == ALife::_OBJECT_ID(-1));
	creature->m_killer_id		= e_src->ID;
}
