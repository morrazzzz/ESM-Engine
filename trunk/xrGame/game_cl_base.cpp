#include "pch_script.h"
#include "hudmanager.h"
#include "game_cl_base.h"
#include "level.h"
#include "GamePersistent.h"
#include "UIGameCustom.h"
#include "script_engine.h"
#include "xr_Level_controller.h"
#include "ui/UIMainIngameWnd.h"
#include "UI/UIMessagesWindow.h"
#include "string_table.h"

game_cl_GameState::game_cl_GameState()
{
	m_game_type_name			= 0;
}

game_cl_GameState::~game_cl_GameState()
{
	PLAYERS_MAP_IT I	= players.begin();
	for(;I!=players.end(); ++I)
		xr_delete(I->second);
	players.clear();
}

void	game_cl_GameState::net_import_GameTime		(NET_Packet& P)
{
	// time
	u64 GameTime;
	P.r_u64(GameTime);
	float TimeFactor;
	P.r_float(TimeFactor);

	Level().SetGameTimeFactor(GameTime, TimeFactor);

	u64 GameEnvironmentTime;
	P.r_u64(GameEnvironmentTime);
	float EnvironmentTimeFactor;
	P.r_float(EnvironmentTimeFactor);

	// u64 OldTime = Level().GetEnvironmentGameTime();
	Level().SetEnvironmentGameTimeFactor(GameEnvironmentTime, EnvironmentTimeFactor);
	// if (OldTime > GameEnvironmentTime)
	//	GamePersistent().Environment().Invalidate(/*false*/);
}

void	game_cl_GameState::net_import_state	(NET_Packet& P)
{
	// Generic
	P.r_s32			(m_type);
	
	u16 ph;
	P.r_u16			(ph);
	
	if(Phase()!=ph)
		switch_Phase(ph);

	P.r_s32			(m_round);
	P.r_u32			(m_start_time);

	// Players
	u16	p_count;
	P.r_u16			(p_count);
	
	PLAYERS_MAP players_new;

/*
	players.clear	();
*/
	PLAYERS_MAP_IT I;
	for (u16 p_it=0; p_it<p_count; ++p_it)
	{
		ClientID			ID;
		P.r_clientID		(ID);
		
		game_PlayerState*   IP;
		I = players.find(ID);

		IP = createPlayerState();
		IP->net_Import		(P);

		players_new.insert(mk_pair(ID,IP));
	}

	I	= players.begin();
	for(;I!=players.end(); ++I)
		xr_delete(I->second);
	players.clear();
	
	players = players_new;

	net_import_GameTime(P);
}

void	game_cl_GameState::net_import_update(NET_Packet& P)
{
	// Read
	ClientID			ID;
	P.r_clientID		(ID);

	// Update
	PLAYERS_MAP_IT I	= players.find(ID);
	if (players.end()!=I)
	{
		game_PlayerState* IP		= I->second;
		IP->net_Import(P);
	}
	else
	{
		game_PlayerState*	PS = createPlayerState();
		PS->net_Import		(P);
		xr_delete(PS);
	};

	//Syncronize GameTime
	net_import_GameTime (P);
}

game_PlayerState* game_cl_GameState::GetPlayerByGameID(u32 GameID)
{
	PLAYERS_MAP_IT I=players.begin();
	PLAYERS_MAP_IT E=players.end();

	for (;I!=E;++I)
	{
		game_PlayerState* P = I->second;
		if (P->GameID == GameID) return P;
	};
	return NULL;
};

bool game_cl_GameState::IR_OnKeyboardPress		(int dik)
{
	return false;
}

bool game_cl_GameState::IR_OnKeyboardRelease	(int dik)
{
	return false;
}

bool game_cl_GameState::IR_OnMouseMove			(int dx, int dy)
{
	return false;	
}
bool game_cl_GameState::IR_OnMouseWheel			(int direction)
{
	return false;
}

void game_cl_GameState::u_EventGen(NET_Packet& P, u16 type, u16 dest)
{
	P.w_begin	(M_EVENT);
	P.w_u32		(Level().timeServer());
	P.w_u16		(type);
	P.w_u16		(dest);
}

void game_cl_GameState::u_EventSend(NET_Packet& P)
{
	Level().Send(P,net_flags(TRUE,TRUE));
}

void				game_cl_GameState::OnSwitchPhase			(u32 old_phase, u32 new_phase)
{
}

void game_cl_GameState::set_type_name(LPCSTR s)	
{ 
	m_game_type_name		=s; 
};
