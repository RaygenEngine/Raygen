#include "pch.h"
#include "ShaderImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/util/SpirvCompiler.h"
#include "assets/util/SpirvReflector.h"

BasePodHandle ShaderImporter::Import(const fs::path& path)
{
	auto noExtPath = path;
	noExtPath.replace_extension();

	// TODO: Reimport on load
	auto& [handle, pod]
		= ImporterManager->CreateEntry<Shader>(path.generic_string(), path.filename().replace_extension().string());


	pod->vert.binary = ShaderCompiler::Compile(noExtPath.string() + ".vert");
	if (pod->vert.binary.size()) {
		pod->vert.reflection = SpirvReflector::Reflect(pod->vert.binary);
	}

	pod->frag.binary = ShaderCompiler::Compile(noExtPath.string() + ".frag");
	if (pod->frag.binary.size()) {
		pod->frag.reflection = SpirvReflector::Reflect(pod->frag.binary);
	}

	return handle;
}
