#include "stdafx.h"
#include "xrServer.h"
#include "alife_simulator.h"
#include "xrserver_objects.h"
#include "ai_space.h"
#include "alife_object_registry.h"

#ifdef DEBUG
#include "level_debug.h"
#endif

xr_string xrServer::ent_name_safe(u16 eid)
{
	string1024						buff;
	CSE_Abstract*	e_dest			= game->get_entity_from_eid	(eid);
	if(e_dest)
		sprintf(buff,"[%d][%s:%s]",eid,e_dest->name(),e_dest->name_replace());
	else
		sprintf(buff,"[%d][%s]",eid,"NOTFOUND");

	return buff;
}

void xrServer::Process_event_destroy	(NET_Packet& P, u32 time, u16 ID, NET_Packet* pEPack)
{
	// Parse message
	u16								id_dest	= ID;
#ifdef DEBUG
	if (dbg_net_Draw_Flags.test(dbg_destroy))
		Msg("sv destroy object %s [%d]", ent_name_safe(id_dest).c_str(), Device.dwFrame);
#endif

	CSE_Abstract* e_dest = game->get_entity_from_eid(id_dest);	// кто должен быть уничтожен
	R_ASSERT2(e_dest, "Destroy: [%d] not found on server", id_dest);

	u16								parent_id = e_dest->ID_Parent;

	//---------------------------------------------
	NET_Packet	P2, *pEventPack = pEPack;
	P2.w_begin	(M_EVENT_PACK);
	//---------------------------------------------
	// check if we have children 
	if (!e_dest->children.empty()) {
		if (!pEventPack) pEventPack = &P2;

		while (!e_dest->children.empty())
			Process_event_destroy		(P,time,*e_dest->children.begin(), pEventPack);
	};

	if (0xffff == parent_id && NULL == pEventPack) 
	{
		SendBroadcast				(P);
	}
	else 
	{
		NET_Packet	tmpP;
		if (0xffff != parent_id && Process_event_reject(P,time,parent_id,ID,false)) 
		{
			game->u_EventGen(tmpP, GE_OWNERSHIP_REJECT, parent_id);
			tmpP.w_u16(id_dest);
			tmpP.w_u8(1);
		
			if (!pEventPack) pEventPack = &P2;
			
			pEventPack->w_u8(u8(tmpP.B.count));
			pEventPack->w(&tmpP.B.data, tmpP.B.count);
		};
		
 		game->u_EventGen(tmpP, GE_DESTROY, id_dest);
		
		pEventPack->w_u8(u8(tmpP.B.count));
		pEventPack->w(&tmpP.B.data, tmpP.B.count);
	};

	if (NULL == pEPack && NULL != pEventPack)
	{
		SendBroadcast				(*pEventPack);
	}

	// Everything OK, so perform entity-destroy
	if (e_dest->m_bALifeControl && ai().get_alife()) 
	{
		if (ai().get_alife()->objects().object(id_dest,true))
			ai().get_alife()->release(e_dest, false);
	}

	entity_Destroy					(e_dest);
}
