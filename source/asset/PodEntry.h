#pragma once
#include "asset/AssetPod.h"
#include "asset/PodHandle.h"

class ReflClass;

struct PodDeleter {
	void operator()(AssetPod* p);
};

struct PodEntry {
	struct UnitializedPod {
	};

	UniquePtr<AssetPod, PodDeleter> ptr{};
	TypeId type{ refl::GetId<UnitializedPod>() };
	size_t uid{ 0 };
	uri::Uri path{};

	std::string name{};

	// Mark pods as transient ones when they are just used for importing (or are generated) and "file-like" operations
	// like save and reimport are not allowed on them. eg: GltfFilePod, default constructed empty pods
	bool transient{ true };

	// Only usefull if transient is false
	bool requiresSave{ false };

	// Only usefull if transient is false
	PodMetaData metadata;


	template<typename T>
	T* UnsafeGet()
	{
		static_assert(
			std::is_base_of_v<AssetPod, T> && !std::is_same_v<AssetPod, T>, "Unsafe get called without a pod type");
		return static_cast<T*>(ptr.get());
	}

	template<CONC(CAssetPod) T>
	PodHandle<T> GetHandleAs()
	{
		CLOG_ABORT(type != mti::GetTypeId<T>(), "Entry->GetAs() Cast failure");
		return PodHandle<T>{ uid };
	}

	void MarkSave() { requiresSave = true; }

	// Prefer this from .name; .name will get deprecated in the future
	[[nodiscard]] std::string_view GetName() const { return name; }

	[[nodiscard]] const std::string& GetNameStr() const { return name; }

	//
	// Refl Class Section (optimisation to avoid GetClass for each pod)
	//
	[[nodiscard]] const ReflClass* GetClass() const { return podClass; };

	// Used by importers and serializers
	[[nodiscard]] void Z_AssignClass(const ReflClass* cl) { podClass = cl; }

private:
	const ReflClass* podClass{ nullptr };
};

template<typename T>
size_t ToAssetUid(T t)
{
	return t;
};

template<>
inline size_t ToAssetUid<size_t>(size_t t)
{
	return t;
}

template<>
inline size_t ToAssetUid<BasePodHandle>(BasePodHandle handle)
{
	return handle.podId;
}

template<>
inline size_t ToAssetUid<PodEntry*>(PodEntry* entry)
{
	return entry->uid;
}

template<typename T>
concept CUidConvertible = requires(T a)
{
	{
		ToAssetUid(a)
	}
	->std::convertible_to<size_t>;
};
