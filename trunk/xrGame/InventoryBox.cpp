#include "pch_script.h"
#include "InventoryBox.h"
#include "level.h"
#include "actor.h"
#include "game_object_space.h"
#include "inventory_item.h"

#include "script_callback_ex.h"
#include "script_game_object.h"

CInventoryBox::CInventoryBox()
{
	m_in_use = false;
	InventoryBoxAllWeight = 0.0f;
}

void CInventoryBox::ObjectTakeItem(CGameObject* object)
{
	R_ASSERT(object);
	PIItem itm = object->cast_inventory_item();
	R_ASSERT(itm);
	m_items.emplace_back(itm);
	object->H_SetParent(this);
	object->setVisible(FALSE);
	object->setEnabled(FALSE);

	InventoryBoxAllWeight += itm->Weight();
}

void CInventoryBox::ObjectRejectItem(CGameObject* object, bool just_before_destroy)
{
	R_ASSERT(object);
	PIItem itm = object->cast_inventory_item();
	R_ASSERT(itm);
	auto it = std::find(m_items.begin(), m_items.end(), itm);
	VERIFY(it != m_items.end());
	m_items.erase(it);
	object->H_SetParent(NULL, just_before_destroy);

	InventoryBoxAllWeight -= itm->Weight();

	if (m_in_use)
		Actor()->callback(GameObject::eInvBoxItemTake)(this->lua_game_object(), object->lua_game_object());
}

BOOL CInventoryBox::net_Spawn(CSE_Abstract* DC)
{
	inherited::net_Spawn	(DC);
	setVisible				(TRUE);
	setEnabled				(TRUE);
	set_tip_text			("inventory_box_use");

	return					TRUE;
}

void CInventoryBox::net_Relcase(CObject* O)
{
	inherited::net_Relcase(O);
}

void CInventoryBox::AddAvailableItems(TIItemContainer& items_container) const
{
	for (u32 i = 0; i < m_items.size(); i++)
		items_container.emplace_back(m_items[i]);		
}
