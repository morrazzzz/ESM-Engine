#pragma once

#include "game_base_space.h"
#include "alife_space.h"
#include "script_export_space.h"

class CSE_Abstract;
class xrServer;

class	game_sv_GameState
{

	LPCSTR				type_name() const { return "single"; };
protected:
	xrServer*						m_server;
public:
									game_sv_GameState		();
	virtual							~game_sv_GameState		();
	// Main accessors							
	CSE_Abstract*		get_entity_from_eid		(u16 id);

	// Events
	virtual		void				OnCreate				(u16 id_who)					{};

	// Main
	virtual		void				Create(shared_str& options) {};

	virtual		shared_str			level_name				(const shared_str &server_options) const;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(game_sv_GameState)
#undef script_type_list
#define script_type_list save_type_list(game_sv_GameState)