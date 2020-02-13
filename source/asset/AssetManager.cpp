#include "pch/pch.h"

#include "asset/AssetManager.h"
#include "reflection/PodTools.h"
#include "asset/PodIncludes.h"

#include "asset/Serialization.h"

void AssetManager::Init(const fs::path& assetPath)
{
	m_pods.push_back(std::make_unique<PodEntry>());

	fs::current_path(fs::current_path() / assetPath);

	if (!fs::is_directory("gen-data")) {
		fs::create_directory("gen-data");
	}

	LOG_INFO("Current working dir: {}", fs::current_path());

	serializeUid.function = [&](int32 uid) {
		fs::path p = uri::ToSystemPath(m_pods[uid]->path);
		p.replace_extension("pod");
		SerializePod(m_pods[uid]->ptr.get(), p);
	};

	deserilizeUid.function = [&](int32 uid) {
		fs::path p = uri::ToSystemPath(m_pods[uid]->path);
		p.replace_extension("pod");
		DeserializePod(m_pods[uid]->ptr.get(), p);
	};
}

void AssetManager::PreloadGltf(const uri::Uri& gltfModelPath)
{
	auto pParent = AssetManager::GetOrCreate<GltfFilePod>(gltfModelPath);

	const tinygltf::Model& file = pParent.Lock()->data;

	for (auto& gltfImage : file.images) {
		GetOrCreateFromParent<ImagePod>(gltfImage.uri, pParent);
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

template<>
FORCE_LINK void AssetManager::PostRegisterEntry<ImagePod>(PodEntry* entry)
{
	entry->futureLoaded = std::async(std::launch::async, [&, entry]() -> AssetPod* {
		ImagePod* ptr = new ImagePod();
		TryLoad(ptr, entry->path);
		return ptr;
	});
}

template<>
FORCE_LINK void AssetManager::PostRegisterEntry<TexturePod>(PodEntry* entry)
{
	TexturePod* pod = new TexturePod();
	TryLoad(pod, entry->path);
	entry->ptr.reset(pod);
}

template<>
FORCE_LINK void AssetManager::PostRegisterEntry<ShaderPod>(PodEntry* entry)
{
	ShaderPod* pod = new ShaderPod();
	TryLoad(pod, entry->path);
	entry->ptr.reset(pod);
}

template<>
FORCE_LINK void AssetManager::PostRegisterEntry<StringPod>(PodEntry* entry)
{
	entry->futureLoaded = std::async(std::launch::async, [&, entry]() -> AssetPod* {
		StringPod* ptr = new StringPod();
		TryLoad(ptr, entry->path);
		return ptr;
	});
}

// Dummy to instatiate exports above as to export them to the .lib
void AssetManager::Z_SpecializationExporter()
{
	PodEntry* a{};
	PostRegisterEntry<ShaderPod>(a);
	PostRegisterEntry<StringPod>(a);
	PostRegisterEntry<TexturePod>(a);
	PostRegisterEntry<ImagePod>(a);
}

/*
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include "reflection/ReflectionTools.h"

namespace {

struct PropertyToCerealVisitor {
	cereal::BinaryOutputArchive& archive;

	template<typename T>
	void operator()(T& value, const Property& p)
	{
		archive(value);
	}
};

struct PodSaverVisitor {
	fs::path file;


	PodSaverVisitor(fs::path&& where)
		: file(where)
	{
	}

	template<typename PodType>
	fs::path GetExtension()
	{
		return mti::GetName<PodType>();
	}

	// Specialize this for types that contain non reflected data
	template<typename PodType>
	void AddNonReflectedData(cereal::BinaryOutputArchive& archive)
	{
	}

	template<typename PodType>
	void operator()(PodType* pod)
	{
		file.replace_extension(GetExtension<PodType>());

		std::ofstream os(file, std::ios::binary);
		cereal::BinaryOutputArchive archive(os);

		PropertyToCerealVisitor visitor{ archive };
		refltools::CallVisitorOnEveryProperty(pod, visitor);

		AddNonReflectedData<PodType>(archive);
	}
};
} // namespace

void AssetManager::SaveToDisk(BasePodHandle handle)
{
	AssetPod* pod = Engine::GetAssetManager()->m_pods[handle.podId]->ptr.get();

	auto uri = fs::path(GetPodUri(handle));

	PodSaverVisitor inst(std::move(uri));
	podtools::VisitPod(pod, inst);
}
*/
