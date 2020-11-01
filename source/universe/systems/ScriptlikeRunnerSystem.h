#pragma once
#include "universe/ComponentDetail.h"
#include "universe/BasicComponent.h"


namespace scriptlikecmd {

template<CScriptlikeComp T>
void ExecuteBeginPlayCmds(entt::registry& reg)
{
	auto view = reg.view<T>();
	for (auto& [ent, sc] : view.each()) {
		sc.BeginPlay();
	}
}

template<CScriptlikeComp T>
void ExecuteEndPlayCmds(entt::registry& reg)
{
	auto view = reg.view<T>();
	for (auto& [ent, sc] : view.each()) {
		sc.EndPlay();
	}
}

template<componentdetail::CTickableComp T>
void ExecuteTickCmds(entt::registry& reg, float deltaSeconds)
{
	auto view = reg.view<T>();
	for (auto& [ent, sc] : view.each()) {
		sc.Tick(deltaSeconds);
	}
}


} // namespace scriptlikecmd


// Singleton again (required because it works with auto registration) // CHECK: can init as global from componentsdb
// Stores the type erased functions required to execute all tick / begin-end play gameplay commands for all known
// scriptlike components
class ScriptlikeRunnerSystem {

	// std::function like declaration of function pointers
	template<typename ReturnType, typename... Args>
	using FnPtr = ReturnType (*)(Args...);


	std::vector<FnPtr<void, entt::registry&>> m_beginPlayCmds;
	std::vector<FnPtr<void, entt::registry&>> m_endPlayCmds;

	std::vector<FnPtr<void, entt::registry&, float>> m_tickCmds;


	ScriptlikeRunnerSystem() = default;

	static ScriptlikeRunnerSystem& Get()
	{
		static ScriptlikeRunnerSystem instance;
		return instance;
	}

	template<CScriptlikeComp T>
	void Register()
	{
		namespace cd = componentdetail;
		using namespace entt;

		if constexpr (cd::CBeginPlayComp<T>) {
			m_beginPlayCmds.emplace_back(&scriptlikecmd::ExecuteBeginPlayCmds<T>);
		}

		if constexpr (cd::CEndPlayComp<T>) {
			m_endPlayCmds.emplace_back(&scriptlikecmd::ExecuteEndPlayCmds<T>);
		}

		if constexpr (cd::CTickableComp<T>) {
			m_tickCmds.emplace_back(&scriptlikecmd::ExecuteTickCmds<T>);
		}
	}

	friend class ComponentsDb;

	template<CScriptlikeComp T>
	static void Z_Register()
	{
		Get().Register<T>();
	}

public:
	// Execute begin play on all relevant components in the registry
	// Begin play order is undefined.
	static void BeginPlay(entt::registry& registry);

	// Execute end play on all relevant components in the registry
	// End play order is undefined.
	static void EndPlay(entt::registry& registry);

	// Tick all tickable components in the registry with the appropriate deltaSeconds.
	// Tick order is undefined.
	static void TickRegistry(entt::registry& registry, float deltaSeconds);
};
