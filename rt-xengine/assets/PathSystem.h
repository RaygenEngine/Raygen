#pragma once

#include <unordered_set>
#include <filesystem>

class PathSystem
{

	// assets root path
	std::string m_assetsRootPath;
	std::vector<std::string> m_assetsPaths;

	std::string m_shadersRootPath;
	std::vector<std::string> m_shadersPaths;
	std::unordered_set<std::string> m_shaderExtensions = { ".ptx", ".frag", ".vert" };

	std::string m_scenesRootPath;
	std::vector<std::string> m_scenesPaths;
	std::unordered_set<std::string> m_sceneExtensions = { ".xscn", ".gltf", ".glb", ".bin", ".jpg", ".jpeg", ".png", ".tga" };

	inline bool SetCurrentDir(const std::string& path) const;
	void GatherSubDirectories(const std::string& directoryPath, std::vector<std::string>& dirList) const;

	// searches in shaders directories
	std::string SearchAssetInShadersDirectories(const std::string& relativeAssetPath) const;

	// searches in scenes directories
	std::string SearchAssetInScenesDirectories(const std::string& relativeAssetPath) const;

	// searches everywhere
	std::string SearchAssetInAssetsDirectories(const std::string& relativeAssetPath, const std::string& absoluteHintMask = "") const;

public:
	PathSystem() = default;
	~PathSystem() = default;

	bool Init(const std::string& applicationPath, const std::string& dataDirectoryName);

	// search asset
	std::filesystem::path SearchAsset(const std::string& relativeAssetPath, const std::string& pathHint = "") const;
};
