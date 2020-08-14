#include "pch.h"
#include "ShaderImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/pods/Shader.h"
#include "assets/pods/ShaderStage.h"
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

std::string rtrim(const std::string& s)
{
	size_t end = s.find_last_not_of(' ');
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}


BasePodHandle ShaderStageImporter::Import(const fs::path& path)
{
	auto& [handle, pod] = AssetImporterManager->CreateEntry<ShaderStage>(path.generic_string(),
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
	pod->code = rtrim(StringFromFile(uri));
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
	auto& [handle, pod] = AssetImporterManager->CreateEntry<Shader>(
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

	return handle;
}
