#pragma once
#include "reflection/TypeId.h"
#include <concepts>

namespace uri {
using Uri = std::string;
}

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
