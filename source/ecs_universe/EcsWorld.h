#pragma once

#include "ecs_universe/Entity.h"
#include "ecs_universe/ComponentsDb.h"


struct SceneCompBase {
	size_t sceneUid;

private:
	friend class ComponentsDb;

	template<typename T>
	void Z_RegisterToScene()
	{
		sceneUid = Scene->EnqueueCreateCmd<T>();
		LOG_REPORT("Registered");
	}

	template<typename T>
	void Z_DeregisterFromScene()
	{
		Scene->EnqueueDestroyCmd<T>(sceneUid);
		LOG_REPORT("De registered");
	}
};

struct SceneGeometry;

struct StaticMeshComp : SceneCompBase {
	REFLECTED_SCENE_COMP(StaticMeshComp, SceneGeometry)
	{
		//
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(mesh);
	}


	PodHandle<Mesh> mesh;
};

struct ScriptComp {
	DIRTABLE;
	REFLECTED_COMP(ScriptComp)
	{
		//
		REFLECT_VAR(code);
	}

	std::string code;
};

struct FreeformMovementComp {
	float movespeed{ 15.f };
};


class ECS_World {
public:
	entt::registry reg;
	ECS_World() { ComponentsDb::HookRegistry(reg); }

	Entity CreateEntity(const std::string& name = "")
	{
		Entity ent{ reg.create(), &reg };
		auto& basic = ent.Add<BasicComponent>(name);
		basic.self = ent;
		return ent;
	}

	void DestroyEntity(Entity entity) { reg.destroy(entity.m_entity); }

	void CreateWorld();

	void UpdateWorld();
};
