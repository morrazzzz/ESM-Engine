#pragma once

#include "inventory_item_object.h"

class CMPPlayersBag :
	public	CInventoryItemObject
{
public:
					CMPPlayersBag(void);
	virtual			~CMPPlayersBag(void);

	virtual void OnEvent(NET_Packet& P, u16 type);
protected:
private:
};