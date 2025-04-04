#pragma once

#include "game_base_space.h"
#include "script_export_space.h"
#include "alife_space.h"

#pragma pack(push,1)

class	NET_Packet;

struct	game_PlayerState 
{
	string64	name;
	u8			team;

	u16			flags__;

	u16			ping;

	u16			GameID;
public:
					game_PlayerState		();
					~game_PlayerState		();
	virtual void	clear					();
			bool	testFlag				(u16 f) const;
			void	setFlag					(u16 f);
			void	resetFlag				(u16 f);
			LPCSTR	getName					(){return name;}
			void	setName					(LPCSTR s){strcpy(name,s);}
			bool	IsSkip					() const {return testFlag(GAME_PLAYER_FLAG_SKIP);}

#ifndef AI_COMPILER
	virtual void	net_Export				(NET_Packet& P, BOOL Full = FALSE);
	virtual void	net_Import				(NET_Packet& P);
#endif
	DECLARE_SCRIPT_REGISTER_FUNCTION_STRUCT
};

add_to_type_list(game_PlayerState)
#undef script_type_list
#define script_type_list save_type_list(game_PlayerState)


struct	game_TeamState
{
	int			score;
	u16			num_targets;

	game_TeamState();
};


#pragma pack(pop)

class game_GameState
{
protected:
	s32								m_type;
	u16								m_phase;
	s32								m_round;
	u32								m_start_time;
protected:
	virtual		void				switch_Phase			(u32 new_phase);
	virtual		void				OnSwitchPhase			(u32 old_phase, u32 new_phase)	{};	

public:
									game_GameState			();
	virtual							~game_GameState			()								{}
				u32					Type					() const						{return m_type;};
				u16					Phase					() const						{return m_phase;};
				s32					Round					() const						{return m_round;};
				u32					StartTime				() const						{return m_start_time;};
	virtual		void				Create					(shared_str& options)				{};
	virtual		LPCSTR				type_name				() const						{return "base game";};
//for scripting enhancement
	virtual		game_PlayerState*	createPlayerState()		{return xr_new<game_PlayerState>(); };

//moved from game_sv_base (time routines)
private:
	// scripts
	u64								m_qwStartProcessorTime;
	u64								m_qwStartGameTime;
	float							m_fTimeFactor;
	//-------------------------------------------------------
	u64								m_qwEStartProcessorTime;
	u64								m_qwEStartGameTime;
	float							m_fETimeFactor;
	//-------------------------------------------------------
public:

	virtual		ALife::_TIME_ID		GetGameTime				();	
	virtual		float				GetGameTimeFactor		();	
				void				SetGameTimeFactor		(ALife::_TIME_ID GameTime, const float fTimeFactor);
	virtual		void				SetGameTimeFactor		(const float fTimeFactor);
	

	virtual		ALife::_TIME_ID		GetEnvironmentGameTime	();
	virtual		float				GetEnvironmentGameTimeFactor		();
				void				SetEnvironmentGameTimeFactor		(ALife::_TIME_ID GameTime, const float fTimeFactor);
	virtual		void				SetEnvironmentGameTimeFactor		(const float fTimeFactor);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(game_GameState)
#undef script_type_list
#define script_type_list save_type_list(game_GameState)
