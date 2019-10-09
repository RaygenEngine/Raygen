#pragma once

#include "system/EngineComponent.h"
#include "system/Engine.h"
#include "asset/AssetPod.h"
#include "core/reflection/PodReflection.h"
#include "asset/UriLibrary.h"
#include "asset/PodHandle.h"

#include <future>
#include <thread>

struct PodDeleter {
	void operator()(AssetPod* p);
};

struct PodEntry {
	struct UnitializedPod {
	};

	std::unique_ptr<AssetPod, PodDeleter> ptr{};
	TypeId type{ refl::GetId<UnitializedPod>() };
	size_t uid{ 0 };
	uri::Uri path{ "#" };
	std::string name{};

	std::future<AssetPod*> futureLoaded;

	static PodEntry Create(TypeId type, size_t uid, const uri::Uri& path)
	{
		PodEntry entry;
		entry.type = type;
		entry.uid = uid;
		entry.path = path;
		return entry;
	}

	template<typename T>
	static PodEntry Create(size_t uid, const uri::Uri& path)
	{
		static_assert(std::is_base_of_v<AssetPod, T> && !std::is_same_v<AssetPod, T>, "PodEntry requires a pod type");
		return Create(refl::GetId<T>(), uid, path);
	}

	template<typename T>
	T* UnsafeGet()
	{
		static_assert(
			std::is_base_of_v<AssetPod, T> && !std::is_same_v<AssetPod, T>, "Unsafe get called without a pod type");
		return static_cast<T*>(ptr.get());
	}
};

// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetManager {
	friend class Editor;
	friend class AssetWindow;

	std::vector<std::unique_ptr<PodEntry>> m_pods;
	std::unordered_map<uri::Uri, size_t> m_pathCache;

	PodEntry* GetEntryByPath(const uri::Uri& path)
	{
		auto it = m_pathCache.find(path);
		if (it == m_pathCache.end()) {
			return nullptr;
		}
		return m_pods.at(it->second).get();
	}

	template<typename T>
	void LoadEntry(PodEntry* entry)
	{
		// If we have future data, use them directly.
		if (entry->futureLoaded.valid()) {
			entry->ptr.reset(entry->futureLoaded.get());
			return;
		}
		T* ptr = new T();
		bool loaded = T::Load(ptr, entry->path);
		CLOG_ASSERT(!loaded, "Failed to load: {} {}", entry->type.name(), entry->path);
		entry->ptr.reset(ptr);
	}


	// Specialized in a few cases where instant loading or multithreaded loading is faster
	template<typename T>
	void PostRegisterEntry(PodEntry* p)
	{
	}

	template<typename T>
	PodEntry* CreateAndRegister(const std::string& path)
	{
		assert(m_pathCache.find(path) == m_pathCache.end() && "Path already exists");
		size_t uid = m_pods.size();

		auto& ptr = m_pods.emplace_back(std::make_unique<PodEntry>(PodEntry::Create<T>(uid, path)));

		PodEntry* entry = ptr.get();
		m_pathCache[path] = uid;
		entry->name = uri::GetFilename(path);
		PostRegisterEntry<T>(entry);
		return entry;
	}

public:
	// Returns a new handle from a given path.
	// * if the asset of this path is already registered in the asset list it will return a handle to the existing one
	//   requesting a different type of an existing path will (TODO: ) assert. (possibly should return invalid handle)
	// * if the asset of this path is not registered, it will become registered and associated with this type.
	//   it will also begin loading on another thread.

	template<typename PodType>
	static PodHandle<PodType> GetOrCreate(const uri::Uri& inPath)
	{
		auto inst = Engine::GetAssetManager();
		CLOG_ASSERT(inPath.front() != '/', "Found non absolute uri {}", inPath);

		auto entry = inst->GetEntryByPath(inPath);

		if (entry) {
			CLOG_ASSERT(entry->type != refl::GetId<PodType>(),
				"Incorrect pod type on GetOrCreate:\nPath: '{}'\nPrev Type: '{}' New type: '{}'", inPath,
				entry->type.name(), refl::GetName<PodType>());
		}
		else {
			entry = inst->CreateAndRegister<PodType>(inPath);
		}

		PodHandle<PodType> p;
		p.podId = entry->uid;
		return p;
	}

	template<typename PodType>
	static PodHandle<PodType> GetOrCreateFromParentUri(const fs::path& path, const uri::Uri parentUri)
	{
		uri::Uri resolvedUri;

		if (path.c_str()[0] == '/') {
			resolvedUri = path.string();
			return AssetManager::GetOrCreate<PodType>(resolvedUri);
		}

		auto diskPart = uri::GetDiskPathStrView(parentUri); // remove parents possible json

		const auto cutIndex = diskPart.rfind('/') + 1;      // Preserve the last '/'
		diskPart.remove_suffix(diskPart.size() - cutIndex); // resolve parents directory

		resolvedUri = (fs::path(diskPart) / path).string(); // add path (path may include json data at the end)

		return AssetManager::GetOrCreate<PodType>(resolvedUri);
	}

	template<typename PodType>
	static PodHandle<PodType> GetOrCreateFromParent(const fs::path& path, BasePodHandle parentHandle)
	{
		// Parent: /abc/model.gltf{mat..}
		// Given: bar/foo.jpg{abc}
		// Resulted: /abc/bar/foo.jpg{abc}
		CLOG_ASSERT(path.empty(), "Path was empty. Parent was: {}", AssetManager::GetPodUri(parentHandle));
		return GetOrCreateFromParentUri<PodType>(path, AssetManager::GetPodUri(parentHandle));
	}

	static void SetPodName(const uri::Uri& path, const std::string& newPodName)
	{
		auto& podEntries = Engine::GetAssetManager()->m_pods;
		auto it
			= std::find_if(begin(podEntries), end(podEntries), [&](auto& podEntry) { return podEntry->path == path; });

		if (it != podEntries.end()) {
			(*it)->name = newPodName;
		}
	}

	static PodEntry* GetEntry(BasePodHandle handle) { return Engine::GetAssetManager()->m_pods[handle.podId].get(); }


	// Refreshes the underlying data of the pod.
	template<typename PodType>
	static void Reload(PodHandle<PodType> handle)
	{
		auto inst = Engine::GetAssetManager();
		inst->LoadEntry<PodType>(inst->m_pods[handle.podId].get());
	}

	// Frees the underlying cpu pod memory
	static void Unload(BasePodHandle handle) { Engine::GetAssetManager()->m_pods[handle.podId]->ptr.reset(); }

	static uri::Uri GetPodUri(BasePodHandle handle) { return Engine::GetAssetManager()->m_pods[handle.podId]->path; }

	// For Internal Handle use only.
	template<typename PodType>
	PodType* _Handle_AccessPod(size_t podId)
	{
		PodEntry* entry = m_pods[podId].get();

		assert(entry->type
			   == refl::GetId<PodType>()); // Technically this should never hit because we check during creation.

		if (!entry->ptr) {
			LoadEntry<PodType>(entry);
		}

		return entry->UnsafeGet<PodType>();
	}

	bool Init(const fs::path& assetPath);


	static void PreloadGltf(const std::string& modelPath);

private:
	// Specific pod specializations for loading:
	// Images and string pods are loaded async

	// Textures and shaders are instantly loaded.

	// Async load images.
	template<>
	void PostRegisterEntry<ImagePod>(PodEntry* entry)
	{
		entry->futureLoaded = std::async(std::launch::async, [entry]() -> AssetPod* {
			ImagePod* ptr = new ImagePod();
			ImagePod::Load(ptr, entry->path);
			return ptr;
		});
	}

	template<>
	void PostRegisterEntry<TexturePod>(PodEntry* entry)
	{
		TexturePod* pod = new TexturePod();
		TexturePod::Load(pod, entry->path);
		entry->ptr.reset(pod);
	}

	template<>
	void PostRegisterEntry<ShaderPod>(PodEntry* entry)
	{
		ShaderPod* pod = new ShaderPod();
		ShaderPod::Load(pod, entry->path);
		entry->ptr.reset(pod);
	}

	template<>
	void PostRegisterEntry<StringPod>(PodEntry* entry)
	{
		entry->futureLoaded = std::async(std::launch::async, [entry]() -> AssetPod* {
			StringPod* ptr = new StringPod();
			StringPod::Load(ptr, entry->path);
			return ptr;
		});
	}
};
