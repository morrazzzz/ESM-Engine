#include "stdafx.h"
#include "game_cl_deathmatch.h"
#include "level.h"
#include "actor.h"
#include "inventory.h"
#include "xrServer_Objects_ALife_Items.h"
#include "weapon.h"
#include "xr_level_controller.h"
#include "eatable_item_object.h"
#include "Missile.h"
#include "clsid_game.h"

#define UNBUYABLESLOT		20

//s16	game_cl_Deathmatch::GetBuyMenuItemIndex		(u8 SlotID, u8 ItemID)
s16	game_cl_Deathmatch::GetBuyMenuItemIndex		(u8 Addons, u8 ItemID)
{
//	R_ASSERT2(SlotID != 0xff && ItemID != 0xff, "Bad Buy Manu Item");
//	if (SlotID == OUTFIT_SLOT) SlotID = APPARATUS_SLOT;
	s16	ID = (s16(Addons) << 0x08) | s16(ItemID);
	return ID;
};

void game_cl_Deathmatch::OnBuyMenu_Ok	()
{
};

void	game_cl_Deathmatch::OnBuyMenu_DefaultItems	()
{
	//---------------------------------------------------------
/*	PRESET_ITEMS_it It = PlayerDefItems.begin();
	PRESET_ITEMS_it Et = PlayerDefItems.end();
	for ( ; It != Et; ++It) 
	{
		s16	ItemID = (*It);

		pCurBuyMenu->SectionToSlot(u8((ItemID&0xff00)>>0x08), u8(ItemID&0x00ff), false);
	};
*/	//---------------------------------------------------------
};

void game_cl_Deathmatch::CheckItem			(PIItem pItem, PRESET_ITEMS* pPresetItems, BOOL OnlyPreset)
{
};

void				game_cl_Deathmatch::OnMoneyChanged				()
{	
}