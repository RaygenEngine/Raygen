#include "pch.h"
#include "App.h"

#include "editor/Editor.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/Logger.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "rendering/Rendering.h"
#include "universe/NodeFactory.h"
#include "universe/Universe.h"

#include <glfw/glfw3.h>

App::App()
{

	m_name = "Raygen Engine";
	m_initialScene = "engine-data/default.json";
	m_assetPath = "assets";

	m_windowTitle = "Raygen";

	m_windowWidth = 2304;
	m_windowHeight = 1296;

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
	Engine.InitEngine(this);

	Universe::LoadMainWorld(m_initialScene);

	MainLoop();

	Engine.DeinitEngine();

	return 0;
}

void App::MainLoop()
{
	while (!glfwWindowShouldClose(Platform::GetMainHandle())) {
		Profiler.BeginFrame();
		PROFILE_SCOPE(Engine);

		Universe::LoadPendingWorlds();

		Input.Z_ClearFrameState();
		Platform::PollEvents();

		Universe::GetMainWorld()->Update();
		Rendering::DrawFrame();

		Engine.ReportFrameDrawn();
	}
}

void App::WhileResizing()
{
	Universe::LoadPendingWorlds();
	Universe::GetMainWorld()->Update();

	Rendering::DrawFrame();
	Engine.ReportFrameDrawn();
}

NodeFactory* App::MakeNodeFactory()
{
	return new NodeFactory();
}
