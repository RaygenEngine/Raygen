#pragma once

#include <set>
#include <map>

struct PodEntry;

class AssetWindow {
	std::map<std::string, std::filesystem::path> m_gltf;

public:
	void Init();
	void Draw();

private:
	void DrawFileLibrary();
	void DrawFileAsset(int32& n, const std::string& path);
};
