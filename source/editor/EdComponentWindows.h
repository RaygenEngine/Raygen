#pragma once
#include "core/iterable/IterableSafeHashMap.h"
#include "core/iterable/IterableSafeVector.h"
#include "editor/windows/EdWindow.h"
#include "reflection/TypeId.h"
#include "assets/PodFwd.h"

#include <functional>
#include <unordered_set>
#include <vector>
#include <array>


namespace ed {
namespace detail {
	template<typename T, typename = void>
	struct HasCategory : std::false_type {
	};

	template<typename T>
	struct HasCategory<T, std::enable_if_t<std::is_same_v<decltype(T::Category), const char*>>> : std::true_type {
	};
} // namespace detail
// Responsible for handling and aggregating editor windows.
// There are 2 "types" of windows, 'singleton-like' windows where only one can be open at a time and multi windows (eg
// asset editors)
class ComponentWindows {
private:
	void OpenWindow(){};

public:
	struct UniqueWindowEntry {
		std::function<UniqueWindow*(const std::string&)> constructor;
		size_t hash;
		std::string name;
		const char* category{ nullptr };
	};

public:
	std::vector<UniqueWindowEntry> m_entries;
	std::set<const char*> m_categories;
	std::unordered_map<mti::Hash, size_t> m_entriesHash;

	IterableSafeHashMap<mti::Hash, UniqueWindow*> m_openUniqueWindows;
	std::unordered_map<mti::Hash, UniqueWindow*> m_closedUniqueWindows;

	IterableSafeHashMap<PodEntry*, UniquePtr<AssetEditorWindow>> m_assetWindows;


	template<UniqueWindowClass T>
	void AddWindowEntry(const std::string& name)
	{
		auto hash = mti::GetHash<T>();
		if (m_entriesHash.insert({ hash, m_entries.size() }).second) {

			UniqueWindowEntry entry{ [](const std::string& name) { return new T(name); }, hash, name };

			if constexpr (detail::HasCategory<T>::value) {
				entry.category = T::Category;
				m_categories.insert(T::Category);
			}

			m_entries.emplace_back(entry);
		}

		LoadWindowFromSettings(name, hash);
	};


	[[nodiscard]] bool IsUniqueOpen(mti::Hash hash) const noexcept { return m_openUniqueWindows.map.count(hash); };

	template<UniqueWindowClass T>
	[[nodiscard]] bool IsUniqueOpen() const noexcept
	{
		return IsUniqueOpen(mti::GetHash<T>());
	};


	void OpenUnique(mti::Hash hash);

	template<UniqueWindowClass T>
	void OpenUnique()
	{
		OpenUnique(mti::GetHash<T>());
	};


	void CloseUnique(mti::Hash hash);

	template<UniqueWindowClass T>
	void CloseUnique()
	{
		CloseUnique(mti::GetHash<T>());
	};


	void ToggleUnique(mti::Hash hash);

	template<UniqueWindowClass T>
	void ToggleUnique()
	{
		ToggleUnique(mti::GetHash<T>());
	};

	void Draw();

	~ComponentWindows();

	template<UniqueWindowClass T>
	T* GetUniqueWindow()
	{
		if (IsUniqueOpen<T>()) {
			return static_cast<T*>(m_openUniqueWindows.map.at(mti::GetHash<T>()));
		}

		ConstructUniqueIfNotExists(mti::GetHash<T>());
		return static_cast<T*>(m_closedUniqueWindows.at(mti::GetHash<T>()));
	}


	//
	// Assets section (should probably be a different class)
	//
	static constexpr size_t c_podWindowSize = GetPodTypesCount() + 1;
	std::array<std::function<UniquePtr<AssetEditorWindow>(PodEntry*)>, c_podWindowSize> m_podWindowConstructors;

	template<AssetEditorWindowClass T>
	void RegisterAssetWindowEditor()
	{
		using PodType = typename T::PodType;
		m_podWindowConstructors[GetDefaultPodUid<PodType>()] = [](PodEntry* entry) {
			return std::make_unique<T>(entry);
		};
	};


	void OpenAsset(PodEntry* entry);
	bool IsOpenAsset(PodEntry* entry) const;
	void CloseAsset(PodEntry* entry);


	void OpenAsset(BasePodHandle handle);
	void CloseAsset(BasePodHandle handle);


	UniquePtr<AssetEditorWindow> CreateAssetEditorWindow(PodEntry* entry);

private:
	void ConstructUniqueIfNotExists(mti::Hash hash);


	template<typename DrawFunc>
	void InternalDraw(DrawFunc&& func);

	void LoadWindowFromSettings(const std::string& name, mti::Hash hash);
	void UpdateSettingsForWindow(const std::string& name, bool currentIsOpen);
};
} // namespace ed
