#include "stdafx.h"
#include "xrServer.h"
#include "xrServer_Objects_ALife_Monsters.h"

void xrServer::Process_event	(NET_Packet& P)
{
	u32			timestamp;
	u16			type;
	u16			destination;

	// correct timestamp with server-unique-time (note: direct message correction)
	P.r_u32		(timestamp	);

	// read generic info
	P.r_u16		(type		);
	P.r_u16		(destination);

	switch		(type)
	{
	case GEG_PLAYER_ACTIVATEARTEFACT:
		{
		SendBroadcast			(P);
		}break;
	case GE_HIT:
		{
			P.r_pos -=2;
			SendBroadcast(P);
		} break;
	case GE_CHANGE_POS:
		{			
			SendTo(P);
		}break;
	case GEG_PLAYER_WEAPON_HIDE_STATE:
		{
			SendTo(P);
		}break;
	case GE_TELEPORT_OBJECT:
		{
			game->teleport_object	(P,destination);
		}break;
	case GE_ADD_RESTRICTION:
		{
			game->add_restriction	(P,destination);
		}break;
	case GE_REMOVE_RESTRICTION:
		{
			game->remove_restriction(P,destination);
		}break;
	case GE_REMOVE_ALL_RESTRICTIONS:
		{
			game->remove_all_restrictions(P,destination);
		}break;
	default:
		R_ASSERT2	(0,"Game Event not implemented!!!");
		break;
	}
}
