#include "pch/pch.h"

#include "system/Engine.h"
#include "system/Input.h"
#include "AppBase.h"
#include "asset/AssetManager.h"
#include "editor/Editor.h"
#include "renderer/Renderer.h"
#include "world/NodeFactory.h"
#include "world/World.h"
#include "renderer/renderers/vulkan/VkRendererBase.h"
#include "platform/GlfwUtil.h"
#include <glfw/glfw3.h>
#include <algorithm>

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
	m_assetManager = new AssetManager();
	m_assetManager->Init(m_app->m_assetPath);

	InitRenderer();
}

void Engine::CreateWorldFromFile(const std::string& filename)
{
	if (m_world) {
		if (m_renderer) {
			delete m_renderer;
			m_renderer = nullptr;
		}
		delete m_world;
	}
	m_world = new World(m_app->MakeNodeFactory());

	auto json = AssetManager::GetOrCreateFromParentUri<JsonDocPod>(filename, "/");
	m_world->LoadAndPrepareWorld(json);
}

void Engine::InitRenderer()
{
	glfwInit();
	// WIP:

	m_renderer = new vk::VkRendererBase();
	m_renderer->SupportsEditor() ? ActivateEditor() : DeactivateEditor();

	// Vk instance and debug messenger
	m_renderer->CreateInstance(glfwutl::GetVulkanExtensions());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window
		= glfwCreateWindow(m_app->m_windowWidth, m_app->m_windowHeight, m_app->m_windowTitle.c_str(), nullptr, nullptr);

	glfwutl::SetupEventCallbacks(m_window);

	m_renderer->CreateSurface(m_window);

	if (m_isEditorEnabled) {
		m_editor = new Editor();
	}

	m_renderer->Init();
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
	return 1.f / std::max(Get().m_lastFrameTime, 0.0001f);
}

void Engine::ReportFrameDrawn()
{
	m_lastFrameTime = m_frameTimer.Get<ch::microseconds>() / 1e6f;
	m_frameTimer.Start();


	static bool hasFrameReport = false;

	if (!hasFrameReport) {
		LOG_WARN("Init to frame took: {} ms", m_initToFrameTimer.Get<ch::milliseconds>());
		hasFrameReport = true;
	}
}

void Engine::ToggleEditor()
{
	if (m_isEditorEnabled && m_renderer && m_renderer->SupportsEditor()) {
		m_isEditorActive ? DeactivateEditor() : ActivateEditor();
	}
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
	delete m_renderer;
	delete m_editor;
	delete m_world;
	glfwDestroyWindow(m_window);
	glfwTerminate();
	delete m_assetManager;
	delete m_input;
}
