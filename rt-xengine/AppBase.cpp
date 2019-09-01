#include "pch.h"
#include "AppBase.h"
#include "platform/windows/Win32Window.h"
#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "world/World.h"

AppBase::AppBase()
{
	m_name = "Default Engine";
	m_initialScene = "test.xscn";
	m_assetPath = "assets";

	m_windowTitle = "RT-XEngine";

	m_windowWidth = 1920;
	m_windowHeight = 1080;

	m_handleControllers = false;
	m_lockMouse = false;
}

void AppBase::PreMainInit(int32 argc, char* argv[])
{
	if (argc > 1) 
	{
		m_initialScene = argv[1];
	}
}

int32 AppBase::Main(int32 argc, char* argv[])
{
	RT_XENGINE_LOG_FATAL("Running app: {}", m_name);

	std::unique_ptr<System::Engine> engine = CreateEngine();
	std::unique_ptr<Platform::Window> window = CreateAppWindow(engine.get());
	
	RegisterRenderers(engine.get());
	
	// Init engine file system.
	if (!engine->InitDirectories(argv[0], m_assetPath))
	{
		RT_XENGINE_LOG_FATAL("Failed to create Engine!");
		return -1;
	}
	
	std::unique_ptr<World::NodeFactory> factory = MakeNodeFactory();
	if (!engine->CreateWorldFromFile(m_initialScene, factory.get()))
	{
		RT_XENGINE_LOG_FATAL("Failed to create World!");
		return -1;
	}

	// Create window
	if (!window)
	{
		RT_XENGINE_LOG_FATAL("Failed to create Window!");
		return -1;
	}

	// Start the renderer
	if (!window->StartRenderer(0))
	{
		RT_XENGINE_LOG_FATAL("Failed to create Renderer!");
		return -1;
	}

	window->Show();

	if (m_lockMouse) 
	{
		window->RestrictMouseMovement();
	}

	MainLoop(engine.get(), window.get());

	window->ReleaseMouseMovement();
	return 0;
}

void AppBase::MainLoop(System::Engine* engine, Platform::Window* window)
{
	while (!window->IsClosed())
	{
		// clear input soft state (pressed keys, etc.)
		engine->GetInput().ClearSoftState();

		// Let our window handle any events.
		window->HandleEvents(m_handleControllers);

		// update world 
		engine->GetWorld()->Update();
		// update renderer (also checks world updates, eg. camera/ entity moved, light color changed)
		engine->GetRenderer()->Update();
		// render
		engine->GetRenderer()->Render();
		engine->GetRenderer()->SwapBuffers();
	}
}

void AppBase::RegisterRenderers(System::Engine* engine) 
{
	// NOTE:
	// Default behavior for an app is to start the FIRST renderer registered here.
	engine->RegisterRenderer<Renderer::OpenGL::GLTestRenderer>();
}

std::unique_ptr<System::Engine> AppBase::CreateEngine()
{
	return std::make_unique<System::Engine>();
}

std::unique_ptr<Platform::Window> AppBase::CreateAppWindow(System::Engine* engineRef)
{
	return std::move(Platform::Win32Window::CreateWin32Window(
		engineRef, m_windowTitle, 150, 150, m_windowWidth, m_windowHeight
	));
}

std::unique_ptr<World::NodeFactory> AppBase::MakeNodeFactory()
{
	return std::make_unique<World::NodeFactory>(World::NodeFactory());
}


