#pragma once

#include <set>
#include <map>

struct PodEntry;

class AssetWindow {
	std::map<std::string, std::filesystem::path> m_gltf;

	bool m_needsRefresh{ true };

public:
	void ReloadCache();
	void Draw();

private:
	void DrawFileLibrary();
};
