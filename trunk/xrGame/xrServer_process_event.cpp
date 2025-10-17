#include "stdafx.h"
#include "xrServer.h"
#include "xrServer_Objects_ALife_Monsters.h"

void xrServer::Process_event	(NET_Packet& P)
{
	u32			timestamp;
	u16			type;
	u16			destination;

	// correct timestamp with server-unique-time (note: direct message correction)
	P.r_u32		(timestamp	);

	// read generic info
	P.r_u16		(type		);
	P.r_u16		(destination);

	switch		(type)
	{
	default:
		R_ASSERT2	(0,"Game Event not implemented!!!");
		break;
	}
}
