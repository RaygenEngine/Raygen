#pragma once
#include "universe/World.h"
#include "ecs_universe/EcsWorld.h"

class Universe {
	friend class Engine_;

	static void Init();
	static void Destroy();

public:
	static World* GetMainWorld() { return MainWorld; }

	// Expected to be called at the beginning of a frame, this will actually update the world pointers and construct
	// deconstruct them. To load a world use Universe::LoadMainWorld();
	static void LoadPendingWorlds();

	// Loads a world. Call is deferred until its safe to load the new world. (aka the beginning of the frame)
	static void LoadMainWorld(const fs::path& path);


	static void ECS_LoadMainWorld(const fs::path& path);


	inline static ECS_World* ecsWorld{ nullptr };
	inline static ECS_World* ecsWorld2{ nullptr };
};
