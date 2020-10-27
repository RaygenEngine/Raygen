#include "ScriptlikeRunnerSystem.h"


void ScriptlikeRunnerSystem::BeginPlay(entt::registry& registry)
{
	auto& inst = Get();
	for (auto cmd : inst.m_beginPlayCmds) {
		cmd(registry);
	}
}

void ScriptlikeRunnerSystem::EndPlay(entt::registry& registry)
{
	auto& inst = Get();
	for (auto cmd : inst.m_endPlayCmds) {
		cmd(registry);
	}
}

void ScriptlikeRunnerSystem::TickRegistry(entt::registry& registry, float deltaSeconds)
{
	auto& inst = Get();
	for (auto cmd : inst.m_tickCmds) {
		cmd(registry, deltaSeconds);
	}
}
