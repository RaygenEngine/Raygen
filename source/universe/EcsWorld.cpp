#include "pch.h"
#include "EcsWorld.h"

#include "assets/AssetManager.h"
#include "universe/components/ScriptComponent.h"
#include "universe/components/StaticMeshComponent.h"
#include "universe/components/SpotlightComponent.h"
#include "universe/components/CameraComponent.h"
#include "universe/components/ReflectionProbeComponent.h"
#include "universe/components/DirectionalLightComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/systems/AnimatorSystem.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "editor/Editor.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"

#include <nlohmann/json.hpp>
#include <fstream>

EcsWorld::EcsWorld(const fs::path& path)
	: srcPath(path)
{
	if (!path.empty()) {
		LoadFromSrcPath();
	}
}

EcsWorld::~EcsWorld()
{
	delete Scene;
	Scene = new Scene_(2);
	Scene->EnqueueCreateCmd<SceneCamera>();
}

void EcsWorld::LoadFromSrcPath()
{
	std::ifstream file(srcPath);
	CLOG_ERROR(!file.is_open(), "Failed to open file: {} when loading world", srcPath);
	if (!file.is_open()) {
		return;
	}
	nlohmann::json j;
	file >> j;

	ComponentsDb::JsonToRegistry(j, reg);
}

void EcsWorld::SaveToDisk(const fs::path& path, bool updateSrcPath)
{
	if (path.empty()) {
		if (srcPath.empty()) {
			LOG_ERROR("Attempting to save world without path, but world was not loaded from a file");
			return;
		}
		SaveToDisk(srcPath);
		return;
	}

	if (updateSrcPath) {
		srcPath = path;
	}

	nlohmann::json j;
	ComponentsDb::RegistryToJson(reg, j);

	std::ofstream file(srcPath);
	CLOG_ERROR(!file.is_open(), "Failed to open file: {} when saving world", srcPath);
	if (!file.is_open()) {
		return;
	}
	file << std::setw(2) << j;
}

void EcsWorld::DestroyEntity(Entity entity)
{
	entity->SetParent();

	ComponentsDb::VisitWithType(
		entity, [&](const ComponentMetaEntry& ent) { ent.markDestroy(*entity.registry, entity.entity); });

	auto current = entity->firstChild;
	while (current) {
		DestroyEntity(current);
		current = current->next;
	}

	entity.registry->get_or_emplace<CDestroyFlag>(entity.entity);
}

void EcsWorld::UpdateWorld()
{
	clock.UpdateFrame();

	//
	// Game Systems
	//

	Editor::Update();

	//
	// Update Transforms
	//


	//
	// Scene Commands
	//

	AnimatorSystem::UpdateAnimations(reg, clock.deltaSeconds);

	SceneCmdSystem::WriteSceneCmds(Scene, reg);

	AnimatorSystem::UploadAnimationsToScene(reg, Scene);

	// Clean Up
	reg.clear<DirtyMovedComp, DirtySrtComp>();
	ComponentsDb::ClearDirties(reg); // Also destroyes all pairs T, T::Destroy


	for (const auto ent : reg.view<CDestroyFlag>()) {
		reg.destroy(ent);
	}
	CLOG_ERROR(reg.view<CDestroyFlag>().size(), "Error deleting");


	Scene->EnqueueEndFrame();
}
