#pragma once


template<CComponent T>
T& Entity::GetNonDirty()
{
	static_assert(!std::is_empty_v<T>, "Attempting to get an empty structure. This is not allowed by entt.");
	return registry->get<T>(entity);
}

template<CComponent T>
void Entity::UnsafeRemove()
{
	return registry->remove<T>(entity);
}

inline World& Entity::GetWorld()
{
	CLOG_ABORT(!world, "Requesting world from null entity!");
	return *world;
}

template<CComponent T, typename... Args>
T& Entity::Add(Args&&... args)
{
	using namespace componentdetail;

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

	if constexpr (CBeginPlayComp<T>) {
		if (world.IsPlaying()) {
			component.BeginPlay();
		}
	}

	return component;
}

template<CComponent T, typename... Args>
T& Entity::AddOrGet(Args&&... args)
{
	if (Has<T>()) {
		return GetDirty<T>();
	}
	return Add<T>(std::forward(args)...);
}

template<CComponent T>
[[nodiscard]] bool Entity::Has() const
{
	return registry->has<T>(entity);
}

template<CComponent T>
auto Entity::Get() const -> std::conditional_t<componentdetail::CDirtableComp<T>, const T&, T&>
{
	static_assert(!std::is_empty_v<T>, "Attempting to get an empty structure. This is not allowed by entt.");
	return registry->get<T>(entity);
}

// Always returns T& and marks the component as dirty. Use this when you intend to write to the component.
template<CComponent T>
T& Entity::GetDirty()
{
	MarkDirty<T>();
	return registry->get<T>(entity);
}

template<componentdetail::CDirtableComp T>
void Entity::MarkDirty()
{
	registry->get_or_emplace<typename T::Dirty>(entity);
}

template<componentdetail::CCreateDestoryComp T>
void Entity::MarkDestroy()
{
	registry->get_or_emplace<typename T::Destroy>(entity);
}

template<CComponent T>
void Entity::SafeRemove()
{
	if (!Has<T>()) {
		return;
	}

	if constexpr (componentdetail::CEndPlayComp<T>) {
		GetNonDirty<T>().EndPlay();
	}

	if constexpr (componentdetail::CCreateDestoryComp<T>) {
		registry->get_or_emplace<typename T::Destroy>(entity);
	}
	else {
		registry->remove<T>(entity);
	}
}

constexpr Entity::operator bool() const noexcept
{
	return entity != entt::null && registry->valid(entity);
}

constexpr bool Entity::operator==(const Entity& rhs) const noexcept
{
	return entity == rhs.entity && registry == rhs.registry;
}

constexpr bool Entity::operator!=(const Entity& rhs) const noexcept
{
	return !(operator==(rhs));
}


inline BasicComponent& Entity::Basic()
{
	return *operator->();
}

inline uint32 Entity::EntID()
{
	return static_cast<uint32>(entity);
};
