#include "Universe.h"

#include "App.h"

namespace {
std::optional<fs::path> worldToLoad{};
} // namespace

void Universe::InitFromApp()
{
	Init(App->templateScene, App->localScene);
}

void Universe::Init(const fs::path& defaultWorldPath, const fs::path& localPath)
{
	if (localPath.empty()) {
		MainWorld = new World(defaultWorldPath);
		return;
	}

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
		// delete Layer->mainScene; NEW::
		// Layer->mainScene = new Scene();

		MainWorld = new World(*worldToLoad);
		worldToLoad.reset();
	}
}
