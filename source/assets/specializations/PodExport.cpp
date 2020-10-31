#include "PodExport.h"

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
	f << "//@ Shared Section:\n";
	f << src->sharedFunctions;
	f << "//@ Gbuffer Frag Section:\n";
	f << src->gbufferFragMain;
	f << "//@ Depthmap Pass Section:\n";
	f << src->depthShader;
	f << "//@ Gbuffer Vert Section:\n";
	f << src->gbufferVertMain;
	f << "//@ Unlit Frag Section:\n";
	f << src->unlitFragMain;
}

template<>
inline void ExportPod(MaterialInstance* src, const fs::path& path)
{
	src->Export(path);
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
