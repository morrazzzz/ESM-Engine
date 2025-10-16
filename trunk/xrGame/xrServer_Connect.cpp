#include "stdafx.h"
#include "xrServer.h"
#include "game_sv_single.h"

void xrServer::Connect(shared_str& options)
{
	game = new game_sv_Single();
	game->Create(options);
}

