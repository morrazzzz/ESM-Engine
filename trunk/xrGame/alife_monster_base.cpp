////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_monster_base.cpp
//	Created 	: 07.02.2007
//  Modified 	: 07.02.2007
//	Author		: Dmitriy Iassenev
//	Description : ALife mnster base class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "alife_simulator.h"
#include "xrServer.h"
#include "alife_monster_brain.h"
#include "ai_space.h"
#include "alife_object_registry.h"
#include "alife_graph_registry.h"
#include "xrServer.h"
#include "alife_schedule_registry.h"

void CSE_ALifeMonsterBase::on_spawn				()
{
	inherited1::on_spawn		();

	if (!pSettings->line_exist(s_name,"Spawn_Inventory_Item_Section"))
		return;

	LPCSTR						item_section = pSettings->r_string(s_name,"Spawn_Inventory_Item_Section");
	float						spawn_probability = pSettings->r_float(s_name,"Spawn_Inventory_Item_Probability");
	float						probability = randF();
	if ((probability >= spawn_probability) && !fsimilar(spawn_probability,1.f))
		return;

	alife().spawn_item(item_section,o_Position,m_tNodeID,m_tGraphID,ID)->ID_Parent = ID;
}

void CSE_ALifeMonsterBase::add_online(const bool& update_registries)
{
	NET_Packet					tNetPacket;

	for (u16 i = 0; i < children.size(); i++) {
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

		//		R_ASSERT3								(ai().level_graph().valid_vertex_id(l_tpALifeDynamicObject->m_tNodeID),"Invalid vertex for object ",l_tpALifeInventoryItem->name_replace());
		l_tpALifeDynamicObject->o_Position = o_Position;
		l_tpALifeDynamicObject->m_tNodeID = m_tNodeID;
		alife().server().Process_spawn(tNetPacket, l_tpALifeInventoryItem->base());
		l_tpALifeDynamicObject->s_flags.And(u16(-1) ^ M_SPAWN_UPDATE);
		l_tpALifeDynamicObject->m_bOnline = true;
	}

	if (update_registries)
	{
		alife().scheduled().remove(this);
		alife().graph().remove(this, m_tGraphID, false);
	}
	brain().on_switch_online();
}

void CSE_ALifeMonsterBase::add_offline(CSE_Abstract* children, const bool &update_registries)
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

		ALife::_OBJECT_ID item_id = inventory_item->base()->ID;
		inventory_item->base()->ID = alife().server().PerformIDgen(item_id);

		if (!child->can_save()) {
			alife().release(child);
			return;
		}

		if (!child->client_data.empty())
		{
#ifdef DEBUG
			Msg("! [%s]: client_data is cleared for [%d][%s]!!!", __FUNCTION__, child->ID, child->name_replace());
#endif
			child->client_data.clear();
		}
	}

	if (update_registries)
	{
		alife().scheduled().add(this);
		alife().graph().add(this, m_tGraphID, false);
	}
	brain().on_switch_offline	();
}
