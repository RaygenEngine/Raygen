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
#include "world/NodeFactory.h"
#include "world/World.h"

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

	m_lastRecordTime = ch::system_clock::now();

	InitRenderer();
}

void S_Engine::CreateWorldFromFile(const std::string& filename)
{
	if (m_world) {

		delete m_world;
	}
	m_world = new World(m_app->MakeNodeFactory());

	auto json = AssetImporterManager::OLD_ResolveOrImportFromParentUri<JsonDocPod>(filename, "/");
	m_world->LoadAndPrepareWorld(json);
}

void S_Engine::InitRenderer()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window
		= glfwCreateWindow(m_app->m_windowWidth, m_app->m_windowHeight, m_app->m_windowTitle.c_str(), nullptr, nullptr);

	glfwutl::SetupEventCallbacks(m_window);

	// WIP
	Layer = new VulkanLayer(glfwutl::GetVulkanExtensions(), m_window);
	Layer->Init();

	m_editor = new Editor();

	ImguiImpl::InitVulkan();
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
	return m_editor->ShouldUpdateWorld();
}

float S_Engine::GetFPS()
{
	return m_steadyFps;
}

void S_Engine::ReportFrameDrawn()
{
	m_lastFrameTime = m_frameTimer.Get<ch::microseconds>() / 1e6f;
	m_frameTimer.Start();

	using namespace std::literals;
	++m_framesSinceLastRecord;

	auto now = ch::system_clock::now();
	auto diff = now - m_lastRecordTime;
	constexpr auto c_reportPeriod = 100ms;
	if (diff >= c_reportPeriod) {
		m_steadyFps
			= static_cast<float>(m_framesSinceLastRecord) / ((ch::duration_cast<ch::nanoseconds>(diff).count() / 1e9f));
		m_lastRecordTime = now;
		m_framesSinceLastRecord = 0;

		static int32 titleCounter = 0;
		if (titleCounter == 0) {
			static std::string s_title;
			s_title = fmt::format("{} - {:4.2f} FPS", m_app->m_windowTitle, m_steadyFps);
			glfwSetWindowTitle(m_window, s_title.c_str());
		}
		titleCounter = (titleCounter + 1) % 5;
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
	delete Layer;
	delete Device;
	delete m_editor;
	delete m_world;
	glfwDestroyWindow(m_window);
	glfwTerminate();
	delete m_assetImporterManager;
	delete m_assetFrontEndManager;
	delete m_input;
}
