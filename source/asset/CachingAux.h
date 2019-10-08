#pragma once

#include <tuple>
#include <unordered_map>
#include <memory>

// create multi key hashing maps: std::unordered_map<Key<Types...>, V> (e.g. std::unordered_map<int, bool>,
// optix::Geometry> !each type must have an std::hash<Type>()(item) and an operator== overload otherwise there will be a
// compiler error! to add additional supported types inject std::hash<newType>()(item) similarly to the way it is done
// below (also you need operator== overloading) otherwise break your types into primitives(do not use floating-point
// number keys) or don't use them as keys at all with a multi key hashing map you can create asset caches for each
// renderer, or even renderers that share the same rendering contexts

namespace CachingAux {
template<typename... Types>
struct Keys {
	std::tuple<Types...> items;
	explicit Keys(Types... args) { items = std::make_tuple(args...); }
	bool operator==(const Keys& other) const { return Compare(other.items); }

private:
	template<size_t I = 0>
	bool Compare(const std::tuple<Types...>& other) const
	{
		// compare each items for equality
		bool cmp = std::get<I>(items) == std::get<I>(other);
		if constexpr (I + 1 != sizeof...(Types))
			return cmp && Compare<I + 1>(other); // last tuple
		return cmp;
	}
};

// AssetType should derive from FileAsset
template<typename AssetType, typename... KeyTypes>
using MultiKeyAssetCache = std::unordered_map<Keys<KeyTypes...>, std::weak_ptr<AssetType>>;

// AssetType must derive from MapAssetType (MapAssetType from FileAsset)
template<typename AssetType, typename MapAssetType, typename... Args>
std::shared_ptr<AssetType> LoadAssetAtMultiKeyCache(
	MultiKeyAssetCache<MapAssetType, Args...>& cache, const std::string& description, Args... args)
{
	// static_assert(std::is_base_of<FileAsset, MapAssetType>::value);
	static_assert(std::is_base_of<MapAssetType, AssetType>::value);

	auto key = Keys(args...);
	const auto it = cache.find(key);

	constexpr std::size_t argc = sizeof...(Args);
	// TODO: create compile time const char*
	// constexpr char tp* =


	// if found placeholder
	if (it != cache.end()) {
		auto ptr = std::dynamic_pointer_cast<AssetType>(it->second.lock());
		// if is not loaded
		if (!ptr->IsLoaded()) {
			// if couldn't load
			if (!ptr->Load(args...)) {
				// LOG_WARN("Failed to load asset's data in memory, type: {}, args: " + tp, typeid(AssetType).name(),
				// args...);
				// remove dangling ptr
				cache.erase(key);
				return nullptr;
			}

			ptr->MarkLoaded();
		}

		LOG_TRACE("Cache hit!");
		// return cast: we may group some assets together (e.g disk asset)
		return ptr;
	}

	// placeholder not found, add placeholder and load asset

	LOG_TRACE("Cache size increased, size: {}", cache.size());

	// make asset, custom deleter to free asset cache from this asset (when ref count == 0)
	auto asset = std::shared_ptr<AssetType>(new AssetType(description), [key, &cache](AssetType* assetPtr) {
		LOG_DEBUG("Unregistering asset, {}", assetPtr);
		LOG_TRACE("Cache size decreased, size: {}", cache.size());
		cache.erase(key);

		delete assetPtr;
	});

	// check load state
	if (!asset->Load(args...)) {
		//	LOG_WARN(("Failed to load asset's data in memory, {}, args: " + tp).c_str(), asset.get(), args...);
		return nullptr;
	}

	// asset will be registered only if loaded successfully
	// LOG_DEBUG("Registering asset, {}, args: " tp, asset.get(), args...);


	// is loaded == true
	asset->MarkLoaded();

	// cache it
	cache[key] = asset;

	// return it
	return asset;
}
} // namespace CachingAux

// inject key hasher in std
namespace std {
using namespace CachingAux;

template<typename... Types>
struct hash<Keys<Types...>> {
	size_t operator()(const Keys<Types...>& k) const
	{
		auto v = Hash(17, k.items);
		return v;
	}

private:
	template<size_t I = 0>
	size_t Hash(size_t res, const tuple<Types...>& items) const
	{
		size_t sz = hash<typename tuple_element<I, tuple<Types...>>::type>()(get<I>(items));
		// http://stackoverflow.com/a/1646913/126995
		res = res * 31 + sz;
		if constexpr (I + 1 != sizeof...(Types))
			return Hash<I + 1>(res, items); // last tuple
		return res;
	}
};
} // namespace std
