#include "App.h"

#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "rendering/Layer.h"
#include "rendering/Rendering.h"
#include "universe/Universe.h"

#include <glfw/glfw3.h>

App_::App_()
{
	CLOG_ABORT(App, "Two app instances found.");
	App = this;
}

App_::~App_()
{
	App = nullptr;
}

void App_::PreMainInit(int32 argc_, char* argv_[])
{
	// Copy the arguments for later use.
	argc = argc_;
	argv = argv_;
}

int32 App_::Main(int32 argc_, char* argv_[])
{
	Engine.InitEngine(this);

	MainLoop();

	Engine.DeinitEngine();

	return 0;
}

void App_::MainLoop()
{
	while (!glfwWindowShouldClose(Platform::GetMainHandle())) {
		Profiler.BeginFrame();
		PROFILE_SCOPE(Engine);

		Universe::LoadPendingWorlds();

		Input.Z_ClearFrameState();
		Platform::PollEvents();

		Universe::MainWorld->UpdateWorld(Layer->mainScene);

		Rendering::DrawFrame();

		Engine.ReportFrameDrawn();
	}
}

void App_::WhileResizing()
{
	Universe::LoadPendingWorlds();
	Universe::MainWorld->UpdateWorld(Layer->mainScene);

	Rendering::DrawFrame();
	Engine.ReportFrameDrawn();
}
