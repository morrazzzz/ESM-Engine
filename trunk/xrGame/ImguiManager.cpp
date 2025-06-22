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

	ImGui::End();
}

void CImguiManager::SpawnMenuWindow(bool* OpenedSpawnWindow)
{
	ImGui::Begin("Spawn menu", OpenedSpawnWindow, ImGuiWindowFlags_MenuBar);

	xr_vector<const char*>& sections = managerSpawnMenu->SectionsSpawnMenu;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("More"))
		{
			if (ImGui::MenuItem("Views all sections"))
			{
				
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

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

	int& typeLocation = managerSpawnMenu->typeLocationSpawn;

	if (ImGui::RadioButton("Camera direction", typeLocation == 0))
		typeLocation = 0;

	ImGui::SameLine();

	if (ImGui::RadioButton("Actor position", typeLocation == 1))
		typeLocation = 1;

	ImGui::SameLine();

	if (ImGui::RadioButton("Actor inventory", typeLocation == 2))
		typeLocation = 2;

	ImGui::InputInt("Count item spawn", &managerSpawnMenu->countItemToSpawn,
		1, 50);

	if (managerSpawnMenu->RegisterDoubleMouseClick)
	{
		if (ImGui::IsMouseDoubleClicked(MOUSE_LEFT_BUTTON))
			__debugbreak();
	}

	string64& findSect = managerSpawnMenu->findSections;
	ImGui::InputText("Find section", findSect, sizeof findSect);

	static int ItemCurrentInList = 1;

	bool finding = false;
	if (xr_strlen(findSect) > 0)
	{
		xr_vector<const char*> findSections{};
		
		for (u32 i = 0; i < sections.size(); i++)
		{
			if (strstr(sections[i], findSect))
				findSections.emplace_back(sections[i]);
		}

		if (ImGui::ListBox("Sections", &ItemCurrentInList, findSections.data(), findSections.size(), 10))
		{
			if (ImGui::IsMouseDoubleClicked(MOUSE_LEFT_BUTTON))
				__debugbreak();
		}

		finding = true;
	}
	else
	{
		if (sections.empty())
		{
			for (const auto& sect : pSettings->sections())
			{
				sections.emplace_back(sect->Name.c_str());
			}
		}

		ImGui::ListBox("Sections", &ItemCurrentInList, sections.data(), sections.size(), 10);
	}
	ImGui::SameLine();

	managerSpawnMenu->RenderTextureEquipment(false, ItemCurrentInList);
	
	ImGui::End();
}