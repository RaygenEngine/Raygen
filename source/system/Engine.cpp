#include "pch/pch.h"

#include "system/Engine.h"
#include "system/Input.h"
#include "AppBase.h"
#include "asset/AssetManager.h"
#include "editor/Editor.h"
#include "world/NodeFactory.h"
#include "world/World.h"
#include "platform/GlfwUtil.h"
#include "system/reflection/ReflectionDb.h"
#include <glfw/glfw3.h>
#include "renderer/VulkanLayer.h"
#include <algorithm>

ConsoleFunction<> debugCoords{ "d.viewport", []() {
								  auto& c = g_ViewportCoordinates;
								  LOG_REPORT("\n viewport.Size: {}, {}\n viewport.Pos: {} {}", c.size.x, c.size.y,
									  c.position.x, c.position.y);
							  } };

Engine::~Engine()
{
	// Destruction of objects is done at Deinit
}

void Engine::DrawReporter::Reset()
{
	tris = 0ull;
	draws = 0ull;
}

void Engine::InitEngine(AppBase* app)
{
	m_initToFrameTimer.Start();

	m_app = app;
	m_isEditorEnabled = app->m_enableEditor;

	m_input = new Input();
	m_assetImporterManager = new AssetImporterManager();
	m_assetFrontEndManager = new AssetFrontEndManager();
	m_assetImporterManager->Init(m_app->m_assetPath);

	m_lastRecordTime = ch::system_clock::now();

	InitRenderer();
}

void Engine::CreateWorldFromFile(const std::string& filename)
{
	if (m_world) {

		delete m_world;
	}
	m_world = new World(m_app->MakeNodeFactory());

	auto json = AssetImporterManager::ResolveOrImportFromParentUri<JsonDocPod>(filename, "/");
	m_world->LoadAndPrepareWorld(json);
}

void Engine::InitRenderer()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window
		= glfwCreateWindow(m_app->m_windowWidth, m_app->m_windowHeight, m_app->m_windowTitle.c_str(), nullptr, nullptr);

	glfwutl::SetupEventCallbacks(m_window);

	VulkanLayer::InitVulkanLayer(glfwutl::GetVulkanExtensions(), m_window);

	if (m_isEditorEnabled) {
		m_editor = new Editor();
	}

	ImguiImpl::InitVulkan();
}

bool Engine::HasCmdArgument(const std::string& argument)
{
	Engine& eng = Engine::Get();

	int32 argc = eng.m_app->m_argc;
	char** argv = eng.m_app->m_argv;
	for (int32 i = 0; i < argc; ++i) {
		if (strcmp(argv[i], argument.c_str())) {
			return true;
		}
	}
	return false;
}

bool Engine::ShouldUpdateWorld()
{
	if (IsEditorActive()) {
		return Get().m_editor->ShouldUpdateWorld();
	}
	return true;
}

bool Engine::IsEditorActive()
{
	return Get().m_editor && Get().m_isEditorActive;
}

bool Engine::IsEditorEnabled()
{
	return Get().m_isEditorEnabled;
}

float Engine::GetFPS()
{
	return Get().m_steadyFps;
}

void Engine::ReportFrameDrawn()
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
			s_title = fmt::format("Raygen - {:4.2f} FPS", m_steadyFps);
			glfwSetWindowTitle(m_window, s_title.c_str());
		}
		titleCounter = (titleCounter + 1) % 5;
	}

	static bool hasFrameReport = false;

	if (!hasFrameReport) {
		LOG_WARN("Init to frame took: {} ms", m_initToFrameTimer.Get<ch::milliseconds>());
		hasFrameReport = true;
	}
}

void Engine::ToggleEditor()
{
	// TODO:
	// if (m_isEditorEnabled && m_renderer) {
	//	m_isEditorActive ? DeactivateEditor() : ActivateEditor();
	//}
}

void Engine::ActivateEditor()
{
	if (!m_isEditorActive) {
		m_editor->OnEnableEditor();
		m_isEditorActive = true;
	}
}

void Engine::DeactivateEditor()
{
	if (m_isEditorActive) {
		m_editor->OnDisableEditor();
		m_isEditorActive = false;
	}
}

void Engine::DeinitEngine()
{
	// NOTE: It is REALLY important to remember the reverse order here
	delete m_editor;
	delete m_world;
	glfwDestroyWindow(m_window);
	glfwTerminate();
	delete m_assetImporterManager;
	delete m_assetFrontEndManager;
	delete m_input;
}
