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


#include "universe/Entity.h"


class ComponentClassRegistry {
	using hasht = entt::id_type;

	std::unordered_map<hasht, const ReflClass*> m_types;

	std::vector<std::function<void(entt::registry&)>> m_hookSceneFunctions;


	template<typename Component>
	static void InitScene(entt::registry& reg, entt::entity ent)
	{
		reg.get<Component>(ent).Z_RegisterToScene<typename Component::RenderSceneType>();
		reg.emplace<typename Component::Dirty>(ent);
	}

	template<typename Component>
	static void DestoryScene(entt::registry& reg, entt::entity ent)
	{
		reg.get<Component>(ent).Z_DeregisterFromScene<typename Component::RenderSceneType>();
	}


public:
	template<typename Component>
	void Register()
	{
		const ReflClass* cl = &Component::StaticClass();

		m_types[entt::type_info<Component>::id()] = cl;

		if constexpr (std::is_base_of_v<SceneCompBase, Component>) {
			m_hookSceneFunctions.emplace_back([](entt::registry& reg) {
				reg.on_construct<Component>().template connect<&ComponentClassRegistry::InitScene<Component>>();
				reg.on_destroy<Component>().template connect<&ComponentClassRegistry::DestoryScene<Component>>();
			});
		}
	}


	bool HasClass(entt::id_type type) const { return m_types.contains(type); }

	const ReflClass* GetClass(entt::id_type type)
	{
		CLOG_ERROR(!m_types.contains(type), "Asking for component class of an unregistered component.");
		return m_types[type];
	}


	void HookRegistry(entt::registry& reg)
	{
		for (auto& func : m_hookSceneFunctions) {
			func(reg);
		}
	}

	static ComponentClassRegistry& Get()
	{
		static ComponentClassRegistry instance;
		return instance;
	}

	template<typename T>
	static void RegisterClass()
	{
		Get().Register<T>();
	}
};

template<typename K>
struct ComponentReflectionRegistar {
	ComponentReflectionRegistar() { ComponentClassRegistry::RegisterClass<K>(); }
};


struct SceneCompBase {
	size_t sceneUid;


	template<typename T>
	void Z_RegisterToScene()
	{
		sceneUid = Scene->EnqueueCreateCmd<T>();
		LOG_REPORT("Registered");
	}

	template<typename T>
	void Z_DeregisterFromScene()
	{
		Scene->EnqueueDestroyCmd<T>(sceneUid);
		LOG_REPORT("De registered");
	}
};

struct StaticMeshComp : SceneCompBase {
	// DECL_DIRTY(MeshChange);

	REFLECTED_SCENE_COMP(StaticMeshComp, SceneGeometry)
	{
		//
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(mesh);
	}


	PodHandle<Mesh> mesh;
};

struct ScriptComp {

	REFLECTED_COMP(ScriptComp)
	{
		//
		REFLECT_VAR(code);
	}

	std::string code;
};

struct FreeformMovementComp {
	float movespeed{ 15.f };
};


class ECS_World {
public:
	entt::registry reg;
	UniquePtr<entt::observer> obs;
	ComponentClassRegistry& classRegsitry = ComponentClassRegistry::Get();
	ECS_World() { classRegsitry.HookRegistry(reg); }

	Entity CreateEntity(const std::string& name = "")
	{
		Entity ent{ reg.create(), &reg };
		auto& basic = ent.Add<BasicComponent>(name);
		basic.self = ent;
		return ent;
	}

	void DestroyEntity(Entity entity) { reg.destroy(entity.m_entity); }

	void CreateWorld();

	void UpdateWorld();
};


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
