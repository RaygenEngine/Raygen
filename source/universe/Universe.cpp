#include "pch.h"
#include "Universe.h"

#include "rendering/Layer.h"
#include "rendering/scene/Scene.h"

namespace {
std::optional<fs::path> worldToLoad{};
} // namespace


namespace {
} // namespace

void Universe::Init(const fs::path& defaultWorldPath, const fs::path& localPath)
{
	if (!fs::exists(localPath)) {
		if (!fs::copy_file(defaultWorldPath, localPath)) {
			LOG_ERROR("Failed to copy default world file to local.");
			MainWorld = new World();
			return;
		}
	}
	MainWorld = new World(localPath);
}

void Universe::Destroy()
{
	delete MainWorld;
}

void Universe::LoadMainWorld(const fs::path& path)
{
	worldToLoad = path;
}

void Universe::ReloadMainWorld()
{
	Universe::LoadMainWorld(Universe::MainWorld->srcPath);
}

void Universe::LoadPendingWorlds()
{
	if (worldToLoad.has_value()) {
		delete MainWorld;
		vl::Layer->mainScene = new Scene();

		MainWorld = new World(*worldToLoad);
		worldToLoad.reset();
	}
}
