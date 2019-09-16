#pragma once
#include "system/reflection/Property.h"
/*

int32
bool
float
glm::vec3
std::string
PodHandle<CubemapPod>
PodHandle<GltfFilePod>
PodHandle<ImagePod>
PodHandle<MaterialPod>
PodHandle<ModelPod>
PodHandle<ShaderPod>
PodHandle<TextPod>
PodHandle<TexturePod>
PodHandle<XMLDocPod>
std::vector<PodHandle<CubemapPod>>
std::vector<PodHandle<GltfFilePod>>
std::vector<PodHandle<ImagePod>>
std::vector<PodHandle<MaterialPod>>
std::vector<PodHandle<ModelPod>>
std::vector<PodHandle<ShaderPod>>
std::vector<PodHandle<TextPod>>
std::vector<PodHandle<TexturePod>>
std::vector<PodHandle<XMLDocPod>>
std::vector<PodHandle<CubemapPod>*>
std::vector<PodHandle<GltfFilePod>*>
std::vector<PodHandle<ImagePod>*>
std::vector<PodHandle<MaterialPod>*>
std::vector<PodHandle<ModelPod>*>
std::vector<PodHandle<ShaderPod>*>
std::vector<PodHandle<TextPod>*>
std::vector<PodHandle<TexturePod>*>
std::vector<PodHandle<XMLDocPod>*>

*/

namespace impl {
	template<typename T, typename V>
	bool VisitIf(V& visitor, ExactProperty& p)
	{
		if (p.IsA<T>()) { visitor.visit(p.GetRef<T>(), p); return true; }
		return false;
	}
}

// Expects a visitor class with overloads for every type on .visit(ReflectedType, property)
// Overloads can be templated.
template<typename VisitorClass>
void CallVisitorOnProperty(ExactProperty& prop, VisitorClass& v)
{
	bool shortCircuit = 
	    impl::VisitIf<int32>(v, prop)
	 || impl::VisitIf<bool>(v, prop)
	 || impl::VisitIf<float>(v, prop)
	 || impl::VisitIf<glm::vec3>(v, prop)
	 || impl::VisitIf<std::string>(v, prop)

	 || impl::VisitIf<PodHandle<CubemapPod>>(v, prop)
	 || impl::VisitIf<PodHandle<GltfFilePod>>(v, prop)
	 || impl::VisitIf<PodHandle<ImagePod>>(v, prop)
	 || impl::VisitIf<PodHandle<MaterialPod>>(v, prop)
	 || impl::VisitIf<PodHandle<ModelPod>>(v, prop)
	 || impl::VisitIf<PodHandle<ShaderPod>>(v, prop)
	 || impl::VisitIf<PodHandle<TextPod>>(v, prop)
	 || impl::VisitIf<PodHandle<TexturePod>>(v, prop)
	 || impl::VisitIf<PodHandle<XMLDocPod>>(v, prop)

	 || impl::VisitIf<std::vector<PodHandle<CubemapPod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<GltfFilePod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ImagePod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<MaterialPod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ModelPod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ShaderPod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<TextPod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<TexturePod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<XMLDocPod>>>(v, prop)

	 || impl::VisitIf<std::vector<PodHandle<CubemapPod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<GltfFilePod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ImagePod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<MaterialPod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ModelPod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ShaderPod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<TextPod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<TexturePod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<XMLDocPod>*>>(v, prop)
	 ;
}

template<typename ReflectedType, typename VisitorClass>
void CallVisitorOnEveryProperty(ReflectedType* type, VisitorClass& v)
{
	auto reflector = GetReflector(type);
	for (auto& p : reflector.GetProperties())
	{
		CallVisitorOnProperty(p, v);
	}
}