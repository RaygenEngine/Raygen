#include "pch.h"
#include "SpirvReflector.h"

#include "engine/Logger.h"
#include "reflection/ReflEnum.h"
#include "assets/AssetRegistry.h"

#include <spirv-cross/spirv_cross.hpp>

#include <iostream>


using namespace spirv_cross;
using namespace shd;

VarType GetType(SPIRType& type)
{
	if (type.columns > 1 || type.vecsize > 1) {
		if (type.basetype != SPIRType::Float) {
			return VarType::Unsupported;
		}

		// Detect float type
		if (type.columns == 3 && type.vecsize == 3) {
			return VarType::Mat3;
		}
		else if (type.columns == 4 && type.vecsize == 4) {
			return VarType::Mat4;
		}
		else if (type.columns == 1 && type.vecsize == 2) {
			return VarType::Vec2;
		}
		else if (type.columns == 1 && type.vecsize == 3) {
			return VarType::Vec3;
		}
		else if (type.columns == 1 && type.vecsize == 4) {
			return VarType::Vec4;
		}
		return VarType::Unsupported;
	}

	switch (type.basetype) {
		case SPIRType::Struct: return VarType::Struct;
		case SPIRType::Float: return VarType::Float;
		case SPIRType::Int: return VarType::Int;
		case SPIRType::SampledImage: return VarType::Sampler2D;
	}

	return VarType::Unsupported;
}

size_t GetTypeSize(VarType type)
{
	switch (type) {
		case VarType::Int: return 4;
		case VarType::Sampler2D: return 0;
		case VarType::Float: return 4;
		case VarType::Vec2: return 4 * 4;
		case VarType::Vec3: return 4 * 4;
		case VarType::Vec4: return 4 * 4;
		case VarType::Mat3: return 4 * 12;
		case VarType::Mat4: return 4 * 4 * 4;
		case VarType::Struct: return 0;
	}
	return SIZE_MAX;
}

namespace {

void AnalyzeVariable(Compiler& comp, Variable& outVar, std::string name, TypeID base_type_id)
{
	auto type = comp.get_type(base_type_id);

	outVar.name = name;
	outVar.type = GetType(type);
	outVar.bytesWithPadding = GetTypeSize(outVar.type);
}

} // namespace

SpirvReflection SpirvReflector::Reflect(const std::vector<uint32>& code)
{
	SpirvReflection refl{};


	// TODO: avoid copy
	Compiler comp(code);
	ShaderResources resources = comp.get_shader_resources();

	auto populateVar = [&](Variable& var, const Resource& res) {
		AnalyzeVariable(comp, var, res.name, res.base_type_id);

		auto type = comp.get_type(res.base_type_id);
		if (type.basetype == SPIRType::Struct) {
			for (uint32 i = 0; i < type.member_types.size(); i++) {
				Variable memvar;
				AnalyzeVariable(comp, memvar, comp.get_member_name(res.base_type_id, i), type.member_types[i]);
				var.bytesWithPadding += memvar.bytesWithPadding;
			}
		}
	};

	for (const Resource& res : resources.uniform_buffers) {
		uint32 set = comp.get_decoration(res.id, spv::DecorationDescriptorSet);
		uint32 binding = comp.get_decoration(res.id, spv::DecorationBinding);

		DescriptorVariable var{};
		var.binding = binding;
		var.set = set;

		populateVar(var, res);

		refl.uboVariables.emplace_back(std::move(var));
	}

	for (const Resource& res : resources.sampled_images) {
		uint32 set = comp.get_decoration(res.id, spv::DecorationDescriptorSet);
		uint32 binding = comp.get_decoration(res.id, spv::DecorationBinding);

		DescriptorVariable var{};
		var.binding = binding;
		var.set = set;

		populateVar(var, res);

		refl.uboVariables.emplace_back(std::move(var));
	}

	for (const Resource& res : resources.stage_outputs) {
		auto loc = comp.get_decoration(res.id, spv::DecorationLocation);

		InoutVariable var{};
		var.location = loc;

		populateVar(var, res);

		refl.outVariables.emplace_back(std::move(var));
	}

	for (const Resource& res : resources.stage_inputs) {
		auto loc = comp.get_decoration(res.id, spv::DecorationLocation);

		InoutVariable var{};
		var.location = loc;

		populateVar(var, res);

		refl.inVariables.emplace_back(std::move(var));
	}


	CLOG_ABORT(resources.push_constant_buffers.size() > 1, "More than one push constant found");
	if (resources.push_constant_buffers.size() == 0) {
		return refl;
	}


	const Resource& res = resources.push_constant_buffers[0];


	Variable var{};
	populateVar(var, res);
	refl.pushConstant = var;
	return refl;
}
