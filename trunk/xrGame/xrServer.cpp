// xrServer.cpp: implementation of the xrServer class.
//
//////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "xrServer.h"
#include "xrMessages.h"
#include "xrServer_Objects_ALife_All.h"
#include "level.h"
#include "ai_space.h"
#include "../xr_3da/IGame_Persistent.h"

#include "../xr_3da/XR_IOConsole.h"
#include "ui/UIInventoryUtilities.h"

#ifdef DEBUG
#include "level_debug.h"
#endif

xrServer::xrServer(){}

xrServer::~xrServer(){}

CSE_Abstract*	xrServer::ID_to_entity		(u16 ID)
{
	// #pragma todo("??? to all : ID_to_entity - must be replaced to 'game->entity_from_eid()'")	
	if (0xffff==ID)				return 0;
	xrS_entities::iterator	I	= entities.find	(ID);
	if (entities.end()!=I)		return I->second;
	else						return 0;
}

void xrServer::Update	()
{
	// game update
	game->Update	();

#ifdef SLOW_VERIFY_ENTITIES
	VERIFY						(verify_entities());
#endif
}

xr_vector<shared_str>	_tmp_log;
void console_log_cb(LPCSTR text)
{
	_tmp_log.push_back	(text);
}

extern	float	g_fCatchObjectTime;
void xrServer::OnMessage	(NET_Packet& P)			// Non-Zero means broadcasting with "flags" as returned
{
	if (g_pGameLevel && Level().IsDemoSave()) Level().Demo_StoreServerData(P.B.data, P.B.count);
	u16			type;
	P.r_begin	(type);

	csPlayers.Enter			();

	switch (type)
	{
	case M_UPDATE:	
		{
			Process_update			(P);						// No broadcast
		}break;
	case M_SPAWN:	
		{
			Process_spawn(P);	
		}break;
	case M_EVENT:	
		{
			Process_event			(P);
		}break;
	case M_EVENT_PACK:
		{
			NET_Packet	tmpP;
			while (!P.r_eof())
			{
				tmpP.B.count		= P.r_u8();
				P.r					(&tmpP.B.data, tmpP.B.count);

				OnMessage			(tmpP);
			};			
		}break;
	case M_CHANGE_LEVEL:
		{
			if (game->change_level(P))
			{
				SendBroadcast		(P);
			}
		}break;
	case M_SAVE_GAME:
		{
			game->save_game			(P);
		}break;
	case M_LOAD_GAME:
		{
			game->load_game			(P);
			SendBroadcast			(P);
		}break;
	case M_SAVE_PACKET:
		{
			Process_save			(P);
		}break;
	}

#ifdef SLOW_VERIFY_ENTITIES
	VERIFY							(verify_entities());
#endif

	csPlayers.Leave					();
}

void xrServer::SendTo_LL			(void* data, u32 size)
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