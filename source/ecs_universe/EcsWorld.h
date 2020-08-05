#pragma once

#include "ecs_universe/Entity.h"
#include "ecs_universe/ComponentsDb.h"

struct SceneCompBase {
	size_t sceneUid;
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
	COMP_DIRTABLE;
	COMP_CREATEDESTROY;
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
private:
	void LoadFromSrcPath();

public:
	entt::registry reg;

	fs::path srcPath;
	ECS_World(const fs::path& path = {});

	// If path is empty uses the original srcPath as path
	void SaveToDisk(const fs::path& path = {}, bool updateSrcPath = false);


	Entity CreateEntity(const std::string& name = "")
	{
		Entity ent{ reg.create(), &reg };

		auto& basic = ent.Add<BasicComponent>(!name.empty() ? name : "New Entity");
		basic.self = ent;


		return ent;
	}

	void DestroyEntity(Entity entity)
	{
		// WIP: ECS use flag for deletion, instead of directly
		reg.destroy(entity.m_entity);
	}

	void CreateWorld();

	void UpdateWorld();
};
