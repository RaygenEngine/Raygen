#pragma once

#include "asset/AssetPod.h"

#include <imgui.h>
#include <imfilebrowser.h>

class World;

class SceneSave {
	ImGui::FileBrowser m_saveBrowser;
	// std::string m_lastFile;

public:
	SceneSave();

	void OpenBrowser();

	void Draw();

	static void SaveAs(World* world, const uri::Uri& p);
};
