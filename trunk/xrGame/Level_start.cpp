#include "stdafx.h"
#include "level.h"
#include "Level_Bullet_Manager.h"
#include "xrserver.h"
#include "xrmessages.h"
#include "../xr_3da/x_ray.h"
#include "../xr_3da/device.h"
#include "../xr_3da/IGame_Persistent.h"
#include "../xr_3da/xr_ioconsole.h"
#include "MainMenu.h"
#include "UIGameCustom.h"
#include "ai_space.h"
#include "alife_simulator.h"

BOOL CLevel::net_Start	( LPCSTR op_server )
{
	pApp->LoadBegin				();

	m_caServerOptions			    = op_server;

	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start1));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start2));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start3));
	
	return true;
}

bool CLevel::net_start1				()
{
	if (m_caServerOptions.size())
	{
		Server = new xrServer();
		Server->Connect(m_caServerOptions);
		m_name = Server->level_name(m_caServerOptions);

		ai().alife().switch_distance();
	}
	return true;
}

bool CLevel::net_start2				()
{
	g_loading_events.pop_front();

	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client3));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client2));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client1));

	return false;
}

bool CLevel::net_start3()
{
	//init bullet manager
	BulletManager().Clear		();
	BulletManager().Load		();

	pApp->LoadEnd				();

	if (!g_dedicated_server)
	{
		if (CurrentGameUI())
			CurrentGameUI()->OnConnected();
	}

	return true;
}

