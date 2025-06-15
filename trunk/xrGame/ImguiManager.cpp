#include "stdafx.h"
#include "ImguiManager.h"
#include <imgui.h>

CImguiManager ImGuiManager;

CImguiManager::CImguiManager()
{
	R_ASSERT(Device.getImguiContext());
	ImGui::SetCurrentContext(Device.getImguiContext());
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

	if (ImGui::Button("Write to log"))
		Msg("~~~ [%s]: Write to log :)", __FUNCTION__);

	ImGui::End();
}