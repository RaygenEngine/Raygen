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
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/Scene.h"

#include <unordered_set>
#include <entt/src/entt/entity/observer.hpp>

#include <entt/src/entt/entity/handle.hpp>
#include <entt/src/entt/entity/registry.hpp>


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

	UniquePtr<nlohmann::json> m_loadedFrom;
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


	// Push a command to be executed before dirty update.
	// Helps with adding/deleting nodes that would otherwise invalidate the iterating set.
	void PushDelayedCommand(std::function<void()>&& func);


	[[nodiscard]] CameraNode* GetActiveCamera() const { return m_activeCamera; }
	void SetActiveCamera(CameraNode* cam);

	void Update();

	void LoadAndPrepareWorld(const fs::path& scenePath);

private:
	void DirtyUpdateWorld();
	void ClearDirtyFlags();

public:
} * MainWorld{};
