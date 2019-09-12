#pragma once

#include <unordered_set>
#include <filesystem>

namespace fs = std::filesystem;

class PathSystem
{
	// assets root path
	fs::path m_assetsRootPath;

public:
	PathSystem() = default;
	~PathSystem() = default;

	bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);

	// Search UP recursively for path from current
	fs::path SearchPathUpRecursivelyFromCurrent(const fs::path& subPath);

	// Search DOWN recursively for path from current
	fs::path SearchPathDownRecursivelyFromPath(const fs::path& subPath, const fs::path& searchPath = {});

	// Unsafe, searches everywhere
	fs::path SearchAssetPath(const fs::path& asset);
};
