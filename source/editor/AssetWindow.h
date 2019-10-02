#pragma once
#include <set>
#include <map>

class AssetWindow
{
	
	std::set<std::string> m_gltf;
	std::set<std::string> m_xscn;

	std::set<std::string> m_images;

public:

	void Init();
	void Draw();

private:
	void DrawFileLibrary();

	void DrawAssetLibrary();

	void DrawFileAsset(int32& n, const std::string& path);
};