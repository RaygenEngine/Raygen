#pragma once
#include "universe/BasicComponent.h"

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
	entt::registry reg;

	friend class ComponentsDb;

public:
	fs::path srcPath;
	World(const fs::path& path = {});

	// If path is empty uses the original srcPath as path
	void SaveToDisk(const fs::path& path = {}, bool updateSrcPath = false);

	[[nodiscard]] const FrameClock& GetClock() const noexcept { return clock; }

	Entity CreateEntity(const std::string& name = "");

	void UpdateWorld(Scene& scene);

	[[nodiscard]] World::PlayState GetPlayState() const { return playState; }

	template<CComponent... T, CComponent... Exclude>
	auto GetView(entt::exclude_t<Exclude...> excl = {})
	{
		return reg.view<T...>(excl);
	}

public:
	void TogglePause() { playState == PlayState::Paused ? Unpause() : Pause(); }
	void BeginPlay();
	void Pause();
	void Unpause();
	void EndPlay();
};
