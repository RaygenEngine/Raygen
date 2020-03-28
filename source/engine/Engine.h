#pragma once
#include "engine/Listener.h"
#include "engine/Timer.h"

class App;

inline struct ViewportCoordinates {
	glm::uvec2 position{};
	glm::uvec2 size{ 128, 128 };
	bool operator==(const ViewportCoordinates&) const = default;
} g_ViewportCoordinates;


inline class Engine_ : public Listener {

public:
	Engine_() = default;
	~Engine_();

	Engine_(Engine_ const&) = delete;
	Engine_(Engine_&&) = delete;
	Engine_& operator=(Engine_ const&) = delete;
	Engine_& operator=(Engine_&&) = delete;

private:
	// Non owning pointer, expected to be valid for the whole program execution
	App* m_app{ nullptr };

	timer::Timer m_initToFrameTimer{};


	struct ThreadFpsCounter {
		timer::Timer frameTimer{ true };
		float lastFrameTime{ 0.f };

		float steadyFps{ 0.f };
		ch::system_clock::time_point lastRecordTime{};
		size_t framesSinceLastRecord{ 0 };

		float GetSteadyFps() const { return steadyFps; }

		// Returns true every time steadyFps was updated. (every 100ms)
		bool CountFrame();
	};


public:
	ThreadFpsCounter m_gameThreadFps;
	// ThreadFpsCounter m_sceneThreadFps;

	// Init the internal engine systems.
	// You MUST run this to properly init the engine
	//
	// Expects a non-owning pointer to the external App object.
	//
	void InitEngine(App* app);

	[[nodiscard]] float GetFPS();

	void ReportFrameDrawn();

	void DeinitEngine();

	[[nodiscard]] App* GetApp() const { return m_app; }

} Engine{};
