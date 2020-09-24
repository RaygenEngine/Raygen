#pragma once
#include "universe/World.h"

class Universe {
	friend class Engine_;

	static void Init(const fs::path& defaultWorldPath, const fs::path& localPath);
	static void Destroy();

public:
	// Expected to be called at the beginning of a frame, this will actually update the world pointers and construct
	// deconstruct them. To load a world use Universe::LoadMainWorld();
	static void LoadPendingWorlds();

	static void LoadMainWorld(const fs::path& path);

	inline static World* MainWorld{};
};
