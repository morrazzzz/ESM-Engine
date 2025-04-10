#include "stdafx.h"
#include "xrServer.h"
#include "game_sv_single.h"
#include "xrMessages.h"
#include "MainMenu.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

bool xrServer::Connect(shared_str &session_name)
{
#ifdef DEBUG
	Msg						("* sv_Connect: %s",	*session_name);
#endif

	// Parse options and create game
	if (0==strchr(*session_name,'/'))
		return false;

	string1024				options;
	R_ASSERT2(xr_strlen(session_name) <= sizeof(options), "session_name too BIIIGGG!!!");
	strcpy					(options,strchr(*session_name,'/')+1);
	
	// Parse game type
	string1024				type;
	R_ASSERT2(xr_strlen(options) <= sizeof(type), "session_name too BIIIGGG!!!");
	strcpy					(type,options);
	if (strchr(type,'/'))	*strchr(type,'/') = 0;
	game = new game_sv_Single();

	// Options
	if (0 == game)
		return false;
	
	csPlayers.Enter			();

	game->Create			(session_name);
	csPlayers.Leave			();

	return true;
}


void xrServer::new_client()
{
	Update();
}
