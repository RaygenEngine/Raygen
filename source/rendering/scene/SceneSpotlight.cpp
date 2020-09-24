#include "pch.h"
#include "SceneSpotlight.h"

#include "rendering/structures/Depthmap.h"

void SceneSpotlight::MaybeResizeShadowmap(uint32 width, uint32 height)
{
	bool shouldResize = true;
	auto& extent = shadowmap.at(0).framebuffer.extent;
	shouldResize = width != extent.width || height != extent.height;

	for (auto& sm : shadowmap) {
		if (shouldResize) {
			sm = vl::Depthmap{ width, height, name.c_str() };
		}
	}
}
