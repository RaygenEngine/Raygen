#include "pch.h"
#include "engine/Engine.h"

#include "App.h"
#include "assets/Assets.h"
#include "editor/Editor.h"
#include "engine/Input.h"
#include "engine/reflection/ReflectionDb.h"
#include "platform/Platform.h"
#include "renderer/VulkanLayer.h"
#include "renderer/asset/GpuAssetManager.h"
#include "renderer/wrapper/Device.h"
#include "universe/NodeFactory.h"
#include "universe/Universe.h"

#include <glfw/glfw3.h>
#include <algorithm>

ConsoleFunction<> debugCoords{ "d.viewport", //
	[]() {
		auto& c = g_ViewportCoordinates;
		LOG_REPORT("\n viewport.Size: {}, {}\n viewport.Pos: {} {}", c.size.x, c.size.y, c.position.x, c.position.y);
	} };

S_Engine::~S_Engine()
{
	// Destruction of objects is done at Deinit
}

void S_Engine::InitEngine(App* app)
{
	m_initToFrameTimer.Start();

	m_app = app;

	m_input = new Input();
	Assets::Init();

	WindowCreationParams mainWindowParams;
	mainWindowParams.size = { m_app->m_windowWidth, m_app->m_windowHeight };
	Platform::Init(mainWindowParams);

	GpuAssetManager.LoadAll();
	// NEXT:
	Layer = new VulkanLayer(glfwutl::GetVulkanExtensions(), Platform::GetMainHandle());
	Layer->Init();

	Editor::EditorInst = new EditorObject();

	ImguiImpl::InitVulkan();
	Universe::Init();
}

bool S_Engine::ShouldUpdateWorld()
{
	return Editor::EditorInst->ShouldUpdateWorld();
}

float S_Engine::GetFPS()
{
	return m_gameThreadFps.GetSteadyFps();
}

void S_Engine::ReportFrameDrawn()
{
	using namespace std::literals;

	static int32 titleCounter = 0;
	if (m_gameThreadFps.CountFrame()) {
		titleCounter = (titleCounter + 1) % 5;

		if (titleCounter == 1) {
			std::string s_title;
			s_title = fmt::format("{} - {:4.2f}", m_app->m_windowTitle, m_gameThreadFps.GetSteadyFps());
			Platform::GetMainWindow()->SetTitle(s_title.c_str());
		}
	}

	// CHECK:
	static bool hasFrameReport = false;

	if (!hasFrameReport) {
		LOG_WARN("Init to frame took: {} ms", m_initToFrameTimer.Get<ch::milliseconds>());
		hasFrameReport = true;
	}
}

void S_Engine::DeinitEngine()
{
	// NOTE: It is REALLY important to remember the reverse order here
	delete Universe::GetMainWorld();
	delete Layer;
	delete Device;
	delete Editor::EditorInst;
	Platform::Destroy();
	Assets::Destroy();
	delete m_input;
}

bool S_Engine::ThreadFpsCounter::CountFrame()
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
