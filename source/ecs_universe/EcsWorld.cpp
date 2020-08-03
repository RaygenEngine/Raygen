#include "pch.h"
#include "EcsWorld.h"

#include "engine/Input.h"
#include "assets/AssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/SceneCamera.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include <nlohmann/json.hpp> // WIP: ECS
#include "engine/console/ConsoleVariable.h"

Entity globalEnt;

#define DECLARE_DIRTY_FUNC(ComponentStruct)                                                                            \
	template std::function<void(ComponentStruct::RenderSceneType&)> ComponentStruct::DirtyCmd<true>(BasicComponent&);  \
	template std::function<void(ComponentStruct::RenderSceneType&)> ComponentStruct::DirtyCmd<false>(BasicComponent&); \
	template<bool FullDirty>                                                                                           \
	std::function<void(SceneGeometry&)> StaticMeshComp::DirtyCmd
//
//
//


DECLARE_DIRTY_FUNC(StaticMeshComp)(BasicComponent& bc)
{
	return [=](SceneGeometry& geom) {
		geom.transform = bc.world.transform;
		if constexpr (FullDirty) {
			geom.model = vl::GpuAssetManager->GetGpuHandle(mesh);
		}
	};
}


void ECS_World::CreateWorld()
{
	auto mesh = CreateEntity("Global");

	auto& mc = mesh.Add<StaticMeshComp>().mesh
		= AssetManager->ImportAs<Mesh>("_skymesh/UVsphereSmoothShadingInvNormals.gltf", true);

	mesh.Add<ScriptComp>("My script");


	globalEnt = mesh;


	mesh = CreateEntity("Second");
	mesh.Add<StaticMeshComp>().mesh = AssetManager->ImportAs<Mesh>("gltf-samples/2.0/Avocado/glTF/Avocado.gltf", true);

	mesh->SetParent(globalEnt);
}

std::stringstream st;

void ECS_World::UpdateWorld()
{
	//
	// Game Systems
	//
	if (Input.IsJustPressed(Key::R)) {
		globalEnt->local.position += glm::vec3(0.f, 1.f, 0.f);
		globalEnt->MarkDirtyMoved();
	}

	if (Input.IsJustPressed(Key::C)) {
		nlohmann::json j;
		ComponentsDb::RegistryToJson(reg, j);
		st.clear();
		st << j;
		delete Scene;
		Scene = new Scene_(2);
		Scene->EnqueueCreateCmd<SceneCamera>();
		reg.clear();
	}

	if (Input.IsJustPressed(Key::V)) {
		delete Scene;
		Scene = new Scene_(2);
		Scene->EnqueueCreateCmd<SceneCamera>();
		reg.clear();

		nlohmann::json j;
		st >> j;

		ComponentsDb::JsonToRegistry(j, reg);
	}

	//
	// Update Transforms
	//

	//{
	//	auto view = reg.view<BasicComponent, DirtyMovedComp>();

	//	for (auto& [ent, bs] : view.each()) {
	//		bs.UpdateWorldTransforms();
	//	}
	//}


	//
	// Scene Commands
	//

	SceneCmdSystem::WriteSceneCmds(Scene, reg);

	reg.clear<DirtyMovedComp, DirtySrtComp>();

	ComponentsDb::ClearDirties(reg); // Also destroyes all pairs T, T::Destroy
}
