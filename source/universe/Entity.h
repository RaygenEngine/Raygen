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
	friend class ComponentsDb;
	friend struct ComponentMetaEntry;
	friend struct BasicComponent;
	friend class World;

	template<CComponent T>
	T& GetNonDirty()
	{
		static_assert(!std::is_empty_v<T>, "Attempting to get an empty structure. This is not allowed by entt.");
		return registry->get<T>(entity);
	}

	template<CComponent T>
	void UnsafeRemove() const
	{
		return registry->remove<T>(entity);
	}


	// Essentially with this constructor being private we ensure only "World" is allowed to construct real entities.
	// Default constructor exists and acts as a nullptr to entities.
	// CHECK: duplicate data here (world & registry) for performance
	Entity(entt::entity entity_, World* world_, entt::registry& registry_);

	entt::registry* registry{ nullptr };
	World* world{ nullptr };
	entt::entity entity{ entt::null };

public:
	Entity() = default;

	[[nodiscard]] World& GetWorld() const
	{
		CLOG_ABORT(!world, "Requesting world from null entity!");
		return *world;
	}

	// Adds a component to the entity.
	// Forwards arguments to construct in-place (like emplace)
	//
	// UB if entity already has a component of the same type (use AddOrGet if unsure)
	// Faster version from AddOrGet
	// Automatically registers T::Create and T::Dirty if supported by the component
	//
	// Returns a reference to the created component
	template<CComponent T, typename... Args>
	T& Add(Args&&... args)
	{
		using namespace componentdetail;
		// TODO: Code duplicate of ComponentsDb emplace
		if constexpr (CCreateDestoryComp<T>) {
			registry->emplace<typename T::Create>(entity);
		}
		if constexpr (CDirtableComp<T>) {
			registry->emplace<typename T::Dirty>(entity);
		}
		auto& component = registry->emplace<T>(entity, std::forward<Args>(args)...);
		if constexpr (CSelfEntityMember<T> && !std::is_same_v<T, BasicComponent>) {
			component.self = *this;
		}
		return component;
	}

	// Adds a component to the entity if it does not exist, returns the existing otherwise
	// Forwards arguments to construct in-place (like emplace)
	//
	// Automatically registers T::Create and T::Dirty if supported by the component when adding
	// Automatically registers T::Dirty if the component already existed
	//
	// Returns a reference to the created / existing component
	template<CComponent T, typename... Args>
	T& AddOrGet(Args&&... args)
	{
		if (Has<T>()) {
			return GetDirty<T>();
		}
		return Add<T>(std::forward(args)...);
	}

	template<CComponent T>
	[[nodiscard]] bool Has() const
	{
		return registry->has<T>(entity);
	}

	template<CComponent T, typename... Args>
	void AddOrGet(Args&&... args)
	{
		if (Has<T>()) {
			return GetDirty<T>();
		}
		return Add<T>(std::forward(args)...);
	}

	// Returns T& or const T& based on if the substruct supports dirty.
	// Always prefer this if you will not write to the component to save performance.
	template<CComponent T>
	auto Get() const -> std::conditional_t<componentdetail::CDirtableComp<T>, const T&, T&>
	{
		static_assert(!std::is_empty_v<T>, "Attempting to get an empty structure. This is not allowed by entt.");
		return registry->get<T>(entity);
	}

	// Always returns T& and marks the component as dirty. Use this when you intend to write to the component.
	template<CComponent T>
	T& GetDirty()
	{
		MarkDirty<T>();
		return registry->get<T>(entity);
	}

	// TODO: Probably private
	// Just use GetDirty if you intend to write instead of forgetting to manually call this
	template<componentdetail::CDirtableComp T>
	void MarkDirty()
	{
		registry->get_or_emplace<typename T::Dirty>(entity);
	}

	// Mark this component for destruction, requires a CreateDestroy component
	template<componentdetail::CCreateDestoryComp T>
	void MarkDestroy()
	{
		registry->get_or_emplace<typename T::Destroy>(entity);
	}

	// Safely removes a component if it exists.
	// If T::Destroy exists, the actual deletion of the component is deferred and the T::Destroy component flag is added
	template<CComponent T>
	void SafeRemove()
	{
		if (!Has<T>()) {
			return;
		}

		if constexpr (componentdetail::CCreateDestoryComp<T>) {
			registry->get_or_emplace<typename T::Destroy>(entity);
		}
		else {
			registry->remove<T>(entity);
		}
	}

	[[nodiscard]] constexpr operator bool() const noexcept { return entity != entt::null && registry->valid(entity); }
	[[nodiscard]] constexpr bool operator==(const Entity& rhs) const noexcept
	{
		return entity == rhs.entity && registry == rhs.registry;
	}

	[[nodiscard]] constexpr bool operator!=(const Entity& rhs) const noexcept { return !(operator==(rhs)); }


	// Provide a "nice" interface to the common component
	[[nodiscard]] BasicComponent& Basic() { return *operator->(); }

	// Provide a "nice" interface to the common component
	[[nodiscard]] BasicComponent* operator->();

	// Mark the entity for destruction. (It will be deleted at the end of the frame)
	// Also destroys all attached children
	void Destroy();

	// Entt ID of the entity. (Used much like a UID for now)
	uint32 EntID() const { return static_cast<uint32>(entity); }
};
