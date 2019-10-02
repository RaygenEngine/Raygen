#pragma once

#include "core/reflection/ReflClass.h"

#define REFLECTED_BASE(Class, Modifier) private:	\
using Z_ThisType = Class;							\
friend class ReflClass;								\
public:												\
[[nodiscard]]										\
static const ReflClass& GetClass() {				\
	static ReflClass cl								\
		= ReflClass::Generate<Class>();				\
	return cl;										\
} Modifier:											\
static void GenerateReflection(ReflClass& refl)


#define REFLECTED_POD(Class) private:				\
	Class() { type = refl::GetId<Class>(); }		\
	REFLECTED_BASE(Class, public)


#define REFLECT_VAR(Variable, ...)					\
	refl.AddProperty<decltype(Variable)>(offsetof(Z_ThisType, Variable), #Variable, PropertyFlags::Pack(__VA_ARGS__))
