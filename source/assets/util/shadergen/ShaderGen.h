#pragma once

struct DynamicDescriptorSetLayout;
namespace shd {
constexpr int32 c_errorLineModifier = 100000;

// Format for a generic shader gen, ie all the parts required to generate a "whole" shader variant
// NOTE: Using something like this will affect performance
struct ShaderGenGeneric {
	// Code for in out variables
	std::string inOut;
	// Code for descriptor sets & samplers
	std::string descriptorSet;

	// Functions shared between multiple different shader variants
	std::string sharedFunctions;

	// code for "main" function
	std::string mainCode;
};

std::string GenerateShaderGeneric(const std::string& inOutCode, const std::string& descSetCode,
	const std::string& sharedFunctions, const std::string& mainCode);


std::string GenerateGbufferFrag(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode);


std::string GenerateGbufferVert(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode);


std::string GenerateDepthFrag(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode);

std::string GenerateDepthVert(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode);

std::string GenerateUnlitFrag(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode);


std::string GenerateDescriptorSetCode(
	const DynamicDescriptorSetLayout& descriptorSetLayout, std::string_view uboName, uint32 setIndex = 0);

} // namespace shd
