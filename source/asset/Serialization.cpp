#include "pch/pch.h"
#include "system/Logger.h"

#include "asset/Serialization.h"
#include "reflection/PodTools.h"
#include "reflection/ReflectionTools.h"
#include "asset/PodIncludes.h"
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>


//
// CEREAL BINARY VISITORS
//


namespace glm {
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
void serialize(Archive& ar, glm::mat4x4& v)
{
	LOG_ABORT("TODO: Serialize mat4x4 is currently undone");
}
} // namespace glm

template<typename Archive, typename T>
void save(Archive& ar, const PodHandle<T>& v)
{
	ar(AssetHandlerManager::GetPodUri(v));
}

template<typename Archive, typename T>
void load(Archive& ar, PodHandle<T>& v)
{
	std::string path;
	ar(path);
	v = AssetImporterManager::ResolveOrImport<T>(path);
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

template<typename Archive, typename PodType>
void PostReflectionSerialize(Archive& ar, PodType* pod)
{
}

template<typename Archive>
void PostReflectionSerialize(Archive& ar, ImagePod* pod)
{
	ar(pod->data);
	LOG_REPORT("Ser/Deser {} img data", pod->data.size());
}


struct PodMeta {
	int32 myData;

	template<typename Arc>
	void serialize(Arc& ar)
	{
		ar(myData);
	}
};


template<typename Archive>
void load(Archive& ar, AssetPod& podRef)
{
	AssetPod* pod = &podRef;

	PodMeta m;
	ar(m);
	LOG_REPORT("Pod Is: {}", m.myData);

	CerealArchiveVisitor v(ar);
	refltools::CallVisitorOnEveryProperty(pod, v);
	podtools::VisitPod(pod, [&](auto f) { PostReflectionSerialize(ar, f); });
}

template<typename Archive>
void save(Archive& ar, const AssetPod& podRef)
{
	AssetPod* pod = &const_cast<AssetPod&>(podRef);

	PodMeta m;

	m.myData = std::rand();

	ar(m);

	CerealArchiveVisitor v(ar);
	refltools::CallVisitorOnEveryProperty(pod, v);
	podtools::VisitPod(pod, [&](auto f) { PostReflectionSerialize(ar, f); });
}

void SerializePod(AssetPod* pod, const fs::path& file)
{
	std::ofstream os(file, std::ios::binary);
	if (!os) {
		LOG_ERROR("Failed to open file for pod writing: {}", file);
		return;
	}
	cereal::BinaryOutputArchive archive(os);
	archive(*pod);
}

void DeserializePod(AssetPod* pod, const fs::path& file)
{
	std::ifstream is(file, std::ios::binary);
	if (!is) {
		LOG_ERROR("Failed to open file for pod reading: {}", file);
		return;
	}
	cereal::BinaryInputArchive archive(is);
	archive(*pod);
}
