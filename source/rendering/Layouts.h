#pragma once
#include "rendering/objects/DescriptorLayout.h"

#include <vulkan/vulkan.hpp>

namespace vl {
inline struct Layouts_ {

	DescriptorLayout regularMaterialDescLayout;
	DescriptorLayout gBufferDescLayout;
	DescriptorLayout cameraDescLayout;
	DescriptorLayout spotlightDescLayout;
	DescriptorLayout cubemapLayout;
	DescriptorLayout envmapLayout;

	Layouts_();


} * Layouts{};
} // namespace vl
