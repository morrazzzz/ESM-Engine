#include "stdafx.h"
#include "xrServer_Objects_ALife_All.h"
#include "level.h"
#include "GameObject.h"
#include "ai_space.h"
#include "game_level_cross_table.h"
#include "level_graph.h"
#include "client_spawn_manager.h"
#include "../xr_3da/xr_object.h"
#include "../xr_3da/IGame_Persistent.h"

#ifdef DEBUG
	extern Flags32				psAI_Flags;
	extern float				debug_on_frame_gather_stats_frequency;
#	include "ai_debug.h"
#endif // DEBUG

void CLevel::g_sv_Spawn(CObject* obj, CSE_Abstract* E)
{
#ifdef DEBUG_MEMORY_MANAGER
	u32							E_mem = 0;
	if (g_bMEMO)	{
		lua_gc					(ai().script_engine().lua(),LUA_GCCOLLECT,0);
		lua_gc					(ai().script_engine().lua(),LUA_GCCOLLECT,0);
		E_mem					= Memory.mem_usage();	
		Memory.stat_calls		= 0;
	}
#endif // DEBUG_MEMORY_MANAGER
	Msg("ID: [%d]", E->ID);

#ifdef DEBUG_MEMORY_MANAGER
	mem_alloc_gather_stats		(false);
#endif // DEBUG_MEMORY_MANAGER
	if (!obj->net_Spawn(E))
	{
		obj->net_Destroy();
		client_spawn_manager().clear(obj->ID());
		Objects.Destroy(obj);
		Msg("! Failed to spawn entity '%s'", *E->s_name);
#ifdef DEBUG_MEMORY_MANAGER
		mem_alloc_gather_stats(!!psAI_Flags.test(aiDebugOnFrameAllocs));
#endif // DEBUG_MEMORY_MANAGER
	}
	else 
	{
#ifdef DEBUG_MEMORY_MANAGER
		mem_alloc_gather_stats(!!psAI_Flags.test(aiDebugOnFrameAllocs));
#endif // DEBUG_MEMORY_MANAGER
		client_spawn_manager().callback(obj);
		//Msg			("--spawn--SPAWN: %f ms",1000.f*T.GetAsync());
		if (E->s_flags.is(M_SPAWN_OBJECT_LOCAL) && E->s_flags.is(M_SPAWN_OBJECT_ASPLAYER)) {
			if (CurrentEntity())
			{
				CGameObject* pGO = smart_cast<CGameObject*>(CurrentEntity());
				if (pGO) pGO->On_B_NotCurrentEntity();
			}
			SetEntity(obj);
			SetControlEntity(obj);
		}

		if (0xffff != E->ID_Parent)
		{
			CGameObject* objParent = static_cast<CGameObject*>(Objects.net_Find(E->ID_Parent));
			R_ASSERT(objParent);
            
			objParent->TakeItem(static_cast<CGameObject*>(obj));
		}
	}
#ifdef DEBUG_MEMORY_MANAGER
	if (g_bMEMO) {
		lua_gc					(ai().script_engine().lua(),LUA_GCCOLLECT,0);
		lua_gc					(ai().script_engine().lua(),LUA_GCCOLLECT,0);
		Msg						("* %20s : %d bytes, %d ops", *E->s_name,Memory.mem_usage()-E_mem, Memory.stat_calls );
	}
#endif // DEBUG_MEMORY_MANAGER
}

CSE_Abstract *CLevel::spawn_item		(LPCSTR section, const Fvector &position, u32 level_vertex_id, u16 parent_id, bool return_item)
{
	CSE_Abstract			*abstract = F_entity_Create(section);
	R_ASSERT3				(abstract,"Cannot find item with section",section);
	CSE_ALifeDynamicObject	*dynamic_object = smart_cast<CSE_ALifeDynamicObject*>(abstract);
	if (dynamic_object && ai().get_level_graph()) {
		dynamic_object->m_tNodeID	= level_vertex_id;
		if (ai().level_graph().valid_vertex_id(level_vertex_id) && ai().get_game_graph() && ai().get_cross_table())
			dynamic_object->m_tGraphID	= ai().cross_table().vertex(level_vertex_id).game_vertex_id();
	}

	//оружие спавним с полным магазинои
	CSE_ALifeItemWeapon* weapon = smart_cast<CSE_ALifeItemWeapon*>(abstract);
	if(weapon)
		weapon->a_elapsed	= weapon->get_ammo_magsize();
	
	// Fill
	abstract->s_name		= section;
	abstract->set_name_replace	(section);
	abstract->s_gameid		= u8(GameID());
	abstract->o_Position	= position;
	abstract->s_RP			= 0xff;
	abstract->ID			= 0xffff;
	abstract->ID_Parent		= parent_id;
	abstract->ID_Phantom	= 0xffff;
	abstract->s_flags.assign(M_SPAWN_OBJECT_LOCAL);
	abstract->RespawnTime	= 0;

	if (!return_item) {
		NET_Packet				P;
		abstract->Spawn_Write	(P,TRUE);
		Send					(P);
		F_entity_Destroy		(abstract);
		return					(0);
	}
	else
		return				(abstract);
}
