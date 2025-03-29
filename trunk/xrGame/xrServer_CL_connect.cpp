#include "stdafx.h"
#include "xrserver.h"
#include "xrmessages.h"
#include "xrserver_objects.h"
#include "Level.h"

xr_vector<u16> g_perform_spawn_ids;

void xrServer::Perform_connect_spawn(CSE_Abstract* E, xrClientData* CL, NET_Packet& P)
{
	xr_vector<u16>::iterator it = std::find(g_perform_spawn_ids.begin(), g_perform_spawn_ids.end(), E->ID);
	if(it!=g_perform_spawn_ids.end())
		return;

	g_perform_spawn_ids.push_back(E->ID);

	if (E->net_Processed)						return;
	if (E->s_flags.is(M_SPAWN_OBJECT_PHANTOM))	return;

	// Connectivity order
	CSE_Abstract* Parent = ID_to_entity	(E->ID_Parent);
	if (Parent)		Perform_connect_spawn	(Parent,CL,P);

	/*
	// Process
	Flags16			save = E->s_flags;
	//-------------------------------------------------
	E->s_flags.set	(M_SPAWN_UPDATE,TRUE);
	if (0==E->owner)	
	{
		// PROCESS NAME; Name this entity
		if (E->s_flags.is(M_SPAWN_OBJECT_ASPLAYER))
		{
			CL->owner		= E;
			E->set_name_replace	(*CL->name);
		}

		// Associate
		E->owner		= CL;
	}
	*/

//	E->Spawn_Read(P);
//	if (E->s_flags.is(M_SPAWN_UPDATE))
//		E->UPDATE_Read(P);
	//-----------------------------------------------------
//	E->s_flags			= save;
	//SendTo				(CL->ID,P,net_flags(TRUE,TRUE));
	
	//NET_Packet Packet;

	//DC->Spawn_Read(Packet);
	//if (DC->s_flags.is(M_SPAWN_UPDATE))
	//	DC->UPDATE_Read(Packet);

	/*
	if (E->cast_smart_zone() || E->)
	{
		CObject* O = Level().Objects.Create(*E->s_name);;
		VERIFY(O);

		Level().g_sv_Spawn(O, E);
		E->net_Processed = TRUE;

		F_entity_Destroy(E);
	}
	else
	*/
	{
/*
		auto it = std::find(EntitiesToSpawn.begin(), EntitiesToSpawn.end(), E);
		VERIFY(it == EntitiesToSpawn.end());

		if (E->ObjectAlreadySpawning)
		{
			Msg("No spawn!!!!");
		}
		else
			EntitiesToSpawn.push_back(E);
			*/
		E->net_Processed = TRUE;
	}

}

void xrServer::SendConnectionData(IClient* _CL)
{
	g_perform_spawn_ids.clear_not_free();
	xrClientData*	CL				= (xrClientData*)_CL;
	NET_Packet		P;
	u32			mode				= net_flags(TRUE,TRUE);
	// Replicate current entities on to this client

	//R_ASSERT(EntitiesToSpawn.empty());

	xrS_entities::iterator	I=entities.begin(),E=entities.end();
	for (; I!=E; ++I)						I->second->net_Processed	= FALSE;
	//for (I=entities.begin(); I!=E; ++I)		Perform_connect_spawn		(I->second,CL,P);

	// Send "finished" signal
	P.w_begin						(M_SV_CONFIG_FINISHED);
	SendTo							(CL->ID,P,mode);
};

void xrServer::OnCL_Connected		(IClient* _CL)
{
	xrClientData*	CL				= (xrClientData*)_CL;
	CL->net_Accepted = TRUE;
///	Server_Client_Check(CL); 

	Export_game_type(CL);
	Perform_game_export();
	SendConnectionData(CL);

	//
	game->signal_Syncronize();
}
