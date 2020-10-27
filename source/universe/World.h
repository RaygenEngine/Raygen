#pragma once
#include "universe/BasicComponent.h"
#include "universe/Entity.h"

#include "core/FrameClock.h"

struct HiddenFlagComp {
};

struct Scene;


class World {
public:
	enum class PlayState
	{
		Stopped,
		Playing,
		Paused,
	};

private:
	void LoadFromSrcPath();

	FrameClock clock; // TODO: Not using working properly with dilation/pause

	PlayState playState{ PlayState::Stopped };

public:
	entt::registry reg;


	fs::path srcPath;
	World(const fs::path& path = {});

	// If path is empty uses the original srcPath as path
	void SaveToDisk(const fs::path& path = {}, bool updateSrcPath = false);

	[[nodiscard]] const FrameClock& GetClock() const noexcept { return clock; }

	Entity CreateEntity(const std::string& name = "");

	void UpdateWorld(Scene& scene);

	[[nodiscard]] World::PlayState GetPlayState() const { return playState; }


public:
	void TogglePause() { playState == PlayState::Paused ? Unpause() : Pause(); }
	void BeginPlay();
	void Pause();
	void Unpause();
	void EndPlay();
};
