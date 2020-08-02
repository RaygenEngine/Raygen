#include "pch.h"
#include "EcsWorld.h"

#include "engine/Input.h"
#include "assets/AssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"

Entity globalEnt;

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

template<typename T>
void EnqueueCreateCmdsSingle(Scene_* scene, entt::registry& reg)
{
	auto view = reg.view<T, typename T::Create>(entt::exclude<typename T::Destroy>);
	for (auto& [ent, sc] : view.each()) {
		sc.sceneUid = scene->EnqueueCreateCmd<typename T::RenderSceneType>();
	}
}

template<typename T>
void EnqueueDestoryCmdsSingle(Scene_* scene, entt::registry& reg)
{
	auto view = reg.view<T, typename T::Destroy>();
	for (auto& [ent, sc] : view.each()) {
		scene->EnqueueDestroyCmd<typename T::RenderSceneType>(sc.sceneUid);

		// DO LAST: references may become invalidated.
		reg.remove<T>(ent);
	}
}

// TODO: These 2 systems can be replaced by auto registering ones in scene structs in ComponentsDb
template<typename... Args>
void EnqueueCreateCmds(Scene_* scene, entt::registry& reg)
{
	(EnqueueCreateCmdsSingle<Args>(scene, reg), ...);
}

template<typename... Args>
void EnqueueDestoryCmds(Scene_* scene, entt::registry& reg)
{
	(EnqueueDestoryCmdsSingle<Args>(scene, reg), ...);
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
	// Render
	//

	EnqueueCreateCmds<StaticMeshComp>(Scene, reg);

	EnqueueDestoryCmds<StaticMeshComp>(Scene, reg);

	{
		auto view = reg.view<BasicComponent, StaticMeshComp, StaticMeshComp::Dirty>();

		for (auto& [ent, bs, mesh] : view.each()) {
			Scene->EnqueueCmd<SceneGeometry>(mesh.sceneUid, [&](SceneGeometry& geom) {
				geom.model = vl::GpuAssetManager->GetGpuHandle(mesh.mesh);
				geom.transform = bs.world.transform;
			});
		}
	}


	{
		auto view = reg.view<BasicComponent, StaticMeshComp, DirtySrtComp>(entt::exclude<StaticMeshComp::Dirty>);
		for (auto& [ent, bs, mesh] : view.each()) {
			Scene->EnqueueCmd<SceneGeometry>(
				mesh.sceneUid, [&](SceneGeometry& geom) { geom.transform = bs.world.transform; });
		}
	}

	// TODO: Auto delete all T::Destroy's
	reg.clear<DirtyMovedComp, DirtySrtComp>();
	ComponentsDb::ClearDirties(reg);
}
