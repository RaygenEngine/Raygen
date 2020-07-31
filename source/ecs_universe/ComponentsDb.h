#pragma once

#include "ecs_universe/Entity.h"

struct SceneCompBase;

namespace detail {
template<typename T, typename = void>
struct HasDirtySubstruct : std::false_type {
};

template<typename T>
struct HasDirtySubstruct<T, std::enable_if_t<std::is_empty_v<typename T::Dirty>>> : std::true_type {
};

template<typename T>
constexpr bool HasDirtySubstructV = HasDirtySubstruct<T>::value;

} // namespace detail

class ComponentsDb {
	// Singleton class that gets popualated by auto registration of reflected components. (Even before main runs)
	ComponentsDb() = default;

	// Private getter, interface only through static functions (for consistency)
	static ComponentsDb& Get()
	{
		static ComponentsDb instance;
		return instance;
	}

public:
	ComponentsDb(const ComponentsDb&) = delete;
	ComponentsDb(ComponentsDb&&) = delete;
	ComponentsDb& operator=(const ComponentsDb&) = delete;
	ComponentsDb& operator=(ComponentsDb&&) = delete;

private:
	// Maps entt types to ReflClass types.
	std::unordered_map<entt::id_type, const ReflClass*> m_types;

	// WIP: without std function is possible
	std::vector<std::function<void(entt::registry&)>> m_hookSceneFunctions;

	// WIP: without std function is possible
	std::vector<std::function<void(entt::registry&)>> m_clearDirtyFunctions;


	using GetterFuncPtr = void* (*)(entt::registry&, entt::entity);
	std::unordered_map<mti::Hash, GetterFuncPtr> m_componentGetters;


	std::unordered_map<size_t, void (*)(entt::registry&, entt::entity)> m_componentDirters;

	std::unordered_map<size_t, void (*)(entt::registry&, entt::entity)> m_componentRemovers;

	std::unordered_map<size_t, void (*)(entt::registry&, entt::entity)> m_componentAdders;

	std::map<entt::id_type, std::string> m_components;

public:
	static std::map<entt::id_type, std::string> GetComponentList() { return Get().m_components; }

private:
	// WIP: Forward scene ptr in there (probably not from here?)
	template<typename Component>
	static void InitSceneElementForComponent(entt::registry& reg, entt::entity ent)
	{
		reg.get<Component>(ent).Z_RegisterToScene<typename Component::RenderSceneType>();
		reg.emplace<typename Component::Dirty>(ent);
	}

	template<typename Component>
	static void DestorySceneElementForComponent(entt::registry& reg, entt::entity ent)
	{
		reg.get<Component>(ent).Z_DeregisterFromScene<typename Component::RenderSceneType>();
	}


	template<typename Component>
	void Register()
	{
		const ReflClass* cl = &Component::StaticClass();

		auto entid = entt::type_info<Component>::id();
		m_types[entid] = cl;

		if constexpr (std::is_base_of_v<SceneCompBase, Component>) {
			m_hookSceneFunctions.emplace_back([](entt::registry& reg) {
				reg.on_construct<Component>()
					.template connect<&ComponentsDb::InitSceneElementForComponent<Component>>();
				reg.on_destroy<Component>()
					.template connect<&ComponentsDb::DestorySceneElementForComponent<Component>>();
			});
		}


		m_componentGetters[cl->GetTypeId().hash()] = [](entt::registry& reg, entt::entity ent) -> void* {
			return reg.try_get<Component>(ent);
		};

		m_componentRemovers[cl->GetTypeId().hash()] = [](entt::registry& reg, entt::entity ent) -> void {
			reg.remove<Component>(ent);
		};

		if constexpr (detail::HasDirtySubstructV<Component>) {
			m_componentDirters[cl->GetTypeId().hash()] = [](entt::registry& reg, entt::entity ent) -> void {
				reg.get_or_emplace<typename Component::Dirty>(ent);
			};
			m_clearDirtyFunctions.emplace_back([](entt::registry& reg) { reg.clear<typename Component::Dirty>(); });
		}


		m_components.emplace(entid, cl->GetNameStr());
		m_componentAdders[cl->GetTypeId().hash()] = [](entt::registry& reg, entt::entity ent) -> void {
			reg.emplace<Component>(ent);
		};
	}


public:
	// Query if the db contains this entt type
	static bool HasClass(entt::id_type type) { return Get().m_types.contains(type); }

	static const ReflClass& GetClass(entt::id_type type)
	{
		CLOG_ERROR(!Get().m_types.contains(type), "Asking for component class of an unregistered component.");
		return *Get().m_types[type];
	}


	// Type erased pointer to component data. Only recommended for reflection use
	// Return will be nullptr if entity does not have a component of the cl class
	static void* GetClassData(const ReflClass& cl, Entity ent)
	{
		return Get().m_componentGetters[cl.GetTypeId().hash()](*ent.m_registry, ent.m_entity);
	}

	static void SetComponentDirty(const ReflClass& cl, Entity ent)
	{
		auto& dirters = Get().m_componentDirters;
		auto it = dirters.find(cl.GetTypeId().hash());
		if (it != dirters.end()) {
			it->second(*ent.m_registry, ent.m_entity);
		}
	}

	static void RemoveComponent(const ReflClass& cl, Entity ent)
	{
		Get().m_componentRemovers[cl.GetTypeId().hash()](*ent.m_registry, ent.m_entity);
	}

	static void AddComponent(const ReflClass& cl, Entity ent)
	{
		Get().m_componentAdders[cl.GetTypeId().hash()](*ent.m_registry, ent.m_entity);
		SetComponentDirty(cl, ent);
	}

	// Add on_consturct/on_destruct functions for scene components
	static void HookRegistry(entt::registry& reg)
	{
		for (auto& func : Get().m_hookSceneFunctions) {
			func(reg);
		}
	}

	static void ClearDirties(entt::registry& reg)
	{
		for (auto& func : Get().m_clearDirtyFunctions) {
			func(reg);
		}
	}

	template<typename T>
	static void Z_RegisterClass()
	{
		Get().Register<T>();
	}
};

template<typename K>
struct ReflComponentRegistar {
	ReflComponentRegistar() { ComponentsDb::Z_RegisterClass<K>(); }
};
