////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_dynamic_object.cpp
//	Created 	: 27.10.2005
//  Modified 	: 27.10.2005
//	Author		: Dmitriy Iassenev
//	Description : ALife dynamic object class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects_ALife.h"
#include "alife_simulator.h"
#include "alife_schedule_registry.h"
#include "alife_graph_registry.h"
#include "alife_object_registry.h"
#include "level_graph.h"
#include "game_level_cross_table.h"
#include "game_graph.h"
#include "xrServer.h"

void CSE_ALifeDynamicObject::on_spawn				()
{
#ifdef DEBUG
//	Msg			("[LSS] spawning object [%d][%d][%s][%s]",ID,ID_Parent,name(),name_replace());
#endif
}

void CSE_ALifeDynamicObject::on_register			()
{
	CSE_ALifeObject		*object = this;
	while (object->ID_Parent != ALife::_OBJECT_ID(-1)) {
		object			= ai().alife().objects().object(object->ID_Parent);
		VERIFY			(object);
	}

//	if (!alife().graph().level().object(object->ID,true) && !keep_saved_data_anyway())
//		client_data.clear					();
}

void CSE_ALifeDynamicObject::on_before_register		()
{
}

#include "level.h"
#include "map_manager.h"

void CSE_ALifeDynamicObject::on_unregister()
{
	Level().MapManager().RemoveMapLocationByObjectID(ID);
}

void CSE_ALifeDynamicObject::switch_online			()
{
	R_ASSERT					(!m_bOnline);
	m_bOnline					= true;
	alife().add_online			(this);
}

void CSE_ALifeDynamicObject::switch_offline			()
{
	R_ASSERT					(m_bOnline);
	m_bOnline					= false;
	alife().remove_online		(this);
	if (!client_data.empty())
	{
#ifdef DEBUG
		Msg("! [%s]: client_data is cleared for [%d][%s]!!!", __FUNCTION__, ID, name_replace());
#endif
		client_data.clear();
	}
}

void CSE_ALifeDynamicObject::add_online				(const bool &update_registries)
{
	if (!update_registries)
		return;

	alife().scheduled().remove	(this);
	alife().graph().remove		(this,m_tGraphID,false);
}

void CSE_ALifeDynamicObject::add_offline			(CSE_Abstract* children, const bool &update_registries)
{
	if (!update_registries)
		return;

	alife().scheduled().add		(this);
	alife().graph().add			(this,m_tGraphID,false);
}

void CSE_ALifeDynamicObject::synchronize_location()
{
	if (!ai().level_graph().valid_vertex_position(o_Position) || ai().level_graph().inside(ai().level_graph().vertex(m_tNodeID), o_Position))
		return;

	m_tNodeID = ai().level_graph().vertex(m_tNodeID, o_Position);

	GameGraph::_GRAPH_ID		tGraphID = ai().cross_table().vertex(m_tNodeID).game_vertex_id();
	if (tGraphID != m_tGraphID) {
		if (!m_bOnline) {
			Fvector					position = o_Position;
			u32						level_vertex_id = m_tNodeID;
			alife().graph().change(this, m_tGraphID, tGraphID);
			if (ai().level_graph().inside(ai().level_graph().vertex(level_vertex_id), position)) {
				level_vertex_id = m_tNodeID;
				o_Position = position;
			}
		}
		else {
			VERIFY(ai().game_graph().vertex(tGraphID)->level_id() == alife().graph().level().level_id());
			m_tGraphID = tGraphID;
		}
	}

	m_fDistance = ai().cross_table().vertex(m_tNodeID).distance();
}

void CSE_ALifeDynamicObject::try_switch_online		()
{
	CSE_ALifeSchedulable						*schedulable = smart_cast<CSE_ALifeSchedulable*>(this);
	// checking if the abstract monster has just died
	if (schedulable) {
		if (!schedulable->need_update(this)) {
			if (alife().scheduled().object(ID,true))
				alife().scheduled().remove	(this);
		}
		else
			if (!alife().scheduled().object(ID,true))
				alife().scheduled().add		(this);
	}

	if (!can_switch_online()) {
		if (!client_data.empty())
		{
#ifdef DEBUG
			Msg("! [%s] [1]: client_data is cleared for [%d][%s]!!!", __FUNCTION__, ID, name_replace());
#endif // DEBUG
			client_data.clear();
		}
		return;
	}
	
	if (!can_switch_offline()) {
		alife().switch_online	(this);
		return;
	}

	if (alife().graph().actor()->o_Position.distance_to(o_Position) > alife().online_distance()) {
		if (!client_data.empty())
		{
#ifdef DEBUG
			Msg("! [%s] [2]: client_data is cleared for [%d][%s]!!!", __FUNCTION__, ID, name_replace());
#endif // DEBUG
			client_data.clear();
		}
		return;
	}

	alife().switch_online		(this);
}

void CSE_ALifeDynamicObject::try_switch_offline		()
{
	if (!can_switch_offline())
		return;
	
	if (!can_switch_online()) {
		alife().switch_offline	(this);
		return;
	}

	if (alife().graph().actor()->o_Position.distance_to(o_Position) <= alife().offline_distance())
		return;

	alife().switch_offline		(this);
}

bool CSE_ALifeDynamicObject::redundant				() const
{
	return						(false);
}

void CSE_InventoryBox::add_online	(const bool &update_registries)
{
	NET_Packet					tNetPacket;

	for (u32 i = 0; i < children.size(); i++) {
		CSE_Abstract* l_tpAbstract = children[i];
		VERIFY(l_tpAbstract);

		CSE_ALifeDynamicObject* l_tpALifeDynamicObject = l_tpAbstract->cast_alife_dynamic_object();
		CSE_ALifeInventoryItem* l_tpALifeInventoryItem = l_tpALifeDynamicObject->cast_inventory_item();
		R_ASSERT2(l_tpALifeInventoryItem, "Non inventory item object has parent?!");
		l_tpALifeInventoryItem->base()->s_flags.Or(M_SPAWN_UPDATE);
		alife().server().entity_Destroy(l_tpAbstract);

#ifdef DEBUG
		if (psAI_Flags.test(aiALife))
		{
			Msg(
				"[LSS][%d] Going online [%d][%s][%d] with parent [%d][%s] on '%s'",
				Device.dwFrame,
				Device.dwTimeGlobal,
				l_tpALifeInventoryItem->base()->name_replace(),
				l_tpALifeInventoryItem->base()->ID,
				ID,
				name_replace(),
				"*SERVER*"
			);
		}
#endif

		l_tpALifeDynamicObject->o_Position		= o_Position;
		l_tpALifeDynamicObject->m_tNodeID		= m_tNodeID;
		alife().server().Process_spawn	(tNetPacket,l_tpALifeInventoryItem->base());
		l_tpALifeDynamicObject->s_flags.And		(u16(-1) ^ M_SPAWN_UPDATE);
		l_tpALifeDynamicObject->m_bOnline		= true;
	}

	CSE_ALifeDynamicObjectVisual::add_online(update_registries);
}

void CSE_InventoryBox::add_offline(CSE_Abstract* children, const bool& update_registries)
{
	if (children)
	{
		CSE_ALifeDynamicObject* child = static_cast<CSE_ALifeDynamicObject*>(children);
		R_ASSERT(child && child->ID_Parent == ID);
		child->m_bOnline = false;

		CSE_ALifeInventoryItem* inventory_item = child->cast_inventory_item();
		R_ASSERT2(inventory_item, "Non inventory item object has parent?!");
#ifdef DEBUG
		if (psAI_Flags.test(aiALife))
		{
			Msg(
				"[LSS][%d] Going offline [%d][%s][%d] with parent [%d][%s] on '%s'",
				Device.dwFrame,
				Device.dwTimeGlobal,
				inventory_item->base()->name_replace(),
				inventory_item->base()->ID,
				ID,
				name_replace(),
				"*SERVER*"
			);
		}
#endif

		ALife::_OBJECT_ID				item_id = inventory_item->base()->ID;
		inventory_item->base()->ID = alife().server().PerformIDgen(item_id);

		if (!child->can_save()) {
			alife().release(child);
			return;
		}

		if (!client_data.empty())
		{
#ifdef DEBUG
			Msg("! [%s]: client_data is cleared for [%d][%s]!!!", __FUNCTION__, ID, name_replace());
#endif
			child->client_data.clear();
		}
	}

	CSE_ALifeDynamicObjectVisual::add_offline(children, update_registries);
}
