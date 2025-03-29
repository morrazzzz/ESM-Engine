// xrServer.cpp: implementation of the xrServer class.
//
//////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "xrServer.h"
#include "xrMessages.h"
#include "xrServer_Objects_ALife_All.h"
#include "level.h"
#include "game_cl_base.h"
#include "ai_space.h"
#include "../xr_3da/IGame_Persistent.h"

#include "../xr_3da/XR_IOConsole.h"
#include "ui/UIInventoryUtilities.h"

#ifdef DEBUG
#include "level_debug.h"
#endif

xrClientData::xrClientData	():IClient(Device.GetTimerGlobal())
{
	ps					= Level().Server->game->createPlayerState();
	ps->clear			();
	ps->m_online_time	= Level().timeServer();

	Clear				();
}

void	xrClientData::Clear()
{
	owner									= NULL;
	net_Ready								= FALSE;
	net_Accepted							= FALSE;
};

xrClientData::~xrClientData()
{
	xr_delete(ps);
}


xrServer::xrServer():IPureServer(Device.GetTimerGlobal(), g_dedicated_server)
{
	m_iCurUpdatePacket = 0;
	m_aUpdatePackets.push_back(NET_Packet());
	m_aDelayedPackets.clear();
}

xrServer::~xrServer()
{
	while (net_Players.size())
	{
		client_Destroy(net_Players[0]);
	}
	
	while (net_Players_disconnected.size())
	{
		client_Destroy(net_Players_disconnected[0]);
	}		
	m_aUpdatePackets.clear();
	m_aDelayedPackets.clear();
}

bool  xrServer::HasBattlEye()
{
#ifdef BATTLEYE
	return (g_pGameLevel && Level().battleye_system.server)? true : false;
#else
	return false;
#endif // BATTLEYE
}

//--------------------------------------------------------------------

CSE_Abstract*	xrServer::ID_to_entity		(u16 ID)
{
	// #pragma todo("??? to all : ID_to_entity - must be replaced to 'game->entity_from_eid()'")	
	if (0xffff==ID)				return 0;
	xrS_entities::iterator	I	= entities.find	(ID);
	if (entities.end()!=I)		return I->second;
	else						return 0;
}

//--------------------------------------------------------------------
IClient*	xrServer::client_Create		()
{
	return xr_new<xrClientData> ();
}
void		xrServer::client_Replicate	()
{
}

IClient*	xrServer::client_Find_Get	(ClientID ID)
{
	DWORD	dwPort			= 0;

	IClient* newCL = client_Create();
	newCL->ID = ID;
	
	csPlayers.Enter();
	net_Players.push_back( newCL );
	csPlayers.Leave();

	Msg		("# Player not found. New player created.");
	return newCL;
};

INT	g_sv_Client_Reconnect_Time = 0;

void		xrServer::client_Destroy	(IClient* C)
{
	csPlayers.Enter	();
	
	// Delete assosiated entity
	// xrClientData*	D = (xrClientData*)C;
	// CSE_Abstract* E = D->owner;
	for (u32 DI=0; DI<net_Players_disconnected.size(); DI++)
	{
		if (net_Players_disconnected[DI] == C)
		{
			xr_delete(C);
			net_Players_disconnected.erase(net_Players_disconnected.begin()+DI);
			break;
		};
	};

	for (u32 I=0; I<net_Players.size(); I++)
	{
		if (net_Players[I] == C)
		{
			{
				DelayedPacket pp;
				pp.SenderID = C->ID;

				xr_deque<DelayedPacket>::iterator it;
				do{
					it						=std::find(m_aDelayedPackets.begin(),m_aDelayedPackets.end(),pp);
					if(it!=m_aDelayedPackets.end())
					{
						m_aDelayedPackets.erase	(it);
						Msg("removing packet from delayed event storage");
					}else
						break;
				}while(true);
			}

			if (!g_sv_Client_Reconnect_Time)
			{
				xr_delete(C);				
			}
			else
			{
				C->dwTime_LastUpdate = Device.dwTimeGlobal;
				net_Players_disconnected.push_back(C);				
				((xrClientData*)C)->Clear();
			};
			net_Players.erase	(net_Players.begin()+I);
			break;
		};
	}

	csPlayers.Leave();
}

//--------------------------------------------------------------------
int	g_Dump_Update_Write = 0;

#ifdef DEBUG
INT g_sv_SendUpdate = 0;
#endif

void xrServer::Update	()
{
	csPlayers.Enter	();

	ProceedDelayedPackets();
	// game update
	game->ProcessDelayedEvent();
	game->Update	();

	SendUpdatesToAll();


	if (game->sv_force_sync)	Perform_game_export();

#ifdef SLOW_VERIFY_ENTITIES
	VERIFY						(verify_entities());
#endif
	//-----------------------------------------------------
	//Remove any of long time disconnected players
	for (u32 DI = 0; DI<net_Players_disconnected.size(); )
	{
		IClient* CL				= net_Players_disconnected[DI];
		if (CL->dwTime_LastUpdate+g_sv_Client_Reconnect_Time*60000<Device.dwTimeGlobal)
		{
			client_Destroy(CL);
			continue;
		}
		DI++;
	}
	csPlayers.Leave					();
}

void xrServer::SendUpdatesToAll()
{
	m_iCurUpdatePacket = 0;
	NET_Packet* pCurUpdatePacket = &(m_aUpdatePackets[0]);
	pCurUpdatePacket->B.count = 0;
	u32	 position;

	R_ASSERT2(net_Players.size() == 1, "Player can only one!!!!");

	for (u32 client=0; client<net_Players.size(); ++client)
	{// for each client
		// Initialize process and check for available bandwidth
		xrClientData*	Client			= (xrClientData*) net_Players	[client];
		if (!Client->net_Ready)			continue;

		// Send relevant entities to client
		NET_Packet						Packet;
		u16 PacketType					= M_UPDATE;
		Packet.w_begin					(PacketType);
		// GameUpdate
		game->net_Export_Update			(Packet,Client->ID,Client->ID);
		game->net_Export_GameTime		(Packet);

		{
			SendTo			(Client->ID,Packet,net_flags(FALSE,TRUE));
			continue;
		}
	};	// for each client	

#pragma todo("What is a "double" export? The same code element is in Update (Look at the line 259).")
	if (game->sv_force_sync)	
		Perform_game_export();

#ifdef SLOW_VERIFY_ENTITIES
	VERIFY(verify_entities());
#endif
}

xr_vector<shared_str>	_tmp_log;
void console_log_cb(LPCSTR text)
{
	_tmp_log.push_back	(text);
}

u32 xrServer::OnDelayedMessage	(NET_Packet& P, ClientID sender)			// Non-Zero means broadcasting with "flags" as returned
{
	if (g_pGameLevel && Level().IsDemoSave()) 
		Level().Demo_StoreServerData(P.B.data, P.B.count);
	u16						type;
	P.r_begin				(type);

	csPlayers.Enter			();

	xrClientData* CL				= ID_to_client(sender);
	R_ASSERT2						(CL, make_string("packet type [%d]",type).c_str());

	switch (type)
	{
		case M_CLIENT_REQUEST_CONNECTION_DATA:
		{
			OnCL_Connected				(CL);
		}break;
	}

#ifdef SLOW_VERIFY_ENTITIES
	VERIFY							(verify_entities());
#endif

	csPlayers.Leave					();
	return 0;
}

extern	float	g_fCatchObjectTime;
u32 xrServer::OnMessage	(NET_Packet& P, ClientID sender)			// Non-Zero means broadcasting with "flags" as returned
{
	if (g_pGameLevel && Level().IsDemoSave()) Level().Demo_StoreServerData(P.B.data, P.B.count);
	u16			type;
	P.r_begin	(type);

	csPlayers.Enter			();

	xrClientData* CL				= ID_to_client(sender);

	switch (type)
	{
	case M_UPDATE:	
		{
			Process_update			(P,sender);						// No broadcast
		}break;
	case M_SPAWN:	
		{
			Process_spawn(P,sender);	
		}break;
	case M_EVENT:	
		{
			Process_event			(P,sender);
		}break;
	case M_EVENT_PACK:
		{
			NET_Packet	tmpP;
			while (!P.r_eof())
			{
				tmpP.B.count		= P.r_u8();
				P.r					(&tmpP.B.data, tmpP.B.count);

				OnMessage			(tmpP, sender);
			};			
		}break;
	case M_GAMEMESSAGE:
		{
			SendBroadcast			(BroadcastCID,P,net_flags(TRUE,TRUE));
		}break;
	case M_CLIENTREADY:
		{
			xrClientData* CL		= ID_to_client(sender);
			if ( CL )	
			{
				CL->net_Ready	= TRUE;
				CL->ps->DeathTime = Device.dwTimeGlobal;
				CL->ps->setName( CL->name.c_str() );
			};
			game->signal_Syncronize	();
		}break;
	case M_SWITCH_DISTANCE:
		{
			game->switch_distance	(P,sender);
		}break;
	case M_CHANGE_LEVEL:
		{
			if (game->change_level(P,sender))
			{
				SendBroadcast		(BroadcastCID,P,net_flags(TRUE,TRUE));
			}
		}break;
	case M_SAVE_GAME:
		{
			game->save_game			(P,sender);
		}break;
	case M_LOAD_GAME:
		{
			game->load_game			(P,sender);
			SendBroadcast			(BroadcastCID,P,net_flags(TRUE,TRUE));
		}break;
	case M_SAVE_PACKET:
		{
			Process_save			(P,sender);
		}break;
	case M_CLIENT_REQUEST_CONNECTION_DATA:
		{
			AddDelayedPacket(P, sender);
		}break;
	case M_STATISTIC_UPDATE:
		{
			if (SV_Client)
				SendBroadcast			(SV_Client->ID,P,net_flags(TRUE,TRUE));
			else
				SendBroadcast			(BroadcastCID,P,net_flags(TRUE,TRUE));
		}break;
	case M_STATISTIC_UPDATE_RESPOND:
		{
			if (SV_Client) SendTo	(SV_Client->ID, P, net_flags(TRUE, TRUE));
		}break;
	}

#ifdef SLOW_VERIFY_ENTITIES
	VERIFY							(verify_entities());
#endif

	csPlayers.Leave					();

	return							IPureServer::OnMessage(P, sender);
}

void xrServer::SendTo_LL			(ClientID ID, void* data, u32 size, u32 dwFlags, u32 dwTimeout)
{
	// optimize local traffic
	Level().OnMessage			(data,size);
}

//--------------------------------------------------------------------
CSE_Abstract*	xrServer::entity_Create		(LPCSTR name)
{
	return F_entity_Create(name);
}

void			xrServer::entity_Destroy	(CSE_Abstract *&P)
{
#ifdef DEBUG
	if (dbg_net_Draw_Flags.test(dbg_destroy))
		Msg("xrServer::entity_Destroy : [%d][%s][%s]", P->ID, P->name(), P->name_replace());
#endif
	R_ASSERT					(P);
	entities.erase				(P->ID);
	m_tID_Generator.vfFreeID	(P->ID,Device.TimerAsync());

	if(P->owner && P->owner->owner==P)
		P->owner->owner		= NULL;

	P->owner = NULL;
	if (!ai().get_alife() || !P->m_bALifeControl)
	{
		F_entity_Destroy		(P);
	}
}

CSE_Abstract*	xrServer::GetEntity			(u32 Num)
{
	xrS_entities::iterator	I=entities.begin(),E=entities.end();
	for (u32 C=0; I!=E; ++I,++C)
	{
		if (C == Num) return I->second;
	};
	return NULL;
};

#ifdef DEBUG

static	BOOL	_ve_initialized			= FALSE;
static	BOOL	_ve_use					= TRUE;

bool xrServer::verify_entities				() const
{
	if (!_ve_initialized)	{
		_ve_initialized					= TRUE;
		if (strstr(Core.Params,"-~ve"))	_ve_use=FALSE;
	}
	if (!_ve_use)						return true;

	xrS_entities::const_iterator		I = entities.begin();
	xrS_entities::const_iterator		E = entities.end();
	for ( ; I != E; ++I) {
		VERIFY2							((*I).first != 0xffff,"SERVER : Invalid entity id as a map key - 0xffff");
		VERIFY2							((*I).second,"SERVER : Null entity object in the map");
		VERIFY3							((*I).first == (*I).second->ID,"SERVER : ID mismatch - map key doesn't correspond to the real entity ID",(*I).second->name_replace());
		verify_entity					((*I).second);
	}
	return								(true);
}

void xrServer::verify_entity				(const CSE_Abstract *entity) const
{
	VERIFY(entity->m_wVersion!=0);
	if (entity->ID_Parent != 0xffff) {
		xrS_entities::const_iterator	J = entities.find(entity->ID_Parent);
		VERIFY3							(J != entities.end(),"SERVER : Cannot find parent in the map",entity->name_replace());
		VERIFY3							((*J).second,"SERVER : Null entity object in the map",entity->name_replace());
		VERIFY3							((*J).first == (*J).second->ID,"SERVER : ID mismatch - map key doesn't correspond to the real entity ID",(*J).second->name_replace());
		VERIFY3							(std::find((*J).second->children.begin(),(*J).second->children.end(),entity->ID) != (*J).second->children.end(),"SERVER : Parent/Children relationship mismatch - Object has parent, but corresponding parent doesn't have children",(*J).second->name_replace());
	}

	xr_vector<u16>::const_iterator		I = entity->children.begin();
	xr_vector<u16>::const_iterator		E = entity->children.end();
	for ( ; I != E; ++I) {
		VERIFY3							(*I != 0xffff,"SERVER : Invalid entity children id - 0xffff",entity->name_replace());
		xrS_entities::const_iterator	J = entities.find(*I);
		VERIFY3							(J != entities.end(),"SERVER : Cannot find children in the map",entity->name_replace());
		VERIFY3							((*J).second,"SERVER : Null entity object in the map",entity->name_replace());
		VERIFY3							((*J).first == (*J).second->ID,"SERVER : ID mismatch - map key doesn't correspond to the real entity ID",(*J).second->name_replace());
		VERIFY3							((*J).second->ID_Parent == entity->ID,"SERVER : Parent/Children relationship mismatch - Object has children, but children doesn't have parent",(*J).second->name_replace());
	}
}

#endif // DEBUG

shared_str xrServer::level_name				(const shared_str &server_options) const
{
	return								(game->level_name(server_options));
}

void xrServer::create_direct_client()
{
	SClientConnectData cl_data;
	cl_data.clientID.set(1);
	strcpy_s( cl_data.name, "single_player" );
	cl_data.process_id = GetCurrentProcessId();
	
	new_client( &cl_data );
}


void xrServer::ProceedDelayedPackets()
{
	DelayedPackestCS.Enter();
	while (!m_aDelayedPackets.empty())
	{
		DelayedPacket& DPacket	= *m_aDelayedPackets.begin();
		OnDelayedMessage(DPacket.Packet, DPacket.SenderID);
//		OnMessage(DPacket.Packet, DPacket.SenderID);
		m_aDelayedPackets.pop_front();
	}
	DelayedPackestCS.Leave();
};

void xrServer::AddDelayedPacket	(NET_Packet& Packet, ClientID Sender)
{
	DelayedPackestCS.Enter();

	m_aDelayedPackets.push_back(DelayedPacket());
	DelayedPacket* NewPacket = &(m_aDelayedPackets.back());
	NewPacket->SenderID = Sender;
	CopyMemory	(&(NewPacket->Packet),&Packet,sizeof(NET_Packet));	

	DelayedPackestCS.Leave();
}

u32 g_sv_dwMaxClientPing		= 2000;
u32 g_sv_time_for_ping_check	= 15000;// 15 sec
u8	g_sv_maxPingWarningsCount	= 5;

//extern	s32		g_sv_dm_dwFragLimit;
//extern  s32		g_sv_ah_dwArtefactsNum;
//extern	s32		g_sv_dm_dwTimeLimit;
//extern	int		g_sv_ah_iReinforcementTime;

xr_token game_types[];
void xrServer::GetServerInfo( CServerInfo* si )
{
	string32  tmp;
	string256 tmp256;

	si->AddItem( "Server port", itoa( GetPort(), tmp, 10 ), RGB(128,128,255) );
	LPCSTR time = InventoryUtilities::GetTimeAsString( Device.dwTimeGlobal, InventoryUtilities::etpTimeToSecondsAndDay ).c_str();
	si->AddItem( "Uptime", time, RGB(255,228,0) );

	strcpy_s( tmp256, get_token_name(game_types, game->Type() ) );
/*	if ( game->Type() == GAME_DEATHMATCH || game->Type() == GAME_TEAMDEATHMATCH )
	{
		strcat_s( tmp256, " [" );
		strcat_s( tmp256, itoa( g_sv_dm_dwFragLimit, tmp, 10 ) );
		strcat_s( tmp256, "] " );
	}
	else if ( game->Type() == GAME_ARTEFACTHUNT )
	{
		strcat_s( tmp256, " [" );
		strcat_s( tmp256, itoa( g_sv_ah_dwArtefactsNum, tmp, 10 ) );
		strcat_s( tmp256, "] " );
		g_sv_ah_iReinforcementTime;
	}
	
	//if ( g_sv_dm_dwTimeLimit > 0 )
	{
		strcat_s( tmp256, " time limit [" );
		strcat_s( tmp256, itoa( g_sv_dm_dwTimeLimit, tmp, 10 ) );
		strcat_s( tmp256, "] " );
	}
	if ( game->Type() == GAME_ARTEFACTHUNT )
	{
		strcat_s( tmp256, " RT [" );
		strcat_s( tmp256, itoa( g_sv_ah_iReinforcementTime, tmp, 10 ) );
		strcat_s( tmp256, "]" );
	}
	si->AddItem( "Game type", tmp256, RGB(128,255,255) );*/
}

void xrServer::SpawnNewObjects()
{
	if (EntitiesToSpawn.empty())
		return;

	for (u32 i = 0; i < EntitiesToSpawn.size(); i++)
	{
		//NET_Packet Packet;
		CSE_Abstract* DC = EntitiesToSpawn[i];
		VERIFY(DC);

		if (OnServer()) {
			DC->s_flags.set(M_SPAWN_OBJECT_LOCAL, TRUE);
		};

		//DC->Spawn_Read(Packet);
		//if (DC->s_flags.is(M_SPAWN_UPDATE))
		//	DC->UPDATE_Read(Packet);

		CObject* O{};
		if (!DC->ObjectCustomSpawn)
		{
			O = Level().Objects.Create(*DC->s_name);

			if (!O)
			{
				Msg("! An attempt to spawn a nullptr object.");
				F_entity_Destroy(DC);
				return;
			}
		}
		else
		{
			O = Level().Objects.net_Find(DC->ID);
			DC->ObjectCustomSpawn = false;
		}

		VERIFY(O);

		Level().g_sv_Spawn(O, DC);
		//F_entity_Destroy(DC);
	}

	EntitiesToSpawn.clear();
}