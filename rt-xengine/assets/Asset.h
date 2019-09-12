#pragma once
#include "system/reflection/Reflector.h"

#include <filesystem>

namespace fs = std::filesystem;

class Asset
{
public:
	AssetReflector m_reflector;

	fs::path GetUri() const { return m_uri; }
protected:
	Asset(const fs::path& uri)
		: m_uri(uri)
	{
		assert(!uri.string().empty());
	}
	virtual ~Asset() = default;

	fs::path m_uri;
	bool m_isLoaded{ false };

	virtual bool Load() = 0;
	virtual void Unload() = 0;
private:
	bool FriendLoad() { return Load(); }
	void FriendUnload() { Unload(); }

	friend class AssetManager;
};

class ReflectableAssetPod
{
	PodReflector m_reflector;
};

inline AssetReflector& GetReflector(Asset* object)
{
	return object->m_reflector;
}
