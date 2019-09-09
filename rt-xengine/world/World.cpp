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

bool RootNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

World::World(NodeFactory* factory)
	: Node(),
	m_deltaTime(0),
	m_worldTime(GetTimeMs()),
	m_lastTime(GetTimeMs()),
	m_nodeFactory(factory)
{
	using namespace PropertyFlags;
		
	//REFLECT_VAR(m_background, Color);
	//REFLECT_VAR(m_ambient, Color);
	REFLECT_VAR(m_deltaTime, Transient);
	REFLECT_VAR(m_worldTime, Transient);
	REFLECT_VAR(m_lastTime, Transient);
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

bool World::LoadAndPrepareWorldFromXML(XMLDoc* sceneXML)
{
	if (!sceneXML)
	{
		LOG_FATAL("Missing World scene XML data file!");
		return false;
	}


	LOG_INFO("Loading World data from XML: \'{}\'", sceneXML->GetFilePath());

	auto* rootNode = sceneXML->GetRootElement();

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

		if (GetEngine()->ShouldUpdateWorld())
		{
			// Update after input and delta calculation
			for (auto* node : m_nodes)
				node->Update(m_deltaTime);
		}
		
		if (GetEngine()->IsUsingEditor())
		{
			GetEngine()->GetEditor()->UpdateEditor();
		}

	// Update dirty leaf node instances
	for (auto* dirtyLeafNode : m_dirtyLeafNodes)
		dirtyLeafNode->CacheWorldTransform();
}


//void World::WindowResize(int32 width, int32 height)
//{
//	for (auto* node : m_nodes)
//		node->WindowResize(width, height);
//}
