#include "stdafx.h"
#include "xrServer.h"
#include "LevelGameDef.h"
#include "script_process.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "script_engine.h"
#include "script_engine_space.h"
#include "level.h"
#include "xrserver.h"
#include "ai_space.h"
#include "game_sv_event_queue.h"
#include "../xr_3da/XR_IOConsole.h"
#include "../xr_3da/xr_ioc_cmd.h"
#include "string_table.h"

#ifdef DEBUG
#include "debug_renderer.h"
#include "level_debug.h"
#endif

ENGINE_API	bool g_dedicated_server;

#define			MAPROT_LIST_NAME		"maprot_list.ltx"
string_path		MAPROT_LIST		= "";
BOOL	net_sv_control_hit	= FALSE		;

//-----------------------------------------------------------------
u32		g_sv_base_dwRPointFreezeTime	= 0;
int		g_sv_base_iVotingEnabled		= 0x00ff;
//-----------------------------------------------------------------

// Main
game_PlayerState*	game_sv_GameState::get_it					(u32 it)
{
	xrClientData*	C	= (xrClientData*)m_server->client_Get			(it);
	if (0==C)			return 0;
	else				return C->ps;
}

game_PlayerState*	game_sv_GameState::get_id					(ClientID id)							
{
	xrClientData*	C	= (xrClientData*)m_server->ID_to_client	(id);
	if (0==C)			return NULL;
	else				return C->ps;
}

ClientID				game_sv_GameState::get_it_2_id				(u32 it)
{
	xrClientData*	C	= (xrClientData*)m_server->client_Get		(it);
	if (0==C){
		ClientID clientID;clientID.set(0);
		return clientID;
	}
	else				return C->ID;
}

u32					game_sv_GameState::get_players_count		()
{
	return				m_server->client_Count();
}

CSE_Abstract*		game_sv_GameState::get_entity_from_eid		(u16 id)
{
	return				m_server->ID_to_entity(id);
}

void				game_sv_GameState::signal_Syncronize		()
{
	sv_force_sync	= TRUE;
}

// Network
void game_sv_GameState::net_Export_State						(NET_Packet& P, ClientID to)
{
	// Generic
	P.w_s32			(m_type);
	P.w_u16			(m_phase);
	P.w_s32			(m_round);
	P.w_u32			(m_start_time);

	// Players
//	u32	p_count			= get_players_count() - ((g_dedicated_server)? 1 : 0);
	u32 p_count = 0;
	for (u32 p_it=0; p_it<get_players_count(); ++p_it)
	{
		xrClientData*	C		=	(xrClientData*)	m_server->client_Get	(p_it);		
		if (!C->net_Ready || (C->ps->IsSkip() && C->ID != to)) continue;
		p_count++;
	};

	P.w_u16				(u16(p_count));
	game_PlayerState*	Base	= get_id(to);
	for (u32 p_it=0; p_it<get_players_count(); ++p_it)
	{
		string64	p_name;
		xrClientData*	C		=	(xrClientData*)	m_server->client_Get	(p_it);
		game_PlayerState* A		=	get_it			(p_it);
		if (!C->net_Ready || (A->IsSkip() && C->ID != to)) continue;
		if (0==C)	strcpy(p_name,"Unknown");
		else 
		{
			CSE_Abstract* C_e		= C->owner;
			if (0==C_e)		strcpy(p_name,"Unknown");
			else 
			{
				strcpy	(p_name,C_e->name_replace());
			}
		}

		A->setName(p_name);
		u16 tmp_flags = A->flags__;

		if (Base==A)	
			A->setFlag(GAME_PLAYER_FLAG_LOCAL);

		ClientID clientID = get_it_2_id	(p_it);
		P.w_clientID			(clientID);
		A->net_Export			(P, TRUE);
		
		A->flags__ = tmp_flags;
	}

	net_Export_GameTime(P);
}

void game_sv_GameState::net_Export_Update(NET_Packet& P, ClientID id_to, ClientID id)
{
	game_PlayerState* A		= get_id		(id);
	if (A)
	{
		u16 bk_flags = A->flags__;
		if (id==id_to)	
		{
			A->setFlag(GAME_PLAYER_FLAG_LOCAL);
		}

		P.w_clientID	(id);
		A->net_Export	(P);
		A->flags__		= bk_flags;
	};
};

void game_sv_GameState::net_Export_GameTime						(NET_Packet& P)
{
	//Syncronize GameTime 
	P.w_u64(GetGameTime());
	P.w_float(GetGameTimeFactor());
	//Syncronize EnvironmentGameTime 
	P.w_u64(GetEnvironmentGameTime());
	P.w_float(GetEnvironmentGameTimeFactor());
};

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

	LPCSTR		svcfg_ltx_name = "-svcfg ";
	if (strstr(Core.Params, svcfg_ltx_name))
	{
		string_path svcfg_name = "";
		int		sz = xr_strlen(svcfg_ltx_name);
		sscanf		(strstr(Core.Params,svcfg_ltx_name)+sz,"%[^ ] ",svcfg_name);
//		if (FS.exist(svcfg_name))
		{
			Console->ExecuteScript(svcfg_name);
		}
	};
}

CSE_Abstract*		game_sv_GameState::spawn_begin				(LPCSTR N)
{
	CSE_Abstract*	A	=   F_entity_Create(N);	R_ASSERT(A);	// create SE
	A->s_name			=   N;							// ltx-def
	A->s_gameid			=	u8(m_type);							// game-type
	A->s_RP				=	0xFE;								// use supplied
	A->ID				=	0xffff;								// server must generate ID
	A->ID_Parent		=	0xffff;								// no-parent
	A->ID_Phantom		=	0xffff;								// no-phantom
	A->RespawnTime		=	0;									// no-respawn
	return A;
}

CSE_Abstract*		game_sv_GameState::spawn_end				(CSE_Abstract* E, ClientID id)
{
	NET_Packet						P;
	u16								skip_header;
	E->Spawn_Write					(P,TRUE);
	P.r_begin						(skip_header);
	CSE_Abstract* N = m_server->Process_spawn	(P,id);
	F_entity_Destroy				(E);

	return N;
}

void game_sv_GameState::u_EventGen(NET_Packet& P, u16 type, u16 dest)
{
	P.w_begin	(M_EVENT);
	P.w_u32		(Level().timeServer());//Device.TimerAsync());
	P.w_u16		(type);
	P.w_u16		(dest);
}

void game_sv_GameState::u_EventSend(NET_Packet& P, u32 dwFlags)
{
	m_server->SendBroadcast(BroadcastCID,P,dwFlags);
}

void game_sv_GameState::Update		()
{	
	if (!g_dedicated_server)
	{
		if (Level().game) {
			CScriptProcess				*script_process = ai().script_engine().script_process(ScriptEngine::eScriptProcessorGame);
			if (script_process)
				script_process->update	();
		}
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

bool game_sv_GameState::change_level (NET_Packet &net_packet, ClientID sender)
{
	return						(true);
}

void game_sv_GameState::save_game (NET_Packet &net_packet, ClientID sender)
{
}

bool game_sv_GameState::load_game (NET_Packet &net_packet, ClientID sender)
{
	return						(true);
}

void game_sv_GameState::reload_game (NET_Packet &net_packet, ClientID sender)
{
}

void game_sv_GameState::switch_distance (NET_Packet &net_packet, ClientID sender)
{
}

bool game_sv_GameState::NewPlayerName_Exists( void* pClient, LPCSTR NewName )
{
	if ( !pClient || !NewName ) return false;
	IClient* CL = (IClient*)pClient;
	if ( !CL->name || xr_strlen( CL->name.c_str() ) == 0 ) return false;

	u32	cnt	= get_players_count();
	for ( u32 it = 0; it < cnt; ++it )	
	{
		IClient*	pIC	= m_server->client_Get(it);
		if ( !pIC || pIC == CL ) continue;
		string64 xName;
		strcpy( xName, pIC->name.c_str() );
		if ( !xr_strcmp(NewName, xName) ) return true;
	};
	return false;
}

void game_sv_GameState::NewPlayerName_Generate( void* pClient, LPSTR NewPlayerName )
{
	if ( !pClient || !NewPlayerName ) return;
	NewPlayerName[21] = 0;
	for ( int i = 1; i < 100; ++i )
	{
		string64 NewXName;
		sprintf_s( NewXName, "%s_%d", NewPlayerName, i );
		if ( !NewPlayerName_Exists( pClient, NewXName ) )
		{
			strcpy( NewPlayerName, NewXName );
			return;
		}
	}
}

void game_sv_GameState::NewPlayerName_Replace( void* pClient, LPCSTR NewPlayerName )
{
	if ( !pClient || !NewPlayerName ) return;
	IClient* CL = (IClient*)pClient;
	if ( !CL->name || xr_strlen( CL->name.c_str() ) == 0 ) return;
	
	CL->name._set( NewPlayerName );
	
	//---------------------------------------------------------
	NET_Packet P;
	P.w_begin( M_CHANGE_SELF_NAME );
	P.w_stringZ( NewPlayerName );	
	m_server->SendTo( CL->ID, P );
}

void game_sv_GameState::OnSwitchPhase(u32 old_phase, u32 new_phase)
{
	inherited::OnSwitchPhase(old_phase, new_phase);
	signal_Syncronize	(); 
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
