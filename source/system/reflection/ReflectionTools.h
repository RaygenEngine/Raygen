#pragma once
#include "system/reflection/Property.h"

#include <type_traits>


#define DECLARE_HAS_FUNCTION_DETECTOR(FuncName)	\
template<typename T, typename = void>			\
struct Has##FuncName: std::false_type { };		\
												\
template<typename  T>							\
struct Has##FuncName<T, std::enable_if_t<std::is_member_function_pointer<decltype(&T::##FuncName)>::value>> : std::true_type { };


DECLARE_HAS_FUNCTION_DETECTOR(Begin);
DECLARE_HAS_FUNCTION_DETECTOR(End);

DECLARE_HAS_FUNCTION_DETECTOR(PreProperty);
DECLARE_HAS_FUNCTION_DETECTOR(PostProperty);



namespace ReflectionTools
{
struct Example
{
	void Begin(const Reflector&) {} // Pre Begin iteration on reflector's properties
	void End(const Reflector&) {} // Post End iteration on reflector's properties

	void PreProperty(ExactProperty& p) { }  // Pre Call visitor on proprety
	void PostProperty(ExactProperty& p) { } // Post Call visitor on proprety

	template<typename T>
	void Visit(T& value, ExactProperty& p) {} // Overload this for T.
};

static_assert(HasPreProperty<Example>::value, "Reflection tools test failed.");
}

namespace impl {
	template<typename T, typename V>
	bool VisitIf(V& visitor, ExactProperty& p)
	{
		if (p.IsA<T>()) 
		{ 
			visitor.Visit(p.GetRef<T>(), p); 
			return true; 
		}
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
	 || impl::VisitIf<glm::vec4>(v, prop)
	 || impl::VisitIf<std::string>(v, prop)

	 || impl::VisitIf<PodHandle<GltfFilePod>>(v, prop)
	 || impl::VisitIf<PodHandle<ImagePod>>(v, prop)
	 || impl::VisitIf<PodHandle<MaterialPod>>(v, prop)
	 || impl::VisitIf<PodHandle<ModelPod>>(v, prop)
	 || impl::VisitIf<PodHandle<ShaderPod>>(v, prop)
	 || impl::VisitIf<PodHandle<StringPod>>(v, prop)
	 || impl::VisitIf<PodHandle<TexturePod>>(v, prop)
	 || impl::VisitIf<PodHandle<XMLDocPod>>(v, prop)

	 || impl::VisitIf<std::vector<PodHandle<GltfFilePod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ImagePod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<MaterialPod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ModelPod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ShaderPod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<StringPod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<TexturePod>>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<XMLDocPod>>>(v, prop)

	 || impl::VisitIf<std::vector<PodHandle<GltfFilePod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ImagePod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<MaterialPod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ModelPod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<ShaderPod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<StringPod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<TexturePod>*>>(v, prop)
	 || impl::VisitIf<std::vector<PodHandle<XMLDocPod>*>>(v, prop)
	 ;
}

template<typename ReflectedType, typename VisitorClass>
void CallVisitorOnEveryProperty(ReflectedType* type, VisitorClass& v)
{
	Reflector reflector = GetReflector(type);
	
	if constexpr (HasBegin<VisitorClass>::value)
	{
		v.Begin(reflector);
	}

	for (auto& p : reflector.GetProperties())
	{
		if constexpr (HasPreProperty<VisitorClass>::value)
		{
			v.PreProperty(p);
		}
		
		CallVisitorOnProperty(p, v);

		if constexpr (HasPostProperty<VisitorClass>::value)
		{
			v.PostProperty(p);
		}
	}

	if constexpr (HasEnd<VisitorClass>::value)
	{
		v.End(reflector);
	}

}