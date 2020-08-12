#include "pch.h"
#include "SceneSpotlight.h"

#include "rendering/wrappers/RDepthmap.h"

void SceneSpotlight::MaybeResizeShadowmap(uint32 width, uint32 height)
{
	bool shouldResize = true;

	auto extent = shadowmap.at(0).attachment.extent;
	shouldResize = width != extent.width || height != extent.height;

	for (auto& sm : shadowmap) {
		if (shouldResize) {
			sm = vl::RDepthmap{ width, height, name.c_str() };
		}
	}
}
