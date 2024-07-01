#include "stdafx.h"
#include "game_cl_deathmatch.h"
#include "xrMessages.h"
#include "hudmanager.h"
#include "Spectator.h"
#include "level.h"
#include "xr_level_controller.h"
#include "clsid_game.h"
#include "actor.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIPdaWnd.h"
#include "ui/UIInventoryWnd.h"
#include "ui/UIMessageBoxEx.h"
#include "dinput.h"
#include "gamepersistent.h"
#include "string_table.h"
#include "map_manager.h"
#include "map_location.h"

#include "game_cl_deathmatch_snd_messages.h"
#include "ActorCondition.h"

#define	TEAM0_MENU		"deathmatch_team0"

game_cl_Deathmatch::game_cl_Deathmatch()
{	
	PresetItemsTeam0.clear();
	PlayerDefItems.clear();
	pCurPresetItems	= NULL;;

	m_bBuyEnabled	= TRUE;

	m_bSkinSelected	= FALSE;	

	m_iCurrentPlayersMoney = 0;
	Actor_Spawn_Effect = "";

	LoadSndMessages();
	m_cl_dwWarmUp_Time = 0;
	m_bMenuCalledFromReady = FALSE;
	m_bFirstRun = TRUE;
}

void game_cl_Deathmatch::Init ()
{
	LoadTeamData(TEAM0_MENU);

	if (pSettings->line_exist("deathmatch_gamedata", "actor_spawn_effect"))
		Actor_Spawn_Effect = pSettings->r_string("deathmatch_gamedata", "actor_spawn_effect");
}

game_cl_Deathmatch::~game_cl_Deathmatch()
{
	PresetItemsTeam0.clear();
	PlayerDefItems.clear();
}

void game_cl_Deathmatch::SetCurrentSkinMenu		()	
{
}

void game_cl_Deathmatch::net_import_state	(NET_Packet& P)
{
	inherited::net_import_state	(P);

	m_s32FragLimit				= P.r_s32();
	m_s32TimeLimit				= P.r_s32() * 60000;
	m_u32ForceRespawn			= P.r_u32() * 1000;
	m_cl_dwWarmUp_Time			= P.r_u32();
	m_bDamageBlockIndicators	= !!P.r_u8();
	// Teams
	u16							t_count;
	P.r_u16						(t_count);
	teams.clear					();

	for (u16 t_it=0; t_it<t_count; ++t_it)
	{
		game_TeamState	ts;
		P.r				(&ts,sizeof(game_TeamState));
		teams.push_back	(ts);
	};

	switch (Phase())
	{
	case GAME_PHASE_PLAYER_SCORES:
		{
			P.r_stringZ(WinnerName);
			bool NeedSndMessage = (xr_strlen(WinnerName) != 0);
			if (NeedSndMessage && local_player && !xr_strcmp(WinnerName, local_player->getName()))
			{
				PlaySndMessage(ID_YOU_WON);
			}
		}break;
	}
}

void	game_cl_Deathmatch::net_import_update		(NET_Packet& P)
{
}
void game_cl_Deathmatch::OnMapInfoAccept			()
{
};

void game_cl_Deathmatch::OnSkinMenuBack			()
{
};

void game_cl_Deathmatch::OnSkinMenu_Ok			()
{
	CObject *l_pObj = Level().CurrentEntity();

	CGameObject *l_pPlayer = smart_cast<CGameObject*>(l_pObj);
	if(!l_pPlayer) return;

	NET_Packet		P;
	l_pPlayer->u_EventGen		(P, GE_GAME_EVENT, l_pPlayer->ID()	);
	P.w_u16(GAME_EVENT_PLAYER_GAME_MENU);;

	l_pPlayer->u_EventSend		(P);
	//-----------------------------------------------------------------
	m_bSkinSelected = TRUE;

	// second stub here
};

void game_cl_Deathmatch::OnSkinMenu_Cancel		()
{
};

BOOL game_cl_Deathmatch::CanCallBuyMenu			()
{
	return m_bBuyEnabled;
};

BOOL game_cl_Deathmatch::CanCallSkinMenu			()
{
	return TRUE;
};

BOOL game_cl_Deathmatch::CanCallInventoryMenu			()
{
	if (Phase()!=GAME_PHASE_INPROGRESS) return false;
	if (Level().CurrentEntity() && Level().CurrentEntity()->CLS_ID != CLSID_OBJECT_ACTOR)
	{
		return FALSE;
	}
	return TRUE;
};


void game_cl_Deathmatch::SetCurrentBuyMenu	()	
{
};

bool game_cl_Deathmatch::CanBeReady				()
{
	if (!local_player) return false;

	m_bMenuCalledFromReady = TRUE;

	SetCurrentSkinMenu();

	SetCurrentBuyMenu();


	if (!m_bSkinSelected)
	{
		m_bMenuCalledFromReady = FALSE;
		return false;
	};
	
	m_bMenuCalledFromReady = FALSE;

	OnBuyMenu_Ok();
	return true;
};

void	game_cl_Deathmatch::OnSpectatorSelect		()
{
	m_bMenuCalledFromReady = FALSE;
	m_bSkinSelected = FALSE;
	inherited::OnSpectatorSelect();
};


char* game_cl_Deathmatch::getTeamSection(int Team)
{
	return (char*)"deathmatch_team0";
};

void game_cl_Deathmatch::Check_Invincible_Players()
{
};

void game_cl_Deathmatch::ConvertTime2String		(string64* str, u32 Time)
{
	if (!str) return;
	
	u32 RHour = Time / 3600000;
	Time %= 3600000;
	u32 RMinutes = Time / 60000;
	Time %= 60000;
	u32 RSecs = Time / 1000;

	sprintf_s(*str,"%02d:%02d:%02d", RHour, RMinutes, RSecs);
};

int game_cl_Deathmatch::GetPlayersPlace			(game_PlayerState* ps)
{
	if (!ps) return -1;
	game_cl_GameState::PLAYERS_MAP_IT I=Game().players.begin();
	game_cl_GameState::PLAYERS_MAP_IT E=Game().players.end();

	// create temporary map (sort by kills)
	xr_vector<game_PlayerState*>	Players;
	for (;I!=E;++I)		Players.push_back(I->second);
	std::sort			(Players.begin(),Players.end(),DM_Compare_Players);

	int Place = 1;
	for (u32 i=0; i<Players.size(); i++)
	{
		if (Players[i] == ps)
			return Place;
		Place++;
	};
	return -1;
}

string16 places[] = {
	"1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th",
	"9th", "10th", "11th", "12th", "13th", "15th", "15th", "16th",
	"17th", "18th", "19th", "20th", "21th", "22th", "23th", "24th",
	"25th", "26th", "27th", "28th", "29th", "30th", "31th", "32th"
};

void game_cl_Deathmatch::shedule_Update			(u32 dt)
{
	CStringTable st;

	inherited::shedule_Update(dt);

	if(g_dedicated_server)	return;

	switch (Phase())
	{
	case GAME_PHASE_INPROGRESS:
		{
			Check_Invincible_Players();

			if (m_s32TimeLimit && m_cl_dwWarmUp_Time == 0)
			{
				if (Level().timeServer()<(StartTime() + m_s32TimeLimit))
				{
					u32 lts = Level().timeServer();
					u32 Rest = (StartTime() + m_s32TimeLimit) - lts;
					string64 S;
					ConvertTime2String(&S, Rest);
				}
				else
				{
				}
			};
			
			if(local_player && !local_player->IsSkip())
			{
				if (m_bFirstRun)
				{
					m_bFirstRun = FALSE;
				};		

				if (m_cl_dwWarmUp_Time > Level().timeServer())
				{
					u32 TimeRemains = m_cl_dwWarmUp_Time - Level().timeServer();
					string64 S;
					ConvertTime2String(&S, TimeRemains);
					string1024 tmpStr = "";
					if (TimeRemains > 10000)
						strconcat(sizeof(tmpStr),tmpStr, *st.translate("mp_time2start"), " ", S);
					else
					{
						if (TimeRemains < 1000)
							strconcat(sizeof(tmpStr),tmpStr, *st.translate("mp_go"), "");
						else
						{
							static u32 dwLastTimeRemains = 10;
							u32 dwCurTimeRemains = TimeRemains/1000;
							if (dwLastTimeRemains != dwCurTimeRemains)
							{
								if (dwCurTimeRemains > 0 && dwCurTimeRemains <= 5)
									PlaySndMessage(ID_COUNTDOWN_1 + dwCurTimeRemains - 1);
							}
							dwLastTimeRemains = dwCurTimeRemains;
							_itoa(dwCurTimeRemains, S, 10);								
							strconcat(sizeof(tmpStr),tmpStr, *st.translate("mp_ready"), "...", S);
						}
					};
			
				}

				u32 CurTime = Level().timeServer();
				if (IsVotingEnabled() && IsVotingActive() && m_dwVoteEndTime>=CurTime)
				{
					u32 TimeLeft = m_dwVoteEndTime - Level().timeServer();
					string1024 VoteTimeResStr;
					u32 SecsLeft = (TimeLeft % 60000) / 1000;
					u32 MinitsLeft = (TimeLeft - SecsLeft) / 60000;

					u32 NumAgreed = 0;
					PLAYERS_MAP_IT I;
					I	= players.begin();
					for(;I!=players.end(); ++I)
					{
						game_PlayerState* ps = I->second;
						if (ps->m_bCurrentVoteAgreed == 1) NumAgreed++;
					}
					
					sprintf_s	(VoteTimeResStr, st.translate("mp_timeleft").c_str(), MinitsLeft, SecsLeft, float(NumAgreed)/players.size());
				};

				if (local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD) && 
					m_u32ForceRespawn &&
					!local_player->testFlag(GAME_PLAYER_FLAG_SPECTATOR) )
				{
				};
			};
		}break;
	case GAME_PHASE_PENDING:
		{
		}break;
	case GAME_PHASE_PLAYER_SCORES:
		{
		}break;
	};
	
	//-----------------------------------------
	if (!CanCallBuyMenu()) HideBuyMenu();
		
	//-----------------------------------------
}

void	game_cl_Deathmatch::SetScore				()
{
};

bool	game_cl_Deathmatch::OnKeyboardPress			(int key)
{
	//---------------------------------------------

	return inherited::OnKeyboardPress(key);
}

bool	game_cl_Deathmatch::OnKeyboardRelease		(int key)
{
	return inherited::OnKeyboardRelease(key);
}

#define MAX_VOTE_PARAMS		5
void game_cl_Deathmatch::OnVoteStart(NET_Packet& P)
{
};

void game_cl_Deathmatch::OnVoteStop				(NET_Packet& P)	
{
	inherited::OnVoteStop(P);
};

void game_cl_Deathmatch::OnVoteEnd				(NET_Packet& P)
{
	inherited::OnVoteEnd(P);
};

void game_cl_Deathmatch::GetMapEntities(xr_vector<SZoneMapEntityData>& dst)
{
	/*
	SZoneMapEntityData D;
	u32 color_self_team		=		0xff00ff00;
	D.color					=		color_self_team;

	PLAYERS_MAP_IT it = players.begin();
	for(;it!=players.end();++it)
	{
		game_PlayerState* ps = it->second;
		u16 id = ps->GameID;
		if (ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)) continue;
		CObject* pObject = Level().Objects.net_Find(id);
		if (!pObject) continue;
		if (!pObject || pObject->CLS_ID != CLSID_OBJECT_ACTOR) continue;

		VERIFY(pObject);
		D.pos = pObject->Position();
		dst.push_back(D);
	}
	*/
}

bool		game_cl_Deathmatch::IsEnemy					(game_PlayerState* ps)
{
	return true;
}

bool		game_cl_Deathmatch::IsEnemy					(CEntityAlive* ea1, CEntityAlive* ea2)
{
	return true;
};

void		game_cl_Deathmatch::OnRender				()
{
}

IC bool	DM_Compare_Players(game_PlayerState* p1, game_PlayerState* p2)
{
	if (p1->testFlag(GAME_PLAYER_FLAG_SPECTATOR) && !p2->testFlag(GAME_PLAYER_FLAG_SPECTATOR)) return false;
	if (!p1->testFlag(GAME_PLAYER_FLAG_SPECTATOR) && p2->testFlag(GAME_PLAYER_FLAG_SPECTATOR)) return true;	
	if (p1->frags()==p2->frags())
	{
		return p1->m_iDeaths < p2->m_iDeaths;
	}
	return p1->frags() > p2->frags();
};

void game_cl_Deathmatch::PlayParticleEffect(LPCSTR EffName, Fvector& pos)
{
	if (!EffName) return;
	// вычислить позицию и направленность партикла
	Fmatrix M; 
	M.translate(pos);

//	CParticlesPlayer::MakeXFORM(pObj,0,Fvector().set(0.f,1.f,0.f),Fvector().set(0.f,0.f,0.f),pos);

	// установить particles
	CParticlesObject* ps = NULL;

	ps = CParticlesObject::Create(EffName,TRUE);

	ps->UpdateParent(M,Fvector().set(0.f,0.f,0.f));
	GamePersistent().ps_needtoplay.push_back(ps);
}

void game_cl_Deathmatch::OnSpawn(CObject* pObj)
{
	inherited::OnSpawn(pObj);
	if (!pObj) return;
	if (pObj->CLS_ID == CLSID_OBJECT_ACTOR)
	{
		if (xr_strlen(Actor_Spawn_Effect))
			PlayParticleEffect(Actor_Spawn_Effect.c_str(), pObj->Position());
	};
}

void game_cl_Deathmatch::LoadSndMessages()
{
	LoadSndMessage("dm_snd_messages", "you_won", ID_YOU_WON);

	LoadSndMessage("dm_snd_messages", "dm_rank1", ID_RANK_1);
	LoadSndMessage("dm_snd_messages", "dm_rank2", ID_RANK_2);
	LoadSndMessage("dm_snd_messages", "dm_rank3", ID_RANK_3);
	LoadSndMessage("dm_snd_messages", "dm_rank4", ID_RANK_4);

	LoadSndMessage("dm_snd_messages", "countdown_5", ID_COUNTDOWN_5);
	LoadSndMessage("dm_snd_messages", "countdown_4", ID_COUNTDOWN_4);
	LoadSndMessage("dm_snd_messages", "countdown_3", ID_COUNTDOWN_3);
	LoadSndMessage("dm_snd_messages", "countdown_2", ID_COUNTDOWN_2);
	LoadSndMessage("dm_snd_messages", "countdown_1", ID_COUNTDOWN_1);
};

void				game_cl_Deathmatch::OnSwitchPhase_InProgress()
{
};

void				game_cl_Deathmatch::OnSwitchPhase			(u32 old_phase, u32 new_phase)
{
	inherited::OnSwitchPhase(old_phase, new_phase);
	switch (new_phase)
	{
	case GAME_PHASE_INPROGRESS:
		{
			WinnerName[0] = 0;
		}break;
	case GAME_PHASE_PLAYER_SCORES:
		{
			if (local_player)
			{
				if (!xr_strcmp(WinnerName, local_player->getName()))
				{
					PlaySndMessage(ID_YOU_WON);
				}
			}			
		}break;
	default:
		{			
		}break;
	};
}

void				game_cl_Deathmatch::OnGameRoundStarted				()
{
}

void				game_cl_Deathmatch::OnRankChanged			(u8 OldRank)
{
};

void				game_cl_Deathmatch::PlayRankChangesSndMessage()
{
	if (local_player)
	{
		switch (local_player->rank)
		{
		case 0:
			break;		
		default:
			PlaySndMessage(ID_RANK_0+local_player->rank);
			break;
		}
	}
}

void game_cl_Deathmatch::OnTeamChanged()
{
};

void game_cl_Deathmatch::OnGameMenuRespond_ChangeSkin(NET_Packet& P)
{
};

void game_cl_Deathmatch::OnPlayerFlagsChanged(game_PlayerState* ps)
{
	inherited::OnPlayerFlagsChanged	(ps);
	if (!ps)						return;

	if (ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD)) return;

	CObject* pObject				= Level().Objects.net_Find(ps->GameID);
	if (!pObject)					return;

	if (pObject->CLS_ID != CLSID_OBJECT_ACTOR) return;

	CActor* pActor					= smart_cast<CActor*>(pObject);
	if (!pActor)					return;

	pActor->conditions().SetCanBeHarmedState(!ps->testFlag(GAME_PLAYER_FLAG_INVINCIBLE));
};

void game_cl_Deathmatch::SendPickUpEvent(u16 ID_who, u16 ID_what)
{
	NET_Packet						P;
	u_EventGen						(P,GE_OWNERSHIP_TAKE_MP_FORCED, ID_who);
	P.w_u16							(ID_what);
	u_EventSend						(P);
};

const shared_str game_cl_Deathmatch::GetTeamMenu(s16 team)
{
	return TEAM0_MENU;
}

#define SELF_LOCATION	"mp_self_location"
void game_cl_Deathmatch::UpdateMapLocations		()
{
	inherited::UpdateMapLocations();
	if (local_player)
	{
		if (!Level().MapManager().HasMapLocation(SELF_LOCATION, local_player->GameID))
		{
			(Level().MapManager().AddMapLocation(SELF_LOCATION, local_player->GameID))->EnablePointer();
		}
	}
}

void		game_cl_Deathmatch::ShowBuyMenu				()
{
};

void		game_cl_Deathmatch::HideBuyMenu				()
{
}