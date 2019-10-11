#pragma once

#include "reflection/ReflClass.h"

class Node;
struct AssetPod;

#define REFLECTED_POD(Class)                                                                                           \
private:                                                                                                               \
	Class() { type = refl::GetId<Class>(); }                                                                           \
                                                                                                                       \
	using Z_ThisType = Class;                                                                                          \
	friend class ReflClass;                                                                                            \
	friend class AssetManager;                                                                                         \
                                                                                                                       \
public:                                                                                                                \
	[[nodiscard]] static const ReflClass& StaticClass()                                                                \
	{                                                                                                                  \
		static ReflClass cl = ReflClass::Generate<Class>();                                                            \
		return cl;                                                                                                     \
	}                                                                                                                  \
                                                                                                                       \
public:                                                                                                                \
	static void GenerateReflection(ReflClass& refl)


#define REFLECTED_NODE(Class, ParentClass, /*optional DF_FLAGS()*/...)                                                 \
public:                                                                                                                \
	using Parent = ParentClass;                                                                                        \
	[[nodiscard]] const ReflClass& GetClass() const override { return Class::StaticClass(); }                          \
	[[nodiscard]] const ReflClass& GetParentClass() const override { return Parent::StaticClass(); }                   \
	[[nodiscard]] static const ReflClass& StaticClass()                                                                \
	{                                                                                                                  \
		static ReflClass cl = ReflClass::Generate<Class, Parent>();                                                    \
		return cl;                                                                                                     \
	}                                                                                                                  \
	__VA_ARGS__;                                                                                                       \
                                                                                                                       \
private:                                                                                                               \
	using Z_ThisType = Class;                                                                                          \
	friend class ReflClass;                                                                                            \
	static void GenerateReflection(ReflClass& refl)

#define REFLECT_VAR(Variable, ...)                                                                                     \
	refl.AddProperty<decltype(Variable)>(offsetof(Z_ThisType, Variable), #Variable, PropertyFlags::Pack(__VA_ARGS__))

#define REFL_EQUALS_PROPERTY(PropertyRef, Variable)                                                                    \
	((PropertyRef).GetName() == ReflClass::RemoveVariablePrefix(#Variable) && (&(Variable)))


#define DF_FLAGS(...)                                                                                                  \
public:                                                                                                                \
	struct DF : Parent::DF {                                                                                           \
		enum                                                                                                           \
		{                                                                                                              \
			_PREV = Parent::DF::_COUNT - 1,                                                                            \
			__VA_ARGS__,                                                                                               \
			_COUNT                                                                                                     \
		};                                                                                                             \
	}
