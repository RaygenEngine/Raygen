#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "world/nodes/TransformNode.h"
#include "world/nodes/light/PunctualLightNode.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/user/UserNode.h"
#include "asset/loaders/XMLDocLoader.h"
#include "system/EngineComponent.h"
#include "asset/AssetManager.h"
#include "system/Input.h"
#include "world/nodes/geometry/InstancedGeometryNode.h"
#include "world/nodes/light/DirectionalLightNode.h"
#include "world/nodes/light/SpotLightNode.h"


class CameraNode;

// TODO:
class RootNode : public Node {
	REFLECTED_NODE(RootNode, Node)
	{
		REFLECT_VAR(m_background, PropertyFlags::Color);
		REFLECT_VAR(m_ambient, PropertyFlags::Color);
	}

	glm::vec3 m_background{};
	glm::vec3 m_ambient{};

public:
	RootNode()
		: Node(nullptr)
	{
	}

	[[nodiscard]] glm::vec3 GetBackgroundColor() const { return m_background; }
	[[nodiscard]] glm::vec3 GetAmbientColor() const { return m_ambient; }

	void SetBackgroundColor(const glm::vec3& color) { m_background = color; }
	void SetAmbientColor(const glm::vec3& color) { m_ambient = color; }

	~RootNode() { m_children.clear(); }
	RootNode(const RootNode&) = default;
	RootNode(RootNode&&) = default;
	RootNode& operator=(const RootNode&) = default;
	RootNode& operator=(RootNode&&) = default;
};

class World {
	mutable std::unordered_set<Node*> m_nodes;
	mutable std::unordered_set<GeometryNode*> m_triangleModelGeometries;
	mutable std::unordered_set<TransformNode*> m_transforms;
	mutable std::unordered_set<PunctualLightNode*> m_punctualLights;
	mutable std::unordered_set<DirectionalLightNode*> m_directionalLights;
	mutable std::unordered_set<SpotLightNode*> m_spotLights;
	mutable std::unordered_set<CameraNode*> m_cameras;
	mutable std::unordered_set<UserNode*> m_users;

	NodeFactory* m_nodeFactory;
	std::unique_ptr<RootNode> m_root;
	PodHandle<XMLDocPod> m_loadedFrom;

	// TODO:
	CameraNode* m_activeCamera{ nullptr };

	using FrameClock = std::chrono::steady_clock;
	ch::time_point<FrameClock> m_loadedTimepoint;
	ch::time_point<FrameClock> m_lastFrameTimepoint;
	long long m_deltaTimeMicros;

	float m_deltaTime{ 0.0f };

	void UpdateFrameTimers();

	friend class Editor;

public:
	// Returns float seconds
	float GetDeltaTime() { return m_deltaTime; }

	// Returns integer microseconds;
	long long GetIntegerDeltaTime() { return m_deltaTimeMicros; }


	[[nodiscard]] RootNode* GetRoot() const { return m_root.get(); }

	World(NodeFactory* factory);
	~World();


	template<typename NodeType>
	void AddNode(NodeType* node)
	{
		// WIP:
		if constexpr (std::is_base_of_v<CameraNode, NodeType>) { // NOLINT
			if (!m_activeCamera) {
				m_activeCamera = node;
			}
		}
		GetNodeMap<NodeType>().insert(node);
		m_nodes.insert(node);
		node->m_dirty.set(Node::DF::Created);
	}

	template<typename NodeType>
	void RemoveNode(NodeType* node)
	{
		Event::OnWorldNodeRemoved.Broadcast(node);
		GetNodeMap<NodeType>().erase(node);
		m_nodes.erase(node);
	}

	// available node may differ later in runtime
	template<typename NodeType>
	NodeType* GetAnyAvailableNode() const
	{
		for (auto& node : m_nodes) {
			auto ptr = dynamic_cast<NodeType*>(node);
			if (ptr) {
				return ptr;
			}
		}
		return nullptr;
	}

	// available node may differ later in runtime
	// template <typename NodeType>
	// NodeType* GetFirstAvailableNode() const
	//{
	//
	//	return *GetNodeMap<NodeType>().begin();
	//} TODO: Use in observer renderer

	// available node may differ later in runtime
	template<typename NodeSubType>
	NodeSubType* GetAvailableNodeSpecificSubType() const
	{
		for (auto node : GetNodeMap<NodeSubType>()) {
			auto* snode = dynamic_cast<NodeSubType*>(node);

			if (snode)
				return snode;
		}
		// not found
		return nullptr;
	}

	template<typename NodeType>
	constexpr auto& GetNodeMap() const
	{
		if constexpr (std::is_base_of<PunctualLightNode, NodeType>::value) { // NOLINT
			return m_punctualLights;
		}
		else if constexpr (std::is_base_of<GeometryNode, NodeType>::value) { // NOLINT
			return m_triangleModelGeometries;
		}
		else if constexpr (std::is_base_of<TransformNode, NodeType>::value) { // NOLINT
			return m_transforms;
		}
		else if constexpr (std::is_base_of<CameraNode, NodeType>::value) { // NOLINT
			return m_cameras;
		}
		else if constexpr (std::is_base_of<UserNode, NodeType>::value) { // NOLINT
			return m_users;
		}
		else if constexpr (std::is_base_of<DirectionalLightNode, NodeType>::value) { // NOLINT
			return m_directionalLights;
		}
		else if constexpr (std::is_base_of<SpotLightNode, NodeType>::value) { // NOLINT
			return m_spotLights;
		}

		// general nodes
		else if constexpr (std::is_base_of<Node, NodeType>::value) { // NOLINT
			return m_nodes;
		}
	}

	std::vector<Node*> GetNodesByName(const std::string& name) const;
	Node* GetNodeByName(const std::string& name) const;

	CameraNode* GetActiveCamera() const { return m_activeCamera; }

	template<typename ChildType>
	ChildType* CreateNode(Node* parent)
	{
		std::shared_ptr<ChildType> node = std::shared_ptr<ChildType>(new ChildType(parent), [&](ChildType* assetPtr) {
			// custom deleter to remove node from world when it is deleted
			this->RemoveNode(assetPtr);
			delete assetPtr;
		}); //

		// add it to world maps
		this->AddNode(node.get());
		parent->AddChild(node);
		// return observer
		return node.get();
	}

	void Update();
	// void WindowResize(int32 width, int32 height) override;

	bool LoadAndPrepareWorldFromXML(PodHandle<XMLDocPod> sceneXML);

	NodeFactory* GetNodeFactory() const { return m_nodeFactory; }

	PodHandle<XMLDocPod> GetLoadedFromHandle() { return m_loadedFrom; }

	void DirtyUpdateWorld();
	void ClearDirtyFlags();

private:
	// Only reflected properties get copied.
	// Transient properties do not get copied.
	// Children are ignored. Use DeepDuplicateNode to properly instanciate children.
	// Does not properly call the correct events / functions
	Node* DuplicateNode(Node* src, Node* newParent = nullptr);


public:
	// Only reflected properties get copied.
	// Transient properties do not get copied.
	// Children are iteratively duplicated and attached at their new respective parents.
	// Uses the factory and m_type of each node to generate the new ones.
	Node* DeepDuplicateNode(Node* src, Node* newParent = nullptr);


	// This will also remove the children.
	void DeleteNode(Node* src);
};
