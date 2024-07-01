#pragma once

#include "game_cl_base.h"
#include "script_export_space.h"
#include "game_cl_mp_snd_messages.h"
#include "../../xrSound/Sound.h"
#include "Spectator.h"

#define MP_SHIT return false; // xD Lol

class CUIMessageBoxEx;


struct SND_Message{
	ref_sound	pSound;
	u32			priority;
	u32			SoundID;
	u32			LastStarted;
	bool operator == (u32 ID){return SoundID == ID;}
	void Load(u32 ID, u32 prior, LPCSTR name)
	{
		SoundID = ID;
		priority = prior;
		pSound.create(name,st_Effect,sg_SourceType);
		LastStarted = 0;
	}
	~SND_Message()
	{
		SoundID = 0;
		priority = 0;
		pSound.destroy();
	}
};

struct cl_MessageMenu
{
};

struct Bonus_Struct
{
};

class CUIMessageBoxEx;

class game_cl_mp :public game_cl_GameState
{
	typedef game_cl_GameState	inherited;
protected:
	virtual void			LoadTeamData			(const shared_str&	TeamName);
	virtual	void			ChatSayTeam				(const shared_str&	phrase);
	virtual	void			ChatSayAll				(const shared_str&	phrase);
	virtual	void			OnChatMessage			(NET_Packet* P);
	virtual	void			OnWarnMessage			(NET_Packet* P);
	virtual	void			OnRadminMessage			(u16 type, NET_Packet* P);

	virtual void			UpdateMapLocations		() {};

	virtual void			OnPlayerKilled			(NET_Packet& P);

	virtual bool			NeedToSendReady_Actor			(int key, game_PlayerState* ps);
	virtual bool			NeedToSendReady_Spectator		(int key, game_PlayerState* ps);

	virtual	void			LoadSndMessage			(LPCSTR caSection, LPCSTR caLine, u32 ID);
	virtual		void				LoadSndMessages				();
	virtual		void				PlaySndMessage			(u32 ID);	
	virtual		void				UpdateSndMessages		();

	u8			m_u8SpectatorModes;
	bool		m_bSpectator_FreeFly;
	bool		m_bSpectator_FirstEye;
	bool		m_bSpectator_LookAt;
	bool		m_bSpectator_FreeLook;
	bool		m_bSpectator_TeamCamera;

	virtual		void		LoadBonuses				();

public:
									game_cl_mp();
	virtual							~game_cl_mp();

	virtual		void				TranslateGameMessage	(u32 msg, NET_Packet& P);
	virtual		void				CommonMessageOut		(LPCSTR msg);

	virtual		bool				OnKeyboardPress			(int key);
	virtual		bool				OnKeyboardRelease		(int key);

	virtual		bool				CanBeReady				();
	virtual		CUIGameCustom*		createGameUI			();
	virtual		void				shedule_Update			(u32 dt);

	//// VOTING
	virtual		void				SendStartVoteMessage	(LPCSTR args);
	virtual		void				SendVoteYesMessage		();
	virtual		void				SendVoteNoMessage		();
				void				VotingBegin				();
				void				Vote					();
				void				OnCantVoteMsg			(LPCSTR Text);
	virtual		void				OnVoteStart				(NET_Packet& P);
	virtual		void				OnVoteStop				(NET_Packet& P);
	virtual		void				OnVoteEnd				(NET_Packet& P);
	virtual		void				OnPlayerChangeName		(NET_Packet& P);
	virtual		void				OnPlayerVoted			(game_PlayerState* ps);
	virtual		void				OnSpectatorSelect		();
	virtual		void				OnSkinMenuBack			() {};
	virtual		void				OnTeamMenuBack			() {};
	virtual		void				OnTeamMenu_Cancel		() {};
	virtual		void				OnMapInfoAccept			() {};
	virtual		void				OnSkinMenu_Ok			() {};
	virtual		void				OnSkinMenu_Cancel		() {};
	virtual		void				OnBuySpawnMenu_Ok		() {};
	virtual		void				OnSellItemsFromRuck		() {};


	virtual		void				OnGameMenuRespond				(NET_Packet& P);
	virtual		void				OnGameMenuRespond_Spectator		(NET_Packet& P) {};
	virtual		void				OnGameMenuRespond_ChangeTeam	(NET_Packet& P) {};
	virtual		void				OnGameMenuRespond_ChangeSkin	(NET_Packet& P) {};
	virtual		void				OnGameRoundStarted				();
	

	virtual		void				OnSwitchPhase			(u32 old_phase, u32 new_phase);	
	virtual		void				OnRankChanged			(u8 OldRank);
	virtual		void				OnTeamChanged			() {};
	virtual		void				OnMoneyChanged			() {};

	virtual		void				OnSwitchPhase_InProgress();

	virtual		u8					GetTeamCount			() { return 0; };
	virtual		s16					ModifyTeam				(s16 Team)	{return Team;};

	virtual		bool				Is_Spectator_TeamCamera_Allowed () {return m_bSpectator_TeamCamera;};
	virtual		bool				Is_Spectator_Camera_Allowed			(CSpectator::EActorCameras Camera);
	
//-------------------------------------------------------------------------------------------------
#include "game_cl_mp_messages_menu.h"

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(game_cl_mp)
#undef script_type_list
#define script_type_list save_type_list(game_cl_mp)
