#include "stdafx.h"
#include "level.h"
#include "Level_Bullet_Manager.h"
#include "xrserver.h"
#include "game_cl_base.h"
#include "xrmessages.h"
#include "xrGameSpyServer.h"
#include "../xr_3da/x_ray.h"
#include "../xr_3da/device.h"
#include "../xr_3da/IGame_Persistent.h"
#include "../xr_3da/xr_ioconsole.h"
#include "MainMenu.h"
#include "UIGameCustom.h"

BOOL CLevel::net_Start	( LPCSTR op_server, LPCSTR op_client )
{
	net_start_result_total				= TRUE;

	pApp->LoadBegin				();

	//make Client Name if options doesn't have it
	LPCSTR	NameStart	= strstr(op_client,"/name=");
	if (!NameStart)
	{
		string512 tmp;
		strcpy_s(tmp, op_client);
		strcat_s(tmp, "/name=");
		strcat_s(tmp, xr_strlen(Core.UserName) ? Core.UserName : Core.CompName);
		m_caClientOptions			= tmp;
	} else {
		string1024	ret="";
		LPCSTR		begin	= NameStart + xr_strlen("/name="); 
		sscanf			(begin, "%[^/]",ret);
		if (!xr_strlen(ret))
		{
			string1024 tmpstr;
			strcpy_s(tmpstr, op_client);
			*(strstr(tmpstr, "name=")+5) = 0;
			strcat_s(tmpstr, xr_strlen(Core.UserName) ? Core.UserName : Core.CompName);
			const char* ptmp = strstr(strstr(op_client, "name="), "/");
			if (ptmp)
				strcat_s(tmpstr, ptmp);
			m_caClientOptions = tmpstr;
		}
		else
		{
			m_caClientOptions			= op_client;
		};		
	};
	m_caServerOptions			    = op_server;
	//---------------------------------------------------------------------
	m_bDemoPlayMode = FALSE;
	m_aDemoData.clear();
	m_bDemoStarted	= FALSE;
	//---------------------------------------------------------------------------
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start1));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start2));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start3));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start4));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start5));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start6));
	
	return net_start_result_total;

}

bool CLevel::net_start1				()
{
	// Start client and server if need it
	if (m_caServerOptions.size())
	{
//		g_pGamePersistent->LoadTitle		("st_server_starting");
		g_pGamePersistent->LoadTitle();

		typedef IGame_Persistent::params params;
		params							&p = g_pGamePersistent->m_game_params;
		// Connect
		Server					= xr_new<xrServer>();
		
//		if (!strstr(*m_caServerOptions,"/alife")) 
		if (xr_strcmp(p.m_alife,"alife"))
		{
			string64			l_name = "";
			const char* SOpts = *m_caServerOptions;
			strncpy(l_name, *m_caServerOptions, strchr(SOpts, '/') - SOpts);
			// Activate level
			if (strchr(l_name,'/'))
				*strchr(l_name,'/')	= 0;

			m_name					= l_name;

			int						id = pApp->Level_ID(l_name);

			if (id<0) {
				pApp->LoadEnd				();
				Log							("Can't find level: ",l_name);
				net_start_result_total		= FALSE;
				return true;
			}
			pApp->Level_Set			(id);
		}
	}
	return true;
}

bool CLevel::net_start2				()
{
	if (net_start_result_total && m_caServerOptions.size())
	{
		if ((m_connect_server_err=Server->Connect(m_caServerOptions))!=xrServer::ErrNoError)
		{
			net_start_result_total = false;
			Msg				("! Failed to start server.");
//			Console->Execute("main_menu on");
			return true;
		}
		Server->SLS_Default		();
		m_name					= Server->level_name(m_caServerOptions);
	}
	return true;
}

bool CLevel::net_start3				()
{
	if(!net_start_result_total) return true;
	return true;
}

bool CLevel::net_start4				()
{
	if(!net_start_result_total) return true;

	g_loading_events.pop_front();

	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client6));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client5));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client4));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client3));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client2));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client1));

	return false;
}

bool CLevel::net_start5				()
{
	if (net_start_result_total)
	{
		NET_Packet		NP;
		NP.w_begin		(M_CLIENTREADY);
		Send			(NP,net_flags(TRUE,TRUE));

		if (OnClient() && Server)
		{
			Server->SLS_Clear();
		};
	};
	return true;
}
#include "hudmanager.h"
bool CLevel::net_start6()
{
	//init bullet manager
	BulletManager().Clear		();
	BulletManager().Load		();

	pApp->LoadEnd				();

	if(net_start_result_total)
	{
		if (strstr(Core.Params,"-$")) 
		{
			string256				buf,cmd,param;
			sscanf					(strstr(Core.Params,"-$")+2,"%[^ ] %[^ ] ",cmd,param);
			strconcat				(sizeof(buf),buf,cmd," ",param);
			Console->Execute		(buf);
		}

		if (!g_dedicated_server)
		{
			if (CurrentGameUI())
				CurrentGameUI()->OnConnected();
		}
	}

	return true;
}


void CLevel::InitializeClientGame	(NET_Packet& P)
{
	string256 game_type_name;
	P.r_stringZ(game_type_name);
	if(game && !xr_strcmp(game_type_name, game->type_name()) )
		return;
	
	xr_delete(game);
	Msg("- Game configuring : Started ");
	CLASS_ID clsid			= game_GameState::getCLASS_ID(game_type_name,false);
	game					= smart_cast<game_cl_GameState*> ( NEW_INSTANCE ( clsid ) );
	game->set_type_name(game_type_name);
	game->Init();
	m_bGameConfigStarted	= TRUE;
}

