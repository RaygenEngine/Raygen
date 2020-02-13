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
	ar(AssetManager::GetPodUri(v));
}

template<typename Archive, typename T>
void load(Archive& ar, PodHandle<T>& v)
{
	std::string path;
	ar(path);
	v = AssetManager::GetOrCreate<T>(path);
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
void serialize(Archive& ar, AssetPod* pod)
{
	/*	podtools::VisitPod(
			pod, [](auto f) { LOG_REPORT("Serialize pod: {}", mti::GetName<std::remove_pointer_t<decltype(f)>>()); });*/
	CerealArchiveVisitor v(ar);
	refltools::CallVisitorOnEveryProperty(pod, v);
}

void SerializePod(AssetPod* pod, const fs::path& file)
{
	std::ofstream os(file, std::ios::binary);
	if (!os) {
		LOG_ERROR("Failed to open file for pod writing: {}", file);
		return;
	}
	cereal::BinaryOutputArchive archive(os);
	archive(pod);
}

void DeserializePod(AssetPod* pod, const fs::path& file)
{
	std::ifstream is(file, std::ios::binary);
	if (!is) {
		LOG_ERROR("Failed to open file for pod reading: {}", file);
		return;
	}
	cereal::BinaryInputArchive archive(is);
	archive(pod);
}
