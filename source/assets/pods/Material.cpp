#include "pch.h"
#include "Material.h"

#include "assets/util/SpirvCompiler.h"
#include "assets/util/SpirvReflector.h"
#include "assets/PodEditor.h"
#include "reflection/ReflectionTools.h"

void MaterialArchetype::OnShaderUpdated()
{
	// Shader compilation error
	if (binary.empty()) {
		return;
	}

	auto newParams = SpirvReflector::ReflectArchetype(binary);

	// TODO: Refactor unq ptrs here, actual garbage
	auto newClass = newParams.GenerateClass();
	for (auto& instance : instances) {
		PodEditor editor(instance);
		editor->RegenerateUbo(classDescr.get(), *newClass);
		editor->RegenerateSamplers(parameters.samplers2d, newParams.samplers2d);
	}

	parameters = newParams;
	classDescr = std::move(newClass);
}

void MaterialInstance::RegenerateUbo(const RuntimeClass* oldClass, const RuntimeClass& newClass)
{
	if (!oldClass) {
		uboData.resize(newClass.GetSize());
		return;
	}
	std::vector<byte> newData(newClass.GetSize());
	// Attempt to match as many properties as possible, but don't really care about errors.
	// CHECK: Note that data here is zeroed out if not overwritten.
	// This is currently fine for all types supported in custom ubos.

	refltools::CopyClassToEx(uboData.data(), newData.data(), *oldClass, newClass);
	uboData.swap(newData);
}

void MaterialInstance::RegenerateSamplers(std::vector<std::string>& oldSamplers, std::vector<std::string>& newSamplers)
{
	// WIP: unfinished
	samplers2d.resize(newSamplers.size());
}
