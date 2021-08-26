#include "Engine.h"

#include "App.h"
#include "assets/Assets.h"
#include "editor/Editor.h"
#include "platform/Platform.h"
#include "rendering/Rendering.h"
#include "universe/Universe.h"


static ConsoleFunction<> cons_debugCoords{ "e.viewport.printCoords", //
	[]() {
		auto& c = g_ViewportCoordinates;
		LOG_REPORT("\n viewport.Size: {}, {}\n viewport.Pos: {} {}", c.size.x, c.size.y, c.position.x, c.position.y);
	} };

Engine_::~Engine_()
{
	// Destruction of objects is done at Deinit
}

static void FixStartupPath()
{
	auto path = fs::current_path();

	auto absStr = fs::absolute(path).generic_string();

	size_t index = absStr.rfind("/bin/");
	if (index != std::string::npos) {
		fs::current_path(absStr.substr(0, index));
	}
}

void Engine_::InitEngine(App_* app)
{
	m_initToFrameTimer.Start();

	FixStartupPath();


	fs::current_path(fs::current_path() / app->workingDirPath);

	Assets::Init();

	WindowCreationParams mainWindowParams;
	mainWindowParams.size = { app->windowSize.x, app->windowSize.y };
	Platform::Init(mainWindowParams);

	Rendering::Init();

	Editor::Init();


	Universe::Init(app->templateScene, app->localScene);
}

float Engine_::GetFPS()
{
	return m_gameThreadFps.GetSteadyFps();
}

void Engine_::ReportFrameDrawn()
{
	using namespace std::literals;

	static int32 titleCounter = 0;
	if (m_gameThreadFps.CountFrame()) {
		titleCounter = (titleCounter + 1) % 5;

		if (titleCounter == 1) {
			std::string s_title;
			s_title = fmt::format("{} - {:4.2f}", App->windowTitle, m_gameThreadFps.GetSteadyFps());
			Platform::GetMainWindow()->SetTitle(s_title.c_str());
		}
	}

	static bool hasFrameReport = false;

	if (!hasFrameReport) {
		LOG_WARN("Init to frame took: {} ms", m_initToFrameTimer.Get<ch::milliseconds>());
		hasFrameReport = true;
	}
}

void Engine_::DeinitEngine()
{
	// NOTE: It is REALLY important to remember the reverse order here
	Universe::Destroy();
	Editor::Destroy();
	Rendering::Destroy();
	Platform::Destroy();
	Assets::Destroy();
}

bool Engine_::ThreadFpsCounter::CountFrame()
{
	using namespace std::literals;
	constexpr static auto c_reportPeriod = 100ms;


	lastFrameTime = frameTimer.Get<ch::microseconds>() / 1e6f;
	frameTimer.Start();

	++framesSinceLastRecord;

	auto now = ch::system_clock::now();
	auto diff = now - lastRecordTime;
	if (diff >= c_reportPeriod) {
		steadyFps
			= static_cast<float>(framesSinceLastRecord) / ((ch::duration_cast<ch::nanoseconds>(diff).count() / 1e9f));
		lastRecordTime = now;
		framesSinceLastRecord = 0;
		return true;
	}
	return false;
}
