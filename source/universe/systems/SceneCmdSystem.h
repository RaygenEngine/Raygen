#pragma once
#include "universe/ComponentDetail.h"
#include "universe/BasicComponent.h"
#include "rendering/scene/Scene.h"

struct Scene_;
//@ MODULES:
#include "universe/components/ReflProbeComponent.h"

namespace scenecmds {

template<typename T>
void EnqueueCreateDestroyCmds(Scene* scene, entt::registry& reg)
{
	std::vector<size_t> destructions;
	auto destroyView = reg.view<T, typename T::Destroy>();


	for (auto&& [ent, sc] : destroyView.each()) {
		destructions.emplace_back(sc.sceneUid);
		reg.remove<T>(ent);
	}


	std::vector<size_t*> constructions;
	auto createView = reg.view<T, typename T::Create>(entt::exclude<typename T::Destroy>);

	for (auto&& [ent, sc] : createView.each()) {
		constructions.emplace_back(&sc.sceneUid);
	}

	scene->EnqueueCreateDestoryCmds<typename T::RenderSceneType>(std::move(destructions), std::move(constructions));
}


template<typename T>
void EnqueueDirtyCmds(Scene* scene, entt::registry& reg)
{
	scene->SetCtx<typename T::RenderSceneType>();

	auto view = reg.view<BasicComponent, T, typename T::Dirty>();
	for (auto&& [ent, basic, sc] : view.each()) {
		scene->EnqueueCmdInCtx<typename T::RenderSceneType>(sc.sceneUid, sc.DirtyCmd<true>(basic));
	}
}


template<typename T>
void EnqueueTransformCmds(Scene* scene, entt::registry& reg)
{
	scene->SetCtx<typename T::RenderSceneType>();
	auto view = reg.view<BasicComponent, T, DirtySrtComp>(entt::exclude<typename T::Dirty>);

	for (auto&& [ent, basic, sc] : view.each()) {
		scene->EnqueueCmdInCtx<typename T::RenderSceneType>(sc.sceneUid, sc.DirtyCmd<false>(basic));
	}
}

template<typename T>
void EnqueueRecreateCmds(Scene* scene, entt::registry& reg)
{
	{
		std::vector<size_t*> constructions;
		auto createView = reg.view<T>(entt::exclude<typename T::Destroy>);

		for (auto&& [ent, sc] : createView.each()) {
			constructions.emplace_back(&sc.sceneUid);
		}

		scene->EnqueueCreateDestoryCmds<typename T::RenderSceneType>({}, std::move(constructions));
	}


	{
		scene->SetCtx<typename T::RenderSceneType>();
		auto view = reg.view<BasicComponent, T>();
		for (auto&& [ent, basic, sc] : view.each()) {
			scene->EnqueueCmdInCtx<typename T::RenderSceneType>(sc.sceneUid, sc.DirtyCmd<true>(basic));
		}
	}
}
} // namespace scenecmds


// Singleton (required because it works with auto registration) // CHECK: can init as global from componentsdb
// Stores the type erased functions required to propagate SceneCmds for all known scene compoentnts to a scene
class SceneCmdSystem {

	// std::function like declaration of function pointers
	template<typename ReturnType, typename... Args>
	using FnPtr = ReturnType (*)(Args...);


	// Commands to repopoulate a scene, (useful when transfering to a new scene).
	// The equivelant destory is not required (yet) as you can just delete the whole scene
	std::vector<FnPtr<void, Scene*, entt::registry&>> m_recreateCmds;

	std::vector<FnPtr<void, Scene*, entt::registry&>> m_createDestroyCmds;

	std::vector<FnPtr<void, Scene*, entt::registry&>> m_dirtyCmds;
	std::vector<FnPtr<void, Scene*, entt::registry&>> m_transformCmds;

	SceneCmdSystem() = default;

	static SceneCmdSystem& Get()
	{
		static SceneCmdSystem instance;
		return instance;
	}

	template<CSceneComp T>
	void Register()
	{
		namespace cd = componentdetail;
		using namespace entt;

		using SceneT = typename T::RenderSceneType;

		m_recreateCmds.emplace_back(&scenecmds::EnqueueRecreateCmds<T>);
		m_createDestroyCmds.emplace_back(&scenecmds::EnqueueCreateDestroyCmds<T>);

		m_dirtyCmds.emplace_back(&scenecmds::EnqueueDirtyCmds<T>);
		m_transformCmds.emplace_back(&scenecmds::EnqueueTransformCmds<T>);
	}

	friend class ComponentsDb;

	template<CSceneComp T>
	static void Z_Register()
	{
		Get().Register<T>();
	}

public:
	static void WriteSceneCmds(Scene* scene, entt::registry& registry);
	static void WriteRecreateCmds(Scene* scene, entt::registry& registry);
};
