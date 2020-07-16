#pragma once
// DOC:
// column-major ordering for matrices (may require glm::transpose in some renderers)
// clip space: negative one to one (convert when you need zero to one)
// right-handed coordinate system (mary require conversion to left when used by DirectX, Vulkan and other renderers)
// values are stored in rads (use (constexpr) glm::radians to convert)

#include "engine/Listener.h"
#include "engine/Timer.h"
#include "reflection/ReflClass.h"
#include "universe/NodeFactory.h" // Not required directly, used in templates
#include "universe/nodes/NodeIterator.h"

#include <nlohmann/json.hpp>
#include <unordered_set>

class Node;
class RootNode;
class CameraNode;
class PunctualLightNode;
class GeometryNode;
class SpotlightNode;
class DirectionalLightNode;
class ReflectionProbeNode;

inline class World {

	friend class EditorObject_;
	friend class NodeFactory;

	template<typename T>
	friend struct NodeIterator;

	UniquePtr<RootNode> m_root;
	std::unordered_set<Node*> m_nodes;

	std::unordered_map<size_t, std::unordered_set<Node*>> m_typeHashToNodes;

	NodeFactory* m_nodeFactory;

	nlohmann::json m_loadedFrom;
	fs::path m_loadedFromPath;

	CameraNode* m_activeCamera{ nullptr };

	using FrameClock = std::chrono::steady_clock;
	ch::time_point<FrameClock> m_loadedTimepoint;
	ch::time_point<FrameClock> m_lastFrameTimepoint;
	long long m_deltaTimeMicros{ 0 };

	bool m_isIteratingNodeSet{ false };

	std::vector<std::function<void()>> m_postIterateCommandList;

	float m_deltaTime{ 0.0f };

	void UpdateFrameTimers();


	// Removes node from any tracking sets / active cameras etc.
	void CleanupNodeReferences(Node* node);


public:
	// Parent == nullptr means root
	void Z_RegisterNode(Node* node, Node* parent = nullptr);


	// Returns float seconds
	float GetDeltaTime() { return m_deltaTime; }

	// Returns integer microseconds;
	long long GetIntegerDeltaTime() { return m_deltaTimeMicros; }


	[[nodiscard]] RootNode* GetRoot() const { return m_root.get(); }

	World(NodeFactory* factory);
	~World();

	// available node may differ later in runtime.
	// returns nodes of subclasses ONLY IF no exact type was found.
	// returns nullptr if no node of this class or subclasses where found
	template<typename NodeType>
	NodeType* GetAnyAvailableNode() const
	{
		Node* node = GetAnyAvailableNodeFromClass(&NodeType::StaticClass());
		if (!node) {
			return nullptr;
		}
		return NodeCast<NodeType>(node);
	}

	[[nodiscard]] const std::unordered_set<Node*>& GetNodes() const { return m_nodes; }


	// Push a command to be executed before dirty update.
	// Helps with adding/deleting nodes that would otherwise invalidate the iterating set.
	void PushDelayedCommand(std::function<void()>&& func);

	// Returns an iterable that can be used to access every node of this type or subtypes.
	// Best used in a for each loop:
	//
	// for(CameraNode* camera : world->GetNodeIterator<CameraNode>()) {
	//   ... // Runs for every CameraNode in the world
	// }
	//
	template<typename T>
	NodeIterable<T> GetNodeIterator()
	{
		return NodeIterable<T>(m_typeHashToNodes);
	}

	std::vector<Node*> GetNodesByName(const std::string& name) const;
	Node* GetNodeByName(const std::string& name) const;

	[[nodiscard]] CameraNode* GetActiveCamera() const { return m_activeCamera; }
	void SetActiveCamera(CameraNode* cam);

	void Update();

	void LoadAndPrepareWorld(const fs::path& scenePath);

	[[nodiscard]] NodeFactory* GetNodeFactory() const { return m_nodeFactory; }

	[[nodiscard]] nlohmann::json& GetLoadedFromJson() { return m_loadedFrom; }
	[[nodiscard]] fs::path& GetLoadedFromPath() { return m_loadedFromPath; }

private:
	void DirtyUpdateWorld();
	void ClearDirtyFlags();

public:
	// Default nullptr parent means this will be registered to root.
	// DOC: No documentation as to when it is safe to call this. Usually it is not.
	template<typename T>
	T* CreateNode(const std::string& name, Node* parent = nullptr)
	{
		auto node = m_nodeFactory->NewNode<T>();
		node->SetName(name);
		Z_RegisterNode(node, parent);
		return node;
	}

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
	// Uses the factory for each node to generate the new ones.
	// Unsafe to call during update / dirtyupdates, invalidates iterators
	Node* DeepDuplicateNode(Node* src, Node* newParent = nullptr);


	// TODO: make async safe to be called from updates / dirtyupdates
	// This will also remove the children.
	// Do not delete nodes in any Node::Update() or Node::DirtyUpdate() this will invalidate the loop iterators
	void DeleteNode(Node* src);

private:
	// Will give priority to exact class first, then if not found may return any subclass in no particular order.
	Node* GetAnyAvailableNodeFromClass(const ReflClass* cl) const
	{
		// Check for exact
		if (auto it = m_typeHashToNodes.find(cl->GetTypeId().hash());
			it != end(m_typeHashToNodes) && it->second.size() > 0) {
			return *it->second.begin();
		}

		// Exact not found, check for subclasses

		for (auto& childClass : cl->GetChildClasses()) {
			Node* node = GetAnyAvailableNodeFromClass(childClass);
			if (node) {
				return node;
			}
		}
		return nullptr;
	}
} * MainWorld{};
