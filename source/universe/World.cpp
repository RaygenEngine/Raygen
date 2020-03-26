#include "pch.h"
#include "World.h"

#include "editor/Editor.h"
#include "engine/Events.h"
#include "engine/profiler/ProfileScope.h"
#include "reflection/ReflectionTools.h"
#include "renderer/VulkanLayer.h"
#include "universe/NodeFactory.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "universe/nodes/light/DirectionalLightNode.h"
#include "universe/nodes/light/PunctualLightNode.h"
#include "universe/nodes/light/SpotLightNode.h"
#include "universe/nodes/RootNode.h"

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

std::vector<Node*> World::GetNodesByName(const std::string& name) const
{
	std::vector<Node*> nodes;

	for (auto* node : m_nodes) {
		if (node->GetName() == name) {
			nodes.push_back(node);
		}
	}

	return nodes;
}

Node* World::GetNodeByName(const std::string& name) const
{
	for (auto* node : m_nodes) {
		if (node->GetName() == name) {
			return node;
		}
	}
	return nullptr;
}

void World::SetActiveCamera(CameraNode* cam)
{
	m_activeCamera = cam;
}

void World::LoadAndPrepareWorld(const fs::path& scene)
{
	LOG_INFO("Loading World file: \'{}\'", scene);

	std::ifstream f(scene);
	f >> m_loadedFrom;
	m_loadedFromPath = scene;

	m_root = std::make_unique<RootNode>();


	m_nodeFactory->LoadChildren(m_loadedFrom, m_root.get());
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

Node* World::DuplicateNode_Utl(Node* src, Node* newParent)
{
	if (!newParent) {
		newParent = src->GetParent();
	}

	Node* created = GetNodeFactory()->NewNodeFromType(src->GetClass().GetNameStr());

	created->m_name = src->m_name + "_Copy";

	Z_RegisterNode(created, newParent);

	created->SetNodeTransformLCS(src->GetNodeTransformLCS());

	auto result = refltools::CopyClassTo(src, created);

	CLOG_ERROR(!result.IsExactlyCorrect(), "Duplicate node did not exactly match properties!");
	return created;
}

Node* World::DeepDuplicateNode(Node* src, Node* newParent)
{
	Node* result = DuplicateNode_Utl(src, newParent);

	for (auto& child : src->GetChildren()) {
		DeepDuplicateNode(child.get(), result);
	}

	return result;
}

void World::DeleteNode(Node* src)
{
	if (!src->GetParent()) {
		return;
	}
	src->GetParent()->DeleteChild(src);
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
		Universe::MainWorld->CleanupNodeReferences(node);
		delete node;
	});

	Event::OnWorldNodeAdded.Broadcast(node);


	if (!m_activeCamera && node->IsA<CameraNode>()) {
		SetActiveCamera(NodeCast<CameraNode>(node));
	}
}

void World::CleanupNodeReferences(Node* node)
{
	m_typeHashToNodes[node->GetClass().GetTypeId().hash()].erase(node);
	m_nodes.erase(node);

	Event::OnWorldNodeRemoved.Broadcast(node);

	if (node == m_activeCamera) {
		SetActiveCamera(GetAnyAvailableNode<CameraNode>());
	}
}

void World::Update()
{
	{
		PROFILE_SCOPE(World);
		UpdateFrameTimers();

		if (Engine.ShouldUpdateWorld()) {
			m_isIteratingNodeSet = true;
			// Update after input and delta calculation
			for (auto* node : m_nodes) {
				node->Update(m_deltaTime);
			}
			m_isIteratingNodeSet = false;
		}
	}

	Editor::EditorInst->UpdateEditor();

	do {
		for (auto& cmd : m_postIterateCommandList) {
			cmd();
		}
		m_postIterateCommandList.clear();

		DirtyUpdateWorld();
	} while (!m_postIterateCommandList.empty());


	Scene->EnqueueEndFrame();
}