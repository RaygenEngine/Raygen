#include "pch/pch.h"

#include "editor/SceneSave.h"
#include "system/Engine.h"
#include "world/nodes/Node.h"
#include "reflection/ReflectionTools.h"
#include "world/World.h"
#include "asset/AssetManager.h"
#include "asset/util/ParsingAux.h"
#include "asset/PodIncludes.h"
#include "asset/UriLibrary.h"
#include "world/nodes/RootNode.h"
#include "world/nodes/camera/EditorCameraNode.h"

#include <imgui/imgui.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>

SceneSave::SceneSave()
{
	m_saveBrowser = ImGui::FileBrowser(ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_EnterNewFilename
									   | ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CreateNewDir
									   | ImGuiFileBrowserFlags_::ImGuiFileBrowserFlags_CloseOnEsc);
	m_saveBrowser.SetPwd();
	m_saveBrowser.SetTitle("Save Scene As");
}

void SceneSave::OpenBrowser()
{
	m_saveBrowser.Open(m_lastFile);
}

void SceneSave::Draw()
{
	m_saveBrowser.Display();
	if (m_saveBrowser.HasSelected()) {
		fs::path file = m_saveBrowser.GetSelected();
		file.replace_extension(".json");
		m_lastFile = file.filename().string();
		m_saveBrowser.ClearSelected();
		SaveAs(Engine::GetWorld(), file.string());
	}
}

namespace {
using json = nlohmann::json;
void GenerateJsonForNode(json& j, Node* node)
{
	using namespace sceneconv;

	j[nameLabel] = node->GetName();
	j[typeLabel] = FilterNodeClassName(node->GetClass().GetName());
	j[trsLabel] = { { posLabel, node->GetLocalTranslation() }, { rotLabel, node->GetLocalPYR() },
		{ scaleLabel, node->GetLocalScale() } };

	refltools::PropertiesToJson(node, j);

	json childrenArray = json::array();
	for (auto& childNode : node->GetChildren()) {
		if (childNode->IsA<EditorCameraNode>()) {
			continue;
		}
		json jsonChild = json::object();
		GenerateJsonForNode(jsonChild, childNode.get());
		childrenArray.emplace_back(jsonChild);
	}
	j[childrenLabel] = std::move(childrenArray);
}
} // namespace

void SceneSave::SaveAs(World* world, const uri::Uri& p)
{
	std::ofstream ofile(p);

	if (!ofile.is_open()) {
		Engine::SetStatusLine("Failed to save scene.");
		LOG_ERROR("Failed to open file for world save: {}", p);
		return;
	}

	json scene = json::array();

	for (auto& child : world->GetRoot()->GetChildren()) {
		if (child->IsA<EditorCameraNode>()) {
			continue;
		}
		json jsonChild = json::object();
		GenerateJsonForNode(jsonChild, child.get());
		scene.emplace_back(jsonChild);
	}

	ofile << std::setw(4) << scene;
	LOG_REPORT("Written scene at: {}", p);
}
