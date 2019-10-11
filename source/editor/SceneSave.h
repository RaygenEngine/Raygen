#pragma once

#include "asset/AssetPod.h"

#include <imgui/imgui.h>
#include <imgui_ext/imfilebrowser.h>

class World;
class Node;

class SceneSave {
	ImGui::FileBrowser m_saveBrowser;

public:
	SceneSave();

	void OpenBrowser();

	void Draw();

	static bool SaveAsXML(World* world, const uri::Uri& paths);
};
