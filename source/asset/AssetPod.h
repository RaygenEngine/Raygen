#pragma once

#include "reflection/GenMacros.h"

namespace uri {
using Uri = std::string;
}

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

	bool operator==(const BasePodHandle& other) const { return other.podId == this->podId && podId != 0; }
};
