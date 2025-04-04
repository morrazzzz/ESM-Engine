#include "stdafx.h"
#include "entity.h"
#include "level.h"
#include "xrmessages.h"
#include "game_cl_base.h"
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
		case M_MAP_SYNC:
			{
				shared_str map_name;
				P->r_stringZ(map_name);

				shared_str _name		= net_Hosts.size() ? net_Hosts.front().dpSessionName:"";

				if(_name.size() && _name!=map_name && OnClient())
				{
					Msg("!!! map sync failed. current is[%s] server is[%s]",m_name.c_str(), map_name.c_str());
					Engine.Event.Defer	("KERNEL:disconnect");
					Engine.Event.Defer	("KERNEL:start",m_caServerOptions.size() ? size_t( xr_strdup(*m_caServerOptions)) : 0, m_caClientOptions.size() ? size_t(xr_strdup(*m_caClientOptions)) : 0);
				}
			}break;
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
		case M_UPDATE:
		{
			game->net_import_update(*P);
			//-------------------------------------------
			break;
		};
		//------------------------------------------------
		//---------------------------------------------------
		case 	M_SV_CONFIG_NEW_CLIENT:
			InitializeClientGame(*P);
			break;
		case M_SV_CONFIG_GAME:
			game->net_import_state	(*P);
			break;
		case M_SV_CONFIG_FINISHED:
			game_configured			= TRUE;
			Msg("- Game configuring : Finished ");
			break;		
		case M_CHAT:
			{
				char	buffer[256];
				P->r_stringZ(buffer);
				Msg		("- %s",buffer);
			}
			break;
		case M_GAMEMESSAGE:
			{
			}break;
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
		case M_GAMESPY_CDKEY_VALIDATION_CHALLENGE:
			{
			}break;
		case M_CHANGE_SELF_NAME:
			{
				net_OnChangeSelfName(P);
			}break;
		case M_BULLET_CHECK_RESPOND:
			{
			}break;
		case M_STATISTIC_UPDATE:
			{
			}break;
		case M_STATISTIC_UPDATE_RESPOND:
			{
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

