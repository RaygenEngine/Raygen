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
} // namespace

namespace shd {
ShaderStage LoadAndCompileStage(const std::string& pathNoExt, const std::string& ext, TextCompilerErrors* outErrors)
{
	std::string path = pathNoExt + ext;

	ShaderStage stage;
	stage.code = StringFromFile(path);
	stage.binary = ShaderCompiler::Compile(stage.code, path, outErrors);
	if (stage.binary.size() > 0) {
		stage.reflection = SpirvReflector::Reflect(stage.binary);
	}
	return stage;
}

} // namespace shd


BasePodHandle ShaderImporter::Import(const fs::path& path)
{
	auto noExtPath = path;
	noExtPath.replace_extension();

	// TODO: Reimport on load
	auto& [handle, pod] = ImporterManager->CreateTransientEntryFromFile<Shader>(
		path.filename().replace_extension().string(), path.generic_string());

	pod->vert = shd::LoadAndCompileStage(noExtPath.string(), ".vert");
	pod->frag = shd::LoadAndCompileStage(noExtPath.string(), ".frag");

	return handle;
}
