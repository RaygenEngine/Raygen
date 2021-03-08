#include "Layer.h"

#include "assets/GpuAssetManager.h"
#include "editor/EditorObject.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "rendering/scene/Scene.h"

ConsoleFunction<> console_BuildAll{ "s.buildAll", []() { Layer->mainScene->BuildAll(); },
	"Builds all build-able scene nodes" };
// TODO: uncomment, change name
// ConsoleFunction<> console_BuildAS{ "s.buildTestAccelerationStructure", []() {},
//	"Builds a top level acceleration structure, for debugging purposes, todo: remove" };


Layer_::Layer_() {} // namespace vl

Layer_::~Layer_() {}

void Layer_::DrawFrame()
{
	PROFILE_SCOPE(Renderer);
}

void Layer_::ResetMainScene()
{
	delete mainScene;
	mainScene = new Scene();
}
