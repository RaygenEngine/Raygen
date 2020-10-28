#include "World.h"

#include "assets/AssetManager.h"
#include "editor/Editor.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"
#include "universe/components/CameraComponent.h"
#include "universe/components/DirlightComponent.h"
#include "universe/components/PointlightComponent.h"
#include "universe/components/ReflProbeComponent.h"
#include "universe/components/ScriptComponent.h"
#include "universe/components/SpotlightComponent.h"
#include "universe/components/StaticMeshComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/systems/AnimatorSystem.h"
#include "universe/systems/ScriptlikeRunnerSystem.h"

#include <nlohmann/json.hpp>
#include <fstream>

World::World(const fs::path& path)
	: srcPath(path)
{
	if (!path.empty()) {
		LoadFromSrcPath();
	}
}


void World::LoadFromSrcPath()
{
	std::ifstream file(srcPath);
	CLOG_ERROR(!file.is_open(), "Failed to open file: {} when loading world", srcPath);
	if (!file.is_open()) {
		return;
	}
	nlohmann::json j;
	file >> j;

	ComponentsDb::JsonToRegistry(j, *this);
}

void World::SaveToDisk(const fs::path& path, bool updateSrcPath)
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
	ComponentsDb::RegistryToJson(*this, j);

	std::ofstream file(srcPath);
	CLOG_ERROR(!file.is_open(), "Failed to open file: {} when saving world", srcPath);
	if (!file.is_open()) {
		return;
	}
	file << std::setw(2) << j;
}

Entity World::CreateEntity(const std::string& name)
{
	Entity ent{ reg.create(), this, reg };

	auto& basic = ent.Add<BasicComponent>();
	basic.self = ent;
	basic.name = !name.empty() ? name : "New Entity";

	return ent;
}

void World::UpdateWorld(Scene* scene)
{
	clock.UpdateFrame();

	//
	// Game Systems
	//

	if (playState == PlayState::Playing) {
		ScriptlikeRunnerSystem::TickRegistry(reg, clock.deltaSeconds);
	}


	Editor::Update();

	//
	// Update Transforms
	//


	//
	// Scene Commands
	//

	AnimatorSystem::UpdateAnimations(reg, clock.deltaSeconds);

	if (scene) {
		SceneCmdSystem::WriteSceneCmds(scene, reg);
		AnimatorSystem::UploadAnimationsToScene(reg, *scene);
		scene->EnqueueEndFrame();
	}

	// Clean Up
	reg.clear<DirtyMovedComp, DirtySrtComp>();
	ComponentsDb::ClearDirties(*this); // Also destroyes all pairs T, T::Destroy


	for (const auto ent : reg.view<CDestroyFlag>()) {
		reg.destroy(ent);
	}
	CLOG_ERROR(reg.view<CDestroyFlag>().size(), "Error deleting");
}

void World::BeginPlay()
{
	// CHECK: Log failed requirements for play/pause/stop
	if (playState != PlayState::Stopped) {
		return;
	}
	playState = PlayState::Playing;
	clock.Restart();
	ScriptlikeRunnerSystem::BeginPlay(reg);
}

void World::Pause()
{
	if (playState == PlayState::Playing) {
		playState = PlayState::Paused;
	}
}

void World::Unpause()
{
	if (playState == PlayState::Paused) {
		playState = PlayState::Playing;
	}
}

void World::EndPlay()
{
	const bool canEndPlay = playState == PlayState::Playing || playState == PlayState::Paused;
	if (!canEndPlay) {
		return;
	}
	playState = PlayState::Stopped;
	ScriptlikeRunnerSystem::EndPlay(reg);
}
