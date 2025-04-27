////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_trader_abstract.cpp
//	Created 	: 27.10.2005
//  Modified 	: 27.10.2005
//	Author		: Dmitriy Iassenev
//	Description : ALife trader abstract class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "alife_simulator.h"
#include "specific_character.h"
#include "ai_space.h"
#include "alife_object_registry.h"
#include "ai_debug.h"
#include "alife_graph_registry.h"
#include "xrServer.h"
#include "alife_schedule_registry.h"

#ifdef DEBUG
	extern Flags32 psAI_Flags;
#endif

void CSE_ALifeTraderAbstract::spawn_supplies	()
{
	CSE_ALifeDynamicObject		*dynamic_object = smart_cast<CSE_ALifeDynamicObject*>(this);
	VERIFY						(dynamic_object);
	CSE_Abstract				*abstract = dynamic_object->alife().spawn_item("device_pda",base()->o_Position,dynamic_object->m_tNodeID,dynamic_object->m_tGraphID,base()->ID);
	CSE_ALifeItemPDA			*pda = smart_cast<CSE_ALifeItemPDA*>(abstract);
	pda->m_original_owner		= base()->ID;

#ifdef XRGAME_EXPORTS
	character_profile			();
	m_SpecificCharacter			= shared_str();
	m_community_index			= NO_COMMUNITY_INDEX;
	pda->m_specific_character	= specific_character();
#endif

	if(m_SpecificCharacter.size())
	{
		//если в custom data объекта есть
		//секция [dont_spawn_character_supplies]
		//то не вызывать spawn из selected_char.SupplySpawn()
		bool specific_character_supply = true;	

		if (xr_strlen(dynamic_object->m_ini_string))
		{
			IReader r((void*)dynamic_object->m_ini_string.c_str(), xr_strlen(dynamic_object->m_ini_string));
			CInifile ini(&r, FS.get_path("$game_config$")->m_Path);

			if (ini.section_exist("dont_spawn_character_supplies")) 
				specific_character_supply = false;
		}

		if(specific_character_supply)
		{
			CSpecificCharacter selected_char;
			selected_char.Load(m_SpecificCharacter);
			dynamic_object->spawn_supplies(selected_char.SupplySpawn());
		}
	}
}

void CSE_ALifeTraderAbstract::vfInitInventory()
{
//	m_fCumulativeItemMass		= 0.f;
//	m_iCumulativeItemVolume		= 0;
}

void CSE_ALifeDynamicObject::attach	(CSE_ALifeInventoryItem *tpALifeInventoryItem, bool bAddChildren)
{
	tpALifeInventoryItem->base()->ID_Parent	= ID;

	if (!bAddChildren)
		return;

	children.push_back	(tpALifeInventoryItem->base());
}

void CSE_ALifeDynamicObject::detach(CSE_ALifeInventoryItem *tpALifeInventoryItem, bool bRemoveChildren)
{
	CSE_ALifeDynamicObject					*l_tpALifeDynamicObject1 = smart_cast<CSE_ALifeDynamicObject*>(tpALifeInventoryItem);
	R_ASSERT2								(l_tpALifeDynamicObject1,"Invalid children objects");
	l_tpALifeDynamicObject1->o_Position		= o_Position;
	l_tpALifeDynamicObject1->m_tNodeID		= m_tNodeID;
	l_tpALifeDynamicObject1->m_tGraphID		= m_tGraphID;
	l_tpALifeDynamicObject1->m_fDistance	= m_fDistance;

	tpALifeInventoryItem->base()->ID_Parent	= 0xffff;

	if (!bRemoveChildren)
		return;

	auto i = std::find(children.begin(),children.end(),tpALifeInventoryItem->base());
	R_ASSERT2					(children.end() != i,"Can't detach an item which is not on my own");
	children.erase				(i);
}

void CSE_ALifeTraderAbstract::add_online	(const bool &update_registries)
{
	CSE_ALifeDynamicObject		*object = smart_cast<CSE_ALifeDynamicObject*>(this);
	VERIFY						(object);

	NET_Packet					tNetPacket;

    auto I = object->children.begin();
	auto E = object->children.end();
	for (; I != E; ++I) {
		//	this was for the car only
		//		if (*I == ai().alife().graph().actor()->ID)
		//			continue;
		//
		CSE_Abstract* l_tpAbstract = *I;
		
		if (!l_tpAbstract->m_bALifeControl)
			continue;

		CSE_ALifeDynamicObject* l_tpALifeDynamicObject = l_tpAbstract->cast_alife_dynamic_object();
		CSE_ALifeInventoryItem* l_tpALifeInventoryItem = l_tpALifeDynamicObject->cast_inventory_item();
		R_ASSERT2(l_tpALifeInventoryItem, "Non inventory item object has parent?!");
		l_tpALifeInventoryItem->base()->s_flags.Or(M_SPAWN_UPDATE);
		object->alife().server().entity_Destroy(l_tpAbstract);

#ifdef DEBUG
		if (psAI_Flags.test(aiALife))
		{
			Msg(
				"[LSS][%d] Going online [%d][%s][%d] with parent [%d][%s] on '%s'",
				Device.dwFrame,
				Device.dwTimeGlobal,
				l_tpALifeInventoryItem->base()->name_replace(),
				l_tpALifeInventoryItem->base()->ID,
				object->ID,
				object->name_replace(),
				"*SERVER*"
			);
		}
#endif

		//		R_ASSERT3								(ai().level_graph().valid_vertex_id(l_tpALifeDynamicObject->m_tNodeID),"Invalid vertex for object ",l_tpALifeInventoryItem->name_replace());
		l_tpALifeDynamicObject->o_Position = object->o_Position;
		l_tpALifeDynamicObject->m_tNodeID = object->m_tNodeID;
		object->alife().server().Process_spawn(tNetPacket, l_tpALifeInventoryItem->base());
		l_tpALifeDynamicObject->s_flags.And(u16(-1) ^ M_SPAWN_UPDATE);
		l_tpALifeDynamicObject->m_bOnline = true;
	}

	if (!update_registries)
		return;

	object->alife().scheduled().remove(object);
	object->alife().graph().remove(object, object->m_tGraphID, false);
}

void CSE_ALifeTraderAbstract::add_offline	(CSE_Abstract* children, const bool &update_registries)
{
	CSE_ALifeDynamicObject		*object = smart_cast<CSE_ALifeDynamicObject*>(this);
	VERIFY						(object);	

	if (children)
	{
		CSE_ALifeDynamicObject* child = static_cast<CSE_ALifeDynamicObject*>(children);
		R_ASSERT(child && child->ID_Parent == object->ID);
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
				object->ID,
				object->name_replace(),
				"*SERVER*"
			);
		}
#endif 

		ALife::_OBJECT_ID				item_id = inventory_item->base()->ID;
		inventory_item->base()->ID = object->alife().server().PerformIDgen(item_id);

		if (!child->can_save()) {
			object->alife().release(child);
			return;
		}

		if (!child->client_data.empty())
		{
#ifdef DEBUG
			Msg("! [%s]: client_data is cleared for [%d][%s]!!!", __FUNCTION__, child->ID, child->name_replace());
#endif
			child->client_data.clear();
		}
	//	object->alife().graph().add(child, child->m_tGraphID, false);
	//	object->alife().graph().attach(*object, inventory_item, child->m_tGraphID, true);
	}

	if (!update_registries)
		return;

	object->alife().scheduled().add(object);
	object->alife().graph().add(object, object->m_tGraphID, false);
}
