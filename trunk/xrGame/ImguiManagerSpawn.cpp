#include "stdafx.h"
#include "ImguiManager.h"
#include <ui/UIInventoryUtilities.h>
#include <string_table.h>

constexpr const char* InvGridParams[4] = { "inv_grid_x", "inv_grid_y",
  "inv_grid_width", "inv_grid_height" };

bool ButtonImageInterface = false;
bool visibleSectName = false;
int CurrentItemInList = 1;
int countColumnTableImageB = 5;
float ParamsItem[4];

void CImguiManagerSpawnMenu::RenderTextureEquipment(const bool find, const int currentItem)
{
	if (GetTextureRef)
	{
		Render->getImguiTextureRef(EQUIPMENT_ICONS,
			equipmentTextureRef, textureWidth, textureHeight);

		GetTextureRef = false;
	}

	xr_vector<CImguiManagerObjectSpawn>& vector = find ? SectionsSpawnMenu :
		SectionsSpawnMenu;

	for (int i = 0; i < 4; ++i)
	{
		if (!pSettings->line_exist(vector[currentItem].sectName.c_str(), InvGridParams[i]))
			return;

		ParamsItem[i] = pSettings->r_float(vector[currentItem].sectName.c_str(), InvGridParams[i]);
	}

	float x = ParamsItem[0] * INV_GRID_WIDTH;
	float y = ParamsItem[1] * INV_GRID_HEIGHT;

	float width_render = INV_GRID_WIDTH * ParamsItem[2];
	float height_render = INV_GRID_HEIGHT * ParamsItem[3];

	ImVec2 uv0 = ImVec2(x / textureWidth, y / textureHeight);
	ImVec2 uv1 = ImVec2((x + width_render) / textureWidth, 
		(y + height_render) / textureHeight);

	ImGui::Image(equipmentTextureRef, ImVec2(width_render, height_render), uv0, uv1);
}

void CImguiManagerSpawnMenu::SpawnMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("More"))
		{
			if (ImGui::MenuItem("Views all sections"))
			{

			}
			if (ImGui::MenuItem("View all icon equipments"))
			{
				iconEquipmentsView = !iconEquipmentsView;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (iconEquipmentsView)
		WindowEquipment();
}

float color[4]{};
void CImguiManagerSpawnMenu::WindowEquipment()
{
	ImGui::Begin("Viewport icon equipments");

	if (GetTextureRef)
	{
		Render->getImguiTextureRef(EQUIPMENT_ICONS,
			equipmentTextureRef, textureWidth, textureHeight);

		GetTextureRef = false;
	}

	ImGui::Image(equipmentTextureRef, ImVec2(textureWidth, textureHeight));

	ImGui::End();
}

static const char* SpawnMenuGetter(void* data, int idx)
{
	CImguiManagerObjectSpawn* const objects = (CImguiManagerObjectSpawn* const)data;

	if (xr_strlen(objects[idx].translateName.c_str()) > 0)
	{
		if (visibleSectName)
			return objects[idx].translateAndSectName.c_str();

		return objects[idx].translateName.c_str();
	}

	return objects[idx].sectName.c_str();
}

xr_string lastStringFind{};
int sizeFindSections{};

void CImguiManagerSpawnMenu::UISpawnMenu()
{
	SpawnMenuBar();

	if (ImGui::RadioButton("Camera direction", typeLocationSpawn == 0))
		typeLocationSpawn = 0;

	ImGui::SameLine();

	if (ImGui::RadioButton("Actor position", typeLocationSpawn == 1))
		typeLocationSpawn = 1;

	ImGui::SameLine();

	if (ImGui::RadioButton("Actor inventory", typeLocationSpawn == 2))
		typeLocationSpawn = 2;

	ImGui::SameLine();

	ImGui::Checkbox("Visible sect name item", &visibleSectName);

	ImGui::SameLine();
	ImGui::Checkbox("Image button interface", &ButtonImageInterface);	

	ImGui::InputInt("Count item spawn", &countItemToSpawn,
		1, 50);

	ImGui::InputText("Find section", findSections, sizeof findSections);

	if (SectionsSpawnMenu.empty())
	{
		for (const auto& sect : pSettings->sections())
		{
			CImguiManagerObjectSpawn object{ "", sect->Name.c_str(), "" };

			if (pSettings->line_exist(sect->Name, "inv_name"))
			{
				const char* inv_name = pSettings->r_string(sect->Name, "inv_name");
				const char* translateInvName = CStringTable().translate(inv_name).c_str();
				xr_string NameUTF8 = ANSIToUTF8(translateInvName);

				object.translateName = NameUTF8;

				string128 traslateAndSect{};
				sprintf_s(traslateAndSect, "%s (%s)", object.translateName.c_str(), object.sectName.c_str());

				object.translateAndSectName = traslateAndSect;
			}

			SectionsSpawnMenu.emplace_back(object);
		}
	}

	bool find = xr_strlen(findSections) > 0;
	if (find && lastStringFind != findSections)
	{
		int checkSections = 0;
		for (u32 i = 0; i < findSectionsSpawnMenu.size(); i++)
		{
			if (strstr(findSectionsSpawnMenu[i].sectName.c_str(), findSections))
				checkSections++;
		}

		if (!sizeFindSections || sizeFindSections != checkSections)
		{
			if (sizeFindSections)
				findSectionsSpawnMenu.clear();

			for (u32 i = 0; i < SectionsSpawnMenu.size(); i++)
			{
				if (strstr(SectionsSpawnMenu[i].sectName.c_str(), findSections))
					findSectionsSpawnMenu.emplace_back(SectionsSpawnMenu[i]);
			}
		}

		lastStringFind = findSections;
		sizeFindSections = findSectionsSpawnMenu.size();
	}
	else if (!find)
		findSectionsSpawnMenu.clear();

	if (ButtonImageInterface)
	{
		UISpawnMenuImageButton(find);
	}
	else
		UISpawnMenuList(find);

	ImGui::SameLine();
}

void CImguiManagerSpawnMenu::UISpawnMenuList(const bool find)
{
	auto& vec = find ? findSectionsSpawnMenu : SectionsSpawnMenu;

	ImGui::ListBox("Sections", &CurrentItemInList, SpawnMenuGetter, vec.data(), vec.size(), 10);

	RenderTextureEquipment(find, CurrentItemInList);
}

void CImguiManagerSpawnMenu::UISpawnMenuImageButton(const bool find)
{	
	auto& vec = find ? findSectionsSpawnMenu : SectionsSpawnMenu;

	for (u32 i = 0; i < vec.size(); i++)
	{
		CImguiManagerObjectSpawn& object_spawn = vec[i];

		bool RenderImageButton = true;
		for (int k = 0; k < 4; ++k)
		{
			if (!pSettings->line_exist(object_spawn.sectName.c_str(), InvGridParams[k]))
			{
				RenderImageButton = false;
				break;
			}

			ParamsItem[k] = pSettings->r_float(object_spawn.sectName.c_str(), InvGridParams[k]);
		}

		if (!RenderImageButton)
			continue;

		/*
		bool NeedSameLine = countObjectsTable < countColumnTableImageB;

		if (NeedSameLine)
			countObjectsTable++;
		else
			countObjectsTable = 0;
        */

		float x = ParamsItem[0] * INV_GRID_WIDTH;
		float y = ParamsItem[1] * INV_GRID_HEIGHT;

		float width_render = INV_GRID_WIDTH * ParamsItem[2];
		float height_render = INV_GRID_HEIGHT * ParamsItem[3];

		ImVec2 uv0 = ImVec2(x / textureWidth, y / textureHeight);
		ImVec2 uv1 = ImVec2((x + width_render) / textureWidth,
			(y + height_render) / textureHeight);

		xr_string& string = visibleSectName ? object_spawn.translateAndSectName
			: object_spawn.translateName;

//		if (NeedSameLine)
//			ImGui::SameLine();

		ImGui::Text("%s", string.c_str());

//		ImGui::PushStyleColor(ImGuiCol_Button, color_vec);
		ImGui::ImageButton(object_spawn.sectName.c_str(), equipmentTextureRef,
			ImVec2(width_render, height_render), uv0, uv1);
//		ImGui::PopStyleColor();
	}
}