#include "stdafx.h"
#include "entity.h"
#include "level.h"
#include "xrmessages.h"
#include "net_queue.h"
#include "xrServer.h"
#include "Actor.h"
#include "ai_space.h"
#include "saved_game_wrapper.h"
#include "level_graph.h"
#include "clsid_game.h"
#include "../xrPhysics/IPHWorld.h"

void CLevel::ClientReceive()
{
	for (NET_Packet* P = net_msg_Retreive(); P; P=net_msg_Retreive())
	{
		u16			m_type;
		u16			ID;
		P->r_begin	(m_type);
		switch (m_type)
		{
		case M_EVENT:
			game_events->insert		(*P);
			if (g_bDebugEvents)		ProcessGameEvents();
			break;
		case M_EVENT_PACK:
			NET_Packet	tmpP;
			while (!P->r_eof())
			{
				tmpP.B.count = P->r_u8();
				P->r(&tmpP.B.data, tmpP.B.count);
				tmpP.timeReceive = P->timeReceive;

				game_events->insert		(tmpP);
				if (g_bDebugEvents)		ProcessGameEvents();
			};			
			break;
		case M_LOAD_GAME:
		case M_CHANGE_LEVEL:
			{
				if(m_type==M_LOAD_GAME)
				{
					string256						saved_name;
					P->r_stringZ					(saved_name);
					if(xr_strlen(saved_name) && ai().get_alife())
					{
						CSavedGameWrapper			wrapper(saved_name);
						if (wrapper.level_id() == ai().level_graph().level_id()) 
						{
							Engine.Event.Defer	("Game:QuickLoad", size_t(xr_strdup(saved_name)), 0);

							break;
						}
					}
				}
				Engine.Event.Defer	("KERNEL:disconnect");
				Engine.Event.Defer	("KERNEL:start",size_t(xr_strdup(*m_caServerOptions)),size_t(xr_strdup(*m_caClientOptions)));
			}break;
		case M_SAVE_GAME:
			{
				ClientSave			();
			}break;
		}

		net_msg_Release();
	}	

//	if (!g_bDebugEvents) ProcessGameSpawns();
}

void				CLevel::OnMessage				(void* data, u32 size)
{	
	IPureClient::OnMessage(data, size);	
};

NET_Packet*				CLevel::net_msg_Retreive		()
{
	return IPureClient::net_msg_Retreive();
}

