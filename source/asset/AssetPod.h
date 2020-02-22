#pragma once
#include "reflection/TypeId.h"
#include <concepts>

namespace uri {
using Uri = std::string;
}
struct PodEntry;

// ADDING PODS:
//
// To properly add a pod to the engine, you need to register it to the pod type system as well.
// This allows pods to remain "pods" in memory without virtual functions while allowing the engine to safely type cast
// them to generic pods and backwards.
//
// CHECKLIST: (Last updated at Raygen first pod reflection refactor 2020/2/10)
// 1. Create the struct in the header file (see an example pod eg: StringPod)
// 2. struct MUST include Load function
// 3. struct MUST be REFLECTED_POD (even if you reflect no members)
// 4. Add your header in PodIncludes.h
// 5. Add the new type in PodFwd.h ENGINE_POD_TYPES list
// 6. Forward declare your pod in PodFwd.h
// 7: Add the loader in PodLoaders.h

struct AssetPod {
	TypeId type;

	template<typename T>
	[[nodiscard]] bool IsOfType() const
	{
		static_assert(std::is_base_of_v<AssetPod, T>, "This check would always fail. T is not a child pod type");
		return type == refl::GetId<T>();
	};

protected:
	// Do not ever delete generic asset pod pointer
	AssetPod() = default;
	~AssetPod() = default;
	AssetPod(const AssetPod&) = delete;
	AssetPod(AssetPod&&) = delete;
	AssetPod& operator=(const AssetPod&) = delete;
	AssetPod& operator=(AssetPod&&) = delete;
};

struct BasePodHandle {
	size_t podId{ 0 };

	[[nodiscard]] bool HasBeenAssigned() const { return podId != 0; }

	[[nodiscard]] bool operator==(const BasePodHandle& other) const { return other.podId == this->podId && podId != 0; }
};

template<typename T>
concept CAssetPod = std::derived_from<T, AssetPod>;


enum class PodDiskType
{
	Binary,
	Json
};

// This metadata is saved on disk as a header for the disk asset
struct PodMetaData {
	// This hash is the result from mti::GetHash<> and has the same limitations
	mti::Hash podTypeHash{};

	// The original file that this asset got imported from. Allows us to "reimport" an asset.
	// This string a hybrid of the kaleido uri convention and can contain "meta" json data in it.
	// It is required if we want reimport for example a single material from a .gltf asset
	// This field can be empty
	std::string originalImportLocation;

	// Determines how should this asset wants be saved on disk. (binary/json)
	PodDiskType preferedDiskType{ PodDiskType::Binary };

	// This will overwrite the file under originalImportLocation with the result of the asset exporter (if an exporter
	// is available for this asset type)
	// This functionality allows us to "save" shader editing back to the original source file.
	bool exportOnSave{ false };

	// Reimport on load will reimport the original import file when the asset loads.
	// This can be usefull for debugging, external asset editing (eg: reimporting textures while editing) or even as a
	// general switch for updating asset versions or fixing corrupt assets.
	bool reimportOnLoad{ false };
};
