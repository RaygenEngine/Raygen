#include "pch.h"

#include "system/Engine.h"
#include "asset/loaders/XMLDocLoader.h"
#include "asset/AssetManager.h"
#include "world/World.h"
#include "renderer/Renderer.h"
#include "world/NodeFactory.h"
#include "platform/windows/Win32Window.h"
#include "AppBase.h"
#include "editor/Editor.h"

// Leave this empty, use InitEngine.
// Having a different function here enables us to 'construct' engine with parameters.
Engine::Engine()
	: m_assetManager(nullptr)
	, m_world(nullptr)
	, m_renderer(nullptr)
	, m_window(nullptr)
	, m_input(nullptr)
{
}

Engine::~Engine()
{
	// NOTE: It is REALLY important to remember the reverse order here
	if (m_renderer)
		delete m_renderer;
	if (m_editor)
		delete m_editor;
	if (m_world)
		delete m_world;
	if (m_window)
		delete m_window;

	delete m_assetManager;
	delete m_input;
}

void Engine::InitEngine(AppBase* app)
{
	m_initToFrameTimer.Start();

	m_app = app;

	m_input = new Input();
	m_assetManager = new AssetManager();

	m_assetManager->Init(m_app->m_assetPath);

	app->RegisterRenderers();

	m_window = app->CreateAppWindow();

	if (app->m_enableEditor) {
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

	auto sceneXML = AssetManager::GetOrCreate<XMLDocPod>(filename);
	AssetManager::Reload(sceneXML);
	return m_world->LoadAndPrepareWorldFromXML(sceneXML);
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

bool Engine::ShouldUpdateWorld() const
{
	if (m_editor) {
		return m_editor->ShouldUpdateWorld();
	}
	return true;
}

bool Engine::IsUsingEditor() const
{
	// TODO: in the future disable editor with this flag.
	return m_editor != nullptr;
}
