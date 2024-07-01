#include "stdafx.h"
#include "game_cl_mp.h"
#include "xr_level_controller.h"
#include "xrMessages.h"
#include "GameObject.h"
#include "Actor.h"
#include "level.h"
#include "hudmanager.h"
#include "ui/UIGameLog.h"
#include "../Include/xrRender/UIShader.h"
#include "clsid_game.h"
#include <dinput.h>
#include "UIGameCustom.h"
#include "ui/UIInventoryUtilities.h"
#include "ui/UIMessagesWindow.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIMessageBoxEx.h"
#include "CustomZone.h"
#include "game_base_kill_type.h"
#include "ui/UITextureMaster.h"
#include "ui/UIMessageBoxEx.h"
#include "string_table.h"
#include "../xr_3da/IGame_Persistent.h"
#include "MainMenu.h"

#define KILLEVENT_GRID_WIDTH	64
#define KILLEVENT_GRID_HEIGHT	64

#include "game_cl_mp_snd_messages.h"

game_cl_mp::game_cl_mp()
{
	//-------------------------------------
	m_u8SpectatorModes		= 0xff;
	m_bSpectator_FreeFly	= true;
	m_bSpectator_FirstEye	= true;
	m_bSpectator_LookAt		= true;
	m_bSpectator_FreeLook	= true;
	m_bSpectator_TeamCamera	= true;
	//-------------------------------------
	LoadBonuses();
};

game_cl_mp::~game_cl_mp()
{
}

CUIGameCustom*		game_cl_mp::createGameUI			()
{
	return NULL;
};

bool game_cl_mp::CanBeReady	()
{
	return true;
}

bool game_cl_mp::NeedToSendReady_Actor(int key, game_PlayerState* ps)
{
	return ((GAME_PHASE_PENDING == Phase() ) || 
			true == ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD) ) && 
			(kWPN_FIRE == key);
}

bool game_cl_mp::NeedToSendReady_Spectator(int key, game_PlayerState* ps)
{
	return (GAME_PHASE_PENDING==Phase() && kWPN_FIRE==key) || 
			(	kJUMP==key && 
				GAME_PHASE_INPROGRESS==Phase() && 
				CanBeReady() && 
				ps->DeathTime > 1000);
}

bool game_cl_mp::OnKeyboardPress(int key)
{
	bool FuckingMPShit_ = false;
	return FuckingMPShit_; //:)
}

void	game_cl_mp::VotingBegin()
{
}

void	game_cl_mp::Vote()
{
}

void	game_cl_mp::OnCantVoteMsg(LPCSTR Text)
{
}

bool	game_cl_mp::OnKeyboardRelease		(int key)
{
	return inherited::OnKeyboardRelease(key);
}

void game_cl_mp::TranslateGameMessage	(u32 msg, NET_Packet& P)
{
}


//////////////////////////////////////////////////////////////////////////

void game_cl_mp::ChatSayAll(const shared_str &phrase)
{
}

//////////////////////////////////////////////////////////////////////////

void game_cl_mp::ChatSayTeam(const shared_str &phrase)
{
}

void game_cl_mp::OnWarnMessage(NET_Packet* P)
{
}

void game_cl_mp::OnChatMessage(NET_Packet* P)
{
};

void game_cl_mp::CommonMessageOut		(LPCSTR msg)
{
};


void game_cl_mp::shedule_Update(u32 dt)
{
}

void game_cl_mp::SendStartVoteMessage	(LPCSTR args)
{
};

void game_cl_mp::SendVoteYesMessage		()	
{
};
void game_cl_mp::SendVoteNoMessage		()	
{
};

void game_cl_mp::OnVoteStart				(NET_Packet& P)	
{
};
void game_cl_mp::OnVoteStop				(NET_Packet& P)	
{
};

void game_cl_mp::OnVoteEnd				(NET_Packet& P)
{
};
void game_cl_mp::OnPlayerVoted			(game_PlayerState* ps)
{
}
void game_cl_mp::LoadTeamData			(const shared_str& TeamName)
{
}

void game_cl_mp::OnSwitchPhase_InProgress()
{
};

void game_cl_mp::OnSwitchPhase			(u32 old_phase, u32 new_phase)
{
}

void game_cl_mp::OnPlayerKilled			(NET_Packet& P)
{
};

void	game_cl_mp::OnPlayerChangeName		(NET_Packet& P)
{

}

void	game_cl_mp::LoadSndMessages				()
{
};

void	game_cl_mp::OnRankChanged	(u8 OldRank)
{
};

bool	game_cl_mp::Is_Spectator_Camera_Allowed			(CSpectator::EActorCameras Camera)
{
	return (!!(m_u8SpectatorModes & (1<<Camera)));
};

void	game_cl_mp::OnSpectatorSelect		()
{
	CObject *l_pObj = Level().CurrentEntity();

	CGameObject *l_pPlayer = smart_cast<CGameObject*>(l_pObj);
	if(!l_pPlayer) return;

	NET_Packet		P;
	l_pPlayer->u_EventGen		(P, GE_GAME_EVENT, l_pPlayer->ID()	);
//	P.w_u16(GAME_EVENT_PLAYER_SELECT_SPECTATOR);
	P.w_u16(GAME_EVENT_PLAYER_GAME_MENU);
	l_pPlayer->u_EventSend		(P);
};

void	game_cl_mp::OnGameMenuRespond		(NET_Packet& P)
{
};

void	game_cl_mp::OnGameRoundStarted				()
{
}

void game_cl_mp::LoadBonuses()
{
}

void game_cl_mp::OnRadminMessage(u16 type, NET_Packet* P)
{
}
