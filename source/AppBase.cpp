#include "pch/pch.h"

#include "AppBase.h"
#include "editor/Editor.h"
#include "system/Engine.h"
#include "system/Input.h"
#include "system/Logger.h"
#include "world/NodeFactory.h"
#include "world/World.h"
#include "renderer/renderers/vulkan/VkSampleRenderer.h"
#include "system/profiler/ProfileScope.h"
#include <glfw/glfw3.h>

AppBase::AppBase()
{

	m_name = "Raygen Engine";
	m_initialScene = "engine-data/default.json";
	m_assetPath = "assets";

	m_windowTitle = "Raygen";

	m_windowWidth = 1920;
	m_windowHeight = 1080;

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

	WindowType* window = Engine::GetMainWindow();

	engine.CreateWorldFromFile(m_initialScene);

	// Allow for world to update any flags that became dirty since InitWorld to here. (eg: resize events, nodes added
	// later etc)
	Engine::GetWorld()->DirtyUpdateWorld();

	MainLoop();

	engine.DeinitEngine();

	return 0;
}

void AppBase::MainLoop()
{
	GLFWwindow* window = Engine::GetMainWindow();
	while (!glfwWindowShouldClose(Engine::GetMainWindow())) {
		PROFILE_SCOPE(System)
		if (Engine::IsEditorActive()) {
			Engine::GetEditor()->PreBeginFrame();
		}

		// clear input soft state (pressed keys, etc.)
		Engine::GetInput().Z_ClearFrameState();
		Engine::GetWorld()->ClearDirtyFlags();

		// WIP:
		// Let our window handle any events.
		glfwPollEvents();

		if (Engine::GetInput().IsJustPressed(Key::Tilde)) {
			Engine::Get().ToggleEditor();
		}

		Engine::GetWorld()->Update();

		Engine::GetRenderer()->DrawFrame();

		Engine::Get().ReportFrameDrawn();

		window = Engine::GetMainWindow();
	}
}

NodeFactory* AppBase::MakeNodeFactory()
{
	return new NodeFactory();
}
