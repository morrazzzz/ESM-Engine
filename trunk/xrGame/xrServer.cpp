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
#include "GameObject.h"

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
}

void xrServer::OnMessage	(NET_Packet& P)			// Non-Zero means broadcasting with "flags" as returned
{
	u16			type;
	P.r_begin	(type);

	csPlayers.Enter			();

	switch (type)
	{
	case M_SPAWN:	
		{
			Process_spawn(P);	
		}break;
	}

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

		if (DC->objectCreateAlife)
			game->OnCreate(DC->ID);

		DC->s_flags.set(M_SPAWN_OBJECT_LOCAL, TRUE);

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

void xrServer::DestroyAllEntities()
{
	for (const auto& i: entities)
	{
		if (i.second->ID_Parent != static_cast<u16>(-1))
			continue;

		CObject* O = Level().Objects.net_Find(i.first);
		if (!O)
			continue;
		VERIFY(O);

		static_cast<CGameObject*>(O)->DestroyObject(true);
	}
}