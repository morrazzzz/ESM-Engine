#include "stdafx.h"
#include "HUDmanager.h"
#include "level.h"
#include "../xr_3da/x_ray.h"
#include "../xr_3da/igame_persistent.h"
#include "PhysicsGamePars.h"
#include "ai_space.h"
#include "game_cl_base.h"

#include "../xrPhysics/iphworld.h"
#include "PHCommander.h"
#include "physics_game.h"

extern	pureFrame*				g_pNetProcessor;

BOOL CLevel::net_Start_client	( LPCSTR options )
{
	return FALSE;
}

bool	CLevel::net_start_client1				()
{
	pApp->LoadBegin	();
	// name_of_server
	string64					name_of_server = "";
//	strcpy						(name_of_server,*m_caClientOptions);
	if (strchr(*m_caClientOptions, '/'))
		strncpy(name_of_server,*m_caClientOptions, strchr(*m_caClientOptions, '/')-*m_caClientOptions);

	if (strchr(name_of_server,'/'))	*strchr(name_of_server,'/') = 0;

	// Startup client
/*
	string256					temp;
	sprintf_s						(temp,"%s %s",
								CStringTable().translate("st_client_connecting_to").c_str(), name_of_server);

	g_pGamePersistent->LoadTitle				(temp);*/
	g_pGamePersistent->LoadTitle();
	return true;
}

#include "xrServer.h"

bool	CLevel::net_start_client2				()
{
	if(psNET_direct_connect)
	{
		Server->create_direct_client();
	}

	connected_to_server = Connect2Server();
	
	return true;
}

bool	CLevel::net_start_client3				()
{
	if(connected_to_server){
		LPCSTR					level_name = NULL;
		if(psNET_direct_connect)
		{
			level_name	= ai().get_alife() ? *name() : Server->level_name( Server->GetConnectOptions() ).c_str();
		}else
			level_name	= ai().get_alife() ? *name() : net_SessionName	();

		// Determine internal level-ID
		int						level_id = pApp->Level_ID(level_name);
		if (level_id<0)	{
			Disconnect			();
			pApp->LoadEnd		();
			connected_to_server = FALSE;
			m_name				= level_name;
			m_connect_server_err = xrServer::ErrNoLevel;
			return				false;
		}
		pApp->Level_Set			(level_id);
		m_name					= level_name;
		// Load level
		R_ASSERT2				(Load(level_id),"Loading failed.");

	}
	return true;
}

bool	CLevel::net_start_client4				()
{
	if(connected_to_server){
		// Begin spawn
//		g_pGamePersistent->LoadTitle		("st_client_spawning");
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

		// Send network to single or multithreaded mode
		// *note: release version always has "mt_*" enabled
		Device.seqFrameMT.Remove			(g_pNetProcessor);
		Device.seqFrame.Remove				(g_pNetProcessor);
		if (psDeviceFlags.test(mtNetwork))	Device.seqFrameMT.Add	(g_pNetProcessor,REG_PRIORITY_HIGH	+ 2);
		else								Device.seqFrame.Add		(g_pNetProcessor,REG_PRIORITY_LOW	- 2);

		if(!psNET_direct_connect)
		{
			// Waiting for connection/configuration completition
			CTimer	timer_sync	;	timer_sync.Start	();
			while	(!net_isCompleted_Connect())	Sleep	(5);
			Msg		("* connection sync: %d ms", timer_sync.GetElapsed_ms());
			while	(!net_isCompleted_Sync())	{ ClientReceive(); Sleep(5); }
		}

		while(!game_configured)			
		{ 
			ClientReceive(); 
			if(Server)
				Server->Update()	;
			Sleep(5); 
		}
/*
		if(psNET_direct_connect)
		{
			ClientReceive(); 
			if(Server)
					Server->Update()	;
			Sleep(5);
		}else

			while(!game_configured)			
			{ 
				ClientReceive(); 
				if(Server)
					Server->Update()	;
				Sleep(5); 
			}
*/
		}
	return true;
}

bool	CLevel::net_start_client5				()
{
	if(connected_to_server){
		// HUD

		// Textures
		if	(!g_dedicated_server)
		{
			pHUD->Load							();
//			g_pGamePersistent->LoadTitle				("st_loading_textures");
			g_pGamePersistent->LoadTitle();
			Device.m_pRender->DeferredLoad(FALSE);
			Device.m_pRender->ResourcesDeferredUpload();
			LL_CheckTextures					();
		}
	}
	return true;
}

bool	CLevel::net_start_client6				()
{
	if(connected_to_server){
		// Sync
		if (g_hud)
		{
			g_hud->Load();
			g_hud->OnConnected();
		}

		if (game)
			game->OnConnected();

//		g_pGamePersistent->LoadTitle		("st_client_synchronising");
		g_pGamePersistent->LoadTitle();
		Device.PreCache						(60, true, true);
		net_start_result_total				= TRUE;
	}else{
		net_start_result_total				= FALSE;
	}

	pApp->LoadEnd							(); 
	return true;
}