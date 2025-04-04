#include "stdafx.h"
#include "xrserver.h"
#include "game_sv_single.h"
#include "xrserver_objects.h"
#include "level.h"

void xrServer::OnCL_Disconnected	(IClient* CL)
{
	csPlayers.Enter			();

	// Game config (all, info includes deleted player now, excludes at the next cl-update)
	NET_Packet P;
	P.B.count = 0;
	P.w_clientID(CL->ID);
	P.w_stringZ(CL->name);
	xrClientData* xrCData = (xrClientData*)(CL);
	P.w_u16( (NULL != xrCData) ? xrCData->ps->GameID : 0);
	P.r_pos = 0;
	
	ClientID clientID;
	clientID.set(0);;

	//
	xrS_entities::iterator	I=entities.begin(),E=entities.end();	
	csPlayers.Leave			();

	//Server_Client_Check(CL);

#ifdef BATTLEYE
	if ( g_pGameLevel && Level().battleye_system.server )
	{
		Level().battleye_system.server->RemovePlayer( CL->ID.value() );
	}
#endif // BATTLEYE

}
