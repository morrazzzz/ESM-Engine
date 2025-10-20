#include "stdafx.h"
#include "xrserver.h"
#include "xrmessages.h"
#include "xrserver_objects.h"

void xrServer::SLS_Save	(IWriter& fs)
{
	// Generate spawn+update
	NET_Packet		P;
	u32				position;
	xrS_entities::iterator	I=entities.begin(),E=entities.end();
	for (u32 C=0; I!=E; ++I,++C)
	{
		CSE_Abstract*	E	= I->second;

		fs.open_chunk		(C);

		// Spawn
		E->Spawn_Write		(P,TRUE);
		fs.w_u16			(u16(P.wPos));
		fs.w				(P.dataWriting,P.wPos);

		// Update
		P.w_u16				(E->ID);
		P.w_chunk_open8		(position);
		E->UPDATE_Write		(P);
		P.w_chunk_close8	(position);

		fs.w_u16			((u16(P.wPos)));
		fs.w				(P.dataWriting, P.wPos);

		fs.close_chunk		();
	}
}
