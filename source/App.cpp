#include "pch.h"

#include "App.h"
#include "editor/Editor.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/Logger.h"
#include "world/NodeFactory.h"
#include "world/World.h"
#include "renderer/VulkanLayer.h"
#include "engine/profiler/ProfileScope.h"
#include <glfw/glfw3.h>

App::App()
{

	m_name = "Raygen Engine";
	m_initialScene = "engine-data/default.json";
	m_assetPath = "assets";

	m_windowTitle = "Raygen";

	m_windowWidth = 1920;
	m_windowHeight = 1080;

	m_argc = 1;
	m_argv = nullptr;
}

void App::PreMainInit(int32 argc, char* argv[]) // NOLINT
{
	// Copy the arguments for later use.
	m_argc = argc;
	m_argv = argv;

	if (argc > 1) {
		m_initialScene = argv[1];
	}
}

int32 App::Main(int32 argc, char* argv[]) // NOLINT
{
	LOG_INFO("Running app: {}", m_name);


	Engine.InitEngine(this);

	GLFWwindow* window = Engine.GetMainWindow();

	Engine.CreateWorldFromFile(m_initialScene);

	// Allow for world to update any flags that became dirty since InitWorld to here. (eg: resize events, nodes added
	// later etc)
	Engine.GetWorld()->DirtyUpdateWorld();

	MainLoop();


	VulkanLayer::quadDescriptorSet.reset(); // NEXT: This is here to log the proper error
	Engine.DeinitEngine();

	return 0;
}

void App::MainLoop()
{
	GLFWwindow* window = Engine.GetMainWindow();
	while (!glfwWindowShouldClose(Engine.GetMainWindow())) {
		Profiler.BeginFrame();

		PROFILE_SCOPE(Engine);
		Engine.GetEditor()->PreBeginFrame();

		// clear input soft state (pressed keys, etc.)
		Engine.GetInput().Z_ClearFrameState();
		Engine.GetWorld()->ClearDirtyFlags();

		// TODO: full update on window resize
		// Let our window handle any events.
		glfwPollEvents();

		Engine.GetWorld()->Update();

		// Engine.GetRenderer()->DrawFrame();
		VulkanLayer::DrawFrame();

		Engine.ReportFrameDrawn();

		window = Engine.GetMainWindow();
	}
}

NodeFactory* App::MakeNodeFactory()
{
	return new NodeFactory();
}
