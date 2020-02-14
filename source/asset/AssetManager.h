#pragma once

#include "asset/AssetPod.h"
#include "asset/UriLibrary.h"
#include "asset/PodHandle.h"
#include "reflection/PodReflection.h"
#include "system/Engine.h"
#include "system/Logger.h"
#include "system/console/ConsoleVariable.h"

#include <future>

// ASSET URI:
// Documentation of Asset URI convention: (WIP: After refactor is finished)
//
// 4 types of assets: (first letter of the URI categorizes the type)
//
// * Default assets uri: "~X". ---> where X goes the UID index of this type (for debugging ONLY).
//
// * Runtime generated assets uri: "#flatGenPath" ---> flatGenPath needs to be a flat path (Maybe these won't stay in
//     final design and use just interal uid and handles without paths)
//
// * External disk file paths. "C:/Absolute/Disk/Path/x.gltf" OR (decide design) "../../relative/directory/x.gltf" these
//     paths can access "any" filesystem path to import from the user computer. This type of path MAY use additional
//     JSON data at the end of the path to show sub assets. eg: "../model.gltf{"mat"="gold"}". This path is what is
//     saved at the metadata field: originalImportLocation
//
// * Internal pod assets use the old URI system starting with "/" but CANNOT use json data and MUST resolve to a file
//     asset with .pod or .json with proper header metadata.
//

struct PodDeleter {
	void operator()(AssetPod* p);
};

struct ImagePod;
struct TexturePod;
struct ShaderPod;
struct StringPod;

enum class PodDiskType
{
	Binary,
	Json
};

// This metadata is saved on disk as a header for the disk asset
struct PodMetaData {
	// This hash is the result from mti::GetHash<> and has the same limitations
	mti::Hash podTypeHash;

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


struct PodEntry {
	struct UnitializedPod {
	};

	std::unique_ptr<AssetPod, PodDeleter> ptr{};
	TypeId type{ refl::GetId<UnitializedPod>() };
	size_t uid{ 0 };
	uri::Uri path{ "#" };
	std::string name{};

	std::future<AssetPod*> futureLoaded;

	// WIP: optional, only our custom disk assets
	PodMetaData metadata;

	// static PodEntry Create(TypeId type, size_t uid, const uri::Uri& path)
	//{
	//	PodEntry entry;
	//	entry.type = type;
	//	entry.uid = uid;
	//	entry.path = path;
	//	return entry;
	//}

	// template<typename T>
	// static PodEntry Create(size_t uid, const uri::Uri& path)
	//{
	//	static_assert(std::is_base_of_v<AssetPod, T> && !std::is_same_v<AssetPod, T>, "PodEntry requires a pod type");
	//	return Create(refl::GetId<T>(), uid, path);
	//}

	template<typename T>
	T* UnsafeGet()
	{
		static_assert(
			std::is_base_of_v<AssetPod, T> && !std::is_same_v<AssetPod, T>, "Unsafe get called without a pod type");
		return static_cast<T*>(ptr.get());
	}
};


class AssetHandlerManager {
	friend class AssetImporterManager;
	friend class AssetFrontEndManager;

	static AssetHandlerManager& Get()
	{
		static AssetHandlerManager inst = AssetHandlerManager();
		return inst;
	}

	AssetHandlerManager() {}

	std::vector<std::unique_ptr<PodEntry>> m_pods;
	std::unordered_map<uri::Uri, size_t> m_pathCache;

public:
	static uri::Uri GetPodImportPath(BasePodHandle handle)
	{
		return Get().m_pods[handle.podId]->metadata.originalImportLocation;
	}

	template<CONC(CAssetPod) PodT>
	static PodEntry* CreateNew()
	{
		// WIP
		// Add entry here. Return entry to allow edits / w.e else.
		// Whoever "Creates" the asset should be responsible for filling in the metadat
		// (feels like a less decoupled instead of having multple CreateNewFromDisk etc)
		return CreateNew(mti::GetTypeId<PodT>());
	}

	static PodEntry* CreateNew(TypeId podType)
	{
		auto entry = CreateNewTypeless();
		entry->type = podType;
		entry->metadata.podTypeHash = podType.hash();
		return nullptr;
	}

	// Initialized fields:
	// entry->uid
	static PodEntry* CreateNewTypeless()
	{
		auto entryUnq = std::make_unique<PodEntry>();
		auto entry = entryUnq.get();
		entry->uid = Get().m_pods.size();

		Get().m_pods.push_back(std::move(entryUnq));

		return entry;
	}
};


namespace ed {
class AssetsWindow;
}
// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetImporterManager {
public:
private:
	std::unordered_map<uri::Uri, std::unique_ptr<AssetPod>> m_pathCache;
	std::unordered_map<size_t, std::unique_ptr<AssetPod>> m_pathCache;

	// Stores this session's known imported paths. Any ResolveOrImport that hits this just returns the handle
	std::unordered_map<uri::Uri, BasePodHandle> m_importedPathsCache;


public:
	//
	// PUBLIC ASSET IMPORTING INTERFACE
	//

	// Search in main AssetHandlerManager import paths to find this as an absolute path
	// if found return it,
	// otherwise load it right away and forward it in AssetHandlerManager
	// NOTE: this can make infinite recursion on circlar dependencies, do we handle this? (possible, needs extra
	// code)
	// NOTE: We only cache since the execution of the program
	template<CONC(CAssetPod) PodType>
	static PodHandle<PodType> ResolveOrImport(const fs::path& inFullPath)
	{
		auto inst = Engine::GetAssetImporterManager();
		auto [it, didInsert] = inst->m_importedPathsCache.try_emplace(inFullPath, PodHandle<PodType>{});

		if (!didInsert) {
			// TODO: type check this handle
			return PodHandle<PodType>{ it->second };
		}

		PodHandle<PodType> handle = inst->ImportFromDisk<PodType>(inFullPath);
		it->second = handle;
		return handle;
	}

	template<CONC(CAssetPod) PodType>
	static PodHandle<PodType> ResolveOrImportFromParentUri(const fs::path& path, const uri::Uri& parentUri)
	{
		auto parentDir = uri::GetDir(uri::GetDiskPathStrView(parentUri)); // Get parent directory. (Also removoes json)
		uri::Uri resolvedUri
			= (fs::path(parentDir) / path).string(); // add path (path may include json data at the end)

		return AssetImporterManager::ResolveOrImport<PodType>(resolvedUri);
	}

	template<CONC(CAssetPod) PodType>
	static PodHandle<PodType> ResolveOrImportFromParent(const fs::path& path, BasePodHandle parentHandle)
	{
		CLOG_ABORT(path.empty(), "Path was empty. Parent was: {}", AssetHandlerManager::GetPodImportPath(parentHandle));
		return ResolveOrImportFromParentUri<PodType>(path, AssetHandlerManager::GetPodImportPath(parentHandle));
	}


private:
	template<CONC(CAssetPod) PodType>
	PodHandle<PodType> ImportFromDisk(const fs::path& path)
	{
		PodEntry* entry = AssetHandlerManager::CreateNew<PodType>();
		entry->metadata.originalImportLocation = path;
	}


	template<CONC(CAssetPod) T>
	void TryImport(PodEntry* entry)
	{
		if (entry->metadata.originalImportLocation.empty()) {
			LOG_ERROR("Failed to import. No asset location"); // DOC: msg
			return;
		}

		if (entry->type.hash() != mti::GetHash<T>()) {
			LOG_ERROR("Failed to import. Type missmatch"); // DOC: msg
			return;
		}

		try {
			T::Load(entry); // LEFT HERE.
		} catch (std::exception& e) {
			LOG_ABORT("Failed to load: {} {} Exception:\n{}", refl::GetName<T>(), path, e.what());
		}
	}


	// template<typename T>
	// PodEntry* CreateAndRegister(const std::string& path)
	//{
	//	assert(m_pathCache.find(path) == m_pathCache.end() && "Path already exists");
	//	size_t uid = m_pods.size();

	//	auto& ptr = m_pods.emplace_back(std::make_unique<PodEntry>(PodEntry::Create<T>(uid, path)));

	//	PodEntry* entry = ptr.get();
	//	m_pathCache[path] = uid;
	//	entry->name = uri::GetFilename(path);
	//	PostRegisterEntry<T>(entry);
	//	return entry;
	//}


	// template<CONC(CAssetPod) PodT>
	// static PodT* GetFromPath(const uri::Uri& path)
	//{
	//	return PodCast<PodT>(m_pathCache.at(path).get());
	//}


	friend class Editor;
	friend class AssetWindow;
	friend class ed::AssetsWindow;

	ConsoleFunction<int32> serializeUid{ "save_pod" };
	ConsoleFunction<int32> deserilizeUid{ "load_pod" };


	PodEntry* GetEntryByPath(const uri::Uri& path)
	{
		auto it = m_pathCache.find(path);
		if (it == m_pathCache.end()) {
			return nullptr;
		}
		return m_pods.at(it->second).get();
	}

	template<typename T>
	void TryLoad(T* into, const std::string& path)
	{

		try {
			T::Load(into, path);
		} catch (std::exception& e) {
			LOG_ABORT("Failed to load: {} {} Exception:\n{}", refl::GetName<T>(), path, e.what());
		}
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
		TryLoad(ptr, entry->path);

		entry->ptr.reset(ptr);
	}

	// Specialized in a few cases where instant loading or multithreaded loading is faster
	template<typename T>
	void PostRegisterEntry(PodEntry* entry)
	{
	}


public:
	// Returns a new handle from a given path.
	// * if the asset of this path is already registered in the asset list it will return a handle to the existing
	// one
	//   requesting a different type of an existing path will assert.
	// * if the asset of this path is not registered, it will become registered and associated with this type.
	//   it will also begin loading on another thread.

	template<typename PodType>
	static PodHandle<PodType> GetOrCreate(const uri::Uri& inPath)
	{
		// If you hit this assert, it means the pod you are trying to create is not registered in engine pods.
		// To properly add a pod to the engine see AssetPod.h comments
		// TODO: Use concepts for pods
		static_assert(refl::IsValidPod<PodType>,
			"Attempting to create an invalid pod type. Did you forget to register a new pod type?");


		auto inst = Engine::GetAssetImporterManager();
		CLOG_ABORT(inPath.front() != '/', "Found non absolute uri {}", inPath);

		auto entry = inst->GetEntryByPath(inPath);

		if (entry) {
			CLOG_ABORT(entry->type != refl::GetId<PodType>(),
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
			return AssetImporterManager::GetOrCreate<PodType>(resolvedUri);
		}

		auto diskPart = uri::GetDiskPathStrView(parentUri); // remove parents possible json

		const auto cutIndex = diskPart.rfind('/') + 1;      // Preserve the last '/'
		diskPart.remove_suffix(diskPart.size() - cutIndex); // resolve parents directory

		resolvedUri = (fs::path(diskPart) / path).string(); // add path (path may include json data at the end)

		return AssetImporterManager::GetOrCreate<PodType>(resolvedUri);
	}

	template<typename PodType>
	static PodHandle<PodType> GetOrCreateFromParent(const fs::path& path, BasePodHandle parentHandle)
	{
		// Parent: /abc/model.gltf{mat..}
		// Given: bar/foo.jpg{abc}
		// Resulted: /abc/bar/foo.jpg{abc}
		CLOG_ABORT(path.empty(), "Path was empty. Parent was: {}", AssetImporterManager::GetPodUri(parentHandle));
		return GetOrCreateFromParentUri<PodType>(path, AssetImporterManager::GetPodUri(parentHandle));
	}

	static void SetPodName(const uri::Uri& path, const std::string& newPodName)
	{
		auto& podEntries = Engine::GetAssetImporterManager()->m_pods;
		auto it
			= std::find_if(begin(podEntries), end(podEntries), [&](auto& podEntry) { return podEntry->path == path; });

		if (it != podEntries.end()) {
			(*it)->name = newPodName;
		}
	}

	static PodEntry* GetEntry(BasePodHandle handle)
	{
		return Engine::GetAssetImporterManager()->m_pods[handle.podId].get();
	}


	// Refreshes the underlying data of the pod.
	template<typename PodType>
	static void Reload(PodHandle<PodType> handle)
	{
		auto inst = Engine::GetAssetImporterManager();
		inst->LoadEntry<PodType>(inst->m_pods[handle.podId].get());
	}

	// Frees the underlying cpu pod memory
	static void Unload(BasePodHandle handle) { Engine::GetAssetImporterManager()->m_pods[handle.podId]->ptr.reset(); }

	static uri::Uri GetPodUri(BasePodHandle handle)
	{
		return Engine::GetAssetImporterManager()->m_pods[handle.podId]->path;
	}

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

	void Init(const fs::path& assetPath);


	static void PreloadGltf(const std::string& modelPath);

private:
	// Specific pod specializations for loading:
	// Images and string pods are loaded async

	// Textures and shaders are instantly loaded.

	// Async load images.
	template<>
	void PostRegisterEntry<ImagePod>(PodEntry* entry);

	template<>
	void PostRegisterEntry<TexturePod>(PodEntry* entry);

	template<>
	void PostRegisterEntry<ShaderPod>(PodEntry* entry);

	template<>
	void PostRegisterEntry<StringPod>(PodEntry* entry);

	// Dummy Exporter for pod specialization
	void Z_SpecializationExporter();
};


class AssetFrontEndManager {
};
