#include "Serialization.h"

#include "assets/PodSerializers.h"
#include "reflection/PodTools.h"
#include "reflection/ReflectionTools.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

//
// CEREAL BINARY VISITORS
//

namespace glm {
template<typename Archive>
void serialize(Archive& ar, glm::vec2& v)
{
	ar(v.x, v.y);
}

template<typename Archive>
void serialize(Archive& ar, glm::vec3& v)
{
	ar(v.x, v.y, v.z);
}

template<typename Archive>
void serialize(Archive& ar, glm::vec4& v)
{
	ar(v.x, v.y, v.z, v.w);
}

template<typename Archive>
void serialize(Archive& ar, glm::ivec4& v)
{
	ar(v.x, v.y, v.z, v.w);
}

template<typename Archive>
void serialize(Archive& ar, glm::mat4& v)
{
	ar(v[0], v[1], v[2], v[3]);
}


template<typename Archive>
void serialize(Archive& ar, glm::quat& v)
{
	ar(v.x, v.y, v.z, v.w);
}

} // namespace glm

template<typename Archive, typename T>
void save(Archive& ar, const PodHandle<T>& v)
{
	ar(AssetRegistry::GetPodUri(v));
}

template<typename Archive, typename T>
void load(Archive& ar, PodHandle<T>& v)
{
	std::string path;
	ar(path);
	v = AssetRegistry::GetAsyncHandle<T>(path);
}

template<typename Archive>
struct CerealArchiveVisitor {
	Archive& ar;

	CerealArchiveVisitor(Archive& arr)
		: ar(arr)
	{
	}

	template<typename T>
	void operator()(T& ref, const Property& p)
	{
		ar(ref);
	}

	// void operator()(MetaEnumInst& ref, const Property& p) { ar(ref); }
};

template<typename Archive>
void load(Archive& ar, AssetPod& podRef)
{
	AssetPod* pod = &podRef;

	CerealArchiveVisitor v(ar);
	refltools::CallVisitorOnEveryProperty(pod, v);
	podtools::VisitPod(pod, [&](auto f) { AdditionalSerializeLoad(ar, f); });
}

template<typename Archive>
void save(Archive& ar, const AssetPod& podRef)
{
	AssetPod* pod = &const_cast<AssetPod&>(podRef);

	CerealArchiveVisitor v(ar);
	refltools::CallVisitorOnEveryProperty(pod, v);
	podtools::VisitPod(pod, [&](auto f) { AdditionalSerializeSave(ar, f); });
}

//
// POD META DATA
//
template<typename Archive>
void save(Archive& ar, const PodMetaData& metadata)
{
	ar(metadata.podTypeHash, metadata.originalImportLocation, metadata.exportOnSave, metadata.reimportOnLoad);
}

template<typename Archive>
void load(Archive& ar, PodMetaData& metadata)
{
	ar(metadata.podTypeHash, metadata.originalImportLocation, metadata.exportOnSave, metadata.reimportOnLoad);
}


void SerializePodToBinary(PodMetaData& metadata, AssetPod* pod, const fs::path& file)
{
	CLOG_ABORT(file.extension() != ".bin", "Incorrect extension");
	fs::create_directories(file.parent_path());
	std::ofstream os(file, std::ios::binary);
	if (!os) {
		LOG_ERROR("Failed to open file for pod writing: {}", file);
		return;
	}
	cereal::BinaryOutputArchive archive(os);

	archive(metadata);
	archive(*pod);
}

void DeserializePodFromBinary(PodEntry* entry)
{
	fs::path file = entry->path;

	auto& meta = entry->metadata;

	std::ifstream is(file.replace_extension(".bin"), std::ios::binary);
	if (!is) {
		LOG_ERROR("Failed to open file for pod reading: {}", file);
		return;
	}

	cereal::BinaryInputArchive archive(is);
	archive(meta);


	// determine pod type and allocate it
	podtools::VisitPodHash(meta.podTypeHash, [&]<typename PodType>() {
		entry->ptr.reset(new PodType());
		entry->type = mti::GetTypeId<PodType>();
		entry->Z_AssignClass(&PodType::StaticClass());
	});
	entry->transient = false;
	entry->name = uri::GetFilenameNoExt(entry->path);
	archive(*entry->ptr.get());
	assetdetail::podAccessor[entry->uid] = entry->ptr.get();
}
