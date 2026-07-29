#include "stdafx.h"
#include "Level.h"
#include "Level_Bullet_Manager.h"
#include "xrserver.h"
#include "xrmessages.h"
#include "PHCommander.h"
#include "MainMenu.h"
#include "space_restriction_manager.h"
#include "ai_space.h"
#include "script_engine.h"
#include "stalker_animation_data_storage.h"
#include "client_spawn_manager.h"

void CLevel::remove_objects	()
{
	destroyAllObjects = true;

	Server->DestroyAllEntities();

	snd_Events.clear			();

	Objects.DestroyAllObjects();

	BulletManager().Clear		();
	ph_commander().clear		();
	ph_commander_scripts().clear();

	space_restriction_manager().clear	();

	ai().script_engine().collect_all_garbage	();

	stalker_animation_data_storage().clear		();
	
	VERIFY										(Render);
	Render->models_Clear						(FALSE);
	Render->clear_static_wallmarks				();

#ifdef DEBUG
	if (!client_spawn_manager().registry().empty())
		client_spawn_manager().dump				();
#endif // DEBUG

	VERIFY(client_spawn_manager().registry().empty());
	client_spawn_manager().clear();

	g_pGamePersistent->destroy_particles		(false);

	destroyAllObjects = false;
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


void CLevel::SaveAllCSEObj(bool needSaveAll)
{
	for (u32 i = 0; i < Objects.o_count(); i++)
	{
		CGameObject* object = static_cast<CGameObject*>(Objects.o_get_by_iterator(i));
		R_ASSERT(object);

		CSE_Abstract* CSEObject = object->GetCSEObject();
		R_ASSERT(CSEObject);

		object->SaveCSEObj(CSEObject, needSaveAll);
	}
}

extern		float		phTimefactor;

void CLevel::Send(NET_Packet& P)
{
	// optimize the case when server located in our memory
	Server->OnMessage(P);
}
