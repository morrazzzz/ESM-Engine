#include "stdafx.h"
#include "xrServer.h"
#include "alife_simulator.h"
#include "xrserver_objects.h"
#include "ai_space.h"
#include "alife_object_registry.h"
#include "xrServer_Objects_ALife_Items.h"
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

	CSE_Abstract*	receiver	= game->get_entity_from_eid	(destination);
	if (receiver)	
	{
		//R_ASSERT(receiver->owner);
		receiver->OnEvent						(P,type,timestamp);

	};

	switch		(type)
	{
	case GE_INFO_TRANSFER:
	case GE_WPN_STATE_CHANGE:
	case GE_ZONE_STATE_CHANGE:
	case GEG_PLAYER_ATTACH_HOLDER:
	case GEG_PLAYER_DETACH_HOLDER:
	case GEG_PLAYER_ACTIVATEARTEFACT:
		{
		SendBroadcast			(P);
		}break;
	case GE_HIT:
		{
			P.r_pos -=2;
			SendBroadcast(P);
		} break;
	case GE_ASSIGN_KILLER: {
		u16							id_src;
		P.r_u16						(id_src);
		
		CSE_Abstract				*e_dest = receiver;	// кто умер
		// this is possible when hit event is sent before destroy event
		if (!e_dest)
			break;

		CSE_ALifeCreatureAbstract	*creature = smart_cast<CSE_ALifeCreatureAbstract*>(e_dest);
		if (creature)
			creature->m_killer_id	= id_src;

//		Msg							("[%d][%s] killed [%d][%s]",id_src,id_src==u16(-1) ? "UNKNOWN" : game->get_entity_from_eid(id_src)->name_replace(),id_dest,e_dest->name_replace());

		break;
	}
	case GE_CHANGE_VISUAL:
		{
			CSE_Visual* visual		= smart_cast<CSE_Visual*>(receiver); VERIFY(visual);
			string256 tmp;
			P.r_stringZ				(tmp);
			visual->set_visual		(tmp);
		}break;
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
	case GE_MONEY:
		{
			CSE_Abstract				*e_dest = receiver;
			CSE_ALifeTraderAbstract*	pTa = smart_cast<CSE_ALifeTraderAbstract*>(e_dest);
			pTa->m_dwMoney				= P.r_u32();
						
		}break;
	case GE_FREEZE_OBJECT:
		break;
	default:
		R_ASSERT2	(0,"Game Event not implemented!!!");
		break;
	}
}
