#include "pch_script.h"
#include "UIGame_custom_script.h"
#include "xrServer_script_macroses.h"
#include "ui/UIMultiTextStatic.h"

using namespace luabind;

#pragma optimize("s",on)
void UIGame_custom_script::script_register(lua_State *L)
{
	typedef UIGame_custom_script BaseType;
	module(L)
		[
			class_< UIGame_custom_script, CUIGameCustom >("UIGame_custom_script")
			.def(	constructor<>())
		];
}
