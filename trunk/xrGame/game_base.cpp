#include "pch_script.h"
#include "game_base.h"
#include "ai_space.h"
#include "script_engine.h"
#include "level.h"

u64		g_qwStartGameTime		= 12*60*60*1000;
float	g_fTimeFactor			= pSettings->r_float("alife","time_factor");
u64		g_qwEStartGameTime		= 12*60*60*1000;

ENGINE_API	bool g_dedicated_server;

xr_token game_types[];

game_PlayerState::game_PlayerState()
{
	team				= 0;	
	flags__				= 0;

	clear				();
}

void game_PlayerState::clear()
{
	name[0]				= 0;
}

game_PlayerState::~game_PlayerState()
{
};

bool game_PlayerState::testFlag	(u16 f) const
{
	return !!(flags__ & f);
}

void game_PlayerState::setFlag	(u16 f)
{
	flags__ |= f;
}

void game_PlayerState::resetFlag(u16 f)
{
	flags__ &= ~(f);
}

void	game_PlayerState::net_Export(NET_Packet& P, BOOL Full)
{
	P.w_u8			(Full ? 1 : 0);
	if (Full)
		P.w_stringZ		(	name	);

	P.w_u8			(	team			);

	P.w_u16			(	flags__	);
	P.w_u16			(	ping	);

	P.w_u16			(	GameID	);
};

void	game_PlayerState::net_Import(NET_Packet& P)
{
	BOOL	bFullUpdate = !!P.r_u8();

	if (bFullUpdate)
		P.r_stringZ		(name);


	P.r_u8			(	team			);

	P.r_u16			(	flags__	);
	P.r_u16			(	ping	);

	P.r_u16			(	GameID	);
};

game_TeamState::game_TeamState()
{
	score				=	0;
	num_targets			=	0;
}

game_GameState::game_GameState()
{
	m_type						= -1;
	m_phase						= GAME_PHASE_NONE;
	m_round						= -1;

	VERIFY						(g_pGameLevel);
	m_qwStartProcessorTime		= Level().timeServer_Async();
	m_qwStartGameTime			= g_qwStartGameTime;
	m_fTimeFactor				= g_fTimeFactor;
	m_qwEStartProcessorTime		= m_qwStartProcessorTime;	
	m_qwEStartGameTime			= g_qwEStartGameTime	;
	m_fETimeFactor				= m_fTimeFactor			;
}

void game_GameState::switch_Phase		(u32 new_phase)
{
	OnSwitchPhase(m_phase, new_phase);

	m_phase				= u16(new_phase);
	m_start_time		= Level().timeServer();
}


ALife::_TIME_ID game_GameState::GetGameTime()
{
	return			(m_qwStartGameTime + iFloor(m_fTimeFactor*float(Level().timeServer_Async() - m_qwStartProcessorTime)));
}

float game_GameState::GetGameTimeFactor()
{
	return			(m_fTimeFactor);
}

void game_GameState::SetGameTimeFactor (const float fTimeFactor)
{
	m_qwStartGameTime			= GetGameTime();
	m_qwStartProcessorTime		= Level().timeServer_Async();
	m_fTimeFactor				= fTimeFactor;
}

void game_GameState::SetGameTimeFactor	(ALife::_TIME_ID GameTime, const float fTimeFactor)
{
	m_qwStartGameTime			= GameTime;
	m_qwStartProcessorTime		= Level().timeServer_Async();
	m_fTimeFactor				= fTimeFactor;
}

ALife::_TIME_ID game_GameState::GetEnvironmentGameTime()
{
	return			(m_qwEStartGameTime + iFloor(m_fETimeFactor*float(Level().timeServer_Async() - m_qwEStartProcessorTime)));
}

float game_GameState::GetEnvironmentGameTimeFactor()
{
	return			(m_fETimeFactor);
}

void game_GameState::SetEnvironmentGameTimeFactor (const float fTimeFactor)
{
	m_qwEStartGameTime			= GetEnvironmentGameTime();
	m_qwEStartProcessorTime		= Level().timeServer_Async();
	m_fETimeFactor				= fTimeFactor;
}

void game_GameState::SetEnvironmentGameTimeFactor	(ALife::_TIME_ID GameTime, const float fTimeFactor)
{
	m_qwEStartGameTime			= GameTime;
	m_qwEStartProcessorTime		= Level().timeServer_Async();
	m_fETimeFactor				= fTimeFactor;
}
