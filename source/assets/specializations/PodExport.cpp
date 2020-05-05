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
