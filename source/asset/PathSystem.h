#pragma once

#include <set>
#include <filesystem>

namespace fs = std::filesystem;
/*
class PathSystem
{
	//static bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);

	// assets root path
	fs::path m_assetsRootPath;

	std::unordered_map<std::string, std::string> m_fileCache;

public:
	PathSystem() = default;
	~PathSystem() = default;


	// Search UP recursively for path from current
	fs::path SearchPathUpRecursivelyFromCurrent(const fs::path & subPath);

	// Search DOWN recursively for path from current
	fs::path SearchPathDownRecursivelyFromPath(const fs::path & subPath, const fs::path & searchPath = {});

	// Unsafe, searches everywhere
	fs::path SearchAssetPath(const fs::path & asset);

	// Appends all filenames in the fileCache in outFiles. Costs O(m_fileCache.size())
	void GenerateFileListOfType(const fs::path & extension, std::set<std::string>& outFiles) const;

private:
	void CacheAssetFilenames();
};
*/