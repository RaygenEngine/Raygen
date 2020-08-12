#include "pch.h"
#include "Universe.h"

#include "rendering/Layer.h"
#include "rendering/scene/Scene.h"

namespace {
std::optional<fs::path> worldToLoad{};
std::optional<fs::path> ecsWorldToLoad{};
} // namespace


namespace {
} // namespace

void Universe::Init(const fs::path& defaultWorldPath, const fs::path& localPath)
{
	if (!fs::exists(localPath)) {
		if (!fs::copy_file(defaultWorldPath, localPath)) {
			LOG_ERROR("Failed to copy default world file to local.");
			ecsWorld = new EcsWorld();
			return;
		}
	}
	ecsWorld = new EcsWorld(localPath);
}

void Universe::Destroy()
{
	delete ecsWorld;
}

void Universe::ECS_LoadMainWorld(const fs::path& path)
{
	ecsWorldToLoad = path;
}

void Universe::LoadPendingWorlds()
{
	if (ecsWorldToLoad.has_value()) {
		delete ecsWorld;
		vl::Layer->mainScene = std::make_unique<Scene>();

		ecsWorld = new EcsWorld(*ecsWorldToLoad);
		ecsWorldToLoad.reset();
	}
}
