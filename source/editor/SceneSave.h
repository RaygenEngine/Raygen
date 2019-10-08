#pragma once
#include "imgui/imgui.h"
#include "imgui_ext/imfilebrowser.h"

#include "asset/AssetPod.h"

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