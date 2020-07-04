#include "pch.h"
#include "PodExport.h"

#include "core/StringConversions.h"
#include "core/StringUtl.h"
#include "reflection/PodTools.h"
#include "reflection/ReflectionTools.h"


namespace podspec {

template<>
inline void ExportPod(ShaderStage* src, const fs::path& path)
{
	std::ofstream f(path);
	f << src->code;
}


template<>
inline void ExportPod(MaterialArchetype* src, const fs::path& path)
{
	std::ofstream f(path);
	f << "// Raygen exported generated shader backup\n";
	f << "//@ UBO Section:\n";
	f << src->descriptorSetLayout.GetUniformText().str();
	f << "\n//@ Shared Section:\n";
	f << src->sharedFunctions;
	f << "\n//@ Gbuffer Frag Section:\n";
	f << src->gbufferFragMain;
	f << "\n//@ Depthmap Pass Section:\n";
	f << src->depthShader;
	f << "\n//@ Gbuffer Vert Section:\n";
	f << src->gbufferVertMain;
}


void ExportToDisk(PodEntry* entry, const fs::path& inPath)
{
	fs::path path = inPath;
	if (path.empty()) {
		auto entryPath = entry->path;
		if (str::stripIfStartsWithInsensitive(entryPath, "gen-data")) {
			path = fs::absolute("export-data" + entryPath);
		}
		else {
			path = fs::absolute(fs::path("export-data") / entry->path);
		}
	}

	auto v = [&]<typename T>(T* pod) {
		ExportPod<T>(pod, path);
	};

	podtools::VisitPod(entry->ptr.get(), v);
}
} // namespace podspec
