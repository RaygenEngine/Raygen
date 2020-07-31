#pragma once


#define DECL_DIRTY(...)                                                                                                \
	struct Dirty {                                                                                                     \
		bool __VA_ARGS__;                                                                                              \
		Dirty() { Clear(); }                                                                                           \
		void Clear() { std::memset(this, 1, sizeof(Dirty)); }                                                          \
	} df;

#define DIRTABLE                                                                                                       \
	struct Autodirty {                                                                                                 \
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


	BasicComponent* operator->() { return &Get<BasicComponent>(); }
};

struct DirtyMovedComp {
};

struct DirtySrtComp {
};


struct BasicComponent {
	std::string name;
	Entity self; // Code smell

	DECL_DIRTY(Srt, Name, Parent, Children, Components, Created);

	glm::vec3 position{};                                // Local
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };                 // Local
	glm::quat orientation{ glm::identity<glm::quat>() }; // Local

	// Local
	glm::mat4 transform{ glm::identity<glm::mat4>() };
	// World
	glm::mat4 worldTransform{ glm::identity<glm::mat4>() };


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
};
