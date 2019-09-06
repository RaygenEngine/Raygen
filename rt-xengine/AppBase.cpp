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
	LOG_FATAL("Running app: {}", m_name);

	std::unique_ptr<Window> window = CreateAppWindow();
	
	RegisterRenderers();
	
	// Init engine file system.
	if (!Engine::Get().InitDirectories(argv[0], m_assetPath))
	{
		LOG_FATAL("Failed to create Engine!");
		return -1;
	}
	
	std::unique_ptr<NodeFactory> factory = MakeNodeFactory();
	if (!Engine::Get().CreateWorldFromFile(m_initialScene, factory.get()))
	{
		LOG_FATAL("Failed to create World!");
		return -1;
	}

	// Create window
	if (!window)
	{
		LOG_FATAL("Failed to create Window!");
		return -1;
	}

	// Start the renderer
	if (!window->StartRenderer(0))
	{
		LOG_FATAL("Failed to create Renderer!");
		return -1;
	}

	window->Show();

	if (m_lockMouse) 
	{
		window->RestrictMouseMovement();
	}

	MainLoop(window.get());

	window->ReleaseMouseMovement();
	return 0;
}

void AppBase::MainLoop(Window* window)
{
	while (!window->IsClosed())
	{
		// clear input soft state (pressed keys, etc.)
		Engine::GetInput()->ClearSoftState();

		// Let our window handle any events.
		window->HandleEvents(m_handleControllers);

		// update world 
		Engine::GetWorld()->Update();
		// update renderer (also checks world updates, eg. camera/ entity moved, light color changed)
		
		Engine::GetRenderer()->Update();
		// render
		Engine::GetRenderer()->Render();
		Engine::GetRenderer()->SwapBuffers();
	}
}

void AppBase::RegisterRenderers() 
{
	// NOTE:
	// Default behavior for an app is to start the FIRST renderer registered here.
	Engine::Get().RegisterRenderer<OpenGL::GLTestRenderer>();
}

std::unique_ptr<Window> AppBase::CreateAppWindow()
{
	return std::move(Win32Window::CreateWin32Window(
		m_windowTitle, 150, 150, m_windowWidth, m_windowHeight
	));
}

std::unique_ptr<NodeFactory> AppBase::MakeNodeFactory()
{
	return std::make_unique<NodeFactory>(NodeFactory());
}


