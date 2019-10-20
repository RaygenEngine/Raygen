#pragma once

// DOC:
// column-major ordering for matrices (may require glm::transpose in some renderers)
// clip space: negative one to one (convert when you need zero to one)
// right-handed coordinate system (mary require conversion to left when used by DirectX, Vulkan and other renderers)
// values are stored in degrees (use glm::radians to convert)

#include "asset/AssetManager.h"
#include "system/Object.h"

#include <unordered_set>

class Node;
class RootNode;
class CameraNode;
class PunctualLightNode;
class GeometryNode;
class SpotLightNode;
class DirectionalLightNode;

class World : public Object {

	friend class Editor;
	friend class NodeFactory;


	std::unique_ptr<RootNode> m_root;
	mutable std::unordered_set<Node*> m_nodes;
	mutable std::unordered_set<GeometryNode*> m_geomteryNodes;
	mutable std::unordered_set<PunctualLightNode*> m_punctualLights;
	mutable std::unordered_set<DirectionalLightNode*> m_directionalLights;
	mutable std::unordered_set<SpotLightNode*> m_spotLights;
	mutable std::unordered_set<CameraNode*> m_cameraNodes;

	NodeFactory* m_nodeFactory;

	PodHandle<JsonDocPod> m_loadedFrom;

	CameraNode* m_activeCamera{ nullptr };

	using FrameClock = std::chrono::steady_clock;
	ch::time_point<FrameClock> m_loadedTimepoint;
	ch::time_point<FrameClock> m_lastFrameTimepoint;
	long long m_deltaTimeMicros{ 0 };

	float m_deltaTime{ 0.0f };

	void UpdateFrameTimers();


	void RegisterNode(Node* node, Node* parent);

	// Removes node from any tracking sets / active cameras etc.
	void CleanupNodeReferences(Node* node);

public:
	// Returns float seconds
	float GetDeltaTime() { return m_deltaTime; }

	// Returns integer microseconds;
	long long GetIntegerDeltaTime() { return m_deltaTimeMicros; }


	[[nodiscard]] RootNode* GetRoot() const { return m_root.get(); }

	World(NodeFactory* factory);
	~World() override;

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

			if (snode) {
				return snode;
			}
		}
		// not found
		return nullptr;
	}

	template<typename NodeType>
	constexpr auto& GetNodeMap() const
	{
		if constexpr (std::is_base_of_v<PunctualLightNode, NodeType>) { // NOLINT
			return m_punctualLights;
		}
		else if constexpr (std::is_base_of_v<GeometryNode, NodeType>) { // NOLINT
			return m_geomteryNodes;
		}
		else if constexpr (std::is_base_of_v<CameraNode, NodeType>) { // NOLINT
			return m_cameraNodes;
		}
		else if constexpr (std::is_base_of_v<DirectionalLightNode, NodeType>) { // NOLINT
			return m_directionalLights;
		}
		else if constexpr (std::is_base_of_v<SpotLightNode, NodeType>) { // NOLINT
			return m_spotLights;
		}
		// general nodes
		else if constexpr (std::is_base_of_v<Node, NodeType>) { // NOLINT
			return m_nodes;
		}
	}

	std::vector<Node*> GetNodesByName(const std::string& name) const;
	Node* GetNodeByName(const std::string& name) const;

	[[nodiscard]] CameraNode* GetActiveCamera() const { return m_activeCamera; }
	void SetActiveCamera(CameraNode* cam)
	{
		if (m_cameraNodes.count(cam)) {
			m_activeCamera = cam;
		}
	}

	void Update();
	// void WindowResize(int32 width, int32 height) override;

	void LoadAndPrepareWorld(PodHandle<JsonDocPod> scenePod);

	[[nodiscard]] NodeFactory* GetNodeFactory() const { return m_nodeFactory; }

	[[nodiscard]] PodHandle<JsonDocPod> GetLoadedFromHandle() const { return m_loadedFrom; }

	void DirtyUpdateWorld();
	void ClearDirtyFlags();

private:
	// Only reflected properties get copied.
	// Transient properties do not get copied.
	// Children are ignored. Use DeepDuplicateNode to properly instanciate children.
	// Does not properly call the correct events / functions
	Node* DuplicateNode_Utl(Node* src, Node* newParent = nullptr);


public:
	// Only reflected properties get copied.
	// Transient properties do not get copied.
	// Children are iteratively duplicated and attached at their new respective parents.
	// Uses the factory and m_type of each node to generate the new ones.
	Node* DeepDuplicateNode(Node* src, Node* newParent = nullptr);


	// This will also remove the children.
	void DeleteNode(Node* src);
};
