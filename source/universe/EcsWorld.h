#pragma once
#include "universe/BasicComponent.h"
#include "universe/Entity.h"

#include "core/FrameClock.h"

struct HiddenFlagComp {
};

struct Scene;

class EcsWorld {
private:
	void LoadFromSrcPath();
	FrameClock clock;

public:
	entt::registry reg;


	fs::path srcPath;
	EcsWorld(const fs::path& path = {});

	// If path is empty uses the original srcPath as path
	void SaveToDisk(const fs::path& path = {}, bool updateSrcPath = false);

	[[nodiscard]] const FrameClock& GetClock() const noexcept { return clock; }

	Entity CreateEntity(const std::string& name = "");

	void UpdateWorld(Scene& scene);
};
