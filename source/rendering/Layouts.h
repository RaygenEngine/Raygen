#pragma once
#include "rendering/objects/RDescriptorLayout.h"

#include <vulkan/vulkan.hpp>

namespace vl {
inline struct Layouts_ {

	RDescriptorLayout regularMaterialDescLayout;
	RDescriptorLayout gBufferDescLayout;
	RDescriptorLayout cameraDescLayout;
	RDescriptorLayout spotlightDescLayout;
	RDescriptorLayout cubemapLayout;
	RDescriptorLayout envmapLayout;

	Layouts_();


} * Layouts{};
} // namespace vl
