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
	
				CSE_Abstract*		spawn_begin				(LPCSTR N);
				CSE_Abstract*		spawn_end				(CSE_Abstract* E);

	// Utilities
	void							u_EventGen				(NET_Packet& P, u16 type, u16 dest	);
	void							u_EventSend				(NET_Packet& P);

	// Events
	virtual		void				OnCreate				(u16 id_who)					{};
	virtual		BOOL				OnTouch					(u16 eid_who, u16 eid_target)	= 0;			// TRUE=allow ownership, FALSE=denied
	virtual		void				OnDetach				(u16 eid_who, u16 eid_target)	= 0;	

	// Main
	virtual		void				Create					(shared_str& options);
	virtual		void				Update					();

	virtual		bool				change_level			(NET_Packet &net_packet);
	virtual		void				save_game				(NET_Packet &net_packet);
	virtual		bool				load_game				(NET_Packet &net_packet);
	virtual		void				reload_game				(NET_Packet &net_packet);

	virtual		void				teleport_object			(NET_Packet &packet, u16 id);
	virtual		void				add_restriction			(NET_Packet &packet, u16 id);
	virtual		void				remove_restriction		(NET_Packet &packet, u16 id);
	virtual		void				remove_all_restrictions	(NET_Packet &packet, u16 id);
	virtual		bool				custom_sls_default		() {return false;};
	virtual		void				sls_default				() {};
	virtual		shared_str			level_name				(const shared_str &server_options) const;
	virtual		void				on_death				(CSE_Abstract *e_dest, CSE_Abstract *e_src);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(game_sv_GameState)
#undef script_type_list
#define script_type_list save_type_list(game_sv_GameState)