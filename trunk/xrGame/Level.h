// Level.h: interface for the CLevel class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../xr_3da/igame_level.h"
#include "../../xrNetServer/net_client.h"
#include "script_export_space.h"
#include "../xr_3da/StatGraph.h"
#include "xrMessages.h"
#include "alife_space.h"
#include "xrDebug.h"
#include "xrServer.h"

class	CHUDManager;
class	CParticlesObject;
class	xrServer;
class	NET_Queue_Event;
class	CSE_Abstract;
class	CSpaceRestrictionManager;
class	CSeniorityHierarchyHolder;
class	CClientSpawnManager;
class	CGameObject;
class	CAutosaveManager;
class	CPHCommander;
class	CLevelDebug;
class	CLevelSoundManager;

#ifdef DEBUG
	class	CDebugRenderer;
#endif

extern float g_fov;

const int maxRP					= 64;
const int maxTeams				= 32;

//class CFogOfWar;
class CFogOfWarMngr;
class CBulletManager;
class CMapManager;

#include "../xr_3da/feel_touch.h"

class GlobalFeelTouch : public Feel::Touch
{
public:
							GlobalFeelTouch();
	virtual					~GlobalFeelTouch();

			void			update						();
			bool			is_object_denied			(CObject const * O);
};


class CLevel					: public IGame_Level, public IPureClient
{
protected:
	typedef IGame_Level			inherited;
	
	CLevelSoundManager			*m_level_sound_manager;

	// movement restriction manager
	CSpaceRestrictionManager	*m_space_restriction_manager;
	// seniority hierarchy holder
	CSeniorityHierarchyHolder	*m_seniority_hierarchy_holder;
	// client spawn_manager
	CClientSpawnManager			*m_client_spawn_manager;
	// autosave manager
	CAutosaveManager			*m_autosave_manager;
#ifdef DEBUG
	// debug renderer
	CDebugRenderer				*m_debug_renderer;
#endif

	CPHCommander				*m_ph_commander;
	CPHCommander				*m_ph_commander_scripts;
	CPHCommander				*m_ph_commander_physics_worldstep;
	
	// level name
	shared_str					m_name;
public:
#ifdef DEBUG
	// level debugger
	CLevelDebug					*m_level_debug;
#endif

public:
	////////////// network ////////////////////////
	static void 				PhisStepsCallback		( u32 Time0, u32 Time1 );

	virtual void				OnMessage				(void* data, u32 size);
private:
	CObject* pCurrentControlEntity;

public:
	CObject*					CurrentControlEntity	( void ) const		{ return pCurrentControlEntity; }
	void						SetControlEntity		( CObject* O  )		{ pCurrentControlEntity=O; }
public:
	//////////////////////////////////////////////	
	// static particles
	DEFINE_VECTOR				(CParticlesObject*,POVec,POIt);
	POVec						m_StaticParticles;

	NET_Queue_Event				*game_events;
	xrServer*					Server;
	GlobalFeelTouch				m_feel_deny;

private:
	// preload sounds registry
	DEFINE_MAP					(shared_str,ref_sound,SoundRegistryMap,SoundRegistryMapIt);
	SoundRegistryMap			sound_registry;

public:
	void						PrefetchSound (LPCSTR name);

protected:
	bool	xr_stdcall			net_start1				();
	bool	xr_stdcall			net_start2				();
	bool	xr_stdcall			net_start3				();

	bool	xr_stdcall			net_start_client1				();
	bool	xr_stdcall			net_start_client2				();
	bool	xr_stdcall			net_start_client3				();
private:
	void __stdcall BeginFrameLevel();
public:
	// sounds
	xr_vector<ref_sound*>		static_Sounds;

	// startup options
	shared_str					m_caServerOptions;

	// Starting/Loading
	virtual BOOL				net_Start				( LPCSTR op_server);
	virtual void				net_Stop				( );
	virtual void				net_Update				( );


	virtual BOOL				Load_GameSpecific_Before( );
	virtual BOOL				Load_GameSpecific_After ( );
	virtual void				Load_GameSpecific_CFORM	( CDB::TRI* T, u32 count );

	// Events
	virtual void				OnFrame					( void );
	virtual void				OnRender				( );
	void						cl_Process_Event		(u16 dest, u16 type, NET_Packet& P);
	void						ProcessGameEvents		( );

	// Input
	void IR_OnKeyboardPress(int btn) override;
	void IR_OnKeyboardRelease(int btn) override;
	void IR_OnKeyboardHold(int btn) override;
	void IR_OnMousePress(int btn) override;
	void IR_OnMouseRelease(int btn) override;
	void IR_OnMouseHold(int btn) override;
	void IR_OnMouseMove(float x, float y) override;
	virtual void				IR_OnMouseWheel			( int direction);
	virtual void				IR_OnActivate			(void);

	// Game
	void						ClientReceive			();
	void						SaveAllCSEObj();
	void						ClientSave				();
			u32					Objects_net_Save		(NET_Packet* _Packet, u32 start, u32 count);
	virtual	void				Send					(NET_Packet& P);
	
	virtual	NET_Packet* net_msg_Retreive();

	void						g_sv_Spawn				(CObject*, CSE_Abstract*);					// server reply/command spawning
	
	IC CSpaceRestrictionManager		&space_restriction_manager	();
	IC CSeniorityHierarchyHolder	&seniority_holder			();
	IC CClientSpawnManager			&client_spawn_manager		();
	IC CAutosaveManager				&autosave_manager			();
#ifdef DEBUG
	IC CDebugRenderer				&debug_renderer				();
#endif
	void	__stdcall				script_gc					();			// GC-cycle

	IC CPHCommander					&ph_commander				();
	IC CPHCommander					&ph_commander_scripts		();
	IC CPHCommander					&ph_commander_physics_worldstep();

	// C/D
	CLevel();
	virtual ~CLevel();

	//названияе текущего уровня
	virtual shared_str			name				() const;

	//gets the time from the game simulation
	
	//возвращает время в милисекундах относительно начала игры
	ALife::_TIME_ID		GetGameTime				();
	//игровое время в отформатированном виде
	void				GetGameDateTime			(u32& year, u32& month, u32& day, u32& hours, u32& mins, u32& secs, u32& milisecs);

	void SetGameTimeFactor(const float fTimeFactor);
	float GetGameTimeFactor		();
//	void				SetGameTime				(ALife::_TIME_ID GameTime);

	// gets current daytime [0..23]
	u8					GetDayTime				();
	u32					GetGameDayTimeMS		();
	float				GetGameDayTimeSec		();

protected:
//	CFogOfWarMngr*		m_pFogOfWarMngr;
protected:	
	CMapManager *			m_map_manager;
public:
	CMapManager&			MapManager					()	{return *m_map_manager;}
//	CFogOfWarMngr&			FogOfWarMngr				()	{return *m_pFogOfWarMngr;}

	//работа с пулями
protected:	
	CBulletManager*		m_pBulletManager;
public:
	IC CBulletManager&	BulletManager() {return	*m_pBulletManager;}

	CSE_Abstract	*spawn_item					(LPCSTR section, const Fvector &position, u32 level_vertex_id, u16 parent_id, bool return_item = false);
public:
	void			remove_objects				();

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CLevel)
#undef script_type_list
#define script_type_list save_type_list(CLevel)

IC CLevel&				Level()		{ return *((CLevel*) g_pGameLevel);			}
	u32					GameID();


IC CHUDManager&			HUD()		{ return *((CHUDManager*)Level().pHUD);	}

#ifdef DEBUG
IC CLevelDebug&			DBG()		{return *((CLevelDebug*)Level().m_level_debug);}
#endif


IC CSpaceRestrictionManager	&CLevel::space_restriction_manager()
{
	VERIFY				(m_space_restriction_manager);
	return				(*m_space_restriction_manager);
}

IC CSeniorityHierarchyHolder &CLevel::seniority_holder()
{
	VERIFY				(m_seniority_hierarchy_holder);
	return				(*m_seniority_hierarchy_holder);
}

IC CClientSpawnManager &CLevel::client_spawn_manager()
{
	VERIFY				(m_client_spawn_manager);
	return				(*m_client_spawn_manager);
}

IC CAutosaveManager &CLevel::autosave_manager()
{
	VERIFY				(m_autosave_manager);
	return				(*m_autosave_manager);
}

#ifdef DEBUG
IC CDebugRenderer &CLevel::debug_renderer()
{
	VERIFY				(m_debug_renderer);
	return				(*m_debug_renderer);
}
#endif

IC CPHCommander	& CLevel::ph_commander()
{
	VERIFY(m_ph_commander);
	return *m_ph_commander;
}
IC CPHCommander & CLevel::ph_commander_scripts()
{
	VERIFY(m_ph_commander_scripts);
	return *m_ph_commander_scripts;
}
IC CPHCommander& CLevel::ph_commander_physics_worldstep()
{
	VERIFY(m_ph_commander_scripts);
	return *m_ph_commander_physics_worldstep;
}

	bool				IsGameTypeSingle();

