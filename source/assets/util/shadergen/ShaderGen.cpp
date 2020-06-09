#include "pch.h"
#include "ShaderGen.h"


// The type of the Ubo Member
enum class Type // Can be extended for matrices later
{
	Int,
	Float,
	Vec3,
	Vec4,
} type;

std::string shd::GenerateGBufferFrag(const DynamicDescriptorSetLayout& layout, const std::string& gbufferFragMain)
{
	std::stringstream ss;
	ss <<
		R"(/// Raygen: Auto Generated Shader Code
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

// out
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
// rgb: albedo, a: opacity
layout(location = 2) out vec4 gAlbedoOpacity;
// r: metallic, g: roughness, b: occlusion, a: occlusion strength
layout(location = 3) out vec4 gSpecular;
layout(location = 4) out vec4 gEmissive;

// in
layout(location = 0) in Data
{
	vec3 fragPos;
	vec2 uv;
	mat3 TBN;
};
)";
	ss << GenerateDescriptorSetCode(layout, layout.uboName, 0u);

	ss << "\n#line 1\n";
	ss << gbufferFragMain;

	return ss.str();
}

std::string shd::GenerateDescriptorSetCode(
	const DynamicDescriptorSetLayout& descriptorSetLayout, std::string_view uboName, uint32 setIndex)
{
	// PERF: Probably really slow
	std::stringstream ss;

	int32 binding = 0;
	ss << "\n#line 100001\n";
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
