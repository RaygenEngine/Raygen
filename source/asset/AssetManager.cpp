#include "pch/pch.h"

#include "asset/AssetManager.h"
#include "reflection/PodTools.h"
#include "asset/PodIncludes.h"

#include "asset/Serialization.h"


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
	CLOG_ABORT(entry, "Attempting to save null entry");
	CLOG_ABORT(entry->ptr, "Attempting to save unloaded asset");


	entry->requiresSave = false;
}
