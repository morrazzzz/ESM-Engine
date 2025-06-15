#pragma once
class CImguiManager
{
	bool OpenBeginWindow{ true };
public:
	CImguiManager();
	~CImguiManager() = default;

	void UpdateImgui();
};

extern CImguiManager ImGuiManager;
