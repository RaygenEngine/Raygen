#include "pch.h"
#include "World.h"

#include "system/Engine.h"

namespace World
{
	// TODO use integers (micro or milliseconds), also tidy
	using Clock = std::chrono::steady_clock;

	extern auto t_start = Clock::now();

	extern float GetTimeMs()
	{
		return static_cast<float>((Clock::now() - t_start).count() / 1000000.f);
	}


	World::World(System::Engine* engine)
		: Node(engine),
		m_background(0.f, 0.f, 0.4f),
	    m_ambient(0.4f, 0.4f, 0.4f),
		m_deltaTime(0),
		m_worldTime(GetTimeMs()),
		m_lastTime(GetTimeMs())
	{
		RT_XENGINE_LOG_INFO("Created World context, id: {}", EngineObject::GetObjectId());
	}

	World::~World()
	{
		RT_XENGINE_LOG_INFO("Destroyed World context, id: {}", EngineObject::GetObjectId());
		// clear children before destruction - otherwise they would be cleared at the base node class causing issues with world's node maps
		m_children.clear();
	}

	std::string World::PrintWorldTree(bool verbose)
	{
		return ToString(verbose, 0);
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
			if (node->GetObjectId() == id)
				nodes.push_back(node);

		return nodes;
	}

	Node* World::GetNodeById(uint32 id) const
	{
		for (auto* node : m_nodes)
			if (node->GetObjectId() == id)
				return node;
		return nullptr;
	}

	bool World::LoadAndPrepareWorldFromXML(Assets::XMLDoc* sceneXML)
	{
		if (sceneXML)
		{
			RT_XENGINE_LOG_INFO("Loading World data from XML: \'{}\'", sceneXML->GetPath());

			auto* worldElement = sceneXML->GetRootElement();

			if (this->LoadFromXML(worldElement))
			{
				// mark dirty (everything) to cache initial instance data 
				// world will keep dirty leafs too for optimization
				this->MarkDirty();

				// Cache transform data bottom up from leaf nodes
				for (auto* dirtyLeafNode : this->m_dirtyLeafNodes)
					dirtyLeafNode->CacheWorldTransform();


				RT_XENGINE_LOG_INFO("World loaded succesfully, id: {}", this->GetObjectId());

				RT_XENGINE_LOG_ERROR("Scenegraph: \n\n{0}", this->PrintWorldTree(true));

				return true;
			}

			RT_XENGINE_LOG_FATAL("Incorrect world format!");

			return false;
		}

		RT_XENGINE_LOG_FATAL("Missing World scene XML data file!");

		return false;
	}

	bool World::LoadAttributesFromXML(const tinyxml2::XMLElement * xmlData)
	{
		Node::LoadAttributesFromXML(xmlData);

		std::string assetPath{};
		Assets::ReadStringAttribute(xmlData, "asset_path", assetPath);
		this->SetAssetLoadPathHint("scenes\\" + assetPath);

		Assets::ReadFloatsAttribute(xmlData, "background", m_background);
		Assets::ReadFloatsAttribute(xmlData, "ambient", m_ambient);

		return true;
	}

	std::string World::ToString(bool verbose, uint depth) const
	{
		return std::string("    ") * depth + "|--root " + Node::ToString(verbose, depth);
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
		for (auto* nodes : m_nodes)
			nodes->Update();
		
		// Update dirty leaf node instances
		for (auto* dirtyLeafNode : m_dirtyLeafNodes)
			dirtyLeafNode->CacheWorldTransform();
	}

	void World::WindowResize(int32 width, int32 height)
	{
		for (auto* nodes : m_nodes)
			nodes->WindowResize(width, height);
	}
}
