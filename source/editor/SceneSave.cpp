#include "pch.h"
#include "SceneSave.h"

#include "editor/misc/NativeFileBrowser.h"
#include "reflection/ReflectionTools.h"
#include "universe/nodes/camera/EditorCameraNode.h"
#include "universe/nodes/RootNode.h"
#include "universe/Universe.h"

void SceneSave::OpenBrowser()
{
	if (auto file = ed::NativeFileBrowser::SaveFile({ "json" })) {
		file->replace_extension(".json");
		SaveAs(Universe::GetMainWorld(), file->string());
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
		// WIP: ECS
		// if (childNode->IsA<EditorCameraNode>()) {
		//	continue;
		//}
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
		LOG_ERROR("Failed to open file for world save: {}", p);
		return;
	}

	json scene = json::array();

	for (auto& child : world->GetRoot()->GetChildren()) {
		// WIP: ECS
		// if (child->IsA<EditorCameraNode>()) {
		//	continue;
		//}
		json jsonChild = json::object();
		GenerateJsonForNode(jsonChild, child.get());
		scene.emplace_back(jsonChild);
	}

	ofile << std::setw(4) << scene;
	LOG_REPORT("Written scene at: {}", p);
}
