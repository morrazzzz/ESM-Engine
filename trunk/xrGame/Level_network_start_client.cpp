#include "stdafx.h"
#include "HUDmanager.h"
#include "level.h"
#include "../xr_3da/x_ray.h"
#include "../xr_3da/igame_persistent.h"
#include "PhysicsGamePars.h"
#include "ai_space.h"

#include "../xrPhysics/iphworld.h"
#include "PHCommander.h"
#include "physics_game.h"

#include "xrServer.h"

bool CLevel::net_start_client1()
{
	pApp->LoadBegin();

	R_ASSERT2(LoadLevel(), "Loading failed.");

	// Begin spawn
//	g_pGamePersistent->LoadTitle		("st_client_spawning");
	g_pGamePersistent->LoadTitle();

	// Send physics to single or multithreaded mode
	create_physics_world(!!psDeviceFlags.test(mtPhysics), &ObjectSpace, &Objects, &Device);

	R_ASSERT(physics_world());

	m_ph_commander_physics_worldstep = xr_new<CPHCommander>();
	physics_world()->set_update_callback(m_ph_commander_physics_worldstep);

	physics_world()->set_default_contact_shotmark(ContactShotMark);
	physics_world()->set_default_character_contact_shotmark(CharacterContactShotMark);

	VERIFY(physics_world());
	physics_world()->set_step_time_callback((PhysicsStepTimeCallback*)&PhisStepsCallback);
	
	return true;
}

bool	CLevel::net_start_client2				()
{
	// HUD

	// Textures
	pHUD->Load();
//	g_pGamePersistent->LoadTitle				("st_loading_textures");
	g_pGamePersistent->LoadTitle();
	Device.m_pRender->DeferredLoad(FALSE);
	Device.m_pRender->ResourcesDeferredUpload();
	LL_CheckTextures();

	return true;
}

bool	CLevel::net_start_client3				()
{
	// Sync
	if (g_hud)
	{
		g_hud->Load();
		g_hud->OnConnected();
	}

//	g_pGamePersistent->LoadTitle		("st_client_synchronising");
	g_pGamePersistent->LoadTitle();
	Device.PreCache						(60, true, true);

	pApp->LoadEnd							(); 
	return true;
}