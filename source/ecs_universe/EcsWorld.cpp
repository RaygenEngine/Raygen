#include "pch.h"
#include "EcsWorld.h"

#include "engine/Input.h"
#include "assets/AssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"

Entity globalEnt;

std::function<void(SceneGeometry&)> StaticMeshComp::DirtyCmd(BasicComponent& bs)
{
	return [=](SceneGeometry& geom) {
		geom.transform = bs.world.transform;
		geom.model = vl::GpuAssetManager->GetGpuHandle(mesh);
	};
}

std::function<void(SceneGeometry&)> StaticMeshComp::TransformCmd(BasicComponent& bs)
{
	return [=](SceneGeometry& geom) {
		geom.transform = bs.world.transform;
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


void ECS_World::UpdateWorld()
{
	//
	// Game Systems
	//
	if (Input.IsJustPressed(Key::R)) {
		globalEnt->local.position += glm::vec3(0.f, 1.f, 0.f);
		globalEnt->MarkDirtyMoved();
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
