#include "pch.h"
#include "PodExport.h"

#include "core/StringConversions.h"
#include "core/StringUtl.h"
#include "reflection/PodTools.h"
#include "reflection/ReflectionTools.h"


namespace podspec {
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
