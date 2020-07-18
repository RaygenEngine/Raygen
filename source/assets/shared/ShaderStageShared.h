#pragma once

enum class ShaderStageType
{
	Vertex,
	TessControl,
	TessEvaluation,
	Geometry,
	Fragment,
	Compute,
	RayGen,
	Intersect,
	AnyHit,
	ClosestHit,
	Miss,
	Callable
};


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
