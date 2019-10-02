#pragma once

#include "core/auxiliary/MetaTemplates.h"
#include "asset/AssetPod.h"


#define Z_REFL_TYPES		\
int32,						\
bool,						\
float,						\
glm::vec3,					\
glm::vec4,					\
std::string


namespace refl
{
	namespace detail
	{
		template<typename T>
		constexpr bool IsHandleToPodF()
		{
			if constexpr (std::is_base_of_v<BasePodHandle, T>)
			{
				return true;
			}
			return false;
		}

		template<typename T, typename... ReflTypes>
		constexpr bool CanBePropertyBaseType_ImplF()
		{
			return std::disjunction_v<std::is_same<T, ReflTypes>, ...>;
		}

		template<typename T>
		constexpr bool CanBePropertyBaseTypeF()
		{
			return IsReflectedBaseType_Impl<Z_REFL_TYPES>();
		}


		template<typename T>
		constexpr bool CanBePropertyF()
		{
			if constexpr (IsHandleToPodF<T>())
			{
				return true;
			}
			else if constexpr (is_vector_of_base_v<T, BasePodHandle>)
			{
				return true;
			}
			else if constexpr (CanBePropertyBaseTypeF<T>())
			{
				return true;
			}
			return false;
		}
	}

	// IsReflected to compile time check if a type can be reflected.
	template<typename Type>
	constexpr bool CanBeProperty = detail::CanBePropertyF<Type>();
}

/*
struct GltfFilePod;
struct ImagePod;
struct MaterialPod;
struct ModelPod;
struct ShaderPod;
struct StringPod;
struct TexturePod;
struct XMLDocPod;


// All the currently supported reflected types.
enum class PropertyType
{
	NONE = 0,
	Int = 1,
	Bool,
	Float,
	Vec3,
	Vec4,
	String,

	Handle_Cubemap,
	Handle_GltfFile,
	Handle_Image,
	Handle_Material,
	Handle_Model,
	Handle_Shader,
	Handle_Text,
	Handle_Texture,
	Handle_XMLDoc,

	Vector_Cubemap,
	Vector_GltfFile,
	Vector_Image,
	Vector_Material,
	Vector_Model,
	Vector_Shader,
	Vector_Text,
	Vector_Texture,
	Vector_XMLDoc,
};

template<typename Type>
constexpr PropertyType ReflectionFromType = PropertyType::NONE;
template<> PropertyType ReflectionFromType<int32> = PropertyType::Int;
template<> PropertyType ReflectionFromType<bool> = PropertyType::Bool;
template<> PropertyType ReflectionFromType<float> = PropertyType::Float;
template<> PropertyType ReflectionFromType<glm::vec3> = PropertyType::Vec3;
template<> PropertyType ReflectionFromType<glm::vec4> = PropertyType::Vec4;
template<> PropertyType ReflectionFromType<std::string> = PropertyType::String;


template<> PropertyType ReflectionFromType<PodHandle<GltfFilePod>> = PropertyType::Handle_GltfFile;
template<> PropertyType ReflectionFromType<PodHandle<ImagePod>> = PropertyType::Handle_Image;
template<> PropertyType ReflectionFromType<PodHandle<MaterialPod>> = PropertyType::Handle_Material;
template<> PropertyType ReflectionFromType<PodHandle<ModelPod>> = PropertyType::Handle_Model;
template<> PropertyType ReflectionFromType<PodHandle<ShaderPod>> = PropertyType::Handle_Shader;
template<> PropertyType ReflectionFromType<PodHandle<StringPod>> = PropertyType::Handle_Text;
template<> PropertyType ReflectionFromType<PodHandle<TexturePod>> = PropertyType::Handle_Texture;
template<> PropertyType ReflectionFromType<PodHandle<XMLDocPod>> = PropertyType::Handle_XMLDoc;

template<> PropertyType ReflectionFromType<std::vector<PodHandle<GltfFilePod>>> = PropertyType::Vector_GltfFile;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<ImagePod>>> = PropertyType::Vector_Image;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<MaterialPod>>> = PropertyType::Vector_Material;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<ModelPod>>> = PropertyType::Vector_Model;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<ShaderPod>>> = PropertyType::Vector_Shader;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<StringPod>>> = PropertyType::Vector_Text;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<TexturePod>>> = PropertyType::Vector_Texture;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<XMLDocPod>>> = PropertyType::Vector_XMLDoc;
*/



// ALWAYS expected to run in member function
#define REFLECT_VAR(Variable, ...) GetReflector(this).AutoAddProperty(#Variable, Variable).InitFlags(PropertyFlags::Pack(__VA_ARGS__));


// Static reflection, this macro leaves the scope on public:
#define STATIC_REFLECTOR(Class)	private:			\
using __ThisType = Class; public:					\
Class() { type = ctti::type_id<Class>(); }			\
[[nodiscard]]										\
static const StaticReflector& StaticReflect()		\
{													\
	static StaticReflector reflector(#Class);		\
	if (!reflector.m_generated)						\
	{												\
		_FillMembers(reflector);					\
		reflector.m_generated = true;				\
	}												\
	return reflector;								\
} public:											\
static void _FillMembers(StaticReflector& reflector)\

// Static reflect var
#define S_REFLECT_VAR(Variable, ...)				\
	reflector.AutoAddProperty<decltype(Variable)>(#Variable, offsetof(__ThisType, Variable)).InitFlags(PropertyFlags::Pack(__VA_ARGS__));

//
// Example expansion
// 
// struct Class
// {
// 	using __ThisType = Class; 
// public:
// 	[[nodiscard]]
// 	static const StaticReflector& StaticReflect()
// 	{
// 		static StaticReflector reflector("Class");
// 		if (!reflector.m_generated)
// 		{
// 			_FillMembers(reflector);
// 			reflector.m_generated = true;
// 		}
// 		return reflector;
// 	} 
// public:
// 	static void _FillMembers(StaticReflector& reflector)
// 
// 	{
// 		reflector.AutoAddProperty<decltype(member[2])>("member", offsetof(__ThisType, member[2])).InitFlags(PropertyFlags::Pack(__VA_ARGS__));
// 	}
// 
// 	int member[10];
// };

/*
#define REFLECTED_POD(Class) private:				\
using __ThisType = Class; public:					\
[[nodiscard]]										\
static const StaticReflector& StaticReflect()		\
{													\
	static StaticReflector reflector(#Class);		\
	if (!reflector.m_generated)						\
	{												\
		_FillMembers(reflector);					\
		reflector.m_generated = true;				\
	}												\
	return reflector;								\
}													\
PodReflector GetPodReflector() override				\
{													\
	return std::move(__ThisType::StaticReflect())	\
			.ToAbsoluteReflector(object);			\
} public:											\
static void _FillMembers(StaticReflector& reflector)\
*/