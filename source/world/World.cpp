#include "pch.h"

#include "world/World.h"
#include "system/Engine.h"
#include "world/NodeFactory.h"
#include "editor/Editor.h"


// TODO use integers (micro or milliseconds), also tidy
using Clock = std::chrono::steady_clock;

extern auto t_start = Clock::now();

extern float GetTimeMs()
{
	return static_cast<float>((Clock::now() - t_start).count() / 1000000.f);
}

World::World(NodeFactory* factory)
	: m_deltaTime(0),
	m_worldTime(GetTimeMs()),
	m_lastTime(GetTimeMs()),
	m_nodeFactory(factory)
{
}

World::~World()
{
	delete m_nodeFactory;
}

void World::AddDirtyNode(Node* node)
{
	m_dirtyNodes.insert(node);
	if (node->IsLeaf())
		m_dirtyLeafNodes.insert(node);
}

std::vector<Node*> World::GetNodesByName(const std::string& name) const
{
	std::vector<Node*> nodes;

	for (auto* node : m_nodes)
		if (node->GetName() == name)
			nodes.push_back(node);

	return nodes;
}

Node* World::GetNodeByName(const std::string& name) const
{
	for (auto* node : m_nodes)
		if (node->GetName() == name)
			return node;
	return nullptr;
}

std::vector<Node*> World::GetNodesById(uint32 id) const
{
	std::vector<Node*> nodes;

	for (auto* node : m_nodes)
		if (node->GetUID() == id)
			nodes.push_back(node);

	return nodes;
}

Node* World::GetNodeById(uint32 id) const
{
	for (auto* node : m_nodes)
		if (node->GetUID() == id)
			return node;
	return nullptr;
}

bool World::LoadAndPrepareWorldFromXML(PodHandle<XMLDocPod> sceneXML)
{
	LOG_INFO("Loading World from XML: \'{}\'", Engine::GetAssetManager()->GetPodPath(sceneXML));

	auto* rootNode = sceneXML->document.RootElement();

	m_root = std::make_unique<RootNode>();
	
	if (!m_root->LoadFromXML(rootNode))
	{
		LOG_FATAL("Incorrect world format!");
		return false;
	}

	// mark dirty (everything) to cache initial instance data 
	// world will keep dirty leafs too for optimization
	m_root->MarkDirty();

	// Cache transform data bottom up from leaf nodes
	for (auto* dirtyLeafNode : this->m_dirtyLeafNodes)
	{
		dirtyLeafNode->CacheWorldTransform();
	}

	LOG_INFO("World loaded succesfully");
	return true;
}

Node* World::DuplicateNode(Node* src, Node* newParent)
{
	if (!newParent)
	{
		newParent = src->GetParent();
	}
	return nullptr;
/*	const ReflClass& srcClass = GetReflector(src);

	Node* created = GetNodeFactory()->LoadNodeFromType(src->m_type, newParent);

	created->m_type = src->m_type;
	created->m_name = src->m_name + "_Copy";

	created->m_localTranslation = src->m_localTranslation;
	created->m_localOrientation = src->m_localOrientation;
	created->m_localScale = src->m_localScale;
	created->MarkDirty();

	auto result = CopyReflectorInto(srcReflector, GetReflector(created));
	CLOG_FATAL(!result.IsExactlyCorrect(), "Duplicate node did not exactly match properties! Node: {}", src);

	GetReflector(created).SetName(created->m_type + "." + created->m_name + "." + std::to_string(created->GetUID()));

	return created;*/
}

Node* World::DeepDuplicateNode(Node* src, Node* newParent)
{
	Node* result = DuplicateNode(src, newParent);
	for (auto& child : src->GetChildren())
	{
		DeepDuplicateNode(child.get(), result);
	}
	result->PostChildrenLoaded();
	Event::OnWorldNodeAdded.Broadcast(result);
	return result;
}

void World::DeleteNode(Node* src)
{
	if (!src->GetParent())
	{
		return;
	}
	src->GetParent()->DeleteChild(src);
}

void World::Update()
{
	// clear prev frames nodes
	m_dirtyNodes.clear();
	m_dirtyLeafNodes.clear();

	const auto timestamp = GetTimeMs();

	// calculate delta
	const auto timeStep = timestamp - m_lastTime;
	m_deltaTime = std::fminf(timeStep, 16.f);
	m_worldTime += m_deltaTime;
	m_lastTime = timestamp;

	if (Engine::Get().ShouldUpdateWorld())
	{
		// Update after input and delta calculation
		for (auto* node : m_nodes)
			node->Update(m_deltaTime);
	}
		
	if (Engine::Get().IsUsingEditor())
	{
		Engine::GetEditor()->UpdateEditor();
	}

	// Update dirty leaf node instances
	for (auto* dirtyLeafNode : m_dirtyLeafNodes)
		dirtyLeafNode->CacheWorldTransform();
}
