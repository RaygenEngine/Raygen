#pragma once
#include "assets/AssetPod.h"
#include "reflection/GenMacros.h"

namespace shd {
enum class VarType
{
	Unsupported,
	Int,
	Sampler2D,
	Float,
	Vec2,
	Vec3,
	Vec4,
	Mat3,
	Mat4,
	// Special
	Struct,
};

struct Variable {
	VarType type{};
	size_t bytesWithPadding{};
	std::string name{};


	// Only used on structs
	std::vector<Variable> children;
};

struct InoutVariable : public Variable {
	uint32 location{ UINT32_MAX };
};

struct DescriptorVariable : public Variable {
	uint32 set{ UINT32_MAX };
	uint32 binding{ UINT32_MAX };
};
} // namespace shd

struct SpirvReflection {
	std::vector<shd::InoutVariable> inVariables;
	std::vector<shd::InoutVariable> outVariables;
	std::vector<shd::DescriptorVariable> uboVariables;
	std::optional<shd::Variable> pushConstant;
};

struct ShaderStage {
	SpirvReflection reflection;
	std::vector<uint32_t> binary;
};


struct Shader : public AssetPod {
	REFLECTED_POD(Shader) { REFLECT_ICON(FA_CODE); }

	ShaderStage frag;
	ShaderStage vert;
};
