#include "pch.h"
#include "Universe.h"

namespace {
std::optional<fs::path> worldToLoad{};
}

void Universe::Init()
{
	MainWorld = new World(new NodeFactory());
}

void Universe::Destroy()
{
	delete MainWorld;
}

void Universe::LoadMainWorld(const fs::path& path)
{
	worldToLoad = path;
}

void Universe::LoadPendingWorlds()
{
	if (!worldToLoad.has_value()) {
		return;
	}

	delete MainWorld;

	MainWorld = new World(new NodeFactory());
	MainWorld->LoadAndPrepareWorld(*worldToLoad);

	worldToLoad.reset();
}
