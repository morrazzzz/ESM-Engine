#include "stdafx.h"
#include "ImguiManager.h"
#include <ui/UIInventoryUtilities.h>

constexpr const char* InvGridParams[4] = { "inv_grid_x", "inv_grid_y",
  "inv_grid_width", "inv_grid_height" };

float ParamsItem[4];

void CImguiManagerSpawnMenu::RenderTextureEquipment(const bool find, const int currentItem)
{
	if (GetTextureRef)
	{
		Render->getImguiTextureRef(EQUIPMENT_ICONS,
			equipmentTextureRef, textureWidth, textureHeight);

		GetTextureRef = false;
	}

	xr_vector<const char*>& vector = find ? SectionsSpawnMenu :
		SectionsSpawnMenu;

	for (int i = 0; i < 4; ++i)
	{
		if (!pSettings->line_exist(vector[currentItem], InvGridParams[i]))
			return;

		ParamsItem[i] = pSettings->r_float(vector[currentItem], InvGridParams[i]);
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