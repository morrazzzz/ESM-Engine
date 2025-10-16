#include "pch_script.h"
#include "Level.h"
#include "Level_Bullet_Manager.h"
#include "xrserver.h"
#include "xrmessages.h"
#include "PHCommander.h"
#include "net_queue.h"
#include "MainMenu.h"
#include "space_restriction_manager.h"
#include "ai_space.h"
#include "script_engine.h"
#include "stalker_animation_data_storage.h"
#include "client_spawn_manager.h"

ENGINE_API bool g_dedicated_server;

const int max_objects_size			= 2*1024;
const int max_objects_size_in_save	= 6*1024;

extern bool	g_b_ClearGameCaptions;

void CLevel::remove_objects	()
{
	Server->DestroyAllEntities();

	snd_Events.clear			();
	for (int i=0; i<6; ++i) {
		// ugly hack for checks that update is twice on frame
		// we need it since we do updates for checking network messages
		++(Device.dwFrame);
		ClientReceive			();
		ProcessGameEvents		();
		Sleep					(100);
	}

	Objects.DestroyAllObjects();

	BulletManager().Clear		();
	ph_commander().clear		();
	ph_commander_scripts().clear();

	if(!g_dedicated_server)
		space_restriction_manager().clear	();

	g_b_ClearGameCaptions		= true;

	if (!g_dedicated_server)
		ai().script_engine().collect_all_garbage	();

	stalker_animation_data_storage().clear		();
	
	VERIFY										(Render);
	Render->models_Clear						(FALSE);
	Render->clear_static_wallmarks				();

#ifdef DEBUG
	if(!g_dedicated_server)
		if (!client_spawn_manager().registry().empty())
			client_spawn_manager().dump				();
#endif // DEBUG
	if(!g_dedicated_server)
	{
		VERIFY										(client_spawn_manager().registry().empty());
		client_spawn_manager().clear			();
	}

	g_pGamePersistent->destroy_particles		(false);
}

#ifdef DEBUG
	extern void	show_animation_stats	();
#endif // DEBUG

void CLevel::net_Stop		()
{
	Msg							("- Disconnect");
	bReady						= false;

	remove_objects				();
	
	IGame_Level::net_Stop		();

	if (Server) {
		Server->Disconnect		();
		xr_delete				(Server);
	}

	ai().script_engine().collect_all_garbage	();

#ifdef DEBUG
	show_animation_stats		();
#endif // DEBUG
}


void CLevel::SaveAllCSEObj()
{
	for (u32 i = 0; i < Objects.o_count(); i++)
	{
		CGameObject* object = static_cast<CGameObject*>(Objects.o_get_by_iterator(i));
		VERIFY(object);

		CSE_Abstract* CSEObject = Server->ID_to_entity(object->ID());
		VERIFY(CSEObject);

		object->SaveCSEObj(CSEObject);
	}
}

u32	CLevel::Objects_net_Save	(NET_Packet* _Packet, u32 start, u32 max_object_size)
{
	NET_Packet& Packet	= *_Packet;
	u32			position;
	for (; start<Objects.o_count(); start++)	{
		CObject		*_P = Objects.o_get_by_iterator(start);
		CGameObject *P = smart_cast<CGameObject*>(_P);
//		Msg			("save:iterating:%d:%s",P->ID(),*P->cName());
		if (P && !P->getDestroy() && P->net_SaveRelevant())	{
			Packet.w_u16			(u16(P->ID())	);
			Packet.w_chunk_open16	(position);
//			Msg						("save:saving:%d:%s",P->ID(),*P->cName());
			P->net_Save				(Packet);
#ifdef DEBUG
			u32 size				= u32		(Packet.w_tell()-position)-sizeof(u16);
//			Msg						("save:saved:%d bytes:%d:%s",size,P->ID(),*P->cName());
			if				(size>=65536)			{
				Debug.fatal	(DEBUG_INFO,"Object [%s][%d] exceed network-data limit\n size=%d, Pend=%d, Pstart=%d",
					*P->cName(), P->ID(), size, Packet.w_tell(), position);
			}
#endif
			Packet.w_chunk_close16	(position);
//			if (0==(--count))		
//				break;
			if (max_object_size > (NET_PacketSizeLimit - Packet.w_tell()))
				break;
		}
	}
	return	++start;
}

void CLevel::ClientSave	()
{
	NET_Packet		P;
	u32				start	= 0;

	while(true)
	{
		P.w_begin	(M_SAVE_PACKET);
		
		start = Objects_net_Save(&P, start, max_objects_size_in_save);

		if (P.B.count>2)
			Send	(P);
		else
			break;
	}
}

extern		float		phTimefactor;

void CLevel::Send(NET_Packet& P)
{
	// optimize the case when server located in our memory
	Server->OnMessage(P);
}

void CLevel::net_Update	()
{
	// If server - perform server-update
	if (Server)	{
		Device.Statistic->netServer.Begin();
		Server->Update					();
		Device.Statistic->netServer.End	();
	}
}

struct _NetworkProcessor	: public pureFrame
{
	virtual void OnFrame	( )
	{
		if (g_pGameLevel && !Device.Paused() )	g_pGameLevel->net_Update();
	}
}	NET_processor;

pureFrame*	g_pNetProcessor	= &NET_processor;
