#pragma once

#include "ecs_universe/ComponentDetail.h"

#define COMP_DIRTABLE                                                                                                  \
	struct Dirty {                                                                                                     \
	}

#define COMP_CREATEDESTROY                                                                                             \
	struct Create {                                                                                                    \
	};                                                                                                                 \
	struct Destroy {                                                                                                   \
	}


struct BasicComponent;

class Entity {

public:
	entt::entity m_entity{ entt::null };
	entt::registry* m_registry{ nullptr };

	Entity() = default;
	Entity(entt::entity ent, entt::registry* reg)
		: m_entity(ent)
		, m_registry(reg)
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

		if constexpr (HasCreateDestorySubstructsV<T>) {
			m_registry->emplace<typename T::Create>(m_entity);
		}
		if constexpr (HasDirtySubstructV<T>) {
			m_registry->emplace<typename T::Dirty>(m_entity);
		}
		return m_registry->emplace<T>(m_entity, std::forward<Args>(args)...);
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
		m_registry->has<T>(m_entity);
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
		return m_registry->get<T>(m_entity);
	}

	// Always returns T& and marks the component as dirty. Use this when you intend to write to the component.
	template<CONC(CComponent) T>
	T& GetDirty()
	{
		MarkDirty<T>();
		return m_registry->get<T>(m_entity);
	}

	// TODO: Probably private
	// Just use GetDirty if you intend to write instead of forgetting to manually call this
	template<CONC(CComponent) T>
	void MarkDirty()
	{
		if constexpr (componentdetail::HasDirtySubstruct<T>) {
			m_registry->get_or_emplace<typename T::Dirty>(m_entity);
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

		if constexpr (componentdetail::HasCreateDestorySubstructsV<T>) {
			m_registry->get_or_emplace<typename T::Destory>();
		}
		else {
			m_registry->remove<T>(m_entity);
		}
	}

	[[nodiscard]] constexpr operator bool() const noexcept { return m_entity != entt::null; }
	[[nodiscard]] constexpr bool operator==(const Entity& rhs) const noexcept
	{
		return m_entity == rhs.m_entity && m_registry == rhs.m_registry;
	}

	// Provide a "nice" interface to the common component
	BasicComponent* operator->() { return &Get<BasicComponent>(); }
};

struct DirtyMovedComp {
};

struct DirtySrtComp {
};


struct BasicComponent {
	std::string name;
	Entity self;


	struct TransformCache {
		glm::vec3 position{};
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
		glm::quat orientation{ glm::identity<glm::quat>() };

		glm::mat4 transform{ glm::identity<glm::mat4>() };

	private:
		friend struct BasicComponent;
		// Updates transform from TRS
		void Compose();
		// Updates TRS from transform
		void Decompose();
	};

	TransformCache local;
	TransformCache world;


	void UpdateWorldTransforms();

	//
	Entity parent;
	Entity firstChild;

	Entity next;
	Entity prev;

	// Also moves children
	void SetParent(Entity newParent = {}, int32 index = -1);

	void MarkDirtySrt();
	void MarkDirtyMoved();


private:
	void DetachFromParent();

public:
	[[nodiscard]] glm::vec3 GetNodePositionLCS() const { return local.position; }
	[[nodiscard]] glm::quat GetNodeOrientationLCS() const { return local.orientation; }

	// pitch, yaw, roll, in degrees
	[[nodiscard]] glm::vec3 GetNodeEulerAnglesLCS() const { return glm::degrees(glm::eulerAngles(local.orientation)); }
	[[nodiscard]] glm::vec3 GetNodeScaleLCS() const { return local.scale; }
	[[nodiscard]] glm::mat4 GetNodeTransformLCS() const { return local.transform; }

	[[nodiscard]] glm::vec3 GetNodeUpLCS() const { return GetNodeOrientationLCS() * glm::vec3(0.f, 1.f, 0.f); }
	[[nodiscard]] glm::vec3 GetNodeRightLCS() const { return GetNodeOrientationLCS() * glm::vec3(1.f, 0.f, 0.f); }
	[[nodiscard]] glm::vec3 GetNodeForwardLCS() const { return GetNodeOrientationLCS() * glm::vec3(0.f, 0.f, -1.f); }

	//[[nodiscard]] glm::vec3 GetNodePositionWCS() const { return m_position; }
	//[[nodiscard]] glm::quat GetNodeOrientationWCS() const { return m_orientation; }
	//// pitch, yaw, roll
	//[[nodiscard]] glm::vec3 GetNodeEulerAnglesWCS() const { return glm::degrees(glm::eulerAngles(m_orientation)); }
	//[[nodiscard]] glm::vec3 GetNodeScaleWCS() const { return m_scale; }
	[[nodiscard]] glm::mat4 GetNodeTransformWCS() const { return world.transform; }

	//[[nodiscard]] glm::vec3 GetNodeUpWCS() const { return GetNodeOrientationWCS() * glm::vec3(0.f, 1.f, 0.f); }
	//[[nodiscard]] glm::vec3 GetNodeRightWCS() const { return GetNodeOrientationWCS() * glm::vec3(1.f, 0.f, 0.f); }
	//[[nodiscard]] glm::vec3 GetNodeForwardWCS() const { return GetNodeOrientationWCS() * glm::vec3(0.f, 0.f, -1.f); }

	void SetNodeTransformWCS(const glm::mat4& newWorldMatrix);


	// void SetNodePositionLCS(glm::vec3 lt);
	// void SetNodeOrientationLCS(glm::quat lo);
	//// in degrees / pitch, yaw, roll
	// void SetNodeEulerAnglesLCS(glm::vec3 pyr);
	// void SetNodeScaleLCS(glm::vec3 ls);

	void SetNodeTransformLCS(const glm::mat4& lm);
	// void SetNodeLookAtLCS(glm::vec3 lookAt);

	// void RotateNodeAroundAxisLCS(glm::vec3 localAxis, float degrees);
	// void AddNodePositionOffsetLCS(glm::vec3 offset);

	// void SetNodePositionWCS(glm::vec3 wt);
	// void SetNodeOrientationWCS(glm::quat wo);
	//// in degrees / pitch, yaw, roll
	// void SetNodeEulerAnglesWCS(glm::vec3 pyr);
	// void SetNodeScaleWCS(glm::vec3 ws);
	// void SetNodeTransformWCS(const glm::mat4& newWorldMatrix);
	// void SetNodeLookAtWCS(glm::vec3 lookAt);

	// void RotateNodeAroundAxisWCS(glm::vec3 worldAxis, float degrees);
	// void AddNodePositionOffsetWCS(glm::vec3 offset);
};
