#include "stdafx.h"
#include "net_server.h"

void	IPureServer::SendTo		(NET_Packet& P)
{
	SendTo_LL(P.B.data, P.B.count);
}

void	IPureServer::SendBroadcast(NET_Packet& P)
{
	// Perform broadcasting
	SendTo(P);
}
