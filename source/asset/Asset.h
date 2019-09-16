#pragma once
#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"

#include <filesystem>
#include "system/Engine.h"

namespace fs = std::filesystem;

class Asset
{	
public:
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
	virtual void Allocate() = 0;
	virtual void Deallocate() = 0;
	
	
private:
	bool FriendLoad() { return Load(); }
	void FriendAllocate() { Allocate(); }
	void FriendDeallocate() { Deallocate(); }

	friend class AssetManager;
};

template<typename T>
class PodedAsset : public Asset
{
protected:
	using PodType = T;
	
	PodType* m_pod;

public:
	PodedAsset(const fs::path& uri)
		: Asset(uri),
	      m_pod(nullptr) {}
	~PodedAsset()
	{
		if (m_pod) delete m_pod;
	}

	void Allocate() override { m_pod = new PodType(); }
	
	void Deallocate() override
	{
		delete m_pod;
		m_pod = nullptr;
	}

	[[nodiscard]] PodType* GetPod() const { return m_pod; }
	
	friend class AssetManager;
};
