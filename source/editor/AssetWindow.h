#pragma once
#include <set>
#include <map>

struct AssetPod;

class AssetWindow
{
	
	std::set<std::string> m_gltf;
	std::set<std::string> m_xscn;

	std::set<std::string> m_images;


	std::unordered_map<size_t, std::string> m_knownPodTypes;

	std::unordered_map<size_t, std::vector<AssetPod*>> m_podLists;

public:
	struct PathDrop
	{
		std::string path;
		
	};

	void Init();
	void Draw();

private:
	void DrawAssetPod(AssetPod* pod);

	void DrawFileLibrary();

	void DrawAssetLibrary();

	void DrawFileAsset(int32& n, const std::string& path);

	void DetectPodCategory(AssetPod* pod);
};