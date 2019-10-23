#include "pch/pch.h"

#include "world/World.h"
#include "world/NodeFactory.h"
#include "world/nodes/RootNode.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/light/DirectionalLightNode.h"
#include "world/nodes/light/PunctualLightNode.h"
#include "world/nodes/light/SpotLightNode.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "editor/Editor.h"
#include "reflection/ReflectionTools.h"
#include "system/EngineEvents.h"

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

void World::LoadAndPrepareWorld(PodHandle<JsonDocPod> scene)
{
	LOG_INFO("Loading World file: \'{}\'", AssetManager::GetPodUri(scene));

	AssetManager::Reload(scene);
	m_loadedFrom = scene;

	m_root = std::make_unique<RootNode>();

	// try {
	m_nodeFactory->LoadChildren(scene.Lock()->document, m_root.get());
	//} //catch (std::exception& e) {
	// LOG_ABORT("Failed to load world: Exception encountered: {}", e.what());
	//}
	m_root->m_dirty.set();
	m_root->UpdateTransforms(glm::identity<glm::mat4>());

	Event::OnWorldLoaded.Broadcast();

	DirtyUpdateWorld();
	LOG_INFO("World loaded succesfully");
	m_root->m_dirty.reset();
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

	RegisterNode(created, newParent);

	created->SetLocalMatrix(src->GetLocalMatrix());

	auto result = refltools::CopyClassTo(src, created);

	CLOG_FATAL(!result.IsExactlyCorrect(), "Duplicate node did not exactly match properties!");
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


void World::RegisterNode(Node* node, Node* parent)
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
		Engine::GetWorld()->CleanupNodeReferences(node);
		delete node;
	});

	Event::OnWorldNodeAdded.Broadcast(node);


	if (!m_activeCamera && node->IsA<CameraNode>()) {
		SetActiveCamera(NodeCast<CameraNode>(node));
	}
}

void World::CleanupNodeReferences(Node* node)
{
	Event::OnWorldNodeRemoved.Broadcast(node);

	m_typeHashToNodes[node->GetClass().GetTypeId().hash()].erase(node);
	m_nodes.erase(node);

	if (node == m_activeCamera) {
		SetActiveCamera(GetAnyAvailableNode<CameraNode>());
	}
}


void World::Update()
{
	UpdateFrameTimers();

	if (Engine::ShouldUpdateWorld()) {
		m_isIteratingNodeSet = true;
		// Update after input and delta calculation
		for (auto* node : m_nodes) {
			node->Update(m_deltaTime);
		}
		m_isIteratingNodeSet = false;
	}

	if (Engine::IsEditorActive()) {
		Engine::GetEditor()->UpdateEditor();
	}


	do {
		for (auto& cmd : m_postIterateCommandList) {
			cmd();
		}
		m_postIterateCommandList.clear();

		DirtyUpdateWorld();
	} while (!m_postIterateCommandList.empty());
}
