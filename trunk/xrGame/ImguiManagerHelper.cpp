#include "StdAfx.h"
#include "ImguiManager.h"

void ImGuiPushTextHelper(const char* text, ImGuiHoveredFlags flags)
{
	if (ImGui::IsItemHovered(flags) && ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(text);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}