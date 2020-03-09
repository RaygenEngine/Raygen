#pragma once
#include "asset/PodIncludes.h"

// These are called after all properties have been serialized / deserialized.
// Included at a different file and should only be included where they are required

// Overload load AND save, OR overload both directly

template<typename Archive, typename PodType>
void AdditionalSerializeBoth(Archive& ar, PodType* pod)
{
}

template<typename Archive, typename PodType>
void AdditionalSerializeLoad(Archive& ar, PodType* pod)
{
	AdditionalSerializeBoth(ar, pod);
}
template<typename Archive, typename PodType>
void AdditionalSerializeSave(Archive& ar, PodType* pod)
{
	AdditionalSerializeBoth(ar, pod);
}

//
// BinaryPod
//
template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, BinaryPod* pod)
{
	ar(pod->data);
}

//
// ImagePod
//
template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, ImagePod* pod)
{
	ar(pod->data);
}

//
// ModelPod
// Model pod requires a few additional struct serialization specializations
// (Mesh, GeometryGroup, VertexData)
//

namespace math {
template<typename Archive>
void serialize(Archive& ar, math::AABB& box)
{
	ar(box.min, box.max);
}
} // namespace math

template<typename Archive>
void serialize(Archive& ar, VertexData& vtx)
{
	ar(vtx.position, vtx.normal, vtx.tangent, vtx.bitangent, vtx.textCoord0, vtx.textCoord1);
}

template<typename Archive>
void serialize(Archive& ar, GeometryGroup& gg)
{
	ar(gg.indices, gg.vertices, gg.mode, gg.materialIndex);
}

template<typename Archive>
void serialize(Archive& ar, Mesh& mesh)
{
	ar(mesh.geometryGroups);
}


template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, ModelPod* pod)
{
	ar(pod->meshes, pod->bbox);
}
