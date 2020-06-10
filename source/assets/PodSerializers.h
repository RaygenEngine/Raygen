#pragma once
#include "assets/PodIncludes.h"
#include "core/math-ext/AABB.h"

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
// Image
//
template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, Image* pod)
{
	ar(pod->data, pod->mipData);
}

//
// NewMaterials
//
template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, MaterialArchetype* pod)
{
	ar(pod->descriptorSetLayout, pod->gbufferFragBinary);
}

template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, MaterialInstance* pod)
{
	ar(pod->descriptorSet);
}


//
// Mesh
// Mesh pod requires a few additional struct serialization specializations
// (Mesh, GeometryGroup, Vertex)
//


template<typename Archive>
void serialize(Archive& ar, Vertex& vtx)
{
	ar(vtx.position, vtx.normal, vtx.tangent, vtx.bitangent, vtx.uv);
}

template<typename Archive>
void serialize(Archive& ar, GeometrySlot& gs)
{
	ar(gs.indices, gs.vertices);
}

template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, Mesh* pod)
{
	ar(pod->geometrySlots);
}


//
// Cubemap
//

template<typename Archive>
void serialize(Archive& ar, Cubemap* pod)
{
	ar(pod);
}
