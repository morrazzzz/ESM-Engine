#pragma once

enum {
	M_SPAWN = 1, //Hack! In all.spawn M_SPAWN has ID = 1
	
	M_CHANGE_LEVEL,				// changing level
	M_LOAD_GAME,
	M_SAVE_GAME,
	M_SAVE_PACKET,
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
};
