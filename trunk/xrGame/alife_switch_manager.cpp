#include "stdafx.h"
#include "alife_switch_manager.h"
#include "xrServer_Objects_ALife.h"
#include "alife_graph_registry.h"
#include "alife_object_registry.h"
#include "alife_schedule_registry.h"
#include "game_level_cross_table.h"
#include "xrserver.h"
#include "ai_space.h"
#include "level_graph.h"
#include "GameObject.h"
#include "Level.h"

using namespace ALife;

void CALifeSwitchManager::add_online(CSE_ALifeDynamicObject *object, bool update_registries)
{
	START_PROFILE("ALife/switch/add_online")
	VERIFY							((ai().game_graph().vertex(object->m_tGraphID)->level_id() == graph().level().level_id()));

	object->m_bOnline				= true;

	NET_Packet						tNetPacket;
	CSE_Abstract					*l_tpAbstract = smart_cast<CSE_Abstract*>(object);
	server().entity_Destroy			(l_tpAbstract);
	object->s_flags.Or				(M_SPAWN_UPDATE);
	server().Process_spawn			(tNetPacket,l_tpAbstract);
	object->s_flags.And				(u16(-1) ^ M_SPAWN_UPDATE);
	R_ASSERT3						(!object->used_ai_locations() || ai().level_graph().valid_vertex_id(object->m_tNodeID),"Invalid vertex for object ",object->name_replace());

#ifdef DEBUG
	if (psAI_Flags.test(aiALife))
		Msg							("[LSS] Spawning object [%s][%s][%d]",object->name_replace(),*object->s_name,object->ID);
#endif

	object->add_online				(update_registries);
	STOP_PROFILE
}

void CALifeSwitchManager::remove_online(CSE_ALifeDynamicObject *object, bool update_registries)
{
	START_PROFILE("ALife/switch/remove_online")
	object->m_bOnline			= false;

	CGameObject* game_object = static_cast<CGameObject*>(Level().Objects.net_Find(object->ID));

	for (u32 i = 0; i < object->children.size(); i++)
	{
		CSE_Abstract* child = object->children[i];
		CGameObject* object_child = static_cast<CGameObject*>(Level().Objects.net_Find(child->ID));

		if (object_child)
		{
			game_object->ObjectRejectItem(object_child, true);

			object_child->setDestroy();
		}

		server().entity_Destroy(child);

		if (!child || !child->m_bALifeControl)
		{
			object->children.erase(object->children.begin() + i);
			i--;
			continue;
		}

		//if (inventory_owner && !child->cast_alife_object()->can_save())
		//	continue;

		object->add_offline(child, false);
	}

	object->add_offline(nullptr, update_registries);

	game_object->setDestroy();

	CSE_Abstract* objectDC = object;

	server().entity_Destroy(objectDC);

	_OBJECT_ID					object_id = object->ID;
	object->ID = server().PerformIDgen(object_id);

#ifdef DEBUG
	if (psAI_Flags.test(aiALife))
		Msg						("[%s] Destroying object [%s][%s][%d]",__FUNCTION__,object->name_replace(),*object->s_name,object->ID);
#endif

	STOP_PROFILE
}

void CALifeSwitchManager::switch_online(CSE_ALifeDynamicObject *object)
{
	START_PROFILE("ALife/switch/switch_online")
#ifdef DEBUG
	if (psAI_Flags.test(aiALife))
		Msg						("[LSS][%d] Going online [%d][%s][%d] ([%f][%f][%f] : [%f][%f][%f]), on '%s'",Device.dwFrame,Device.dwTimeGlobal,object->name_replace(), object->ID,VPUSH(graph().actor()->o_Position),VPUSH(object->o_Position), "*SERVER*");
#endif
	object->switch_online		();
	STOP_PROFILE
}

void CALifeSwitchManager::switch_offline(CSE_ALifeDynamicObject *object)
{
	START_PROFILE("ALife/switch/switch_offline")
#ifdef DEBUG
	if (psAI_Flags.test(aiALife))
		Msg							("[LSS][%d] Going offline [%d][%s][%d] ([%f][%f][%f] : [%f][%f][%f]), on '%s'",Device.dwFrame,Device.dwTimeGlobal,object->name_replace(), object->ID,VPUSH(graph().actor()->o_Position),VPUSH(object->o_Position), "*SERVER*");
#endif
	object->switch_offline			();
	STOP_PROFILE
}

void CALifeSwitchManager::synchronize_location(CSE_ALifeDynamicObject* I)
{
	if (I->m_SyncLocationObject > Device.dwFrame)
		return;

	START_PROFILE("ALife/switch/synchronize_location")
#ifdef DEBUG		
	VERIFY3(ai().level_graph().level_id() == ai().game_graph().vertex(I->m_tGraphID)->level_id(), *I->s_name, I->name_replace());
	
	CTimer T; T.Start();
	for (u16 i = 1; i < I->children.size(); i++)
		R_ASSERT2(I->children[i - 1] != I->children[i], "Child is registered twice in the child list");

	float get_ms = T.GetElapsed_sec() * 1000.f;

	if (get_ms > 0.25f)
		Msg("! [%s]: Slow check child: [%d]!!!", __FUNCTION__, get_ms);
#endif // DEBUG

	// check if we do not use ai locations
	if (!I->used_ai_locations())
		return;

	// check if we are not attached
	if (0xffff != I->ID_Parent)
		return;

	// check if we are not online and have an invalid level vertex id
	if (!I->m_bOnline && !ai().level_graph().valid_vertex_id(I->m_tNodeID))
		return;

	I->synchronize_location();
	
	//Update sync.	
	I->m_SyncLocationObject = Device.dwFrame + 10;

	STOP_PROFILE
}

void CALifeSwitchManager::try_switch_online	(CSE_ALifeDynamicObject	*I)
{
	START_PROFILE("ALife/switch/try_switch_online")
	// so, the object is offline
	// checking if the object is not attached
	if (0xffff != I->ID_Parent) {
		// so, object is attached
		// checking if parent is offline too
#ifdef DEBUG
		if (psAI_Flags.test(aiALife)) {
			CSE_ALifeCreatureAbstract	*l_tpALifeCreatureAbstract = smart_cast<CSE_ALifeCreatureAbstract*>(objects().object(I->ID_Parent));
			if (l_tpALifeCreatureAbstract && (l_tpALifeCreatureAbstract->fHealth < EPS_L))
				Msg				("! uncontrolled situation [%d][%d][%s][%f]",I->ID,I->ID_Parent,l_tpALifeCreatureAbstract->name_replace(),l_tpALifeCreatureAbstract->fHealth);
			VERIFY2				(!l_tpALifeCreatureAbstract || (l_tpALifeCreatureAbstract->fHealth >= EPS_L),"Parent online, item offline...");
			if (objects().object(I->ID_Parent)->m_bOnline)
				Msg				("! uncontrolled situation [%d][%d][%s][%f]",I->ID,I->ID_Parent,l_tpALifeCreatureAbstract->name_replace(),l_tpALifeCreatureAbstract->fHealth);
		}
		VERIFY2					(!objects().object(I->ID_Parent)->m_bOnline,"Parent online, item offline...");
#endif
		return;
	}

/*
	VERIFY2						(
		(
			ai().game_graph().vertex(I->m_tGraphID)->level_id()
			!=
			ai().level_graph().level_id()
		) ||
		!Level().Objects.net_Find(I->ID) ||
		Level().Objects.dump_all_objects(),
		make_string("frame [%d] time [%d] object [%s] with id [%d] is offline, but is on the level",Device.dwFrame,Device.dwTimeGlobal,I->name_replace(),I->ID)
	);
*/

	I->try_switch_online		();

	if (!I->client_data.empty() && !I->m_bOnline)
		I->client_data.clear();

	STOP_PROFILE
}

void CALifeSwitchManager::try_switch_offline(CSE_ALifeDynamicObject	*I)
{
	START_PROFILE("ALife/switch/try_switch_offline")
	// checking if the object is not attached
	if (0xffff != I->ID_Parent) {
#ifdef DEBUG
		// checking if parent is online too
		CSE_ALifeCreatureAbstract	*l_tpALifeCreatureAbstract = smart_cast<CSE_ALifeCreatureAbstract*>(objects().object(I->ID_Parent));
		if (l_tpALifeCreatureAbstract && (l_tpALifeCreatureAbstract->fHealth < EPS_L))
			Msg				("! uncontrolled situation [%d][%d][%s][%f]",I->ID,I->ID_Parent,l_tpALifeCreatureAbstract->name_replace(),l_tpALifeCreatureAbstract->fHealth);

		VERIFY2				(!smart_cast<CSE_ALifeCreatureAbstract*>(objects().object(I->ID_Parent)) || (smart_cast<CSE_ALifeCreatureAbstract*>(objects().object(I->ID_Parent))->fHealth >= EPS_L),"Parent offline, item online...");

		if (!objects().object(I->ID_Parent)->m_bOnline)
			Msg				("! uncontrolled situation [%d][%d][%s][%f]",I->ID,I->ID_Parent,l_tpALifeCreatureAbstract->name_replace(),l_tpALifeCreatureAbstract->fHealth);

		VERIFY2				(objects().object(I->ID_Parent)->m_bOnline,"Parent offline, item online...");
#endif
		return;
	}

	I->try_switch_offline	();
	STOP_PROFILE
}

void CALifeSwitchManager::switch_object	(CSE_ALifeDynamicObject	*I)
{
	if (I->redundant()) {
		release				(I);
		return;
	}

	synchronize_location(I);

	if (I->m_bOnline)
		try_switch_offline	(I);
	else
		try_switch_online	(I);

	if (I->redundant())
		release				(I);
}
