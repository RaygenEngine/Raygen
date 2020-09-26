#pragma once
#include "assets/PodIncludes.h"

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
	ar(pod->data);
}


template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, Cubemap* pod)
{
	ar(pod->data);
}


//
// NewMaterials
//
template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, MaterialArchetype* pod)
{
	ar(pod->descriptorSetLayout, pod->gbufferFragBinary, pod->gbufferVertBinary, pod->depthFragBinary,
		pod->depthVertBinary, pod->unlitFragBinary, pod->raytracingBinary);
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
	ar(vtx.position, vtx.normal, vtx.tangent, vtx.uv);
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
// SkinnedMesh
//

template<typename Archive>
void serialize(Archive& ar, SkinnedVertex& vtx)
{
	ar(vtx.position, vtx.normal, vtx.tangent, vtx.uv, vtx.joint, vtx.weight);
}

template<typename Archive>
void serialize(Archive& ar, SkinnedGeometrySlot& gs)
{
	ar(gs.indices, gs.vertices);
}

template<typename Archive>
void serialize(Archive& ar, Joint& j)
{
	ar(j.parentJoint, j.inverseBindMatrix, j.translation, j.rotation, j.scale, j.name, j.index);
}

template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, SkinnedMesh* pod)
{
	ar(pod->joints, pod->skinnedGeometrySlots);
}

//
// Animation
//

template<typename Archive>
void serialize(Archive& ar, AnimationSampler& s)
{
	ar(s.inputs, s.outputs, s.interpolation);
}

template<typename Archive>
void serialize(Archive& ar, AnimationChannel& c)
{
	ar(c.samplerIndex, c.path, c.targetJoint);
}

template<typename Archive>
void AdditionalSerializeBoth(Archive& ar, Animation* pod)
{
	ar(pod->time, pod->jointCount, pod->channels, pod->samplers);
}
