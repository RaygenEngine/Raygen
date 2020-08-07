#pragma once

#include "ecs_universe/ComponentDetail.h"
#include "ecs_universe/BasicComponent.h"
#include "ecs_universe/systems/SceneCmdSystem.h"

#include <nlohmann/json_fwd.hpp>

// Holds type erased function pointers for a component of generic type
struct ComponentMetaEntry {
	const ReflClass* clPtr{ nullptr };
	entt::id_type entType{};

	template<typename ReturnType>
	using FnPtr = ReturnType (*)(entt::registry&, entt::entity);

	using VoidFnPtr = FnPtr<void>;


	// Create the component on an entry
	// Will also mark dirty & create flags if supported by the actual component class
	VoidFnPtr emplace{ nullptr };

	// Instantly removes the component.
	// Prefer safeRemove
	VoidFnPtr remove{ nullptr };

	// Marks for destruction if possible
	// Prefer safeRemove
	VoidFnPtr markDestroy{ nullptr };

	VoidFnPtr safeRemove{ nullptr };


	FnPtr<void*> get{ nullptr };
	FnPtr<bool> has{ nullptr };

	// Can be nullptr
	VoidFnPtr markDirty{ nullptr };
	VoidFnPtr clearDirties{ nullptr };

private:
	// Use static function Make
	ComponentMetaEntry() = default;


public:
	template<typename T>
	static ComponentMetaEntry Make()
	{
		using namespace componentdetail;
		using namespace entt;
		ComponentMetaEntry entry;

		entry.clPtr = &T::StaticClass();
		entry.entType = entt::type_info<T>().id();

		entry.emplace = [](registry& r, entity e) {
			r.emplace<T>(e);
			if constexpr (HasCreateDestroySubstructsV<T>) {
				r.emplace<typename T::Create>(e);
			}
			if constexpr (HasDirtySubstructV<T>) {
				r.emplace<typename T::Dirty>(e);
			}
		};

		entry.remove = [](registry& r, entity e) {
			r.remove<T>(e);
		};

		entry.markDestroy = [](registry& r, entity e) {
			if constexpr (HasCreateDestroySubstructsV<T>) {
				r.emplace<typename T::Destroy>(e);
			}
		};

		entry.safeRemove = [](registry& r, entity e) {
			if constexpr (HasCreateDestroySubstructsV<T>) {
				r.emplace<typename T::Destroy>(e);
			}
			else {
				r.remove<T>(e);
			}
		};

		entry.get = [](registry& r, entity e) -> void* {
			return &r.get<T>(e);
		};

		entry.has = [](registry& r, entity e) -> bool {
			return r.has<T>(e);
		};

		entry.markDirty = [](registry& r, entity e) {
			if constexpr (HasDirtySubstructV<T>) {
				r.get_or_emplace<typename T::Dirty>(e);
			}
		};


		return entry;
	}

	template<typename T>
	static void RegisterToClearDirties(std::vector<void (*)(entt::registry&)>& clearFunctions)
	{
		using namespace componentdetail;
		if constexpr (HasDirtySubstructV<T> || HasCreateDestroySubstructsV<T>) {

			clearFunctions.emplace_back([](entt::registry& r) {
				if constexpr (HasCreateDestroySubstructsV<T>) {
					for (auto& [ent, comp] : r.view<T, typename T::Destroy>().each()) {
						r.remove<T>(ent);
					}
					r.clear<typename T::Create, typename T::Destroy>();
				}
				if constexpr (HasDirtySubstructV<T>) {
					r.clear<typename T::Dirty>();
				}
			});
		}
	};
};

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
	// Maps entt types to Component Meta Entries types.
	std::unordered_map<entt::id_type, ComponentMetaEntry> m_types;
	std::unordered_map<std::string, entt::id_type> m_nameToType;

	std::vector<void (*)(entt::registry&)> m_clearFuncs;

	std::map<std::string, std::vector<entt::id_type>> m_categoryToTypes;

	template<typename T>
	void Register()
	{
		ComponentMetaEntry comp = ComponentMetaEntry::Make<T>();

		auto entId = entt::type_info<T>().id();
		m_nameToType.emplace(comp.clPtr->GetNameStr(), entId);
		m_types.emplace(entt::type_info<T>().id(), std::move(comp));

		if (auto category = comp.clPtr->GetCategory(); category) {
			m_categoryToTypes[std::string(category)].emplace_back(entId);
		}
		else {
			m_categoryToTypes[""].emplace_back(entId);
		}

		ComponentMetaEntry::RegisterToClearDirties<T>(m_clearFuncs);

		if constexpr (componentdetail::IsSceneComponent<T>) {
			SceneCmdSystem::Z_Register<T>();
		}
	}

	void LoadComponentInto(Entity ent, const std::string& componentName, const nlohmann::json& json);

public:
	// Returns null if type is not found,
	// Can be used with if init statement: if (auto type = GetType; type)
	static const ComponentMetaEntry* GetType(entt::id_type type)
	{
		auto& inst = Get();
		auto it = inst.m_types.find(type);
		return it != inst.m_types.end() ? &it->second : nullptr;
	}

	static const ComponentMetaEntry* GetTypeByName(const std::string& componentClassName)
	{
		auto& inst = Get();
		auto it = inst.m_nameToType.find(componentClassName);
		if (it != inst.m_nameToType.end()) {
			return GetType(it->second);
		}
		return nullptr;
	}

	static void ClearDirties(entt::registry& reg)
	{
		for (auto func : Get().m_clearFuncs) {
			func(reg);
		}
	}

	template<typename T>
	static void Z_RegisterComponentClass()
	{
		Get().Register<T>();
	}

	// If you use this don't edit the ComponentMetaEntry structs
	static const std::unordered_map<entt::id_type, ComponentMetaEntry>& Z_GetTypes() { return Get().m_types; }
	static const std::unordered_map<std::string, entt::id_type>& Z_GetNameToTypes() { return Get().m_nameToType; }
	static const std::map<std::string, std::vector<entt::id_type>>& Z_GetCategories()
	{
		return Get().m_categoryToTypes;
	}


	static void RegistryToJson(entt::registry& reg, nlohmann::json& json);
	static void JsonToRegistry(const nlohmann::json& json, entt::registry& reg);

	// Exports an entity and all its children entities to a json object.
	static void EntityHierarchyToJson(entt::registry& reg, entt::entity ent, nlohmann::json& json);

	// Imports an entity hierarchy from json
	// TODO: ECS: Robustness. This is now used from clipboard and should NEVER ever crash with any json data.
	static Entity JsonToEntityHierarchy(entt::registry& reg, const nlohmann::json& json);


	// Function signature is: void(const ComponentMetaEntry&, entt::registry&, entt::entity)
	template<typename T>
	static void VisitWithType(entt::registry& reg, entt::entity ent, T function)
	{
		reg.visit(ent, [&](entt::id_type idtype) {
			if (const ComponentMetaEntry* type = GetType(idtype); type) {
				function(*type);
			}
		});
	}

	template<typename T>
	static void VisitWithType(Entity ent, T function)
	{
		VisitWithType(*ent.registry, ent.entity, function);
	}
};

template<typename K>
struct ReflComponentRegistrar {
	// Auto registration pattern, (generated through the reflection macros)
	ReflComponentRegistrar() { ComponentsDb::Z_RegisterComponentClass<K>(); }
	bool b = true;
};