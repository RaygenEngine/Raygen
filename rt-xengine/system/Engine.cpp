#include "pch.h"

#include "system/Engine.h"
#include "assets/other/xml/XMLDoc.h"
#include "assets/AssetManager.h"
#include "world/World.h"
#include "renderer/Renderer.h"
#include "world/NodeFactory.h"
#include "platform/windows/Win32Window.h"
#include "AppBase.h"

// Leave this empty, use InitEngine.
// Having a different function here enables us to 'construct' engine with parameters.
Engine::Engine()
	: m_assetManager(nullptr),
	m_world(nullptr),
	m_renderer(nullptr),
	m_window(nullptr),
	m_input(nullptr)
{}

Engine::~Engine()
{			
	// NOTE: It is REALLY important to remember the reverse order here
	if (m_renderer) delete m_renderer;
	if (m_world) delete m_world;
	if (m_window) delete m_window;

	delete m_assetManager;
	delete m_input;
}

void Engine::InitEngine(AppBase* app)
{
	m_app = app;

	m_input = new Input();
	m_assetManager = new AssetManager();
	
	m_assetManager->Init(app->m_argv[0], app->m_assetPath);

	app->RegisterRenderers();

	m_window = app->CreateAppWindow();
}

bool Engine::CreateWorldFromFile(const std::string& filename)
{
	if (m_world) 
	{
		delete m_world;
	}
	m_world = new World(m_app->MakeNodeFactory());

	// load scene file
	const auto sceneXML = m_assetManager->LoadXMLDocAsset(filename);

	return m_world->LoadAndPrepareWorldFromXML(sceneXML.get());
}

void Engine::SwitchRenderer(uint32 registrationIndex)
{
	Engine& eng = Engine::Get();

	if (!Engine::GetWorld()) 
	{
		LOG_ERROR("Attempted to start a renderer without world.");
		return;
	}

	if (registrationIndex < 0 || registrationIndex >= eng.m_rendererRegistrations.size())
	{
		LOG_WARN("Attempted to switch to incorrect renderer index: {} of total registered: {}", registrationIndex, eng.m_rendererRegistrations.size());
		return;
	}

	if (eng.m_renderer)
	{
		delete eng.m_renderer;
	}

	eng.m_renderer = eng.m_rendererRegistrations[registrationIndex].Construct();

	eng.m_renderer->InitRendering(eng.m_window->GetHWND(), eng.m_window->GetHInstance());
	eng.m_renderer->InitScene(eng.m_window->GetWidth(), eng.m_window->GetHeight());
}

void Engine::UnloadDiskAssets()
{
	m_assetManager->UnloadAssets();
}

bool Engine::HasCmdArgument(const std::string& argument)
{
	Engine& eng = Engine::Get();

	int32 argc = eng.m_app->m_argc;
	char** argv = eng.m_app->m_argv;
	for (int32 i = 0; i < argc; ++i) 
	{
		if (strcmp(argv[i], argument.c_str()))
		{
			return true;
		}
	}
	return false;
}
