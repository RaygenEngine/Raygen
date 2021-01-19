#include "ShaderGen.h"

#include "assets/util/DynamicDescriptorSet.h"

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
#version 460
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
layout (location = 0) out vec4 gSNormal;
layout (location = 1) out vec4 gGNormal;
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
    vec3 fragPos;
};

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
)",
		descSetCode, sharedFunctions, mainCode);
}

std::string shd::GenerateGbufferVert(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode)
{
	std::string main = mainCode;


	main += R"(
void main() {
	vec3 vertPos = position + OffsetPosition(textCoord);

	vec4 posWCS = push.modelMat * vec4(vertPos, 1.0);
	gl_Position = camera.viewProj * posWCS;
	fragPos = posWCS.xyz;
	
	uv = textCoord;

	vec3 newNormal = EditNormal(normal);
	vec3 newTangent = EditTangent(tangent);


	vec3 T = normalize(mat3(push.normalMat) * newTangent);
   	vec3 N = normalize(mat3(push.normalMat) * newNormal);

	// Gram-Schmidt process + cross product
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);

	TBN = mat3(T, B, N); 
}    
)";


	return GenerateShaderGeneric(R"(
// out

layout(location=0) out Data
{ 
	vec2 uv;
	mat3 TBN;
    vec3 fragPos;
};

// in

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 textCoord;

// uniforms

layout(push_constant) uniform PC {
	mat4 modelMat;
	mat4 normalMat;
} push;

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };

)",
		descSetCode, sharedFunctions, main);
}

std::string shd::GenerateDepthFrag(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode)
{
	return GenerateShaderGeneric(R"(
layout(location=0) in vec2 uv;
)",
		descSetCode, sharedFunctions, mainCode);
}

std::string shd::GenerateDepthVert(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode)
{
	std::string main = mainCode;


	main += R"(
void main() {
	vec3 vertPos = position + OffsetPosition(textCoord);

	gl_Position = push.mvp * vec4(vertPos, 1.0);
	uv = textCoord;
}    
)";

	return GenerateShaderGeneric(R"(
// out

layout(location = 0) out vec2 uv;

// in

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 textCoord;

layout(push_constant) uniform PC {
	mat4 mvp;
} push;

)",
		descSetCode, sharedFunctions, main);
}

std::string shd::GenerateUnlitFrag(
	const std::string& descSetCode, const std::string& sharedFunctions, const std::string& mainCode)
{
	return GenerateShaderGeneric(
		R"(
// out
layout(location = 0) out vec4 outColor;

// in
layout(location = 0) in Data
{
	vec2 uv;
	mat3 TBN;
    vec3 fragPos;
};

layout(set = 1, binding = 0) uniform UBO_Camera { Camera cam; };
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
