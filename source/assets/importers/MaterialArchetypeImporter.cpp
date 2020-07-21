#include "pch.h"
#include "MaterialArchetypeImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/pods/MaterialArchetype.h"
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

// Views in vector only live for as long as "input" param is valid

std::vector<std::string_view> splitOnToken(const std::string& input, std::string_view token)
{
	std::vector<std::string_view> results;

	size_t pos = input.find(token);

	const char* lastTokenEnd = input.c_str();

	std::string_view remainingView = input;

	while (pos != std::string::npos) {
		results.emplace_back(std::string_view(lastTokenEnd, pos));
		lastTokenEnd += pos + token.size();
		remainingView = { lastTokenEnd };
		pos = remainingView.find(token);
	}

	results.emplace_back(remainingView);
	return results;
}
std::string rtrim(const std::string& s)
{
	size_t end = s.find_last_not_of(' ');
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string_view trimLine(std::string_view s)
{
	size_t first = s.find_first_of('\n');
	return (first == std::string::npos) ? "" : s.substr(first + 1);
}
} // namespace


BasePodHandle MaterialArchetypeImporter::Import(const fs::path& path)
{
	auto& [handle, pod] = AssetImporterManager->CreateEntry<MaterialArchetype>(
		path.generic_string(), "ARCH_" + path.filename().replace_extension().generic_string(), false, true);


	std::string content = StringFromFile(path.generic_string());

	auto segments = splitOnToken(content, "//@");

	CLOG_WARN(segments.size() != 7,
		"MaterialArchetypeImporter: Segments found in {} not 7. Due to the current (unstable) implementation of the "
		"importer it is recommended to always have all the segments in the correct order in the archetype file.",
		path);

	// TODO: for now just import in the exporter order. (should do automatic export/import based on reflection of
	// matarchetype)


	for (auto& seg : segments) {
		seg = trimLine(seg);
	}

	// Segment[0] should be ubo:

	auto uboLines = str::split(segments[1], "\r\n");

	TextCompilerErrors errors;
	auto descSetLayout = ShaderCompiler::GenerateUboClass(uboLines, &errors);

	if (!errors.wasSuccessful) {
		LOG_ERROR("MaterialArchetypeImporter: Errors while importing ubo for {}", path.generic_string());
		std::string errorstr;
		for (auto& [line, error] : errors.errors) {
			errorstr = fmt::format("{}\nLine {}: {}", errorstr, line, error);
		}
		LOG_ERROR("{}", errorstr);
		return handle;
	}

	int32 currentSegIndex = 2;

	auto importNextSegmentInto = [&](std::string& str, std::string_view segmentName) {
		if (currentSegIndex >= segments.size()) {
			LOG_WARN("{}: Missing ubo segment: {}", path.generic_string(), segmentName);
			return;
		}
		auto& seg = segments[currentSegIndex++];
		str = std::string(seg);
	};


	// Should match the exporter
	importNextSegmentInto(pod->sharedFunctions, "Shared Section");
	importNextSegmentInto(pod->gbufferFragMain, "Gbuffer Frag");
	importNextSegmentInto(pod->depthShader, "Depthmap Frag");
	importNextSegmentInto(pod->gbufferVertMain, "Displacement");
	importNextSegmentInto(pod->unlitFragMain, "Unlit Frag");

	shd::GeneratedShaderErrors genErrors;

	pod->CompileAll(std::move(descSetLayout), genErrors);

	{
		if (genErrors.editorErrors.size() > 0) {
			CLOG_ERROR(genErrors.editorErrors.size() != 1,
				"MaterialArchetypeImporter: Compile Errors while importing {}", path.generic_string());

			for (auto& [type, errors] : genErrors.editorErrors) {
				for (auto& [line, error] : errors.errors) {
					// TODO: ShaderGen should allow all shader stages to compile without errors
					if (!(type.starts_with("Vertex") && error.find("OffsetPosition") != std::string::npos)) {
						LOG_ERROR("{}: Line {}: {}", type, line, error);
					}
				}
			}
		}
	}

	return handle;
}
