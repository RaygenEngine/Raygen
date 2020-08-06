#pragma once
#include "ecs_universe/BasicComponent.h"
#include "ecs_universe/ComponentsDb.h"
#include "ecs_universe/SceneComponentBase.h"

struct SceneGeometry;

struct CStaticMesh : CSceneBase {
	REFLECTED_SCENE_COMP(CStaticMesh, SceneGeometry)
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(mesh);
	}

	PodHandle<Mesh> mesh;
};
