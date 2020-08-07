#include "pch.h"
#include "SceneCmdSystem.h"


void SceneCmdSystem::WriteSceneCmds(Scene* scene, entt::registry& registry)
{
	// Write scene creations
	for (auto fn : Get().m_createCmds) {
		fn(scene, registry);
	}

	// Write scene destructions
	for (auto fn : Get().m_destroyCmds) {
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
