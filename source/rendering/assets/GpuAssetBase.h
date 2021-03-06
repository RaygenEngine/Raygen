#pragma once
#include "assets/AssetUpdateInfo.h"

namespace vl {

// Generic base for a gpu asset
struct GpuAssetBase {
	size_t podUid{ 0 };
	virtual ~GpuAssetBase() = default;

	GpuAssetBase(BasePodHandle handle)
		: podUid(handle.uid){};

	GpuAssetBase(const GpuAssetBase&) = delete;
	GpuAssetBase& operator=(const GpuAssetBase&) = delete;

	GpuAssetBase(GpuAssetBase&&) = default;
	GpuAssetBase& operator=(GpuAssetBase&&) = default;

	virtual void Update(const AssetUpdateInfo& info) {}

	[[nodiscard]] std::vector<size_t> GetUsers();
	[[nodiscard]] const std::vector<size_t>& GetDependencies() const { return dependencies; }

protected:
	// Adds "this" asset as a user of the "paramAsset".
	// ie, we will be notified when paramAsset changes.
	void AddDependency(BasePodHandle paramAsset);

	template<typename... Handles>
	void AddDependencies(Handles... handle)
	{

		static_assert(
			(std::is_base_of_v<BasePodHandle, Handles> && ...), "Expects BasePodHandle, PodHandle<>, GpuHandle<>");
		(AddDependency(handle), ...);
	}

	// CHECK: Probably there is a concept for this
	template<typename Iterable>
	void AddDependencies(const Iterable& iterable)
	{
		for (auto& entry : iterable) {
			AddDependency(entry);
		}
	}


	void ClearDependencies();

	// This is synced and reverse of what is saved in the gpu asset manager.
	// ie Saves all dependencies of this asset towards other assets.
	// These vectors should be always kept in sync and are only used as duplicate data for performance reasons
	std::vector<size_t> dependencies;
};

// Templated base for a gpu asset
template<CAssetPod PodType>
struct GpuAssetTemplate : public GpuAssetBase {
	PodHandle<PodType> podHandle;

	GpuAssetTemplate(BasePodHandle handle)
		: GpuAssetBase(handle)
		, podHandle(handle)
	{
		// We should check the underlying pod type here of the handle but that requires an asset manager include
	}
};


} // namespace vl
