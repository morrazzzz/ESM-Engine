#pragma once
#include "game_cl_mp.h"


class CUIDialogWnd;
class CUIPdaWnd;
class CUIInventoryWnd;

class game_cl_Deathmatch :public game_cl_mp
{
typedef game_cl_mp inherited;
	shared_str						Actor_Spawn_Effect;
public :
									game_cl_Deathmatch();
	virtual							~game_cl_Deathmatch();
	s32								m_s32FragLimit; //dm,tdm,ah
	s32								m_s32TimeLimit; //dm
	bool							m_bDamageBlockIndicators;
	xr_vector<game_TeamState>		teams;//dm,tdm,ah
	u32								m_u32ForceRespawn;

	u32								m_cl_dwWarmUp_Time;
	string64						WinnerName;
	
	virtual		void				net_import_state		(NET_Packet& P);
	virtual		void				net_import_update		(NET_Packet& P);	
	virtual		void				Init					();

	virtual		void				LoadSndMessages				();

protected:
	struct PresetItem
	{
		u8	SlotID;
		u8	ItemID;
		s16	BigID;
		PresetItem (u8 Slot, u8 Item) { set(Slot, Item); };
		PresetItem (s16 Big) { set(Big); };
		bool	operator ==		(const s16& ID) 
		{ 
			return (BigID) == (ID);
		}
		void		set(s16 Big) { SlotID = u8((Big>>0x08) & 0x00ff); ItemID = u8(Big & 0x00ff); BigID = Big;}
		void		set(u8 Slot, u8 Item) { SlotID = Slot; ItemID = Item; BigID = (s16(SlotID) << 0x08) | s16(ItemID); };
	};

	DEF_VECTOR						(PRESET_ITEMS, PresetItem);

	PRESET_ITEMS					PresetItemsTeam0;
	PRESET_ITEMS					AdditionalPresetItems;
	PRESET_ITEMS*					pCurPresetItems;
	PRESET_ITEMS					PlayerDefItems;

	BOOL							m_bFirstRun;
	BOOL							m_bMenuCalledFromReady;
	BOOL							m_bSkinSelected;
	
	BOOL							m_bBuyEnabled;
	s32								m_iCurrentPlayersMoney;

	u32								m_dwVoteEndTime;

	virtual const shared_str		GetBaseCostSect			() {return "deathmatch_base_cost";}
			void					CheckItem				(PIItem pItem, PRESET_ITEMS* pPresetItems,  BOOL OnlyPreset);

	virtual bool					CanBeReady				();
	virtual BOOL					CanCallBuyMenu			();
	virtual BOOL					CanCallSkinMenu			();
	virtual	BOOL					CanCallInventoryMenu	();

			void					Check_Invincible_Players ();

	virtual		void				shedule_Update			(u32 dt);
	virtual		bool				OnKeyboardPress			(int key);
	virtual		bool				OnKeyboardRelease		(int key);

	virtual		const shared_str	GetTeamMenu				(s16 team);
///	virtual		s16					GetBuyMenuItemIndex			(u8 SlotID, u8 ItemID);
				s16					GetBuyMenuItemIndex			(u8 Addons, u8 ItemID);

	virtual		void				ConvertTime2String		(string64* str, u32 Time);
	virtual		int					GetPlayersPlace			(game_PlayerState* ps);

	virtual		void				PlayParticleEffect		(LPCSTR EffName, Fvector& pos);

	virtual		void				ShowBuyMenu				();
	virtual		void				HideBuyMenu				();

public:
	virtual s16						ModifyTeam				(s16 Team)	{return Team;};

	virtual char* getTeamSection(int Team);
	virtual	void					SetCurrentBuyMenu		();
	virtual	void					SetCurrentSkinMenu		();//	{pCurSkinMenu = pSkinMenuTeam0; };

	virtual	void					OnSpectatorSelect		();

	virtual	void					OnMapInfoAccept			();
	virtual		void				OnSkinMenuBack			();
	virtual void					OnBuyMenu_Ok			();
	virtual	void					OnBuyMenu_DefaultItems	();
	virtual void					OnSkinMenu_Ok			();
	virtual		void				OnSkinMenu_Cancel		();

	virtual		void				OnGameMenuRespond_ChangeSkin	(NET_Packet& P);

	virtual		void				OnVoteStart				(NET_Packet& P);
	virtual		void				OnVoteStop				(NET_Packet& P);
	virtual		void				OnVoteEnd				(NET_Packet& P);

	virtual		void				GetMapEntities			(xr_vector<SZoneMapEntityData>& dst);

	virtual		void				OnRender				();
	virtual		bool				IsEnemy					(game_PlayerState* ps);
	virtual		bool				IsEnemy					(CEntityAlive* ea1, CEntityAlive* ea2);

	virtual		void				OnSpawn					(CObject* pObj);
	virtual		void				OnSwitchPhase			(u32 old_phase, u32 new_phase);	
	virtual		void				OnRankChanged			(u8 OldRank);
	virtual		void				PlayRankChangesSndMessage ();
	virtual		void				OnTeamChanged			();
	virtual		void				OnMoneyChanged			();

	virtual		void				SetScore				();

	virtual		void				OnSwitchPhase_InProgress();

	virtual		u8					GetTeamCount			() { return 1; };
	
	virtual		void				OnPlayerFlagsChanged	(game_PlayerState* ps);
	virtual		void				SendPickUpEvent			(u16 ID_who, u16 ID_what);

	virtual		void				OnGameRoundStarted				();
	virtual		void				UpdateMapLocations		();
};

IC bool	DM_Compare_Players		(game_PlayerState* p1, game_PlayerState* p2);