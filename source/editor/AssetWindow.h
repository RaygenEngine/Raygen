#pragma once
#include <set>
#include <map>
struct PodEntry;

class AssetWindow
{
	
	std::set<std::string> m_gltf;
	std::set<std::string> m_xscn;

	std::set<std::string> m_images;

	bool recurse{ true };

	std::set<PodEntry*> m_openFiles;
	std::set<PodEntry*> m_openFilesRemove;
public:

	void Init();
	void Draw();

private:
	void DrawFileLibrary();

	void DrawAssetLibrary();

	void DrawFileAsset(int32& n, const std::string& path);

	void DrawEditor(PodEntry* entry);
};