#pragma once

enum{
		AF_GODMODE			=(1<<0),
		AF_INVISIBLE		=(1<<1),
		AF_ALWAYSRUN		=(1<<2),
		AF_UNLIMITEDAMMO	=(1<<3),
		AF_RUN_BACKWARD		=(1<<4),
		AF_AUTOPICKUP		=(1<<5),
		AF_PSP				=(1<<6),
		AF_CROUCH_TOGGLE    =(1<<7),
		AF_WALK_TOGGLE      =(1<<8),
		AF_SPRINT_TOGGLE    =(1<<9),
};

extern Flags32 psActorFlags;

extern BOOL		GodMode	();	

