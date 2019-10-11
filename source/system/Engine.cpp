#include "pch/pch.h"

#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "world/World.h"
#include "renderer/Renderer.h"
#include "world/NodeFactory.h"
#include "platform/windows/Win32Window.h"
#include "AppBase.h"
#include "editor/Editor.h"

Engine::~Engine()
{
	// NOTE: It is REALLY important to remember the reverse order here
	delete m_renderer;
	delete m_editor;
	delete m_world;
	delete m_window;

	delete m_assetManager;
	delete m_input;
}

void Engine::InitEngine(AppBase* app)
{
	m_initToFrameTimer.Start();

	m_app = app;
	m_isEditorEnabled = app->m_enableEditor;

	m_input = new Input();
	m_assetManager = new AssetManager();

	m_assetManager->Init(m_app->m_assetPath);

	app->RegisterRenderers();

	m_window = app->CreateAppWindow();


	if (m_isEditorEnabled) {
		m_editor = new Editor();
	}
}

bool Engine::CreateWorldFromFile(const std::string& filename)
{
	if (m_world) {
		if (m_renderer) {
			delete m_renderer;
			m_renderer = nullptr;
		}
		delete m_world;
	}
	m_world = new World(m_app->MakeNodeFactory());

	auto json = AssetManager::GetOrCreate<JsonDocPod>(filename);
	return m_world->LoadAndPrepareWorld(json);
}

void Engine::SwitchRenderer(uint32 registrationIndex)
{
	Engine& eng = Engine::Get();

	if (!Engine::GetWorld()) {
		LOG_ERROR("Attempted to start a renderer without world.");
		return;
	}

	if (registrationIndex < 0 || registrationIndex >= eng.m_rendererRegistrations.size()) {
		LOG_WARN("Attempted to switch to incorrect renderer index: {} of total registered: {}", registrationIndex,
			eng.m_rendererRegistrations.size());
		return;
	}

	delete eng.m_renderer;


	eng.m_renderer = eng.m_rendererRegistrations[registrationIndex].Construct();

	LOG_REPORT("Switched to renderer: {}", eng.m_rendererRegistrations[registrationIndex].name);

	eng.m_isEditorActive = eng.m_renderer->SupportsEditor();

	eng.m_renderer->InitRendering(eng.m_window->GetHWND(), eng.m_window->GetHInstance());
	eng.m_renderer->InitScene();
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

void Engine::ToggleEditor()
{
	if (m_isEditorEnabled && m_renderer && m_renderer->SupportsEditor()) {
		m_isEditorActive = !m_isEditorActive;
	}
}
