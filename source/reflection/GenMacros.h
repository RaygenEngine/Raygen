#pragma once

#include "reflection/ReflClass.h"

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


// static_assert(std::is_base_of_v<ParentClass, Class>, "Incorrect parent node type");
#define REFLECTED_NODE(Class, ParentClass, /*optional DF_FLAGS()*/...)                                                 \
public:                                                                                                                \
	/* Public interface */                                                                                             \
	using Parent = ParentClass;                                                                                        \
	[[nodiscard]] const ReflClass& GetClass() const override { return Class::StaticClass(); }                          \
	[[nodiscard]] const ReflClass& GetParentClass() const override { return Parent::StaticClass(); }                   \
	[[nodiscard]] static const ReflClass& StaticClass() { return Class::Z_MutableClass(); }                            \
                                                                                                                       \
	/* Expand DF FLAGS declaration here as public */                                                                   \
	__VA_ARGS__;                                                                                                       \
                                                                                                                       \
private:                                                                                                               \
	/* Init of static class, also updates parents' child set. With this we ensure all ReflClasses are unique for each  \
	 * class and therefore we can check if the class is the same by comparing pointers */                              \
	static ReflClass& Z_MutableClass()                                                                                 \
	{                                                                                                                  \
		static ReflClass cl = ReflClass::Generate<Class, Parent>(&cl);                                                 \
		return cl;                                                                                                     \
	}                                                                                                                  \
                                                                                                                       \
	/* Some 'using' trickery to allow enable macros */                                                                 \
	using Z_ThisType = Class;                                                                                          \
	friend class ReflClass;                                                                                            \
	friend class NodeFactory;                                                                                          \
                                                                                                                       \
	/* Instanciates a class of this type. Should really be in ReflClass but not required because its only used by      \
	 * NodeFactory. */                                                                                                 \
	[[nodiscard]] static Node* NewInstance()                                                                           \
	{                                                                                                                  \
		Z_MutableClass(); /* Ensure reflclass for this has been instanciated, therefore the parent class knows this    \
							 class.*/                                                                                  \
		return new Class();                                                                                            \
	}                                                                                                                  \
	/* Called from inside the ReflClass::Generate to generate members, only supposed to be used with the macros        \
	 * below. The user must provide the body. */                                                                       \
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
