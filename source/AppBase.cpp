#include "pch/pch.h"
#include "AppBase.h"
#include "platform/windows/Win32Window.h"
#include "system/Engine.h"
#include "renderer/Renderer.h"
#include "world/World.h"
#include "editor/Editor.h"
#include "renderer/renderers/opengl/deferred/GLDeferredRenderer.h"
#include "renderer/renderers/opengl/forward/GLForwardRenderer.h"

AppBase::AppBase()
{
	m_name = "Default Engine";
	m_initialScene = "/scenes/test/test.xscn";
	m_assetPath = "assets";

	m_windowTitle = "Rayxen";

	m_windowWidth = 1920;
	m_windowHeight = 1080;

	m_handleControllers = false;
	m_lockMouse = false;

	m_argc = 1;
	m_argv = nullptr;

	m_enableEditor = true;
}

void AppBase::PreMainInit(int32 argc, char* argv[]) // NOLINT
{
	// Copy the arguments for later use.
	m_argc = argc;
	m_argv = argv;

	if (argc > 1) {
		m_initialScene = argv[1];
	}
}

int32 AppBase::Main(int32 argc, char* argv[]) // NOLINT
{
	LOG_FATAL("Running app: {}", m_name);

	Engine& engine = Engine::Get();

	engine.InitEngine(this);

	if (!engine.CreateWorldFromFile(m_initialScene)) {
		LOG_FATAL("Failed to create World!");
		return -1;
	}

	Window* window = Engine::GetMainWindow();
	// Allow world to update for the window before any renderer comes in play.
	window->FireFirstResizeEvent();

	// Start the renderer
	engine.SwitchRenderer(0);

	// Show the actuall window after the renderer has initialized.
	window->Show();

	if (m_lockMouse) {
		window->RestrictMouseMovement();
	}

	MainLoop();

	window->ReleaseMouseMovement();

	return 0;
}

void AppBase::MainLoop()
{
	Window* window = Engine::GetMainWindow();
	while (!window->IsClosed()) {

		if (Engine::IsEditorActive()) {
			Engine::GetEditor()->PreBeginFrame();
		}

		// clear input soft state (pressed keys, etc.)
		Engine::GetInput()->ClearSoftState();
		Engine::GetWorld()->ClearDirtyFlags();

		// Let our window handle any events.
		window->HandleEvents(m_handleControllers);

		if (Engine::GetInput()->IsKeyPressed(XVirtualKey::OEM_3)) {
			Engine::Get().ToggleEditor();
		}

		if (Engine::GetInput()->IsKeyPressed(XVirtualKey::TAB)) {
			Engine::Get().NextRenderer();
		}

		// update world
		Engine::GetWorld()->Update();
		// update renderer (also checks world updates, eg. camera/ entity moved, light color changed)

		Engine::GetRenderer()->Update();
		// render
		Engine::GetRenderer()->Render();
		Engine::GetRenderer()->SwapBuffers();


		Engine::Get().ReportFrameDrawn();
	}
}

void AppBase::RegisterRenderers()
{
	// NOTE:
	// Default behavior for an app is to start the FIRST renderer registered here.

	Engine::RegisterRenderer<OpenGL::GLForwardRenderer>();
	Engine::RegisterRenderer<OpenGL::GLDeferredRenderer>();
}

Win32Window* AppBase::CreateAppWindow()
{
	return Win32Window::CreateWin32Window(m_windowTitle, CW_USEDEFAULT, CW_USEDEFAULT, m_windowWidth, m_windowHeight);
}

NodeFactory* AppBase::MakeNodeFactory()
{
	return new NodeFactory();
}
