#include "stdafx.h"
#include "ImguiManager.h"
#include <imgui.h>
#include <xr_3da/InputKeyEnum.h>
#include <string_table.h>

CImguiManager ImGuiManager;

CImguiManager::CImguiManager()
{
	R_ASSERT(Device.getImguiContext());
	ImGui::SetCurrentContext(Device.getImguiContext());

	managerSpawnMenu = new CImguiManagerSpawnMenu();
}

CImguiManager::~CImguiManager()
{
	delete managerSpawnMenu;
}

void CImguiManager::UpdateImgui()
{
	ImGui::Begin("ImGui Manager", &OpenBeginWindow);

	if (!OpenBeginWindow)
	{
		OpenBeginWindow = true;
		Device.setImGuiActivated(false);
		ImGui::End();
		return;
	}

	if (ImGui::Button("Spawn menu"))
		OpenSpawnMenu = true;
	
	if (OpenSpawnMenu)
		SpawnMenuWindow(&OpenSpawnMenu);

	if (ImGui::Button("Write to log"))
		Msg("~~~ [%s]: Write to log :)", __FUNCTION__);

	static int CurreItem = 1;

	const char* item1 = "Привет!";
	const char* item2 = "Пока";
	xr_string test1 = ANSIToUTF8(item1);
	xr_string test2 = ANSIToUTF8(item2);

	const char* test[] = { test1.c_str(), test2.c_str()};

	ImGui::ListBox("Test", &CurreItem, test, std::size(test), 1);

	ImGui::End();
}
bool Test = true;

void CImguiManager::SpawnMenuWindow(bool* OpenedSpawnWindow)
{
	ImGui::Begin("Spawn menu", OpenedSpawnWindow, ImGuiWindowFlags_MenuBar);

	if (OpenedSpawnWindow && *OpenedSpawnWindow == false)
	{
		ImGui::End();
		return;
	}

	if (!g_pGameLevel)
	{
		ImGui::Text("Level is not loaded! Please, load game or create new game!");
		ImGui::End();
		return;
	}

	managerSpawnMenu->UISpawnMenu();
	
	ImGui::End();
}