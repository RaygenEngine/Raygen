#include "pch.h"
#include "World.h"

#include "editor/Editor.h"
#include "engine/Events.h"
#include "engine/profiler/ProfileScope.h"
#include "reflection/ReflectionTools.h"
#include "rendering/Renderer.h"
#include "universe/NodeFactory.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "universe/nodes/light/SpotLightNode.h"
#include "universe/nodes/light/ReflectionProbeNode.h"
#include "universe/nodes/light/DirectionalLightNode.h"
#include "universe/nodes/RootNode.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "editor/imgui/ImguiImpl.h"   // WIP: ECS
#include "editor/imgui/ImEd.h"        // WIP: ECS
#include "editor/imgui/ImAssetSlot.h" // WIP: ECS

#include <nlohmann/json.hpp>
#include <engine\Input.h>

World::World(NodeFactory* factory)
	: m_nodeFactory(factory)
	, m_loadedTimepoint(FrameClock::now())
	, m_lastFrameTimepoint(FrameClock::now())

{
}

World::~World()
{
	m_root.reset();
	delete m_nodeFactory;
}

void World::PushDelayedCommand(std::function<void()>&& func)
{
	m_postIterateCommandList.emplace_back(func);
}

void World::SetActiveCamera(CameraNode* cam)
{
	m_activeCamera = cam;
	if (cam) {
		cam->EnqueueActiveCamera();
	}
}

void World::LoadAndPrepareWorld(const fs::path& scene)
{
	LOG_INFO("Loading World file: \'{}\'", scene);

	std::ifstream f(scene);

	m_loadedFrom = std::make_unique<nlohmann::json>();
	f >> *m_loadedFrom;
	m_loadedFromPath = scene;

	m_root = std::make_unique<RootNode>();

	m_nodeFactory->LoadChildren(*m_loadedFrom, m_root.get());
	m_root->m_dirty.set();

	auto mat = glm::identity<glm::mat4>();
	m_root->UpdateTransforms(mat);

	Event::OnWorldLoaded.Broadcast();

	DirtyUpdateWorld();

	m_root->m_dirty.reset();
	ClearDirtyFlags();
	LOG_INFO("World loaded succesfully");
}

void World::DirtyUpdateWorld()
{
	m_isIteratingNodeSet = true;

	// PERF: Possible to use unordered_set for dirty nodes
	for (auto* node : m_nodes) {
		if (node->m_dirty.any()) {
			node->CallDirtyUpdate();
		}
	}
	m_isIteratingNodeSet = false;
}

void World::ClearDirtyFlags()
{
	for (auto& node : m_nodes) {
		node->m_dirty.reset();
	}
}

void World::UpdateFrameTimers()
{
	auto now = FrameClock::now();
	m_deltaTimeMicros = ch::duration_cast<ch::microseconds>(now - m_lastFrameTimepoint).count();
	m_deltaTime = static_cast<float>(m_deltaTimeMicros / (1e6));
	m_lastFrameTimepoint = now;
}

void World::Z_RegisterNode(Node* node, Node* parent)
{
	CLOG_ABORT(node->m_parent, "Attempting to register a node that already has a parent.");
	if (parent == nullptr) {
		parent = GetRoot();
	}

	node->m_parent = parent;

	if (!m_isIteratingNodeSet) {
		m_nodes.insert(node);
	}
	else {
		PushDelayedCommand([&, node]() { m_nodes.insert(node); });
	}


	m_typeHashToNodes[node->GetClass().GetTypeId().hash()].insert(node);

	node->AutoUpdateTransforms();

	DirtyFlagset temp;
	temp.set();
	node->SetDirtyMultiple(temp);

	parent->m_children.emplace_back(node, [](Node* node) {
		// custom deleter to remove node from world when it is deleted
		// TODO: use node's world instead of this monster
		MainWorld->CleanupNodeReferences(node);
		delete node;
	});

	Event::OnWorldNodeAdded.Broadcast(node);


	// WIP: ECS
	// if (!m_activeCamera && node->IsA<CameraNode>()) {
	//	SetActiveCamera(NodeCast<CameraNode>(node));
	//}
}

void World::CleanupNodeReferences(Node* node)
{
	m_typeHashToNodes[node->GetClass().GetTypeId().hash()].erase(node);
	m_nodes.erase(node);

	Event::OnWorldNodeRemoved.Broadcast(node);

	// WIP: ECS
	// if (node == m_activeCamera) {
	//	SetActiveCamera(GetAnyAvailableNode<CameraNode>());
	//}
}

void World::Update()
{
	{
		PROFILE_SCOPE(World);
		UpdateFrameTimers();

		if (Editor::ShouldUpdateWorld()) {
			m_isIteratingNodeSet = true;
			// Update after input and delta calculation
			for (auto* node : m_nodes) {
				node->Update(m_deltaTime);
			}
			m_isIteratingNodeSet = false;
		}
	}

	// WIP: ECS
	ImguiImpl::NewFrame();
	Editor::Update();

	Universe::ecsWorld->UpdateWorld();


	do {
		for (auto& cmd : m_postIterateCommandList) {
			cmd();
		}
		m_postIterateCommandList.clear();

		DirtyUpdateWorld();
	} while (!m_postIterateCommandList.empty());


	Universe::ecsWorld->attachedScene->EnqueueEndFrame();
	ClearDirtyFlags();
}

namespace {
glm::mat4 CameraViewProj()
{
	const auto ar = static_cast<float>(500) / static_cast<float>(400);

	float vFov = 75.f;

	auto hFov = 2 * atan(ar * tan(vFov * 0.5f));

	auto near = 0.1f;
	auto far = 1000.f;

	const auto top = tan(vFov / 2.f) * near;
	const auto bottom = tan(-vFov / 2.f) * near;

	const auto right = tan(hFov / 2.f) * near;
	const auto left = tan(-hFov / 2.f) * near;

	auto projectionMatrix = glm::frustum(left, right, bottom, top, near, far);
	// Vulkan's inverted y
	projectionMatrix[1][1] *= -1.f;

	return projectionMatrix;
}
} // namespace
