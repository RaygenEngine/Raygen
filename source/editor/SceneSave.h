#pragma once

#include "asset/AssetPod.h"

#include <imgui/imgui.h>
#include <imgui_ext/imfilebrowser.h>

class World;

class SceneSave {
	ImGui::FileBrowser m_saveBrowser;

public:
	SceneSave();

	void OpenBrowser();

	void Draw();

	static void SaveAs(World* world, const uri::Uri& paths);
};
