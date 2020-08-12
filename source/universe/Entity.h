#pragma once

#include "universe/ComponentDetail.h"

#define COMP_DIRTABLE                                                                                                  \
	struct Dirty {                                                                                                     \
	}

#define COMP_CREATEDESTROY                                                                                             \
	struct Create {                                                                                                    \
	};                                                                                                                 \
	struct Destroy {                                                                                                   \
	}

struct CDestroyFlag {
};

struct BasicComponent;

class Entity {

public:
	entt::entity entity{ entt::null };
	entt::registry* registry{ nullptr };

	Entity() = default;
	Entity(entt::entity ent, entt::registry* reg)
		: entity(ent)
		, registry(reg)
	{
	}

	// Adds a component to the entity.
	// Forwards arguments to construct in-place (like emplace)
	//
	// UB if entity already has a component of the same type (use AddOrGet if unsure)
	// Faster version from AddOrGet
	// Automatically registers T::Create and T::Dirty if supported by the component
	//
	// Returns a reference to the created component
	template<CONC(CComponent) T, typename... Args>
	T& Add(Args&&... args)
	{
		using namespace componentdetail;

		if constexpr (HasCreateDestroySubstructsV<T>) {
			registry->emplace<typename T::Create>(entity);
		}
		if constexpr (HasDirtySubstructV<T>) {
			registry->emplace<typename T::Dirty>(entity);
		}
		return registry->emplace<T>(entity, std::forward<Args>(args)...);
	}

	// Adds a component to the entity if it does not exist, returns the existing otherwise
	// Forwards arguments to construct in-place (like emplace)
	//
	// Automatically registers T::Create and T::Dirty if supported by the component when adding
	// Automatically registers T::Dirty if the component already existed
	//
	// Returns a reference to the created / existing component
	template<CONC(CComponent) T, typename... Args>
	T& AddOrGet(Args&&... args)
	{
		if (Has<T>()) {
			return GetDirty<T>();
		}
		return Add<T>(std::forward(args)...);
	}

	template<CONC(CComponent) T>
	[[nodiscard]] bool Has() const
	{
		return registry->has<T>(entity);
	}

	template<CONC(CComponent) T, typename... Args>
	void AddOrGet(Args&&... args)
	{
		if (Has<T>()) {
			return GetDirty<T>();
		}
		return Add<T>(std::forward(args)...);
	}

	// Returns T& or const T& based on if the substruct supports dirty.
	// Always prefer this if you will not write to the component to save performance.
	template<CONC(CComponent) T>
	auto Get() const -> std::conditional_t<componentdetail::HasDirtySubstructV<T>, const T&, T&>
	{
		static_assert(!std::is_empty_v<T>, "Attempting to get an empty structure. This is not allowed by entt.");
		return registry->get<T>(entity);
	}

	// Always returns T& and marks the component as dirty. Use this when you intend to write to the component.
	template<CONC(CComponent) T>
	T& GetDirty()
	{
		MarkDirty<T>();
		return registry->get<T>(entity);
	}

	// TODO: Probably private
	// Just use GetDirty if you intend to write instead of forgetting to manually call this
	template<CONC(CComponent) T>
	void MarkDirty()
	{
		if constexpr (componentdetail::HasDirtySubstruct<T>) {
			registry->get_or_emplace<typename T::Dirty>(entity);
		}
	}

	// Safely removes a component if it exists.
	// If T::Destroy exists, the actual deletion of the component is deferred and the T::Destroy component flag is added
	template<CONC(CComponent) T>
	void SafeRemove()
	{
		if (!Has<T>()) {
			return;
		}

		if constexpr (componentdetail::HasCreateDestroySubstructsV<T>) {
			registry->get_or_emplace<typename T::Destroy>(entity);
		}
		else {
			registry->remove<T>(entity);
		}
	}

	[[nodiscard]] constexpr operator bool() const noexcept { return entity != entt::null; }
	[[nodiscard]] constexpr bool operator==(const Entity& rhs) const noexcept
	{
		return entity == rhs.entity && registry == rhs.registry;
	}

	[[nodiscard]] constexpr bool operator!=(const Entity& rhs) const noexcept { return !(operator==(rhs)); }

	// Provide a "nice" interface to the common component
	BasicComponent* operator->();

	// Mark the entity for destruction. (It will be deleted at the end of the frame)
	// Also destroys all attached children
	void Destroy();
};
