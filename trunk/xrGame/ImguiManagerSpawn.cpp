#include "stdafx.h"
#include "ImguiManager.h"
#include <ui/UIInventoryUtilities.h>
#include <string_table.h>
#include <Actor.h>
#include <ai_object_location.h>
#include <HUDManager.h>
#include "xrServer_Object_Base.h"

constexpr const char* InvGridParams[4] = { "inv_grid_x", "inv_grid_y",
  "inv_grid_width", "inv_grid_height" };

float ParamsItem[4]; 

bool visibleSectName = false;
int currentItemInList = 0;

bool enableButtonImage = false;
bool buttonImageInTable = false;
int countColumnInTable = 5;

xr_string lastStringFind{};

void CImguiManagerSpawnMenu::RenderTextureEquipment(const bool find, const int currentItem)
{
	if (getTextureRef)
	{
		Render->getImguiTextureRef(EQUIPMENT_ICONS,
			equipmentTextureRef, textureWidth, textureHeight);

		getTextureRef = false;
	}

	bool find_vector = find && !findSectionsSpawnMenu.empty();
	xr_vector<CImguiManagerObjectSpawn>& vector = find_vector ? findSectionsSpawnMenu : sectionsSpawnMenu;

	for (int i = 0; i < 4; ++i)
	{
		if (!pSettings->line_exist(vector[currentItem].sectName.c_str(), InvGridParams[i]))
			return;

		ParamsItem[i] = pSettings->r_float(vector[currentItem].sectName.c_str(), InvGridParams[i]);
	}

	ImGui::SameLine();

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
	ImGui::Begin("Viewport icon equipments", &iconEquipmentsView);

	if (!iconEquipmentsView)
	{
		ImGui::End();
		return;
	}

	if (getTextureRef)
	{
		Render->getImguiTextureRef(EQUIPMENT_ICONS,
			equipmentTextureRef, textureWidth, textureHeight);

		getTextureRef = false;
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


void CImguiManagerSpawnMenu::UISpawnObject(const char* section)
{
	NET_Packet tNetPacket;
	u32 LevelVertexIDActor = Actor()->ai_location().level_vertex_id();
	Fvector PosObject{};
	if (typeLocationSpawn == 0)
		PosObject.mad(Device.vCameraPosition, Device.vCameraDirection, HUD().GetCurrentRayQuery().range);

	switch (typeLocationSpawn)
	{
	case 0:
		PosObject.mad(Device.vCameraPosition, Device.vCameraDirection, HUD().GetCurrentRayQuery().range);
		break;
	default:
		PosObject = Actor()->Position();
		break;
	}

	u16 Parent = typeLocationSpawn == 2 ? Actor()->ID() : static_cast<u16>(-1);

	for (int i = 0; i < countItemToSpawn; ++i)
	{
		auto object = Level().spawn_item(section, PosObject, LevelVertexIDActor, Parent, true);

		object->ObjectCustomSpawn = true;

		object->Spawn_Write(tNetPacket, true);

		Level().Server->Process_spawn(tNetPacket, object);

		CObject* O = Level().Objects.Create(*object->s_name);

		if (!O)
		{
			Msg("! Failed spawn object in g_spawn: [%s] :(", object->s_name.c_str());
			Level().Server->entity_Destroy(object);
			continue;
		}

		O->setID(object->ID);
		Level().Objects.net_Register(O);
	}

	processSpawnItem = false;
}

void CImguiManagerSpawnMenu::UISpawnMenuOptions()
{
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Options"))
	{
		if (ImGui::BeginTable("TableOptions", 2, ImGuiTableFlags_BordersV))
		{
			ImGui::TableNextColumn();
			ImGui::Checkbox("Enable ImageButton", &enableButtonImage);

			ImGuiPushTextHelper("Replace ListBox on ImageButton with icon item.");

			ImGui::TableNextColumn();
			ImGui::Checkbox("Visible section item", &visibleSectName);

			ImGuiPushTextHelper("Enable visible item section. It will be displayed nearby item name. Example: Medusa (af_medusa)");

			ImGui::TableNextColumn();
			ImGui::BeginDisabled(!enableButtonImage);
			ImGui::Checkbox("ImageButton interface in table", &buttonImageInTable);
			ImGui::EndDisabled();

			ImGuiPushTextHelper("Becomes actived after enable 'Enable ImageButton' flag.",
				ImGuiHoveredFlags_AllowWhenDisabled);

			ImGui::EndTable();
		}

		ImGui::BeginDisabled(!buttonImageInTable || !enableButtonImage);
		ImGui::InputInt("Count column for ImageButton table", &countColumnInTable);
		clamp(countColumnInTable, 1, 10);
		ImGui::EndDisabled();

		ImGuiPushTextHelper("Becomes actived after enable 'Enable ImageButton' and 'ImageButton interface in table' flag.  If count finded items < this number, that count columns will depend from count finded items.",
			ImGuiHoveredFlags_AllowWhenDisabled);
	}

	ImGui::Separator();
}

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

	UISpawnMenuOptions();

	ImGui::InputInt("Count item spawn", &countItemToSpawn);
	clamp(countItemToSpawn, 1, 75);

	ImGui::InputText("Find section", findSections, sizeof findSections);

	if (sectionsSpawnMenu.empty())
	{
		for (const auto& sect : pSettings->sections())
		{
			if (!pSettings->line_exist(sect->Name, "class"))
				continue;

			CImguiManagerObjectSpawn object{ "", sect->Name.c_str(), sect->Name.c_str() };

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

			sectionsSpawnMenu.emplace_back(object);
		}

		findSectionsSpawnMenu.reserve(sectionsSpawnMenu.size());
	}

	bool find = xr_strlen(findSections) > 0;
	if (find && lastStringFind != findSections)
	{
		if (!findSectionsSpawnMenu.empty())
			findSectionsSpawnMenu.clear();

		for (u32 i = 0; i < sectionsSpawnMenu.size(); i++)
		{
			if (!strstr(sectionsSpawnMenu[i].translateAndSectName.c_str(), findSections))
				continue;

			findSectionsSpawnMenu.emplace_back(sectionsSpawnMenu[i]);
		}

		if (currentItemInList > static_cast<int>(findSectionsSpawnMenu.size()))
			currentItemInList = 0;

		lastStringFind = findSections;
	}
	else if (!find)
	{
		findSectionsSpawnMenu.clear();
		lastStringFind = "";
	}

	if (enableButtonImage)
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_Borders;
		int colums = static_cast<int>(findSectionsSpawnMenu.size()) < countColumnInTable ? findSectionsSpawnMenu.size() : countColumnInTable;

		if (buttonImageInTable && ImGui::BeginTable("tableImageButton", colums, flags))
		{
			UISpawnMenuImageButton(find, true);
			ImGui::EndTable();
		}
		else
			UISpawnMenuImageButton(find, false);
	}        
	else
		UISpawnMenuList(find);
}

void CImguiManagerSpawnMenu::UISpawnMenuList(const bool find)
{
	auto& vec = find ? findSectionsSpawnMenu : sectionsSpawnMenu;

	ImGui::ListBox("Sections", &currentItemInList, SpawnMenuGetter, vec.data(), vec.size(), 10);
	
	RenderTextureEquipment(find, currentItemInList);

	if (ImGui::Button("Spawn item"))
	{
		sectNameToSpawn = vec[currentItemInList].sectName.c_str();
		processSpawnItem = true;
	}
}

void CImguiManagerSpawnMenu::UISpawnMenuImageButton(const bool find, const bool table)
{	
	auto& vec = find ? findSectionsSpawnMenu : sectionsSpawnMenu;

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

		if (table)
			ImGui::TableNextColumn();

		ImGui::Text("%s", string.c_str());

//		ImGui::PushStyleColor(ImGuiCol_Button, color_vec);
		if (ImGui::ImageButton(object_spawn.sectName.c_str(), equipmentTextureRef,
			ImVec2(width_render, height_render), uv0, uv1))
		{
			processSpawnItem = true;
			sectNameToSpawn = object_spawn.sectName.c_str();
		}
//		ImGui::PopStyleColor();
	}
}

void CImguiManagerSpawnMenu::UIProcessSpawn()
{
	if (!processSpawnItem)
		return;

	UISpawnObject(sectNameToSpawn);
}