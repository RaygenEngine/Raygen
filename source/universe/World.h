#pragma once
#include "universe/BasicComponent.h"

#include "core/FrameClock.h"

struct HiddenFlagComp {
};

struct Scene;
struct CCamera;

class World {
public:
	enum class PlayState
	{
		Stopped,
		Playing,
		Paused,
	};

public:
	// Empties this world & restarts timers etc. (keeps srcPath)
	void ResetWorld();


	void TogglePause() { playState == PlayState::Paused ? Unpause() : Pause(); }
	// Instantly begins play
	void BeginPlay();
	// Instantly ends play
	void EndPlay();

	// Sets the flag to begin play whenever possible (preferred version)
	void FlagBeginPlay();
	// Sets the flag to end play whenever possible (preferred version)
	void FlagEndPlay();

	void Pause();
	void Unpause();


	fs::path srcPath;
	World(const fs::path& path = {});

	// If path is empty uses the original srcPath as path
	void SaveToDisk(const fs::path& path = {}, bool updateSrcPath = false);

	[[nodiscard]] const FrameClock& GetClock() const noexcept { return clock; }

	Entity CreateEntity(const std::string& name = "");

	void UpdateWorld(Scene* scene);

	[[nodiscard]] World::PlayState GetPlayState() const { return playState; }

	template<CComponent... T, CComponent... Exclude>
	auto GetView(entt::exclude_t<Exclude...> excl = {})
	{
		return reg.view<T...>(excl);
	}

	[[nodiscard]] bool IsPlaying() const { return playState == PlayState::Playing; }
	[[nodiscard]] bool IsPaused() const { return playState == PlayState::Paused; }
	[[nodiscard]] bool IsStopped() const { return playState == PlayState::Stopped; }

	void SetActiveCamera(const CCamera& camera);


private:
	size_t activeCameraUid{ 0 };
	void LoadFromSrcPath();

	FrameClock clock; // TODO: Not using working properly with dilation/pause

	PlayState playState{ PlayState::Stopped };
	entt::registry reg;

	friend class ComponentsDb;

	BoolFlag beginPlayFlag;
	BoolFlag endPlayFlag;
};
