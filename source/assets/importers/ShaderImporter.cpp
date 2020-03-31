#include "pch.h"
#include "ShaderImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/util/SpirvCompiler.h"
#include "assets/util/SpirvReflector.h"
#include <fstream>

namespace {
std::string StringFromFile(const std::string& path)
{
	std::ifstream t(path);
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	std::string buffer(size, ' ');
	t.seekg(0);
	t.read(&buffer[0], size);
	return buffer;
}

ShaderStage LoadAndCompileStage(const std::string& pathNoExt, const std::string& ext)
{
	std::string path = pathNoExt + ext;

	ShaderStage stage;
	stage.code = StringFromFile(path);
	stage.binary = ShaderCompiler::Compile(stage.code, path);
	stage.reflection = SpirvReflector::Reflect(stage.binary);
	return stage;
}

} // namespace


BasePodHandle ShaderImporter::Import(const fs::path& path)
{
	auto noExtPath = path;
	noExtPath.replace_extension();

	// TODO: Reimport on load
	auto& [handle, pod]
		= ImporterManager->CreateEntry<Shader>(path.generic_string(), path.filename().replace_extension().string());

	pod->vert = LoadAndCompileStage(noExtPath.string(), ".vert");
	pod->frag = LoadAndCompileStage(noExtPath.string(), ".frag");

	return handle;
}
