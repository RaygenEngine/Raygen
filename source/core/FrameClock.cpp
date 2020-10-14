#include "FrameClock.h"

void FrameClock::Restart()
{
	loadedTimepoint = Clock::now();
	lastFrameTimepoint = loadedTimepoint;
	lastFpsUpdateTime = loadedTimepoint;

	framesSinceUpdate = 0;
}

bool FrameClock::UpdateFrame()
{
	using namespace std::literals;
	constexpr static auto c_reportPeriod = 100ms;

	auto now = Clock::now();
	// Delta Time:
	{
		lastFrameDuration = now - lastFrameTimepoint;

		deltaTimeMicros = ch::duration_cast<ch::microseconds>(lastFrameDuration).count();
		deltaSeconds = static_cast<float>(deltaTimeMicros / (1e6));

		lastFrameTimepoint = now;
	}

	// Fps & Steady Fps:
	{
		framesSinceUpdate++;

		if (now - lastFpsUpdateTime >= c_reportPeriod) {
			steadyFps = static_cast<float>(framesSinceUpdate)
						/ ((ch::duration_cast<ch::nanoseconds>(now - lastFpsUpdateTime).count() / 1e9f));
			lastFpsUpdateTime = now;
			framesSinceUpdate = 0;
			return true;
		}
	}
	return false;
}
