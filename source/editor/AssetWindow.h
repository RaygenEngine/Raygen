#pragma once
#include <set>
#include <map>
//#include "system/reflection/PodReflection.h"


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