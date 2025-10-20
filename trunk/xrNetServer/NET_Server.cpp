#include "stdafx.h"
#include "net_server.h"

void	IPureServer::SendTo		(NET_Packet& P)
{
	SendTo_LL(P.dataWriting, P.wPos);
}

void	IPureServer::SendBroadcast(NET_Packet& P)
{
	// Perform broadcasting
	SendTo(P);
}
