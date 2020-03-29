#include "pch.h"
#include "SpirvReflector.h"

#include "engine/Logger.h"
#include <spirv-cross/spirv_cross.hpp>

SpirvReflection SpirvReflector::Reflect(const std::vector<uint32>& code)
{
	using namespace spirv_cross;

	// TODO: avoid copy
	Compiler comp(code);
	ShaderResources res = comp.get_shader_resources();


	return SpirvReflection();
}
