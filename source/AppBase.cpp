#include "pch/pch.h"

#include "AppBase.h"
#include "editor/Editor.h"
#include "platform/windows/Win32Window.h"
#include "renderer/renderers/opengl/deferred/GLDeferredRenderer.h"
#include "renderer/renderers/opengl/forward/GLForwardRenderer.h"
#include "renderer/renderers/opengl/dovr/GLDOVRRenderer.h"
#include "system/Engine.h"
#include "system/Input.h"
#include "system/Logger.h"
#include "world/NodeFactory.h"
#include "world/World.h"


AppBase::AppBase()
{
	m_name = "Rayxen Engine";
	m_initialScene = "engine-data/default.json";
	m_assetPath = "assets";

	m_windowTitle = "Rayxen";

	m_windowWidth = 1920;
	m_windowHeight = 1080;

	// Xinput controller queries are expensive. By default FAST_RELEASE auto disables thems.
	// You can change this to enable XInput controller support.
#if RXN_WITH_FEATURE(FAST_RELEASE)
	m_handleControllers = false;
#else
	m_handleControllers = true;
#endif
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
	LOG_INFO("Running app: {}", m_name);

	Engine& engine = Engine::Get();

	engine.InitEngine(this);

	engine.CreateWorldFromFile(m_initialScene);

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

	// Allow for world to update any flags that became dirty since InitWorld to here. (eg: resize events, nodes added
	// later etc)
	Engine::GetWorld()->DirtyUpdateWorld();

	MainLoop();

	window->ReleaseMouseMovement();

	engine.DeinitEngine();

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

		if (Engine::GetInput()->IsKeyPressed(XVirtualKey::TILDE)) {
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
	// Default behavior for an app is to start the first primary registered here.
	Engine::RegisterPrimaryRenderer<ogl::GLDeferredRenderer>();
	Engine::RegisterPrimaryRenderer<ogl::GLForwardRenderer>();

	// Non primary renderers are skipped when cycling through renderers but can be enabled from the editor menu
	Engine::RegisterRenderer<ogl::GLDOVRRenderer>();
}

Win32Window* AppBase::CreateAppWindow()
{
	return Win32Window::CreateWin32Window(m_windowTitle, CW_USEDEFAULT, CW_USEDEFAULT, m_windowWidth, m_windowHeight);
}

NodeFactory* AppBase::MakeNodeFactory()
{
	return new NodeFactory();
}
