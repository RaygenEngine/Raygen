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

void World::LoadAndPrepareWorld(PodHandle<JsonDocPod> scene)
{
	LOG_INFO("Loading World file: \'{}\'", AssetManager::GetPodUri(scene));

	AssetManager::Reload(scene);
	m_loadedFrom = scene;

	m_root = std::make_unique<RootNode>();

	try {
		m_nodeFactory->LoadChildren(scene->document, m_root.get());
	} catch (std::exception& e) {
		LOG_ABORT("Failed to load world: {}", e.what());
	}
	m_root->m_dirty.set();
	m_root->UpdateTransforms(glm::identity<glm::mat4>());
	DirtyUpdateWorld();
	LOG_INFO("World loaded succesfully");
	m_root->m_dirty.reset();
}

void World::DirtyUpdateWorld()
{
	// PERF: Possible to use unordered_set for dirty nodes
	for (auto* node : m_nodes) {
		if (node->m_dirty.any()) {
			node->CallDirtyUpdate();
		}
	}
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

	Event::OnWorldNodeAdded.Broadcast(created);
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

namespace {
template<typename T>
bool MaybeInsert(std::unordered_set<T*>& set, Node* node)
{
	if (node->IsA<T>()) {
		set.insert(static_cast<T*>(node));
		return true;
	}
	return false;
}
} // namespace

void World::RegisterNode(Node* node, Node* parent)
{
	CLOG_ABORT(node->m_parent, "Attempting to register a node that already has a parent.");


	node->m_parent = parent;
	m_nodes.insert(node);

	node->SetDirty(Node::DF::Created);

	parent->m_children.emplace_back(node, [](Node* node) {
		// custom deleter to remove node from world when it is deleted
		Engine::GetWorld()->RemoveNode(node);
		delete node;
	});

	bool isCamera = MaybeInsert(m_cameraNodes, node);
	if (isCamera) {
		if (!m_activeCamera) {
			m_activeCamera = static_cast<CameraNode*>(node);
		}
		return;
	}

	[[maybe_unused]] bool cc = MaybeInsert(m_geomteryNodes, node) || MaybeInsert(m_punctualLights, node)
							   || MaybeInsert(m_directionalLights, node) || MaybeInsert(m_spotLights, node);
}

void World::Update()
{
	UpdateFrameTimers();

	if (Engine::ShouldUpdateWorld()) {
		// Update after input and delta calculation
		for (auto* node : m_nodes) {
			node->Update(m_deltaTime);
		}
	}

	if (Engine::IsEditorActive()) {
		Engine::GetEditor()->UpdateEditor();
	}

	DirtyUpdateWorld();
}
