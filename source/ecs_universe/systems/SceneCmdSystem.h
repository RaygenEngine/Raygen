#pragma once

#include "ecs_universe/ComponentDetail.h"
#include "ecs_universe/Entity.h" // WIP: Basic component

struct Scene_;

namespace scenecmds {
template<typename T>
void EnqueueCreateCmds(Scene_* scene, entt::registry& reg)
{
	auto view = reg.view<T, typename T::Create>(entt::exclude<typename T::Destroy>);
	for (auto& [ent, sc] : view.each()) {
		sc.sceneUid = scene->EnqueueCreateCmd<typename T::RenderSceneType>();
	}
}

template<typename T>
void EnqueueDestoryCmds(Scene_* scene, entt::registry& reg)
{
	auto view = reg.view<T, typename T::Destroy>();
	for (auto& [ent, sc] : view.each()) {
		scene->EnqueueDestroyCmd<typename T::RenderSceneType>(sc.sceneUid);

		// DO LAST: references may become invalidated.
		reg.remove<T>(ent);
	}
}

template<typename T>
void EnqueueDirtyCmds(Scene_* scene, entt::registry& reg)
{
	auto view = reg.view<BasicComponent, T, typename T::Dirty>();
	for (auto& [ent, basic, sc] : view.each()) {
		scene->EnqueueCmd<typename T::RenderSceneType>(sc.sceneUid, sc.DirtyCmd<true>(basic));
	}
}


template<typename T>
void EnqueueTransformCmds(Scene_* scene, entt::registry& reg)
{
	auto view = reg.view<BasicComponent, T, DirtySrtComp>(entt::exclude<typename T::Dirty>);
	for (auto& [ent, basic, sc] : view.each()) {
		scene->EnqueueCmd<typename T::RenderSceneType>(sc.sceneUid, sc.DirtyCmd<false>(basic));
	}
}
} // namespace scenecmds


// Singleton (required because it works with auto registration) // CHECK: can init as global from componentsdb
// Stores the type erased functions required to propagate SceneCmds for all known scene compoentnts to a scene
class SceneCmdSystem {

	// std::function like declaration of function pointers
	// CHECK: Function returning function & casting
	template<typename ReturnType, typename... Args>
	using FnPtr = ReturnType (*)(Args...);


	std::vector<FnPtr<void, Scene_*, entt::registry&>> m_createCmds;
	std::vector<FnPtr<void, Scene_*, entt::registry&>> m_destroyCmds;

	std::vector<FnPtr<void, Scene_*, entt::registry&>> m_dirtyCmds;
	std::vector<FnPtr<void, Scene_*, entt::registry&>> m_transformCmds;

	SceneCmdSystem() = default;

	static SceneCmdSystem& Get()
	{
		static SceneCmdSystem instance;
		return instance;
	}

	template<CONC(CSceneComp) T>
	void Register()
	{
		namespace cd = componentdetail;
		using namespace entt;

		using SceneT = typename T::RenderSceneType;

		m_createCmds.emplace_back(&scenecmds::EnqueueCreateCmds<T>);
		m_destroyCmds.emplace_back(&scenecmds::EnqueueDestoryCmds<T>);

		m_dirtyCmds.emplace_back(&scenecmds::EnqueueDirtyCmds<T>);
		m_transformCmds.emplace_back(&scenecmds::EnqueueTransformCmds<T>);
	}

	friend class ComponentsDb;

	template<CONC(CSceneComp) T>
	static void Z_Register()
	{
		Get().Register<T>();
	}

public:
	static void WriteSceneCmds(Scene_* scene, entt::registry& registry);
};
