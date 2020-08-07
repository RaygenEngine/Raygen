#include "pch.h"
#include "SceneSpotlight.h"

#include "rendering/wrappers/RDepthmap.h"

void SceneSpotlight::MaybeResizeShadowmap(uint32 width, uint32 height)
{
	bool shouldResize = true;
	if (shadowmap) {
		auto extent = shadowmap->attachment->GetExtent2D();
		shouldResize = width != extent.width || height != extent.height;
	}

	if (shouldResize) {
		shadowmap = std::make_unique<vl::RDepthmap>(width, height, name.c_str());
	}
}
