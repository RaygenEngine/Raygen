#include "pch.h"
#include "SceneSpotlight.h"

#include "rendering/wrappers/RDepthmap.h"

void SceneSpotlight::MaybeResizeShadowmap(uint32 width, uint32 height)
{
	auto extent = shadowmap->attachment->GetExtent2D();

	if (width != extent.width || height != extent.height) {
		shadowmap = std::make_unique<vl::RDepthmap>(width, height, name.c_str());
	}
}
