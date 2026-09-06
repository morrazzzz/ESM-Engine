#include "stdafx.h"
#include "UITabControl.h"
#include "UITabButton.h"

using namespace luabind;

#pragma optimize("s",on)
void CUITabControl::script_register(lua_State *L)
{
	module(L)
	[
		class_<CUITabControl, CUIWindow>("CUITabControl")
		.def(					constructor<>())
		.def("AddItem",			(bool (CUITabControl::*)(CUITabButton*))(&CUITabControl::AddItem), adopt<2>())
		.def("AddItem",			(bool (CUITabControl::*)(const char*, const char*,float,float, float,float))	&CUITabControl::AddItem)
		.def("RemoveAll",				&CUITabControl::RemoveAll)
		.def("GetActiveId",				&CUITabControl::GetActiveId_script)
		.def("GetTabsCount",			&CUITabControl::GetTabsCount)
		.def("SetActiveTab",			&CUITabControl::SetActiveTab_script)
		.def("GetButtonById",			&CUITabControl::GetButtonById_script),

		class_<CUITabButton, CUIButton>("CUITabButton")
		.def(							constructor<>())		
	];

}