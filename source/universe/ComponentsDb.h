#pragma once

#include "universe/ComponentDetail.h"
#include "universe/BasicComponent.h"
#include "universe/systems/SceneCmdSystem.h"
#include "universe/systems/ScriptlikeRunnerSystem.h"
#include "universe/Entity.h"

#include <nlohmann/json_fwd.hpp>

// Holds type erased function pointers for a component of generic type
struct ComponentMetaEntry {
	const ReflClass* clPtr{ nullptr };
	entt::id_type entType{};

	template<typename ReturnType>
	using FnPtr = ReturnType (*)(Entity);

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
	template<CComponent T>
	static ComponentMetaEntry Make()
	{
		using namespace componentdetail;
		using namespace entt;
		ComponentMetaEntry entry;

		entry.clPtr = &T::StaticClass();
		entry.entType = entt::type_info<T>().id();

		entry.emplace = [](Entity ent) {
			ent.Add<T>();
		};

		entry.remove = [](Entity ent) {
			ent.UnsafeRemove<T>();
		};

		entry.markDestroy = [](Entity ent) {
			if constexpr (CCreateDestoryComp<T>) {
				ent.MarkDestroy<T>();
			}
		};

		entry.safeRemove = [](Entity ent) {
			ent.SafeRemove<T>();
		};

		entry.get = [](Entity ent) -> void* {
			return &ent.GetNonDirty<T>();
		};

		entry.has = [](Entity ent) -> bool {
			return ent.Has<T>();
		};

		entry.markDirty = [](Entity ent) {
			if constexpr (CDirtableComp<T>) {
				ent.MarkDirty<T>();
			}
		};

		return entry;
	}

	template<CComponent T>
	static void RegisterToClearDirties(std::vector<void (*)(entt::registry&)>& clearFunctions)
	{
		using namespace componentdetail;
		if constexpr (CDirtableComp<T> || CCreateDestoryComp<T>) {

			clearFunctions.emplace_back([](entt::registry& r) {
				if constexpr (CCreateDestoryComp<T>) {
					for (auto&& [ent, comp] : r.view<T, typename T::Destroy>().each()) {
						r.remove<T>(ent);
					}
					r.clear<typename T::Create, typename T::Destroy>();
				}
				if constexpr (CDirtableComp<T>) {
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

		if constexpr (CSceneComp<T>) {
			SceneCmdSystem::Z_Register<T>();
		}

		if constexpr (CScriptlikeComp<T>) {
			ScriptlikeRunnerSystem::Z_Register<T>();
		}
	}

	void LoadComponentInto(Entity ent, const std::string& componentName, const nlohmann::json& json);

public:
	template<CComponent T>
	static entt::id_type GetIdType()
	{
		return entt::type_info<T>().id();
	}

	// Returns null if type is not found,
	template<CComponent T>
	static const ComponentMetaEntry* GetType()
	{
		return GetType(GetIdType<T>());
	}


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

	static void ClearDirties(World& reg);

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

	// Export a world to json. Note that json is not reset, but appended on.
	static void RegistryToJson(World& reg, nlohmann::json& json);
	static void JsonToRegistry(const nlohmann::json& json, World& reg);

	// Exports an entity and all its children entities to a json object.
	static void EntityHierarchyToJson(Entity ent, nlohmann::json& json);

	// Imports an entity hierarchy from json
	// TODO: ECS: Robustness. This is now used from clipboard and should NEVER ever crash with any json data.
	// param: parent must be present to get proper world positions
	static Entity JsonToEntityHierarchy(World& reg, const nlohmann::json& json);


	// Function signature is: void(const ComponentMetaEntry&, entt::registry&, entt::entity)
	template<typename T>
	static void VisitWithType(Entity ent, T function)
	{
		ent.registry->visit(ent.entity, [&](entt::id_type idtype) {
			if (const ComponentMetaEntry* type = GetType(idtype); type) {
				function(*type);
			}
		});
	}
};

template<typename K>
struct ReflComponentRegistrar {
	// Auto registration pattern, (generated through the reflection macros)
	ReflComponentRegistrar() { ComponentsDb::Z_RegisterComponentClass<K>(); }
	bool b = true;
};
