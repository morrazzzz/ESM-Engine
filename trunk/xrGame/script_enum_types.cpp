#include "pch_script.h"
#include "script_enum_types.h"

using namespace luabind;

void CScriptGameDifficulty::script_register(lua_State* L)
{
	module(L)
		[
			class_<enum_exporter<ACTOR_DEFS::ESingleGameDifficulty> >("game_difficulty")
				.enum_("game_difficulty")
				[
					value("novice", int(egdNovice)),
					value("stalker", int(egdStalker)),
					value("veteran", int(egdVeteran)),
					value("master", int(egdMaster))
				]
		];
}