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

#include "editor/misc/NativeFileBrowser.h"

#include <imgui.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>

SceneSave::SceneSave()
{
}

void SceneSave::OpenBrowser()
{
	if (auto file = ed::NativeFileBrowser::SaveFile({ "json" })) {
		file->replace_extension(".json");
		SaveAs(Engine::GetWorld(), file->string());
	}
}

namespace {
using json = nlohmann::json;
void GenerateJsonForNode(json& j, Node* node)
{
	using namespace sceneconv;

	j[nameLabel] = node->GetName();
	j[typeLabel] = FilterNodeClassName(node->GetClass().GetName());
	j[trsLabel] = { { posLabel, node->GetNodePositionLCS() }, { rotLabel, node->GetNodeEulerAnglesLCS() },
		{ scaleLabel, node->GetNodeScaleLCS() } };

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
