#pragma once

#include "game_base.h"
#include "../../xrNetServer/client_id.h"
#include "WeaponAmmo.h"
//#include "Level_Bullet_Manager.h"

class	NET_Packet;
class	CGameObject;
class	CUIGameCustom;
class	CUIDialogWnd;

class	game_cl_GameState: public game_GameState
{
	typedef game_GameState	inherited;
	shared_str							m_game_type_name;
//	bool								m_bCrosshair;	//был ли показан прицел-курсор HUD перед вызовом меню
public:
	typedef xr_map<ClientID,game_PlayerState*>	PLAYERS_MAP;
	typedef PLAYERS_MAP::iterator				PLAYERS_MAP_IT;
	typedef PLAYERS_MAP::const_iterator			PLAYERS_MAP_CIT;

	PLAYERS_MAP							players;
private:
				void				switch_Phase			(u32 new_phase)		{inherited::switch_Phase(new_phase);};
protected:

	virtual		void				OnSwitchPhase			(u32 old_phase, u32 new_phase);	
public:
									game_cl_GameState		();
	virtual							~game_cl_GameState		();
				LPCSTR				type_name				() const {return *m_game_type_name;};
				void				set_type_name			(LPCSTR s);
	virtual		void				Init					(){};
	virtual		void				net_import_state		(NET_Packet& P);
	virtual		void				net_import_update		(NET_Packet& P);
	virtual		void				net_import_GameTime		(NET_Packet& P);						// update GameTime only for remote clients

				bool				IR_OnKeyboardPress		(int dik);
				bool				IR_OnKeyboardRelease	(int dik);
				bool				IR_OnMouseMove			(int dx, int dy);
				bool				IR_OnMouseWheel			(int direction);


	virtual		bool				OnKeyboardPress			(int key){return false;};
	virtual		bool				OnKeyboardRelease		(int key){return false;};

				game_PlayerState*	GetPlayerByGameID		(u32 GameID);

	void							u_EventGen				(NET_Packet& P, u16 type, u16 dest);
	void							u_EventSend				(NET_Packet& P);

	virtual		void				OnRender				()	{};

	virtual		void				OnSpawn					(CObject* pObj)	{};
	virtual		void				OnDestroy				(CObject* pObj)	{};
};
