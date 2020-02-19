#pragma once

#include <imgui/imgui.h>
#include <set>
#include <map>


struct PodEntry;

class AssetWindow {
	std::map<std::string, std::filesystem::path> m_gltf;

	bool m_needsRefresh{ true };
	ImGuiTextFilter m_filter;

public:
	void ReloadCache();
	bool Draw(); // returns false if the window was closed

private:
	void DrawFileLibrary();
};
