#pragma once

struct FrameClock {
	using Clock = std::chrono::steady_clock;
	FrameClock() { Restart(); }

	Clock::duration lastFrameDuration;

	ch::time_point<Clock> loadedTimepoint;
	ch::time_point<Clock> lastFrameTimepoint;


	long long deltaTimeMicros{ 0 };
	float deltaSeconds{ 0.0f };

	float steadyFps{ 0.0f };

private:
	ch::time_point<Clock> lastFpsUpdateTime;
	uint32 framesSinceUpdate{ 0 };

public:
	// Unstable means that this is 1 / lastDeltaTime
	[[nodiscard]] float GetUnstableFPS() const noexcept { return 1.f / std::max(deltaSeconds, 1e-6f); }

	void Restart();

	// Returns true if steady fps was updated this call
	bool UpdateFrame();
};
