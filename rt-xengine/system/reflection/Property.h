#pragma once

namespace PropertyFlags
{
	using Type = uint32;
	
	constexpr Type NoSave	= (1 << 0);
	constexpr Type NoLoad	= (1 << 1);
	constexpr Type NoEdit	= (1 << 2);
	constexpr Type Color	= (1 << 3);
}

// a generic property that can be of any type.
struct Property
{
protected:
	PropertyType m_type;

	// As noted this is a direct pointer to the memory of the mapped object.
	void* m_ptr;

public:
	bool IsA(PropertyType type)
	{
		return type == m_type;
	}

	template<typename Type>
	bool IsA()
	{
		return IsA(ReflectionFromType<Type>);
	}

	// ALWAYS only request types you are sure are the correct type with IsA() first.
	// this WILL have undefined behavior if you try to convert to a different type.
	template<typename As>
	As& GetRef()
	{
		static_assert(IsReflected<As>,
					  "This type is not supported for reflection and the conversion will always fail.");

		assert(IsA<As>());
		return (*reinterpret_cast<As*>(m_ptr));
	}

	template<typename Type>
	static Property MakeProperty(Type& Variable)
	{
		static_assert(IsReflected<Type>, "This type is not supported for reflection.");

		Property created;
		created.m_type = ReflectionFromType<Type>;
		created.m_ptr = &Variable;
		return created;
	}

	template<typename IntF, typename BoolF, typename FloatF, typename Vec3F, typename StringF>
	auto SwitchOnType(IntF Int, BoolF Bool, FloatF Float, Vec3F Vec3, StringF String) -> return_type_t<IntF>
	{
		switch (m_type)
		{
		case PropertyType::Int:
			return Int(GetRef<int32>());
			break;
		case PropertyType::Bool:
			return Bool(GetRef<bool>());
			break;
		case PropertyType::Float:
			return Float(GetRef<float>());
			break;
		case PropertyType::Vec3:
			return Vec3(GetRef<glm::vec3>());
			break;
		case PropertyType::String:
			return String(GetRef<std::string>());
			break;
		}

		if constexpr (!std::is_void_v<return_type_t<IntF>>) {
			return {};
		}
	}
};