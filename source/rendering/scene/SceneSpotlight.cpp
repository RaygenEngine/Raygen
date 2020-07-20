#include "pch.h"
#include "SceneSpotlight.h"

#include "rendering/wrappers/RDepthmap.h"

void SceneSpotlight::ResizeShadowmap(uint32 width, uint32 height)
{
	shadowmap = std::make_unique<vl::RDepthmap>(width, height, name.c_str());
}
