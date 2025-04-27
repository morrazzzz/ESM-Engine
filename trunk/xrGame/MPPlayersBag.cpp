#include "stdafx.h"

#include "MPPlayersBag.h"
#include "Level.h"
//#include "xrMessages.h"
#include "game_base_space.h"

#define BAG_REMOVE_TIME		60000

CMPPlayersBag::CMPPlayersBag()
{
};

CMPPlayersBag::~CMPPlayersBag()
{
};

void CMPPlayersBag::OnEvent(NET_Packet& P, u16 type) 
{
	CInventoryItemObject::OnEvent		(P,type);
	u16						id;
}
