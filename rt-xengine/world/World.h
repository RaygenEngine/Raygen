#pragma once

#include "world/nodes/Node.h"
#include "nodes/geometry/TriangleModelGeometryNode.h"
#include "nodes/geometry/TriangleModelInstancedGeometryNode.h"
#include "nodes/TransformNode.h"
#include "nodes/light/LightNode.h"
#include "nodes/camera/CameraNode.h"
#include "nodes/user/UserNode.h"
#include "assets/other/xml/XMLDoc.h"
#include "system/EngineComponent.h"
#include "system/Input.h"

class NodeFactory;

// TODO:
class RootNode : public Node
{
public:
	RootNode()
		: Node(nullptr)
	{}

	glm::vec3 m_background;
	glm::vec3 m_ambient;

	glm::vec3 GetBackgroundColor() const { return m_background; }
	void SetBackgroundColor(const glm::vec3& color) { m_background = color; }

	glm::vec3 GetAmbientColor() const { return m_ambient; }
	void SetAmbientColor(const glm::vec3& color) { m_ambient = color; }

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
	
	~RootNode() {
		m_children.clear();
	}
};

class World
{
	mutable std::unordered_set<Node*> m_nodes;
	mutable std::unordered_set<TriangleModelGeometryNode*> m_triangleModelGeometries;
	mutable std::unordered_set<TriangleModelInstancedGeometryNode*> m_triangleModelInstancedGeometries;
	mutable std::unordered_set<TransformNode*> m_transforms;
	mutable std::unordered_set<LightNode*> m_lights;
	mutable std::unordered_set<CameraNode*> m_cameras;
	mutable std::unordered_set<UserNode*> m_users;
		
	// dirty nodes
	std::unordered_set<Node*> m_dirtyNodes; 
	// used for optimizations
	std::unordered_set<Node*> m_dirtyLeafNodes; 
		
	// world time
	float m_deltaTime;

	// world time begins with applications starting time, use with timestamps
	float m_worldTime;
	float m_lastTime;

	NodeFactory* m_nodeFactory;

	std::unique_ptr<RootNode> m_root;
public:
	[[nodiscard]] RootNode* GetRoot() const { return m_root.get();  }

	World(NodeFactory* factory);
	~World();


	template <typename NodeType>
	void AddNode(NodeType* node)
	{
		GetNodeMap<NodeType>().insert(node);
		m_nodes.insert(node);
	}

	template <typename NodeType>
	void RemoveNode(NodeType* node)
	{
		GetNodeMap<NodeType>().erase(node);
		m_nodes.erase(node);
	}

	// available node may differ later in runtime
	template <typename NodeType>
	NodeType* GetAnyAvailableNode() const
	{
		return *GetNodeMap<NodeType>().begin();
	}

	// available node may differ later in runtime
	template <typename NodeSubType>
	NodeSubType* GetAvailableNodeSpecificSubType() const
	{
		for(auto node : GetNodeMap<NodeSubType>())
		{
			auto* snode = dynamic_cast<NodeSubType*>(node);

			if (snode)
				return snode;
		}
		// not found
		return nullptr;
	}

	template <typename NodeType>
	constexpr auto& GetNodeMap() const
	{
		if constexpr (std::is_base_of<LightNode, NodeType>::value) { return m_lights; }
		else if constexpr (std::is_base_of<TriangleModelInstancedGeometryNode, NodeType>::value) { return m_triangleModelInstancedGeometries; }
		else if constexpr (std::is_base_of<TriangleModelGeometryNode, NodeType>::value) { return m_triangleModelGeometries; }
		else if constexpr (std::is_base_of<TransformNode, NodeType>::value) { return m_transforms; }
		else if constexpr (std::is_base_of<CameraNode, NodeType>::value) { return m_cameras; }
		else if constexpr (std::is_base_of<UserNode, NodeType>::value) { return m_users; }


		// general nodes
		else if constexpr (std::is_base_of<Node, NodeType>::value) { return m_nodes; }

		static_assert("Incorrect types!");
	}

	void AddDirtyNode(Node* node);

	const std::unordered_set<Node*>& GetDirtyNodes() const { return m_dirtyNodes; }
	const std::unordered_set<Node*>& GetDirtyLeafNodes() const { return m_dirtyLeafNodes; }

	std::vector<Node*> GetNodesByName(const std::string& name) const;
	Node* GetNodeByName(const std::string& name) const;

	std::vector<Node*> GetNodesById(uint32 id) const;
	Node* GetNodeById(uint32 id) const;

	float GetDeltaTime() const { return m_deltaTime; }
	float GetWorldTime() const { return m_worldTime; }

	// SetIdentificationFromAssociatedDiskAssetIdentification node to world and as child, and return observer (maybe required for special inter-node handling)
	template <typename ChildType>
	ChildType* LoadNode(Node* parent, const tinyxml2::XMLElement* xmlData)
	{
		std::shared_ptr<ChildType> node = std::shared_ptr<ChildType>(new ChildType(parent), [&](ChildType* assetPtr)
		{
			// custom deleter to remove node from world when it is deleted
			this->RemoveNode(assetPtr);
			delete assetPtr;
		}); //

		// load it
		if (!node->LoadFromXML(xmlData))
		{
			LOG_WARN("Failed to load new node: {0}", node->GetName());
			return nullptr;
		}

		// add it to world maps
		this->AddNode(node.get());

		parent->AddChild(node);

		// return observer
		return node.get();
	}

	void Update();
	//void WindowResize(int32 width, int32 height) override;

	bool LoadAndPrepareWorldFromXML(XMLDoc* sceneXML);

	NodeFactory* GetNodeFactory() const { return m_nodeFactory; }
};
