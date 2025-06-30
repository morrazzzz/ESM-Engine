#pragma once

#include <imgui.h>

struct CImguiManagerObjectSpawn
{
	xr_string translateName;
	xr_string sectName;
	xr_string translateAndSectName;
};

struct CImguiManagerSpawnMenu
{
	bool iconEquipmentsView{ false };
	bool getTextureRef{ true };
	bool processSpawnItem{ false };
	int typeLocationSpawn{ 0 };
	int countItemToSpawn{ 1 };

	u32 textureWidth{ 0 }, textureHeight{ 0 };
	const char* sectNameToSpawn;
	ImTextureRef equipmentTextureRef;

	xr_vector<CImguiManagerObjectSpawn> sectionsSpawnMenu{};
	xr_vector<CImguiManagerObjectSpawn> findSectionsSpawnMenu{};
	string64 findSections{};

	CImguiManagerSpawnMenu() = default;
	~CImguiManagerSpawnMenu() = default;

	void RenderTextureEquipment(const bool find, const int currentItem);
	void SpawnMenuBar();
	void UISpawnMenu();
	void UIProcessSpawn();
private:
	void WindowEquipment();
	void UISpawnMenuList(const bool find);
	void UISpawnMenuImageButton(const bool find);
	void UISpawnObject(const char* section);
};

class CImguiManager
{
	bool openBeginWindow{ true };
	bool openSpawnMenu{ true };

	CImguiManagerSpawnMenu* managerSpawnMenu;
public:
	CImguiManager();
	~CImguiManager();

	void UpdateImgui();
	void PreUpdateImGui();
private:
	void SpawnMenuWindow(bool* OpenSpawnWindow);
};

extern CImguiManager ImGuiManager;
