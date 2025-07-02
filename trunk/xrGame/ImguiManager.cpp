#include "stdafx.h"
#include "ImguiManager.h"
#include <imgui.h>

CImguiManager ImGuiManager;

CImguiManager::CImguiManager()
{
	R_ASSERT(Device.getImguiContext());
	ImGui::SetCurrentContext(Device.getImguiContext());

	managerSpawnMenu = new CImguiManagerSpawnMenu();
	managerDebugMenu = new CImguiManagerDebugMenu();
}

CImguiManager::~CImguiManager()
{
	delete managerDebugMenu;
	delete managerSpawnMenu;
}

void CImguiManager::PreUpdateImGui()
{
	managerSpawnMenu->UIProcessSpawn();
}

void CImguiManager::UpdateImgui()
{
	ImGui::Begin("ImGui Manager", &openBeginWindow);

	if (!openBeginWindow)
	{
		openBeginWindow = true;
		Device.setImGuiClosed(true);
		ImGui::End();
		return;
	}

	if (ImGui::Button("Spawn menu"))
		openSpawnMenu = true;
	
	if (ImGui::Button("Debug menu"))
		openDebugMenu = true;

	if (openSpawnMenu)
		SpawnMenuWindow();

	if (openDebugMenu)
		DebugMenuWindow();

	ImGui::ShowDemoWindow();

	if (ImGui::Button("Write to log"))
		Msg("~~~ [%s]: Write to log :)", __FUNCTION__);

	ImGui::End();
}

void CImguiManager::SpawnMenuWindow()
{
	ImGui::Begin("Spawn menu", &openSpawnMenu, ImGuiWindowFlags_MenuBar);

	if (!openSpawnMenu)
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

void CImguiManager::DebugMenuWindow()
{
	ImGui::Begin("Debug menu", &openDebugMenu);

	if (!openDebugMenu)
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

	managerDebugMenu->UIDebugMenu();

	ImGui::End();
}