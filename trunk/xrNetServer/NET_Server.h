#pragma once

#include "net_shared.h"

class XRNETSERVER_API IPureServer
{
protected:
	xrCriticalSection		csPlayers;
public:
	IPureServer() = default;
	virtual	~IPureServer() = default;
	
	// send
	virtual void			SendTo_LL(void* data, u32 size) {}

	void					SendTo				(NET_Packet& P);
	void					SendBroadcast		(NET_Packet& P);
};
