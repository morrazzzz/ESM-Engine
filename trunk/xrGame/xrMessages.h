#pragma once

// CL	== client 2 server message
// SV	== server 2 client message

enum {
	M_UPDATE			= 0,	// DUAL: Update state
	M_SPAWN,					// DUAL: Spawning, full state

	M_EVENT,					// Game Event
	
	M_CHANGE_LEVEL,				// changing level
	M_LOAD_GAME,
	M_SAVE_GAME,
	M_SAVE_PACKET,

	M_EVENT_PACK,					// Pack of M_EVENT

	MSG_FORCEDWORD				= u32(-1)
};

enum {
	GE_OWNERSHIP_TAKE,			// DUAL: Client request for ownership of an item
	GE_OWNERSHIP_REJECT,		// DUAL: Client request ownership rejection
	GE_HIT,						//
	GE_ASSIGN_KILLER,			//
	GE_DESTROY,					// authorative client request for entity-destroy
	GE_TELEPORT_OBJECT,

	GE_ADD_RESTRICTION,
	GE_REMOVE_RESTRICTION,
	GE_REMOVE_ALL_RESTRICTIONS,

	GE_INFO_TRANSFER,			//transfer _new_ info on PDA
	
	GE_TRADE_SELL,
	GE_TRADE_BUY,

	GE_WPN_STATE_CHANGE,

	GE_ZONE_STATE_CHANGE,

	GE_CHANGE_POS,

	GE_CHANGE_VISUAL,
	GE_MONEY,

	GEG_PLAYER_ACTIVATEARTEFACT,

	GEG_PLAYER_WEAPON_HIDE_STATE,
	
	GEG_PLAYER_ATTACH_HOLDER,
	GEG_PLAYER_DETACH_HOLDER,

	GE_FREEZE_OBJECT,
	GE_LAUNCH_ROCKET,

	GE_FORCEDWORD				= u32(-1)
};

enum
{
	M_SPAWN_OBJECT_LOCAL		= (1<<0),	// after spawn it becomes local (authorative)
	M_SPAWN_OBJECT_HASUPDATE	= (1<<2),	// after spawn info it has update inside message
	M_SPAWN_OBJECT_ASPLAYER		= (1<<3),	// after spawn it must become viewable
	M_SPAWN_OBJECT_PHANTOM		= (1<<4),	// after spawn it must become viewable
	M_SPAWN_VERSION				= (1<<5),	// control version
	M_SPAWN_UPDATE				= (1<<6),	// + update packet
	M_SPAWN_TIME				= (1<<7),	// + spawn time
	M_SPAWN_DENIED				= (1<<8),	// don't spawn entity with this flag

	M_SPAWN_OBJECT_FORCEDWORD	= u32(-1)
};
