#include "pch.h"
#include "SceneSpotlight.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"

void SceneSpotlight::ResizeShadowmap(uint32 width, uint32 height)
{
	shadowmap = std::make_unique<vl::RDepthmap>(width, height);
}
