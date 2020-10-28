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

// Entity handle. Prefer pass by copy for this type.
class Entity {
public:
	Entity() = default;

	[[nodiscard]] World& GetWorld();

	// Adds a component to the entity.
	// Forwards arguments to construct in-place (like emplace)
	//
	// UB if entity already has a component of the same type (use AddOrGet if unsure)
	// Faster version from AddOrGet
	// Automatically registers T::Create and T::Dirty if supported by the component
	//
	// Returns a reference to the created component
	template<CComponent T, typename... Args>
	T& Add(Args&&... args);

	// Adds a component to the entity if it does not exist, returns the existing otherwise
	// Forwards arguments to construct in-place (like emplace)
	//
	// Automatically registers T::Create and T::Dirty if supported by the component when adding
	// Automatically registers T::Dirty if the component already existed
	// Automatically runs .BeginPlay() if world is in playing state.
	//
	// Returns a reference to the created / existing component
	template<CComponent T, typename... Args>
	T& AddOrGet(Args&&... args);

	template<CComponent T>
	[[nodiscard]] bool Has() const;

	// Returns T& or const T& based on if the substruct supports dirty.
	// Always prefer this if you will not write to the component to save performance.
	template<CComponent T>
	auto Get() const -> std::conditional_t<componentdetail::CDirtableComp<T>, const T&, T&>;

	// Always returns T& and marks the component as dirty. Use this when you intend to write to the component.
	template<CComponent T>
	[[nodiscard("Use mark dirty if you want to just dirty the component")]] //
	T& GetDirty();

	// TODO: Probably private
	// Just use GetDirty if you intend to write instead of forgetting to manually call this
	template<componentdetail::CDirtableComp T>
	void MarkDirty();

	// Mark this component for destruction, requires a CreateDestroy component
	template<componentdetail::CCreateDestoryComp T>
	void MarkDestroy();

	// Safely removes a component if it exists.
	// If T::Destroy exists, the actual deletion of the component is deferred and the T::Destroy component flag is added
	template<CComponent T>
	void SafeRemove();

	[[nodiscard]] constexpr operator bool() const noexcept;
	[[nodiscard]] constexpr bool operator==(const Entity& rhs) const noexcept;
	[[nodiscard]] constexpr bool operator!=(const Entity& rhs) const noexcept;

	// Provide a "nice" interface to the common component
	[[nodiscard]] BasicComponent& Basic();

	// Provide a "nice" interface to the common component
	[[nodiscard]] BasicComponent* operator->();

	// Mark the entity for destruction. (It will be deleted at the end of the frame)
	// Also destroys all attached children
	void Destroy();

	// Entt ID of the entity. (Used much like a UID for now)
	[[nodiscard]] uint32 EntID();

private:
	friend class ComponentsDb;
	friend struct ComponentMetaEntry;
	friend struct BasicComponent;
	friend class World;

	template<CComponent T>
	T& GetNonDirty();

	template<CComponent T>
	void UnsafeRemove();


	// Essentially with this constructor being private we ensure only "World" is allowed to construct real entities.
	// Default constructor exists and acts as a nullptr to entities.
	// CHECK: duplicate data here (world & registry) for performance
	Entity(entt::entity entity_, World* world_, entt::registry& registry_);

	entt::registry* registry{ nullptr };
	World* world{ nullptr };
	entt::entity entity{ entt::null };
};

#include "Entity.impl.h"
