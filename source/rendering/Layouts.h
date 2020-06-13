#pragma once
#include "rendering/objects/RDescriptorLayout.h"

#include <vulkan/vulkan.hpp>
#include <functional>

namespace vl {
struct RCompatibleRenderPass {
	vk::UniqueRenderPass immutableRenderPass;
	std::function<vk::UniqueRenderPass()> constructor;

public:
	void Initialize(std::function<vk::UniqueRenderPass()> constr)
	{
		constructor = constr;
		immutableRenderPass = constr();
	}

	const vk::RenderPass GetCompatiblePass() const { return *immutableRenderPass; }
	vk::UniqueRenderPass CreateNew() { return constructor(); }
};

inline struct Layouts_ {

	RDescriptorLayout regularMaterialDescLayout;
	RDescriptorLayout gBufferDescLayout;
	RDescriptorLayout singleUboDescLayout;
	RDescriptorLayout singleSamplerDescLayout;
	RDescriptorLayout cubemapLayout;
	RDescriptorLayout envmapLayout;

	RCompatibleRenderPass depthCompatibleRenderPass;

	Layouts_();


} * Layouts{};
} // namespace vl
