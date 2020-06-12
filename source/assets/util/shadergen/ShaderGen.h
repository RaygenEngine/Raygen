#pragma once
#include "assets/pods/Material.h"

namespace shd {

// Format for a generic shader gen, ie all the parts required to generate a "whole" shader variant
// NOTE: Using something like this will affect performance
struct ShaderGenGeneric {
	// Code for in out variables
	std::string inOut;
	// Code for descriptor sets & samplers
	std::string descriptorSet;

	// Functions shared between multiple different shader variants
	std::string sharedFunctions;

	// Functions specific to this shader variant
	std::string specificFunctions;

	// code for "main" function
	std::string mainCode;
};


std::string GenerateGBufferFrag(const DynamicDescriptorSetLayout& layout, const std::string& gbufferFragMain);

std::string GenerateDescriptorSetCode(
	const DynamicDescriptorSetLayout& descriptorSetLayout, std::string_view uboName, uint32 setIndex = 0);

} // namespace shd
