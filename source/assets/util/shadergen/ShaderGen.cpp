#include "pch.h"
#include "ShaderGen.h"

#include "assets/pods/MaterialInstance.h"

#include <sstream>

// The type of the Ubo Member
enum class Type // Can be extended for matrices later
{
	Int,
	Float,
	Vec3,
	Vec4,
} type;

std::string shd::GenerateShaderGeneric(const std::string& inOutCode, const std::string& descSetCode,
	const std::string& sharedFunctions, const std::string& mainCode)
{
	std::stringstream ss;

	ss << "// Raygen: Auto Generated Shader Code";
	ss << R"(
#version 450
#extension GL_GOOGLE_include_directive : enable
)";

	ss << "\n#line 100001\n";
	ss << inOutCode;
	ss << "\n#line 200001\n";
	ss << descSetCode;
	ss << "\n#line 300001\n";
	ss << sharedFunctions;
	ss << "\n#line 1\n";
	ss << mainCode;

	return ss.str();
}

std::string shd::GenerateGbufferFrag(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode)
{
	return GenerateShaderGeneric(
		R"(
// out
layout(location = 1) out vec4 gNormal;
// rgb: albedo, a: opacity
layout(location = 2) out vec4 gAlbedoOpacity;
// r: metallic, g: roughness, b: reflectance, a: occlusion strength
layout(location = 3) out vec4 gSurface;
layout(location = 4) out vec4 gEmissive;

// in
layout(location = 0) in Data
{
	vec2 uv;
	mat3 TBN;
};

layout(set = 1, binding = 0) uniform UBO_Camera {
	vec3 position;
	float pad0;
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 viewInv;
	mat4 projInv;
	mat4 viewProjInv;
} camera;
)",
		descSetCode, sharedFunctions, mainCode);
}

std::string shd::GenerateDepthShader(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode)
{
	return GenerateShaderGeneric(R"(
layout(location=0) in vec2 uv;
)",
		descSetCode, sharedFunctions, mainCode);
}

std::string shd::GenerateDescriptorSetCode(
	const DynamicDescriptorSetLayout& descriptorSetLayout, std::string_view uboName, uint32 setIndex)
{
	// PERF: Probably really slow
	std::stringstream ss;

	int32 binding = 0;
	if (descriptorSetLayout.uboClass.GetProperties().size()) {
		ss << "layout(set = " << setIndex << ", binding = " << binding << ") uniform UBO_Dynamic" << setIndex << " {\n";
		binding++;
		for (auto& member : descriptorSetLayout.uboClass.GetProperties()) {
			if (member.GetType() != mti::GetTypeId<glm::vec4>()) {
				ss << "\t" << member.GetType().name() << " " << member.GetName() << ";\n";
			}
			else {
				ss << "\tvec4 " << member.GetName() << ";\n";
			}
		}
		ss << "} " << uboName << ";\n\n";
	}

	for (auto& sampler : descriptorSetLayout.samplers2d) {
		ss << "layout(set = " << setIndex << ", binding = " << binding++ << ") uniform sampler2D " << sampler << ";\n";
	}

	return ss.str();
}
