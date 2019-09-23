#pragma once
#include "imgui/imgui.h"
#include "imgui_ext/imfilebrowser.h"
#include <filesystem>

namespace fs = std::filesystem;

class World;
class Node;

class SceneSave
{
	ImGui::FileBrowser m_saveBrowser;
public:
	SceneSave();

	void OpenBrowser();

	void Draw();

	static bool SaveAsXML(World* world, const fs::path& path);


	static void SerializeNodeData(Node* node);

};