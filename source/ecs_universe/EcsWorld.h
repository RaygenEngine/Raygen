#pragma once
#include "ecs_universe/BasicComponent.h"
#include "ecs_universe/Entity.h"

#include "core/FrameClock.h"

struct HiddenFlagComp {
};

struct CDestroyFlag {
};

class ECS_World {
private:
	void LoadFromSrcPath();
	FrameClock clock;

public:
	entt::registry reg;


	fs::path srcPath;
	ECS_World(const fs::path& path = {});

	// If path is empty uses the original srcPath as path
	void SaveToDisk(const fs::path& path = {}, bool updateSrcPath = false);

	[[nodiscard]] const FrameClock& GetClock() const noexcept { return clock; }

	Entity CreateEntity(const std::string& name = "")
	{
		Entity ent{ reg.create(), &reg };

		auto& basic = ent.Add<BasicComponent>();
		basic.self = ent;
		basic.name = !name.empty() ? name : "New Entity";


		return ent;
	}

	// WIP: Not a world function
	void DestroyEntity(Entity entity);

	void CreateWorld();

	void UpdateWorld();

	~ECS_World();
};
