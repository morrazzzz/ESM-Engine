#include "stdafx.h"
#include "game_cl_teamdeathmatch.h"
#include "xrMessages.h"
#include "hudmanager.h"
#include "level.h"
#include "xr_level_controller.h"
#include "clsid_game.h"
#include "map_manager.h"
#include "map_location.h"
#include "actor.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIPDAWnd.h"
#include "string_table.h"

//.#define	TEAM0_MENU		"teamdeathmatch_team0"
//.#define	TEAM1_MENU		"teamdeathmatch_team1"
//.#define	TEAM2_MENU		"teamdeathmatch_team2"

#define MESSAGE_MENUS	"tdm_messages_menu"

#include "game_cl_teamdeathmatch_snd_messages.h"

const shared_str game_cl_TeamDeathmatch::GetTeamMenu(s16 team) 
{
	switch (team)
	{
	case 0:
		return "teamdeathmatch_team0";
		break;
	case 1:
		return "teamdeathmatch_team1";
		break;
	case 2:
		return "teamdeathmatch_team2";
		break;
	default:
		NODEFAULT;
	};
#ifdef DEBUG
	return NULL;
#endif// DEBUG
}

game_cl_TeamDeathmatch::game_cl_TeamDeathmatch()
{
	PresetItemsTeam1.clear();
	PresetItemsTeam2.clear();

	m_bTeamSelected		= FALSE;
	m_game_ui			= NULL;	

	m_bShowPlayersNames = false;
	m_bFriendlyIndicators = false;
	m_bFriendlyNames	= false;

	LoadSndMessages();
}
void game_cl_TeamDeathmatch::Init ()
{
//	pInventoryMenu	= xr_new<CUIInventoryWnd>();
//	pPdaMenu = xr_new<CUIPdaWnd>();
//	pMapDesc = xr_new<CUIMapDesc>();
	//-----------------------------------------------------------
	LoadTeamData(GetTeamMenu(1));
	LoadTeamData(GetTeamMenu(2));
}

game_cl_TeamDeathmatch::~game_cl_TeamDeathmatch()
{
	PresetItemsTeam1.clear();
	PresetItemsTeam2.clear();

//	xr_delete(pInventoryMenu);
}

void game_cl_TeamDeathmatch::TranslateGameMessage	(u32 msg, NET_Packet& P)
{

}

void game_cl_TeamDeathmatch::GetMapEntities(xr_vector<SZoneMapEntityData>& dst)
{
	SZoneMapEntityData D;
	u32 color_self_team		=		0xff00ff00;
	D.color					=		color_self_team;

	s16 local_team			=		local_player->team;

	PLAYERS_MAP_IT it = players.begin();
	for(;it!=players.end();++it){
		if(local_team == it->second->team){
			u16 id = it->second->GameID;
			CObject* pObject = Level().Objects.net_Find(id);
			if (!pObject) continue;
			if (!pObject || pObject->CLS_ID != CLSID_OBJECT_ACTOR) continue;

			VERIFY(pObject);
			D.pos = pObject->Position();
			dst.push_back(D);
		}
	}
}

void game_cl_TeamDeathmatch::OnMapInfoAccept			()
{
//	if (CanCallTeamSelectMenu())
//		StartStopMenu(m_game_ui->m_pUITeamSelectWnd, true);
};

void game_cl_TeamDeathmatch::OnTeamMenuBack			()
{
	if (local_player->testFlag(GAME_PLAYER_FLAG_SPECTATOR))
	{
//		StartStopMenu(m_game_ui->m_pMapDesc, true);
//        StartStopMenu(pMapDesc, true);
	}
};

void game_cl_TeamDeathmatch::OnTeamMenu_Cancel		()
{
//	StartStopMenu(m_game_ui->m_pUITeamSelectWnd, true);
	m_bMenuCalledFromReady = FALSE;
};

void game_cl_TeamDeathmatch::OnSkinMenuBack			()
{
//		StartStopMenu(m_game_ui->m_pUITeamSelectWnd, true);
};

void game_cl_TeamDeathmatch::OnSpectatorSelect		()
{
	m_bTeamSelected = FALSE;
	inherited::OnSpectatorSelect();
}

void game_cl_TeamDeathmatch::OnTeamSelect(int Team)
{
	bool NeedToSendTeamSelect = true;
	if (Team != -1)
	{
		if (Team+1 == local_player->team && m_bSkinSelected)
			NeedToSendTeamSelect = false;
		else
		{
				NeedToSendTeamSelect = true;				
		}
	}

	if (NeedToSendTeamSelect)
	{
		CObject *l_pObj = Level().CurrentEntity();

		CGameObject *l_pPlayer = smart_cast<CGameObject*>(l_pObj);
		if(!l_pPlayer) return;

		NET_Packet		P;
		l_pPlayer->u_EventGen		(P,GE_GAME_EVENT,l_pPlayer->ID()	);
		P.w_u16(GAME_EVENT_PLAYER_GAME_MENU);
		
		P.w_s16			(s16(Team+1));
		//P.w_u32			(0);
		l_pPlayer->u_EventSend		(P);
		//-----------------------------------------------------------------
		m_bSkinSelected = FALSE;
	};
	//-----------------------------------------------------------------
	m_bTeamSelected = TRUE;	
	//---------------------------
//	if (m_bMenuCalledFromReady)
//	{
//		OnKeyboardPress(kJUMP);
//	}
};
//-----------------------------------------------------------------
void game_cl_TeamDeathmatch::SetCurrentBuyMenu	()
{
};

void game_cl_TeamDeathmatch::SetCurrentSkinMenu	()
{
};

bool game_cl_TeamDeathmatch::CanBeReady				()
{
	if (!local_player) return false;
	
	m_bMenuCalledFromReady = TRUE;

	if (!m_bTeamSelected)
	{
		m_bMenuCalledFromReady = FALSE;
//		if (CanCallTeamSelectMenu())
//			StartStopMenu(m_game_ui->m_pUITeamSelectWnd,true);
		return false;
	}

	return inherited::CanBeReady();
};

char* game_cl_TeamDeathmatch::getTeamSection(int Team)
{
	switch (Team)
	{
	case 1:
		{
			return (char*)"teamdeathmatch_team1";
		}break;
	case 2:
		{
			return (char*)"teamdeathmatch_team2";
		}break;
	default:
		return NULL;
	};
};

#include "string_table.h"

void game_cl_TeamDeathmatch::shedule_Update			(u32 dt)
{
}

void	game_cl_TeamDeathmatch::SetScore				()
{
	if (local_player)
	{			
		s16 lt = local_player->team;
		if (lt>=0)
		{
		}
	}
};

//BOOL	g_bShowPlayerNames = FALSE;

bool	game_cl_TeamDeathmatch::OnKeyboardPress			(int key)
{
	if (kTEAM == key )
	{
		if (m_game_ui)
		{
			if (CanCallTeamSelectMenu())
			{
			};

			return true;
		}
	};
	
	return inherited::OnKeyboardPress(key);
}

bool		game_cl_TeamDeathmatch::IsEnemy					(game_PlayerState* ps)
{
	if (!local_player) return false;
	return local_player->team != ps->team;
};

bool		game_cl_TeamDeathmatch::IsEnemy					(CEntityAlive* ea1, CEntityAlive* ea2)
{
	return (ea1->g_Team() != ea2->g_Team());
};

#define PLAYER_NAME_COLOR 0xff40ff40

void	game_cl_TeamDeathmatch::OnRender				()
{
}

BOOL game_cl_TeamDeathmatch::CanCallBuyMenu			()
{
	if(!m_game_ui)	return FALSE;
	if (!m_bTeamSelected) return FALSE;
	if (!m_bSkinSelected) return FALSE;

	return inherited::CanCallBuyMenu();
};

BOOL game_cl_TeamDeathmatch::CanCallSkinMenu			()
{
	if(!m_game_ui)	return FALSE;
	if (!m_bTeamSelected) return FALSE;

	return inherited::CanCallSkinMenu();
};

BOOL game_cl_TeamDeathmatch::CanCallInventoryMenu			()
{
	if(!m_game_ui)	return FALSE;

	return inherited::CanCallInventoryMenu();	
};

//#define MP_SHIT return false;
BOOL game_cl_TeamDeathmatch::CanCallTeamSelectMenu			()
{
	MP_SHIT //xD
};

#define FRIEND_LOCATION	"mp_friend_location"


void game_cl_TeamDeathmatch::UpdateMapLocations		()
{
	inherited::UpdateMapLocations();
	if (local_player)
	{
		PLAYERS_MAP_IT it = players.begin();
		for(;it!=players.end();++it)
		{
			game_PlayerState* ps = it->second;			
			u16 id = ps->GameID;
			if (ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)) 
			{
				Level().MapManager().RemoveMapLocation(FRIEND_LOCATION, id);
				continue;
			};
			
			CObject* pObject = Level().Objects.net_Find(id);
			if (!pObject || pObject->CLS_ID != CLSID_OBJECT_ACTOR) continue;
			if (IsEnemy(ps)) 
			{
				if (Level().MapManager().HasMapLocation(FRIEND_LOCATION, id))
				{
					Level().MapManager().RemoveMapLocation(FRIEND_LOCATION, id);
				};
				continue;
			};
			if (!Level().MapManager().HasMapLocation(FRIEND_LOCATION, id))
			{
				(Level().MapManager().AddMapLocation(FRIEND_LOCATION, id))->EnablePointer();
			}
		}
	};
};

void				game_cl_TeamDeathmatch::LoadSndMessages				()
{
//	LoadSndMessage("dm_snd_messages", "you_won", ID_YOU_WON);
	LoadSndMessage("tdm_snd_messages", "team1_win", ID_TEAM1_WIN);
	LoadSndMessage("tdm_snd_messages", "team2_win", ID_TEAM2_WIN);
	LoadSndMessage("tdm_snd_messages", "teams_equal", ID_TEAMS_EQUAL);
	LoadSndMessage("tdm_snd_messages", "team1_lead", ID_TEAM1_LEAD);
	LoadSndMessage("tdm_snd_messages", "team2_lead", ID_TEAM2_LEAD);

	LoadSndMessage("tdm_snd_messages", "team1_rank1", ID_TEAM1_RANK_1);
	LoadSndMessage("tdm_snd_messages", "team1_rank2", ID_TEAM1_RANK_2);
	LoadSndMessage("tdm_snd_messages", "team1_rank3", ID_TEAM1_RANK_3);
	LoadSndMessage("tdm_snd_messages", "team1_rank4", ID_TEAM1_RANK_4);

	LoadSndMessage("tdm_snd_messages", "team2_rank1", ID_TEAM2_RANK_1);
	LoadSndMessage("tdm_snd_messages", "team2_rank2", ID_TEAM2_RANK_2);
	LoadSndMessage("tdm_snd_messages", "team2_rank3", ID_TEAM2_RANK_3);
	LoadSndMessage("tdm_snd_messages", "team2_rank4", ID_TEAM2_RANK_4);
};

void				game_cl_TeamDeathmatch::OnSwitchPhase_InProgress()
{
	if (!m_bSkinSelected) m_bTeamSelected = FALSE;
};

void				game_cl_TeamDeathmatch::OnSwitchPhase			(u32 old_phase, u32 new_phase)
{
	inherited::OnSwitchPhase(old_phase, new_phase);
	switch (new_phase)
	{
	case GAME_PHASE_TEAM1_SCORES:
		{
			if (Level().CurrentViewEntity())
				PlaySndMessage(ID_TEAM1_WIN);
		}break;
	case GAME_PHASE_TEAM2_SCORES:
		{
			if (Level().CurrentViewEntity())
				PlaySndMessage(ID_TEAM2_WIN);
		}break;
	default:
		{			
		}break;
	};
}

void				game_cl_TeamDeathmatch::OnTeamChanged			()
{
};

void				game_cl_TeamDeathmatch::PlayRankChangesSndMessage ()
{
	if (local_player)
	{
		switch (local_player->rank)
		{
		case 0:
			break;		
		default:
			if (local_player->team == 1)
				PlaySndMessage(ID_TEAM1_RANK_0 +local_player->rank);
			if (local_player->team == 2)
				PlaySndMessage(ID_TEAM2_RANK_0 +local_player->rank);
			break;
		}
	}
};

void				game_cl_TeamDeathmatch::OnGameMenuRespond_ChangeTeam	(NET_Packet& P)
{
};