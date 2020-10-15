#include "SceneCmdSystem.h"


void SceneCmdSystem::WriteSceneCmds(Scene* scene, entt::registry& registry)
{
	// Write scene creations destructions
	for (auto fn : Get().m_createDestroyCmds) {
		fn(scene, registry);
	}

	// Write scene dirty updates
	for (auto fn : Get().m_dirtyCmds) {
		fn(scene, registry);
	}

	// Write scene transform updates
	for (auto fn : Get().m_transformCmds) {
		fn(scene, registry);
	}
}

void SceneCmdSystem::WriteRecreateCmds(Scene* scene, entt::registry& registry)
{
	for (auto fn : Get().m_recreateCmds) {
		fn(scene, registry);
	}
}
