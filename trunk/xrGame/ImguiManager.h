#pragma once

#include <imgui.h>

struct CImguiManagerSpawnMenu
{
	bool RegisterDoubleMouseClick{ false };
	bool GetTextureRef{ true };
	int typeLocationSpawn{ 0 };
	int countItemToSpawn{ 1 };

	u32 textureWidth{ 0 }, textureHeight{ 0 };
	ImTextureRef equipmentTextureRef;

	xr_vector<const char*> SectionsSpawnMenu{};
	xr_vector<const char*> findSectionsSpawnMenu{};
	string64 findSections{};

	CImguiManagerSpawnMenu() = default;
	~CImguiManagerSpawnMenu() = default;

	void RenderTextureEquipment(const bool find, const int currentItem);
};

class CImguiManager
{
	bool OpenBeginWindow{ true };
	bool OpenSpawnMenu{ true };

	CImguiManagerSpawnMenu* managerSpawnMenu;
public:
	CImguiManager();
	~CImguiManager();

	void UpdateImgui();

private:
	void SpawnMenuWindow(bool* OpenSpawnWindow);
};

extern CImguiManager ImGuiManager;
