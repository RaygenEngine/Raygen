#include "pch/pch.h"

#include "world/World.h"
#include "system/Engine.h"
#include "world/NodeFactory.h"
#include "editor/Editor.h"
#include "reflection/ReflectionTools.h"
#include "reflection/GetClass.h"

World::World(NodeFactory* factory)
	: m_nodeFactory(factory)
	, m_loadedTimepoint(FrameClock::now())
	, m_lastFrameTimepoint(FrameClock::now())

{
}

World::~World()
{
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

bool World::LoadAndPrepareWorldFromXML(PodHandle<XMLDocPod> sceneXML)
{
	LOG_INFO("Loading World from XML: \'{}\'", AssetManager::GetPodUri(sceneXML));
	m_loadedFrom = sceneXML;

	auto* rootNode = sceneXML->document.RootElement();


	m_root = std::make_unique<RootNode>();

	if (!m_root->LoadFromXML(rootNode)) {
		LOG_FATAL("Incorrect world format!");
		return false;
	}
	DirtyUpdateWorld();
	LOG_INFO("World loaded succesfully");
	m_root->m_dirty.reset();
	return true;
}

void World::DirtyUpdateWorld()
{
	m_root->UpdateTransforms(glm::identity<glm::mat4>());

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

Node* World::DuplicateNode(Node* src, Node* newParent)
{
	if (!newParent) {
		newParent = src->GetParent();
	}

	Node* created = GetNodeFactory()->LoadNodeFromType(src->m_type, newParent);

	created->m_type = src->m_type;
	created->m_name = src->m_name + "_Copy";

	created->m_localTranslation = src->m_localTranslation;
	created->m_localOrientation = src->m_localOrientation;
	created->m_localScale = src->m_localScale;
	created->MarkMatrixChanged();

	auto result = refltools::CopyClassTo(src, created);
	CLOG_FATAL(!result.IsExactlyCorrect(), "Duplicate node did not exactly match properties!");
	return created;
}

Node* World::DeepDuplicateNode(Node* src, Node* newParent)
{
	Node* result = DuplicateNode(src, newParent);
	for (auto& child : src->GetChildren()) {
		DeepDuplicateNode(child.get(), result);
	}
	auto todo_removethisvar = result->PostChildrenLoaded();
	Event::OnWorldNodeAdded.Broadcast(result);
	return result;
}

void World::DeleteNode(Node* src)
{
	if (!src->GetParent()) {
		return;
	}
	src->GetParent()->DeleteChild(src);
}

void World::UpdateFrameTimers()
{
	auto now = FrameClock::now();
	m_deltaTimeMicros = ch::duration_cast<ch::microseconds>(now - m_lastFrameTimepoint).count();
	m_deltaTime = static_cast<float>(m_deltaTimeMicros / (1e6));
	m_lastFrameTimepoint = now;
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
