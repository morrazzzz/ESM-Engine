#include "pch_script.h"
#include "../xr_3da/fdemoplay.h"
#include "../xr_3da/environment.h"
#include "../xr_3da/IGame_Persistent.h"
#include "ParticlesObject.h"
#include "Level.h"
#include "xrServer.h"
#include "net_queue.h"
#include "hudmanager.h"
#include "ai_space.h"
#include "ai_debug.h"
#include "ShootingObject.h"
#include "Level_Bullet_Manager.h"
#include "script_process.h"
#include "script_engine.h"
#include "script_engine_space.h"
#include "team_base_zone.h"
#include "date_time.h"
#include "space_restriction_manager.h"
#include "seniority_hierarchy_holder.h"
#include "client_spawn_manager.h"
#include "autosave_manager.h"
#include "mt_config.h"
#include "phcommander.h"
#include "map_manager.h"
#include "../xr_3da/CameraManager.h"
#include "level_sounds.h"
#include "trade_parameters.h"
#include "clsid_game.h"
#include "MainMenu.h"
#include "player_hud.h"

#include <functional>

#include "../xrPhysics/PhysicsShell.h"
#include "../xrPhysics/iphworld.h"
#include "../xrPhysics/console_vars.h"

#ifdef DEBUG
#include "space_restrictor.h"
#include "level_debug.h"
#include "ai/stalker/ai_stalker.h"
#include "debug_renderer.h"
#include "physicobject.h"
#include "ClimableObject.h"
#include "level_graph.h"
#include "../xr_3da/xr_object.h"
#endif

#include "alife_simulator.h"
#include "alife_object_registry.h"
#include "alife_graph_registry.h"
#include "alife_time_manager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLevel::CLevel():IPureClient	(Device.GetTimerGlobal())
#ifdef PROFILE_CRITICAL_SECTIONS
	,DemoCS(MUTEX_PROFILE_ID(DemoCS))
#endif // PROFILE_CRITICAL_SECTIONS
{
	Server						= NULL;

	game_events					= xr_new<NET_Queue_Event>();

	m_pBulletManager			= xr_new<CBulletManager>();

	m_map_manager				= xr_new<CMapManager>();

//	m_pFogOfWarMngr				= xr_new<CFogOfWarMngr>();
	//VERIFY						( physics_world() );
	//physics_world()->set_step_time_callback((PhysicsStepTimeCallback*) &PhisStepsCallback);
	//physics_step_time_callback	= (PhysicsStepTimeCallback*) &PhisStepsCallback;
	m_seniority_hierarchy_holder= xr_new<CSeniorityHierarchyHolder>();

	if(!g_dedicated_server)
	{
		m_level_sound_manager		= xr_new<CLevelSoundManager>();
		m_space_restriction_manager = xr_new<CSpaceRestrictionManager>();
		m_client_spawn_manager		= xr_new<CClientSpawnManager>();
		m_autosave_manager			= xr_new<CAutosaveManager>();

	#ifdef DEBUG
		m_debug_renderer			= xr_new<CDebugRenderer>();
		m_level_debug				= xr_new<CLevelDebug>();
	#endif

	}else
	{
		m_level_sound_manager		= NULL;
		m_client_spawn_manager		= NULL;
		m_autosave_manager			= NULL;
		m_space_restriction_manager = NULL;
	#ifdef DEBUG
		m_debug_renderer			= NULL;
		m_level_debug				= NULL;
	#endif
	}


	
	m_ph_commander				= xr_new<CPHCommander>();
	m_ph_commander_scripts		= xr_new<CPHCommander>();

	pCurrentControlEntity = NULL;
	
	R_ASSERT				(NULL==g_player_hud);
	g_player_hud			= xr_new<player_hud>();
	g_player_hud->LoadDefaultActorHudIfExist();

//	if ( !strstr( Core.Params, "-tdemo " ) && !strstr(Core.Params,"-tdemof "))
//	{
//		Demo_PrepareToStore();
//	};
	//---------------------------------------------------------
//	m_bDemoPlayMode = FALSE;
//	m_aDemoData.clear();
//	m_bDemoStarted	= FALSE;

	Msg("%s", Core.Params);
	/*
	if (strstr(Core.Params,"-tdemo ") || strstr(Core.Params,"-tdemof ")) {		
		string1024				f_name;
		if (strstr(Core.Params,"-tdemo "))
		{
			sscanf					(strstr(Core.Params,"-tdemo ")+7,"%[^ ] ",f_name);
			m_bDemoPlayByFrame = FALSE;

			Demo_Load	(f_name);	
		}
		else
		{
			sscanf					(strstr(Core.Params,"-tdemof ")+8,"%[^ ] ",f_name);
			m_bDemoPlayByFrame = TRUE;

			m_lDemoOfs = 0;
			Demo_Load_toFrame(f_name, 100, m_lDemoOfs);
		};		
	}
	*/
	//---------------------------------------------------------	
}

CLevel::~CLevel()
{
	xr_delete					(g_player_hud);

	Msg							("- Destroying level");

	if (physics_world())
	{
		destroy_physics_world();
		xr_delete(m_ph_commander_physics_worldstep);
	}

	// destroy PSs
	for (POIt p_it=m_StaticParticles.begin(); m_StaticParticles.end()!=p_it; ++p_it)
		CParticlesObject::Destroy(*p_it);
	m_StaticParticles.clear		();

	// Unload sounds
	// unload prefetched sounds
	sound_registry.clear		();

	// unload static sounds
	for (u32 i=0; i<static_Sounds.size(); ++i){
		static_Sounds[i]->destroy();
		xr_delete				(static_Sounds[i]);
	}
	static_Sounds.clear			();

	xr_delete					(m_level_sound_manager);

	xr_delete					(m_space_restriction_manager);

	xr_delete					(m_seniority_hierarchy_holder);
	
	xr_delete					(m_client_spawn_manager);

	xr_delete					(m_autosave_manager);
	
#ifdef DEBUG
	xr_delete					(m_debug_renderer);
#endif

	if (!g_dedicated_server)
		ai().script_engine().remove_script_process(ScriptEngine::eScriptProcessorLevel);

	xr_delete					(game_events);


	//by Dandy
	//destroy fog of war
//	xr_delete					(m_pFogOfWar);
	//destroy bullet manager
	xr_delete					(m_pBulletManager);

	xr_delete					(m_ph_commander);
	xr_delete					(m_ph_commander_scripts);

	ai().unload					();
	//-----------------------------------------------------------	
#ifdef DEBUG	
	xr_delete					(m_level_debug);
#endif
	//-----------------------------------------------------------
	xr_delete					(m_map_manager);
//	xr_delete					(m_pFogOfWarMngr);

	// here we clean default trade params
	// because they should be new for each saved/loaded game
	// and I didn't find better place to put this code in
	CTradeParameters::clean		();
}

shared_str	CLevel::name		() const
{
	return						(m_name);
}

void CLevel::PrefetchSound		(LPCSTR name)
{
	// preprocess sound name
	string_path					tmp;
	strcpy_s					(tmp,name);
	xr_strlwr					(tmp);
	if (strext(tmp))			*strext(tmp)=0;
	shared_str	snd_name		= tmp;
	// find in registry
	SoundRegistryMapIt it		= sound_registry.find(snd_name);
	// if find failed - preload sound
	if (it==sound_registry.end())
		sound_registry[snd_name].create(snd_name.c_str(),st_Effect,sg_SourceType);
}

void CLevel::cl_Process_Event				(u16 dest, u16 type, NET_Packet& P)
{
	//			Msg				("--- event[%d] for [%d]",type,dest);
	CObject*	 O	= Objects.net_Find	(dest);
	if (0==O)		{
#ifdef DEBUG
		Msg("* WARNING: c_EVENT[%d] to [%d]: unknown dest",type,dest);
#endif // DEBUG
		return;
	}
	CGameObject* GO = smart_cast<CGameObject*>(O);
	if (!GO)		{
		Msg("! ERROR: c_EVENT[%d] : non-game-object",dest);
		return;
	}

	GO->OnEvent		(P,type);
};

void CLevel::ProcessGameEvents		()
{
	// Game events
	while (game_events->available())
	{
		NET_Packet P{};

		u16 ID, dest, type;
		game_events->get(ID, dest, type, P);

		switch (ID)
		{
		case M_EVENT:
		{
			cl_Process_Event(dest, type, P);
		}break;
		default:
		{
			VERIFY(0);
		}break;
		}
	}
}

#ifdef DEBUG_MEMORY_MANAGER
	extern Flags32				psAI_Flags;
	extern float				debug_on_frame_gather_stats_frequency;

struct debug_memory_guard {
	inline debug_memory_guard	()
	{
		mem_alloc_gather_stats				(!!psAI_Flags.test(aiDebugOnFrameAllocs));
		mem_alloc_gather_stats_frequency	(debug_on_frame_gather_stats_frequency);
	}

	inline ~debug_memory_guard	()
	{
//		mem_alloc_gather_stats				(false);
	}
};
#endif // DEBUG_MEMORY_MANAGER

void CLevel::OnFrame	()
{
#ifdef DEBUG_MEMORY_MANAGER
	debug_memory_guard					__guard__;
#endif // DEBUG_MEMORY_MANAGER

	m_feel_deny.update					();

	// commit events from bullet manager from prev-frame
	Device.Statistic->BulletManager.Begin		();
	if (g_mt_config.test(mtBullets))
		Device.seqParallel.emplace_back(fastdelegate::FastDelegate0(m_pBulletManager, &CBulletManager::CommitEvents));
	else
		BulletManager().CommitEvents();
	Device.Statistic->BulletManager.End			();

	// Client receive
	Device.Statistic->netClient1.Begin();

	ClientReceive();

	Device.Statistic->netClient1.End();

//	CTimer T;
//	T.Start();

	ProcessGameEvents	();

	Server->SpawnNewObjects();

	if (g_mt_config.test(mtMap))
		Device.seqParallel.emplace_back(fastdelegate::FastDelegate0(m_map_manager, &CMapManager::Update));
	else
		MapManager().Update	();

	// Inherited update
	inherited::OnFrame		();
	
	g_pGamePersistent->Environment().SetGameTime	(GetGameDayTimeSec(),GetGameTimeFactor());

	//Device.Statistic->cripting.Begin	();
	if (!g_dedicated_server)
		ai().script_engine().script_process	(ScriptEngine::eScriptProcessorLevel)->update();
	//Device.Statistic->Scripting.End	();
	m_ph_commander->update				();
	m_ph_commander_scripts->update		();
//	autosave_manager().update			();

	//просчитать полет пуль
	Device.Statistic->BulletManager.Begin		();
	BulletManager().CommitRenderSet		();
	Device.Statistic->BulletManager.End			();

	// update static sounds
	if(!g_dedicated_server)
	{
		if (g_mt_config.test(mtLevelSounds)) 
			Device.seqParallel.emplace_back(fastdelegate::FastDelegate0<>(m_level_sound_manager,&CLevelSoundManager::Update));
		else								
			m_level_sound_manager->Update	();
	}
	// deffer LUA-GC-STEP
	if (!g_dedicated_server)
	{
		if (g_mt_config.test(mtLUA_GC))
			Device.seqParallel.emplace_back(fastdelegate::FastDelegate0<>(this,&CLevel::script_gc));
		else							script_gc	()	;
	}

	{
		PROF_EVENT("Spatial Move");
		g_SpatialSpace->UpdateSpatialMove();
		g_SpatialSpacePhysic->UpdateSpatialMove();
	}
}

int		psLUA_GCSTEP					= 10			;
void	CLevel::script_gc				()
{
	lua_gc	(ai().script_engine().lua(), LUA_GCSTEP, psLUA_GCSTEP);
}

extern void draw_wnds_rects();

void CLevel::OnRender()
{
	inherited::OnRender	();
	
	//отрисовать трассы пуль
	//Device.Statistic->TEST1.Begin();
	BulletManager().Render();
	//Device.Statistic->TEST1.End();
	//отрисовать интерфейc пользователя
	HUD().RenderUI();

	draw_wnds_rects();


#ifdef DEBUG
	physics_world()->OnRender();
#endif

#ifdef DEBUG
	if (ai().get_level_graph())
		ai().level_graph().render();

	CAI_Stalker				*stalker = smart_cast<CAI_Stalker*>(Level().CurrentEntity());
	if (stalker)
		stalker->OnRender	();

	if (bDebug)	{
		for (u32 I=0; I < Level().Objects.o_count(); I++) {
			CObject*	_O		= Level().Objects.o_get_by_iterator(I);

			CPhysicObject		*physic_object = smart_cast<CPhysicObject*>(_O);
			if (physic_object)
				physic_object->OnRender();

			CSpaceRestrictor	*space_restrictor = smart_cast<CSpaceRestrictor*>	(_O);
			if (space_restrictor)
				space_restrictor->OnRender();
			CClimableObject		*climable		  = smart_cast<CClimableObject*>	(_O);
			if(climable)
				climable->OnRender();
			
			if (GameID() != GAME_SINGLE)
			{
				CInventoryItem* pIItem = smart_cast<CInventoryItem*>(_O);
				if (pIItem) pIItem->OnRender();
			}

			
			if (dbg_net_Draw_Flags.test(dbg_draw_skeleton)) //draw skeleton
			{
				CGameObject* pGO = smart_cast<CGameObject*>	(_O);
				if (pGO && pGO != Level().CurrentViewEntity() && !pGO->H_Parent())
				{
					if (pGO->Position().distance_to_sqr(Device.vCameraPosition) < 400.0f)
					{
						pGO->dbg_DrawSkeleton();
					}
				}
			};
		}
		//  [7/5/2005]
		ObjectSpace.dbgRender	();

		//---------------------------------------------------------------------
		UI().Font().pFontStat->OutSet		(170,630);
		UI().Font().pFontStat->SetHeight	(16.0f);
		UI().Font().pFontStat->SetColor	(0xffff0000);

		if(Server)UI().Font().pFontStat->OutNext	("Client Objects:      [%d]",Server->GetEntitiesNum());
		UI().Font().pFontStat->OutNext	("Server Objects:      [%d]",Objects.o_count());
		UI().Font().pFontStat->SetHeight	(8.0f);
		//---------------------------------------------------------------------
	}
#endif

#ifdef DEBUG
	if (bDebug) {
		DBG().draw_object_info				();
		DBG().draw_text						();
		DBG().draw_level_info				();
	}

	debug_renderer().render					();

	if (psAI_Flags.is(aiVision)) {
		for (u32 I=0; I < Level().Objects.o_count(); I++) {
			CObject						*object = Objects.o_get_by_iterator(I);
			CAI_Stalker					*stalker = smart_cast<CAI_Stalker*>(object);
			if (!stalker)
				continue;
			stalker->dbg_draw_vision	();
		}
	}


	if (psAI_Flags.test(aiDrawVisibilityRays)) {
		for (u32 I=0; I < Level().Objects.o_count(); I++) {
			CObject						*object = Objects.o_get_by_iterator(I);
			CAI_Stalker					*stalker = smart_cast<CAI_Stalker*>(object);
			if (!stalker)
				continue;

			stalker->dbg_draw_visibility_rays	();
		}
	}
#endif
}

void 		CLevel::PhisStepsCallback		( u32 Time0, u32 Time1 )
{
	if (GameID() == GAME_SINGLE)	return;

//#pragma todo("Oles to all: highly inefficient and slow!!!")
//fixed (Andy)
	/*
	for (xr_vector<CObject*>::iterator O=Level().Objects.objects.begin(); O!=Level().Objects.objects.end(); ++O) 
	{
		if( (*O)->CLS_ID == CLSID_OBJECT_ACTOR){
			CActor* pActor = smart_cast<CActor*>(*O);
			if (!pActor || pActor->Remote()) continue;
				pActor->UpdatePosStack(Time0, Time1);
		}
	};
	*/
};


ALife::_TIME_ID CLevel::GetGameTime()
{
	return ai().get_alife()->time_manager().game_time();
}

u8 CLevel::GetDayTime() 
{ 
	u32 dummy32;
	u32 hours;
	GetGameDateTime(dummy32, dummy32, dummy32, hours, dummy32, dummy32, dummy32);
	VERIFY	(hours<256);
	return	u8(hours); 
}

float CLevel::GetGameDayTimeSec()
{
	return	(float(s64(GetGameTime() % (24*60*60*1000)))/1000.f);
}

u32 CLevel::GetGameDayTimeMS()
{
	return	(u32(s64(GetGameTime() % (24*60*60*1000))));
}

void CLevel::GetGameDateTime	(u32& year, u32& month, u32& day, u32& hours, u32& mins, u32& secs, u32& milisecs)
{
	split_time(GetGameTime(), year, month, day, hours, mins, secs, milisecs);
}

void CLevel::SetGameTimeFactor(const float fTimeFactor)
{
	ai().get_alife()->time_manager().set_time_factor(fTimeFactor);
}

float CLevel::GetGameTimeFactor()
{
	return ai().get_alife()->time_manager().time_factor();
}

u32	GameID()
{
	return GAME_SINGLE;
}

bool	IsGameTypeSingle()
{
	return g_pGamePersistent->GameType()==GAME_SINGLE || g_pGamePersistent->GameType()==GAME_ANY;
}

GlobalFeelTouch::GlobalFeelTouch()
{
}

GlobalFeelTouch::~GlobalFeelTouch()
{
}

struct delete_predicate_by_time
{
	bool operator () (Feel::Touch::DenyTouch const & left, DWORD const expire_time) const
	{
		if (left.Expire <= expire_time)
			return true;
		return false;
	};
};
struct objects_ptrs_equal
{
	bool operator() (Feel::Touch::DenyTouch const & left, CObject const * const right) const
	{
		if (left.O == right)
			return true;
		return false;
	}
};

void GlobalFeelTouch::update()
{
	//we ignore P and R arguments, we need just delete evaled denied objects...
	xr_vector<Feel::Touch::DenyTouch>::iterator new_end = std::remove_if(feel_touch_disable.begin(),
		feel_touch_disable.end(), std::bind(delete_predicate_by_time(), std::placeholders::_1, Device.dwTimeGlobal));;
	feel_touch_disable.erase(new_end, feel_touch_disable.end());
}

bool GlobalFeelTouch::is_object_denied(CObject const * O)
{
	/*Fvector temp_vector;
	feel_touch_update(temp_vector, 0.f);*/
	if (std::find_if(feel_touch_disable.begin(), feel_touch_disable.end(), std::bind(objects_ptrs_equal(), std::placeholders::_1, O)) ==
		feel_touch_disable.end())
	{
		return false;
	}
	return true;
}