#pragma once

#include "reflection/ReflClass.h"
#include "engine/reflection/ReflectionDb.h"

#define REFLECTED_POD(Class, ...)                                                                                      \
private:                                                                                                               \
	Class() { type = refl::GetId<Class>(); }                                                                           \
                                                                                                                       \
	using Z_ThisType = Class;                                                                                          \
	friend class ReflClass;                                                                                            \
	friend class AssetImporterManager_;                                                                                \
	friend class AssetHandlerManager;                                                                                  \
	friend void DeserializePodFromBinary(PodEntry*);                                                                   \
                                                                                                                       \
public:                                                                                                                \
	[[nodiscard]] static const ReflClass& StaticClass()                                                                \
	{                                                                                                                  \
		static ReflClass cl = ReflClass::Generate<Class>(FA_MAIL_BULK);                                                \
		return cl;                                                                                                     \
	}                                                                                                                  \
	using GpuAssetType = vl::Gpu##Class;                                                                               \
                                                                                                                       \
public:                                                                                                                \
	static void GenerateReflection(ReflClass& refl)


#define FWD_GPU_POD(Class)                                                                                             \
	namespace vl {                                                                                                     \
	struct Class;                                                                                                      \
	}

//
//
//
//
//

// Functions required for scene commands:
//
// std::function<void(SceneStructType&)> DirtyCmd(BasicComponent&);
// std::function<void(SceneStructType&)> TransformCmd(BasicComponent&);


#define REFLECTED_SCENE_COMP(CompClass, SceneStructType)                                                               \
	using RenderSceneType = SceneStructType;                                                                           \
	struct Dirty {                                                                                                     \
	};                                                                                                                 \
	struct Create {                                                                                                    \
	};                                                                                                                 \
	struct Destroy {                                                                                                   \
	};                                                                                                                 \
	template<bool FullDirty>                                                                                           \
	std::function<void(SceneStructType&)> DirtyCmd(BasicComponent&);                                                   \
                                                                                                                       \
	REFLECTED_COMP(CompClass)


#define REFLECT_ICON(u8_icon) refl.SetIcon(u8_icon)

#define REFLECT_CATEGORY(ConstCharCategory) refl.SetCategory(ConstCharCategory)

#define REFLECTED_COMP(ComponentClass)                                                                                 \
	[[nodiscard]] static const ReflClass& StaticClass() { return ComponentClass::Z_MutableClass(); }                   \
                                                                                                                       \
private:                                                                                                               \
	/* Init of static class, also updates parents' child set. With this we ensure all ReflClasses are unique for each  \
	 * class and therefore we can check if the class is the same by comparing pointers */                              \
	static ReflClass& Z_MutableClass()                                                                                 \
	{                                                                                                                  \
		static ReflClass cl = ReflClass::Generate<ComponentClass>();                                                   \
		return cl;                                                                                                     \
	}                                                                                                                  \
	using Z_ThisType = ComponentClass;                                                                                 \
	friend class ReflClass;                                                                                            \
	static inline ReflComponentRegistar<ComponentClass> Z_InternalRegistar = ReflComponentRegistar<ComponentClass>();  \
	/* Called from inside the ReflClass::Generate to generate members, only supposed to be used with the macros        \
	 * below. The user must provide the body. */                                                                       \
public:                                                                                                                \
	static void GenerateReflection(ReflClass& refl)


//
//
//
//
//

#define REFLECTED_NODE(Class, ParentClass, /*optional DF_FLAGS()*/...)                                                 \
public:                                                                                                                \
	static_assert(std::is_convertible_v<Class*, ParentClass*>, "Reflection parent does not match real parent");        \
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
	friend class ReflectionDb;                                                                                         \
	static inline ReflectionRegistar<Class> Z_InternalRegistar = ReflectionRegistar<Class>();                          \
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


// Generic reflected struct / class. Unregistered (for now). Can be used to allow access to member data & reflection
// tools for any struct that is not Node / Pod. (eg: An auto-saving generic struct like EditorUserSettings)
// This struct should remain a simple POD without inheritance to avoid UB.
#define REFLECTED_GENERIC(Class)                                                                                       \
public:                                                                                                                \
	mti::TypeId type{ refl::GetId<Class>() };                                                                          \
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
                                                                                                                       \
public:                                                                                                                \
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


#define REFLECT_FLAGS(...) refl.AddFlags(NodeFlags::Pack(__VA_ARGS__));


#define DECLARE_DIRTY_FUNC(ComponentStruct)                                                                            \
	template std::function<void(ComponentStruct::RenderSceneType&)> ComponentStruct::DirtyCmd<true>(BasicComponent&);  \
	template std::function<void(ComponentStruct::RenderSceneType&)> ComponentStruct::DirtyCmd<false>(BasicComponent&); \
	template<bool FullDirty>                                                                                           \
	std::function<void(ComponentStruct::RenderSceneType&)> ComponentStruct::DirtyCmd
//
//
//
