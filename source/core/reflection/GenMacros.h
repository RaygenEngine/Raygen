#pragma once

#include "core/reflection/ReflClass.h"

class Node;
struct AssetPod;

// TODO: pod ctor should be private and befriend asset manager
#define REFLECTED_POD(Class)                                                                                           \
public:                                                                                                                \
	Class() { type = refl::GetId<Class>(); }                                                                           \
                                                                                                                       \
private:                                                                                                               \
	using Z_ThisType = Class;                                                                                          \
	friend class ReflClass;                                                                                            \
                                                                                                                       \
public:                                                                                                                \
	[[nodiscard]] static const ReflClass& StaticClass()                                                                \
	{                                                                                                                  \
		static ReflClass cl = ReflClass::Generate<Class>();                                                            \
		return cl;                                                                                                     \
	}                                                                                                                  \
public:                                                                                                                \
	static void GenerateReflection(ReflClass& refl)


#define REFLECTED_NODE(Class, ParentClass)                                                                             \
public:                                                                                                                \
	using Parent = ParentClass;                                                                                        \
	[[nodiscard]] const ReflClass& GetClass() const override { return Class::StaticClass(); }                          \
	[[nodiscard]] const ReflClass& GetParentClass() const override { return Parent::StaticClass(); }                   \
	[[nodiscard]] static const ReflClass& StaticClass()                                                                \
	{                                                                                                                  \
		static ReflClass cl = ReflClass::Generate<Class, Parent>();                                                    \
		return cl;                                                                                                     \
	}                                                                                                                  \
private:                                                                                                               \
	using Z_ThisType = Class;                                                                                          \
	friend class ReflClass;                                                                                            \
	static void GenerateReflection(ReflClass& refl)


#define REFLECT_VAR(Variable, ...)                                                                                     \
	refl.AddProperty<decltype(Variable)>(offsetof(Z_ThisType, Variable), #Variable, PropertyFlags::Pack(__VA_ARGS__))

#define REFL_EQUALS_PROPERTY(PropertyRef, Variable)                                                                    \
	(PropertyRef.GetName() == ReflClass::RemoveVariablePrefix(#Variable) && (&Variable))