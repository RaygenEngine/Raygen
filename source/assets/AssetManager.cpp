#include "AssetManager.h"

#include "assets/PodIncludes.h"
#include "assets/Serialization.h"
#include "assets/ShaderRegistry.h" // TODO: Remove
#include "assets/StdAssets.h"
#include "assets/specializations/PodDuplication.h"
#include "assets/specializations/PodExport.h"
#include "assets/util/FindPodUsers.h"
#include "engine/Timer.h"
#include "reflection/PodTools.h"

#include <set>
#include <thread>

ConsoleFunction<> cons_saveAll{ "a.saveAll", []() { AssetRegistry::SaveAll(); },
	"Saves all currently unsaved assets." };

ConsoleFunction<> cons_reimportAllShaders{ "a.shaders.reload",
	[]() {
		for (auto& entry : AssetRegistry::Z_GetPods()) {
			if (entry->IsA<ShaderStage>()) {
				AssetRegistry::ReimportFromOriginal(entry->GetHandleAs<ShaderStage>());
				AssetRegistry::RequestGpuUpdateFor(entry->uid, {});
			}
		}
	},
	"Reimports and recompiles all currently loaded ShaderStages." };

ConsoleFunction<> cons_reimportRtShaders{ "a.shaders.reloadRaytracingShaders",
	[]() {
		for (auto& entry : AssetRegistry::Z_GetPods()) {
			if (entry->IsA<ShaderStage>()) {
				auto stage = entry->GetHandleAs<ShaderStage>().Lock()->stage;
				if (stage == ShaderStageType::ClosestHit || stage == ShaderStageType::AnyHit
					|| stage == ShaderStageType::RayGen || stage == ShaderStageType::Miss
					|| stage == ShaderStageType::Callable) {
					AssetRegistry::ReimportFromOriginal(entry->GetHandleAs<ShaderStage>());
					AssetRegistry::RequestGpuUpdateFor(entry->uid, {});
				}
			}
		}
	},
	"Reimports and recompiles all rgen, rchit, rmiss loaded ShaderStages." };

void PodDeleter::operator()(AssetPod* p)
{
	auto l = [](auto* pod) {
		static_assert(!std::is_same_v<decltype(pod), AssetPod*>,
			"This should not ever instantiate with AssetPod*. Pod tools has internal error.");
		delete pod;
	};
	podtools::VisitPod(p, l);
}

void AssetRegistry::ExportToLocationImpl(PodEntry* entry, const fs::path& path)
{
	podspec::ExportToDisk(entry, path);
	LOG_REPORT("Exported: {} to {}", entry->path, path);
}

void AssetRegistry::SaveToDiskInternal(PodEntry* entry)
{
	auto& meta = entry->metadata;
	if (meta.exportOnSave) {
		podspec::ExportToDisk(entry, entry->metadata.originalImportLocation);
	}

	// CHECK: starts_with('~') is probably an old hack, entry->transient should be enough. (debug it)
	if (entry->path.size() <= 1 || entry->path.starts_with('~') || !entry->requiresSave || entry->transient) {
		return;
	}
	CLOG_ABORT(!entry, "Attempting to save null entry");
	CLOG_ABORT(!entry->ptr, "Attempting to save unloaded asset");


	fs::path path = entry->path;
	SerializePodToBinary(entry->metadata, entry->ptr.get(), path.replace_extension(".bin"));


	entry->requiresSave = false;
	LOG_INFO("Saved pod at: {}", entry->path);
}

void AssetRegistry::LoadAllPodsInDirectory(const fs::path& path)
{
	auto addPodFromPath = [&](const fs::path& path) {
		auto key = fs::relative(path).replace_extension().generic_string();
		size_t uid = m_pods.size();
		m_pathCache.emplace(key, uid);

		PodEntry e;
		e.uid = uid;
		e.path = std::move(key);
		e.requiresSave = false;
		m_pods.emplace_back(std::make_unique<PodEntry>(std::move(e)));
		assetdetail::podAccessor.emplace_back(nullptr);
		return uid;
	};


	TIMER_SCOPE("Total load & cache gen-data");
	size_t beginUid = m_pods.size();
	size_t jobCount = std::max(std::thread::hardware_concurrency(), static_cast<uint>(2));

	std::vector<size_t> threadSpecificUid;
	{
		// For performance, we try to load pods in the root path directory (gen-data) in different threads.
		// These will usually be geometry files which are slow to load currently.
		std::set<fs::path> rootAssets;

		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_directory() || entry.path().extension() != ".bin") {
				continue;
			}
			rootAssets.insert(entry.path());

			threadSpecificUid.emplace_back(addPodFromPath(entry.path()));

			if (rootAssets.size() == jobCount) {
				break;
			}
		}

		for (const auto& entry : fs::recursive_directory_iterator(path)) {
			if (entry.is_directory()) {
				continue;
			}


			if (entry.path().extension() == ".bin") {
				if (rootAssets.contains(entry.path())) {
					continue;
				}

				addPodFromPath(entry.path());
			}
		}
	}

	LOG_REPORT("Found {} binary pods.", m_pods.size() - beginUid);

	{
		// Offset by our special load uids
		beginUid += threadSpecificUid.size();

		size_t podsToLoad = m_pods.size() - beginUid;
		size_t podsPerJob
			= static_cast<size_t>(std::ceil(static_cast<float>(podsToLoad) / static_cast<float>(jobCount)));

		std::vector<std::vector<PodEntry*>> reimportEntries;
		reimportEntries.resize(jobCount);

		auto loadRange = [&](size_t start, size_t stop, size_t threadIndex) {
			size_t extraUid = threadSpecificUid.size() > threadIndex ? threadSpecificUid[threadIndex] : 0;

			if (extraUid > 0) {
				LoadFromDiskTypelessInternal(m_pods[extraUid].get());

				if (m_pods[extraUid].get()->metadata.reimportOnLoad) [[unlikely]] {
					// reimportEntries[threadIndex].push_back(m_pods[i].get());
					AssetRegistry::ReimportFromOriginal(m_pods[extraUid].get());
				}
			}

			stop = std::min(stop, m_pods.size());
			for (size_t i = start; i < stop; ++i) {
				LoadFromDiskTypelessInternal(m_pods[i].get());

				if (m_pods[i].get()->metadata.reimportOnLoad) [[unlikely]] {
					// reimportEntries[threadIndex].push_back(m_pods[i].get());
					AssetRegistry::ReimportFromOriginal(m_pods[i].get());
				}
			}
			return true;
		};


		std::vector<std::thread> results;

		size_t index = 0;
		for (size_t i = 0; i < podsToLoad; i += podsPerJob) {
			size_t from = i + beginUid;
			size_t to = i + beginUid + podsPerJob;
			results.push_back(std::thread(loadRange, from, to, index));
			index++;
		}

		for (auto& r : results) {
			r.join();
		}

		// for (auto& reimportVec : reimportEntries) {
		//	for (auto& reimportEntry : reimportVec) {
		//		AssetHandlerManager::ReimportFromOriginal(reimportEntry);
		//	}
		//}
	}

	for (auto& pod : m_pods) {
		if (!pod->metadata.originalImportLocation.empty()) {
			RegisterImportPathCache(pod.get());
		}
	}

	for (auto& pod : m_pods) {
		if (pod->IsA<ShaderStage>() && !pod->transient) {
			ShaderRegistry::OnEdited({ pod->uid });
		}
	}
}

void AssetRegistry::ReimportFromOriginalInternal(PodEntry* entry)
{
	AssetImporterManager->m_importerRegistry.ReimportEntry(entry);
}


void AssetRegistry::LoadFromDiskTypelessInternal(PodEntry* entry)
{
	DeserializePodFromBinary(entry);
}

void AssetRegistry::RenameEntryImpl(PodEntry* entry, const std::string_view newFullPath)
{
	if (entry->transient) {
		LOG_ERROR("Renaming transient pod entry. Aborted rename.");
		return;
	}
	if (str::equalInsensitive(entry->path, newFullPath)) {
		return;
	}

	// PERF: Logic here deletes and resaves the asset. We can actually just use fs::move probably to save performance
	// especially when moving big assets, or save a lot of time if moving folders.
	DeleteFromDisk(entry);

	RemoveFromPathCache(entry);
	entry->path = SuggestPathImpl(uri::Uri(newFullPath));
	entry->name = uri::GetFilename(entry->path);
	RegisterPathCache(entry);

	entry->MarkSave();

	for (auto& e : FindAssetUsersOfPod(entry)) {
		e->MarkSave();
	}
}

void AssetRegistry::DeleteFromDiskInternal(PodEntry* entry)
{
	CLOG_WARN(entry->transient, "Attempting to delete transient pod entry: {} {}", entry->path, entry->uid);

	fs::path path = entry->path;
	path.replace_extension(".bin");
	if (fs::is_regular_file(path)) {
		std::error_code er;
		if (!fs::remove(path, er)) {
			LOG_WARN("Failed to remove file: {}: {}", path, er.message());
		}
	}
}

PodEntry* AssetRegistry::DuplicateImpl(PodEntry* entry)
{
	// For transient pods we should require more params when duplicating (like a real name). Also all transient
	// duplications should in theory only be called directly by code and not caused indirectly by the user.
	CLOG_WARN(entry->transient, "Using non transient pod duplicate with a transient entry: {}", entry->path);

	PodEntry* result = nullptr;
	podtools::VisitPodHash(entry->GetClass()->GetTypeId().hash(), [&]<typename PodType> {
		auto&& [newEntry, newPod]
			= CreateEntry<PodType>(entry->path, false, entry->metadata.originalImportLocation, false, false);
		podspec::Duplicate(entry->ptr.get(), newPod, entry, newEntry);
		result = newEntry;
	});

	// CHECK: hack, used to force an address for this function
	Debug(0);
	return result;
}

PathReferenceType AssetRegistry::GenerateRelativeExportPath(
	const fs::path& exporteePath, BasePodHandle dependantAsset, fs::path& outPath)
{
	fs::path depPath = AssetRegistry::GetPodImportPath(dependantAsset);

	if (depPath.empty()) {
		LOG_ERROR("Exporting at: {}. Relative asset: {} does not have an export location.", exporteePath,
			GetPodUri(dependantAsset));

		outPath = AssetRegistry::GetPodUri(dependantAsset);
		return PathReferenceType::BinaryAsset;
	}
	return detail::GenerateExportDependencyPath(exporteePath, fs::absolute(depPath), outPath);
}

void AssetRegistry::GenerateRelativeExportJsonObject(
	nlohmann::json& json, const fs::path& exporteePath, BasePodHandle dependantAsset)
{
	fs::path path;
	auto result = GenerateRelativeExportPath(exporteePath, dependantAsset, path);

	auto enumTie = GenMetaEnum(result);
	auto str = std::string(enumTie.GetValueStr());
	json[str] = path.generic_string();
}

namespace detail {
namespace {
	bool IsSubDirectoryOf(const fs::path& of, const fs::path& who)
	{
		return std::equal(of.begin(), of.end(), who.begin());
	}
} // namespace
PathReferenceType GenerateExportDependencyPath(
	const fs::path& exporteePath, const fs::path& dependencyPath, fs::path& outPath)
{
	auto exporteeDir = fs::absolute(exporteePath).parent_path();
	auto dependencyDir = fs::absolute(dependencyPath).parent_path();

	// Spec case 1:
	if (exporteeDir == dependencyDir) {
		outPath = dependencyPath.filename();
		return PathReferenceType::FileRelative;
	}
	auto cwd = fs::current_path();
	auto cwdStr = fs::current_path();
	const bool isExporteeInCwd = IsSubDirectoryOf(cwd, exporteeDir);
	const bool isDependencyInCwd = IsSubDirectoryOf(cwd, dependencyDir);


	// Case 2:
	if (isExporteeInCwd == isDependencyInCwd && IsSubDirectoryOf(exporteeDir, dependencyDir)) {
		outPath = fs::relative(dependencyPath, exporteeDir);
		return PathReferenceType::FileRelative;
	}

	// Case 3 & 4: (can be merged as: Dependency in Cwd)
	if (isDependencyInCwd) {
		outPath = fs::relative(dependencyPath);
		return PathReferenceType::WorkingDir;
	}

	// Case 5: (dependency not in cwd)

	outPath = fs::absolute(dependencyPath);
	return PathReferenceType::FullPath;
}
} // namespace detail

AssetManager_::AssetManager_(const fs::path& defaultBinPath)
{
	AssetRegistry::Get().m_pods.push_back(std::make_unique<PodEntry>());


	if (!fs::is_directory(defaultBinPath)) {
		fs::create_directory(defaultBinPath);
	}

	podtools::ForEachPodType([]<typename PodType>() {
		auto&& [handle, pod]
			= AssetImporterManager->CreateTransientEntry<PodType>(fmt::format("~{}", mti::GetName<PodType>()));

		if constexpr (std::is_same_v<PodType, MaterialArchetype>) {
			MaterialArchetype::MakeDefaultInto(static_cast<MaterialArchetype*>(pod));
		}
	});

	LOG_INFO("Current working dir: {}", fs::current_path().generic_string());
	StdAssets::LoadAssets();

	AssetRegistry::Get().LoadAllPodsInDirectory(defaultBinPath);
}
