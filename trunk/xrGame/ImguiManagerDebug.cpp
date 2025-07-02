#include "StdAfx.h"
#include "ImguiManager.h"
#include <Level.h>

void CImguiManagerDebugMenu::UIDebugMenu()
{
	ListDebugMenuEntity();
}

static int DebugMenuCurrentItem = 0;

const char* DebugMenuGetterList(void* user_data, int idx)
{
	CObject** objects = static_cast<CObject**>(user_data);

	return objects[idx]->cName().c_str();
}

void CImguiManagerDebugMenu::ListDebugMenuEntity()
{
	ImGui::ListBox("Entities", &DebugMenuCurrentItem, DebugMenuGetterList, 
		Level().Objects.GetDataObjectsActive(),
		Level().Objects.CountActiveObjects(), 10);
}