#include "pch.h"

#include "world/World.h"
#include "system/Engine.h"
#include "world/NodeFactory.h"


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

	ParsingAux::ReadFloatsAttribute(xmlData, "background", m_background);
	ParsingAux::ReadFloatsAttribute(xmlData, "ambient", m_ambient);

	return true;
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

	// Update after input and delta calculation
	for (auto* node : m_nodes)
		node->Update(m_deltaTime);

	// Update dirty leaf node instances
	for (auto* dirtyLeafNode : m_dirtyLeafNodes)
		dirtyLeafNode->CacheWorldTransform();
}


//void World::WindowResize(int32 width, int32 height)
//{
//	for (auto* node : m_nodes)
//		node->WindowResize(width, height);
//}
