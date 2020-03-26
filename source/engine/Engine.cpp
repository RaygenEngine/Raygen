#include "pch.h"
#include "engine/Engine.h"

#include "App.h"
#include "asset/AssetManager.h"
#include "editor/Editor.h"
#include "engine/Input.h"
#include "engine/reflection/ReflectionDb.h"
#include "platform/GlfwUtl.h"
#include "renderer/VulkanLayer.h"
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
	m_assetImporterManager = new AssetImporterManager();
	m_assetFrontEndManager = new AssetFrontEndManager();
	m_assetImporterManager->Init(m_app->m_assetPath);

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window
		= glfwCreateWindow(m_app->m_windowWidth, m_app->m_windowHeight, m_app->m_windowTitle.c_str(), nullptr, nullptr);

	glfwutl::SetupEventCallbacks(m_window);

	// NEXT:
	Layer = new VulkanLayer(glfwutl::GetVulkanExtensions(), m_window);
	Layer->Init();

	Editor::EditorInst = new EditorObject();

	ImguiImpl::InitVulkan();
}

void S_Engine::CreateWorldFromFile(const std::string& filename)
{
	Universe::MainWorld = new World(m_app->MakeNodeFactory());
	Universe::MainWorld->LoadAndPrepareWorld(filename);
}

bool S_Engine::HasCmdArgument(const std::string& argument)
{
	int32 argc = m_app->m_argc;
	char** argv = m_app->m_argv;
	for (int32 i = 0; i < argc; ++i) {
		if (strcmp(argv[i], argument.c_str())) {
			return true;
		}
	}
	return false;
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
			glfwSetWindowTitle(m_window, s_title.c_str());
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
	delete Universe::MainWorld;
	delete Layer;
	delete Device;
	delete Editor::EditorInst;
	glfwDestroyWindow(m_window);
	glfwTerminate();
	delete m_assetImporterManager;
	delete m_assetFrontEndManager;
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
