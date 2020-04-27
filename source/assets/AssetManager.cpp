#include "pch.h"
#include "AssetManager.h"

#include "assets/PodIncludes.h"
#include "assets/Serialization.h"
#include "reflection/PodTools.h"
#include "rendering/assets/GpuAssetManager.h"
#include "assets/util/FindPodUsers.h"
#include "assets/specializations/PodDuplication.h"

#include <vulkan/vulkan.hpp>
#include <iostream>

ConsoleFunction<> console_SaveAll{ "a.saveAll", []() { AssetHandlerManager::SaveAll(); },
	"Saves all currently unsaved assets" };


void PodDeleter::operator()(AssetPod* p)
{
	auto l = [](auto* pod) {
		static_assert(!std::is_same_v<decltype(pod), AssetPod*>,
			"This should not ever instantiate with AssetPod*. Pod tools has internal error.");
		delete pod;
	};
	podtools::VisitPod(p, l);
}

// Dummy to avoid _Debug getting optimzed out
void Code()
{
	auto l = []<typename PodType>() {
		PodHandle<PodType> a;
		[[maybe_unused]] auto debug = a._Debug();
	};
	podtools::ForEachPodType(l);
}

void AssetHandlerManager::SaveToDiskInternal(PodEntry* entry)
{
	if (entry->path.size() <= 1 || entry->path.starts_with('~') || !entry->requiresSave) {
		return;
	}
	CLOG_ABORT(!entry, "Attempting to save null entry");
	CLOG_ABORT(!entry->ptr, "Attempting to save unloaded asset");

	auto& meta = entry->metadata;

	fs::path path = entry->path;
	SerializePodToBinary(entry->metadata, entry->ptr.get(), path.replace_extension(".bin"));


	if (meta.exportOnSave) {
		// TODO: Shader
	}


	entry->requiresSave = false;
	LOG_INFO("Saved pod at: {}", entry->path);
}

void AssetHandlerManager::LoadAllPodsInDirectory(const fs::path& path)
{
	size_t beginUid = m_pods.size();
	{
		for (const auto& entry : fs::recursive_directory_iterator(path)) {
			if (entry.is_directory()) {
				continue;
			}

			if (entry.path().extension() == ".bin") {
				auto key = fs::relative(entry.path()).replace_extension().generic_string();
				size_t uid = m_pods.size();
				m_pathCache.emplace(key, uid);

				PodEntry e;
				e.uid = uid;
				e.path = std::move(key);
				e.requiresSave = false;
				m_pods.emplace_back(std::make_unique<PodEntry>(std::move(e)));
			}
		}
	}

	LOG_REPORT("Found {} binary pods.", m_pods.size() - beginUid);

	{
		TIMER_SCOPE("Loading pods");

		size_t podsToLoad = m_pods.size() - beginUid;
		size_t jobCount = std::max(std::thread::hardware_concurrency(), static_cast<uint>(2));
		size_t podsPerJob
			= static_cast<size_t>(std::ceil(static_cast<float>(podsToLoad) / static_cast<float>(jobCount)));

		auto loadRange = [&](size_t start, size_t stop) {
			stop = std::min(stop, m_pods.size());
			for (size_t i = start; i < stop; ++i) {
				LoadFromDiskTypelessInternal(m_pods[i].get());
			}
			return true;
		};


		std::vector<std::thread> results;

		for (size_t i = 0; i < podsToLoad; i += podsPerJob) {
			size_t from = i + beginUid;
			size_t to = i + beginUid + podsPerJob;
			results.push_back(std::thread(loadRange, from, to));
		}

		for (auto& r : results) {
			r.join();
		}
	}
}

void AssetHandlerManager::LoadFromDiskTypelessInternal(PodEntry* entry)
{
	DeserializePodFromBinary(entry);
}

void AssetHandlerManager::RenameEntryImpl(PodEntry* entry, const std::string_view newFullPath)
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

void AssetHandlerManager::DeleteFromDiskInternal(PodEntry* entry)
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

PodEntry* AssetHandlerManager::DuplicateImpl(PodEntry* entry)
{
	// For transient pods we should require more params when duplicating (like a real name). Also all transient
	// duplications should in theory only be called directly by code and not caused indirectly by the user.
	CLOG_WARN(entry->transient, "Using non transient pod duplicate with a transient entry: {}", entry->path);

	PodEntry* result = nullptr;
	podtools::VisitPodHash(entry->GetClass()->GetTypeId().hash(), [&]<typename PodType> {
		auto& [newEntry, newPod]
			= CreateEntry<PodType>(entry->path, false, entry->metadata.originalImportLocation, false, false);
		podspec::Duplicate(entry->ptr.get(), newPod);
		result = newEntry;
	});

	return result;
}

AssetManager_::AssetManager_(const fs::path& workingDir, const fs::path& defaultBinPath)
{
	AssetHandlerManager::Get().m_pods.push_back(std::make_unique<PodEntry>());

	podtools::ForEachPodType([]<typename PodType>() {
		ImporterManager->CreateTransientEntry<PodType>(fmt::format("~{}", mti::GetName<PodType>()));
	});

	// Default normal image
	auto& [handle, pod] = ImporterManager->CreateTransientEntry<Image>("~NormalImagePod");
	pod->data[0] = 0x80;
	pod->data[1] = 0x80;
	pod->data[2] = 0xFF;
	pod->data[3] = 0xFF;

	fs::current_path(fs::current_path() / workingDir);

	if (!fs::is_directory(defaultBinPath)) {
		fs::create_directory(defaultBinPath);
	}

	LOG_INFO("Current working dir: {}", fs::current_path());

	AssetHandlerManager::Get().LoadAllPodsInDirectory(defaultBinPath);
}
