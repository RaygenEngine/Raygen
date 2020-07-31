#pragma once


#define DIRTABLE                                                                                                       \
	struct Dirty {                                                                                                     \
	};


template<typename T>
concept CComponent = true;

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


	template<CONC(CComponent) T, typename... Args>
	T& Add(Args&&... args)
	{
		return m_registry->emplace<T>(m_entity, std::forward<Args>(args)...);
	}

	template<CONC(CComponent) T, typename... Args>
	T& AddOrGet(Args&&... args)
	{
		return m_registry->get_or_emplace<T>(m_entity, std::forward<Args>(args)...);
	}

	template<CONC(CComponent) T>
	bool Has()
	{
		m_registry->has<T>(m_entity);
	}

	// TODO: Const when dirty exists
	template<CONC(CComponent) T>
	T& Get()
	{
		return m_registry->get<T>(m_entity);
	}

	template<CONC(CComponent) T>
	T& GetDirty()
	{
		MarkDirty<T>();
		return m_registry->get<T>(m_entity);
	}


	template<CONC(CComponent) T>
	void MarkDirty()
	{
		return m_registry->emplace_or_replace<typename T::Dirty>(m_entity);
	}


	[[nodiscard]] constexpr operator bool() const noexcept { return m_entity != entt::null; }
	[[nodiscard]] constexpr bool operator==(const Entity& rhs) const noexcept
	{
		return m_entity == rhs.m_entity && m_registry == rhs.m_registry;
	}


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
