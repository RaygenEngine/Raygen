#include "ShaderImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/pods/Shader.h"
#include "assets/pods/ShaderHeader.h"
#include "assets/pods/ShaderStage.h"

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


BasePodHandle ShaderStageImporter::Import(const fs::path& path)
{
	auto&& [handle, pod] = AssetImporterManager->CreateEntry<ShaderStage>(path.generic_string(),
		path.filename().replace_extension().generic_string() + "_" + path.extension().string().substr(1), true, true);

	CompilePod(pod, path.generic_string());

	return handle;
}

void ShaderStageImporter::Reimport(PodEntry* intoEntry, const uri::Uri& uri)
{
	if (intoEntry->IsA<ShaderStage>()) {
		CompilePod(intoEntry->UnsafeGet<ShaderStage>(), uri);
	}
}

void ShaderStageImporter::CompilePod(ShaderStage* pod, const uri::Uri& uri)
{
	pod->code = str::rtrim(StringFromFile(uri), " \r\n") + "\n";
	pod->stage = shd::ExtToStage(uri::GetDiskExtension(uri));
	TextCompilerErrors errors;
	// PERF: Copying data, can pass by ref and avoid editing in compiler

	auto bincode = ShaderCompiler::Compile(pod->code, std::string(uri::GetFilename(uri)), &errors);

	if (bincode.size() > 0) {
		pod->binary.swap(bincode);
		//	pod->reflection = SpirvReflector::Reflect(pod->binary);
	}

	if (!errors.errors.empty()) {
		LOG_ERROR("Errors during shader imporing: {}", uri);
		for (auto& [key, val] : errors.errors) {
			LOG_ERROR("\nLine {:<3}: {}", key, val);
		}
	}
}

//
// Gen shader importer
//

BasePodHandle ShaderImporter::Import(const fs::path& path)
{
	auto&& [handle, pod] = AssetImporterManager->CreateEntry<Shader>(
		path.generic_string(), path.filename().replace_extension().string());

	fs::path tmpPath = path;


	auto loadStage = [&](PodHandle<ShaderStage>& stageRef, const char* ext) {
		tmpPath.replace_extension(ext);
		if (fs::exists(tmpPath)) {
			stageRef = AssetImporterManager->ImportRequest<ShaderStage>(tmpPath);
		}
	};

	loadStage(pod->vertex, ".vert");
	loadStage(pod->fragment, ".frag");

	loadStage(pod->rayGen, ".rgen");
	loadStage(pod->intersect, ".rint");
	loadStage(pod->anyHit, ".rahit");
	loadStage(pod->closestHit, ".rchit");
	loadStage(pod->miss, ".rmiss");

	loadStage(pod->callable, ".rcall");

	loadStage(pod->compute, ".comp");

	return handle;
}

BasePodHandle ShaderHeaderImporter::Import(const fs::path& path)
{
	auto&& [handle, pod] = AssetImporterManager->CreateEntry<ShaderHeader>(path.generic_string(),
		path.filename().replace_extension().generic_string() + "_" + path.extension().string().substr(1), true, true);

	pod->code = str::rtrim(StringFromFile(path.generic_string()), " \n\r") + "\n";
	return handle;
}

void ShaderHeaderImporter::Reimport(PodEntry* intoEntry, const uri::Uri& uri)
{
	if (intoEntry->IsA<ShaderHeader>()) {
		intoEntry->UnsafeGet<ShaderHeader>()->code = str::rtrim(StringFromFile(uri), " \n\r") + "\n";
	}
}
