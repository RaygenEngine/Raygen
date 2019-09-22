#pragma once
#include <set>


class AssetWindow
{
	
	std::set<std::string> m_gltf;
	std::set<std::string> m_xscn;

	std::set<std::string> m_images;
public:
	struct PathDrop
	{
		std::string path;
		
	};

	void Init();
	void Draw();

	void DrawAsset(int32& n, const std::string& path);
};