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
	{
		auto view = reg.view<BasicComponent, DirtyMovedComp>();

		for (auto& [ent, bs] : view.each()) {
			bs.UpdateWorldTransforms();
		}
	}


	//
	// Render
	//

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


	if (Input.IsJustPressed(Key::C)) {
		reg.visit(globalEnt.m_entity, [&](const entt::id_type type) -> void {
			if (ComponentsDb::HasClass(type)) {
				auto cl = ComponentsDb::GetClass(type);
				for (auto& prop : cl.GetProperties()) {
					LOG_REPORT("Prop: {}", prop.GetNameStr());
				}
			}
		});
	}

	{
		auto view = reg.view<BasicComponent, ScriptComp::Dirty>(entt::exclude<StaticMeshComp::Dirty>);
		for (auto& [ent, bs] : view.each()) {
			LOG_REPORT("{} had dirty script.", bs.name);
		}
	}


	reg.clear<DirtyMovedComp, DirtySrtComp>();
	ComponentsDb::ClearDirties(reg);
}
