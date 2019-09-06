#pragma once

#include "system/Object.h"
#include "assets/PathSystem.h"

class AssetManager;

// asset works as a placeholder to the wrapped data
class Asset : public Object
{
	AssetManager* m_assetManager;
	
	bool m_loaded;

protected:

	// file path
	std::string m_directoryPath;
	std::string m_filePath;
	std::string m_fileName;
		
public:
	Asset(AssetManager* assetManager, const std::string& path)
		: m_assetManager(assetManager),
	      m_loaded(false),
		  m_directoryPath(PathSystem::GetParentPath(path)),
		  m_filePath(path),
		  m_fileName(PathSystem::GetNameWithExtension(path)) {}

	virtual ~Asset() = default;

	// clear asset's inner data
	// TODO: pure virtual, every asset must be able to clean itself (gpu or cpu)
	virtual void Clear();

	bool IsLoaded() const { return m_loaded; }

	// call this in case of successful loading
	void MarkLoaded();

	// clear only if loaded
	void Unload();

	AssetManager* GetAssetManager() const { return m_assetManager; }

	std::string GetFileName() const { return m_fileName; }
	std::string GetDirectory() const { return m_directoryPath; }
	std::string GetFilePath() const { return m_filePath; }
	
	void ToString(std::ostream& os) const override { os << "object-type: Asset, name: " << m_fileName; }
};
