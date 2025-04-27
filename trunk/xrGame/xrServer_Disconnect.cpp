#include "stdafx.h"
#include "xrServer.h"

void xrServer::Disconnect()
{
	xr_delete				(game);
}
