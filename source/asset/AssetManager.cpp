#include "pch/pch.h"

#include "asset/AssetManager.h"
#include "reflection/PodTools.h"
#include "asset/PodIncludes.h"

#include "asset/Serialization.h"

#include <iostream>


void AssetImporterManager::Init(const fs::path& assetPath)
{
	AssetHandlerManager::Get().m_pods.push_back(std::make_unique<PodEntry>());

	podtools::ForEachPodType([](auto dummy) {
		using PodType = std::remove_pointer_t<decltype(dummy)>;
		auto entry = AssetHandlerManager::CreateNew<PodType>();
		entry->path = fmt::format("~{}", GetDefaultPodUid<PodType>());
		entry->ptr.reset(new PodType());
	});


	fs::current_path(fs::current_path() / assetPath);

	if (!fs::is_directory("gen-data")) {
		fs::create_directory("gen-data");
	}

	LOG_INFO("Current working dir: {}", fs::current_path());

	int32 i;
	std::cout << "If you want to load pods type 1:\n";
	std::cin >> i;
	if (i == 1) {
		AssetHandlerManager::Get().LoadAllPodsInDirectory("gen-data/");
	}
}


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
	auto l = [](auto p) {
		using PodType = std::remove_pointer_t<decltype(p)>;
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

	if (meta.preferedDiskType == PodDiskType::Binary) {
		SerializePodToBinary(entry->metadata, entry->ptr.get(), entry->path);
	}
	else if (meta.preferedDiskType == PodDiskType::Json) {
	}
	else {
		LOG_ABORT("Implement");
	}


	if (meta.exportOnSave) {
		// TODO: implement
	}


	entry->requiresSave = false;
	LOG_REPORT("Saved: {} at: {}", entry->name, entry->path);
}

void AssetHandlerManager::LoadAllPodsInDirectory(const fs::path& path)
{
	size_t beginUid = m_pods.size();
	{
		TIMER_STATIC_SCOPE("Searching for pods.");
		for (const auto& entry : fs::recursive_directory_iterator(path)) {
			if (entry.is_directory()) {
				continue;
			}

			// WIP: Handle json
			if (entry.path().extension() == ".bin") {
				auto key = fs::relative(entry.path()).string();
				std::replace(key.begin(), key.end(), '\\', '/');
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

	LOG_REPORT("Found {} pods", m_pods.size() - beginUid);

	{
		TIMER_STATIC_SCOPE("Loading pods.");

		for (size_t i = beginUid; i < m_pods.size(); ++i) {
			LoadFromDiskTypelesskInternal(m_pods[i].get());
		}
	}
}

void AssetHandlerManager::LoadFromDiskTypelesskInternal(PodEntry* entry)
{
	DeserializePodFromBinary(entry);
}
