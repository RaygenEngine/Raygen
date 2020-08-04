#include "pch.h"
#include "EcsWorld.h"

#include "engine/Input.h"
#include "assets/AssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/SceneCamera.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "engine/console/ConsoleVariable.h"

#include <nlohmann/json.hpp> // WIP: ECS
#include <fstream>           // WIP: ECS

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

ECS_World::ECS_World(const fs::path& path)
	: srcPath(path)
{
	if (!path.empty()) {
		LoadFromSrcPath();
	}
	else {
		CreateWorld(); // WIP: ECS
	}
}

void ECS_World::LoadFromSrcPath()
{
	std::ifstream file(srcPath);
	CLOG_ERROR(!file.is_open(), "Failed to open file: {} when loading world", srcPath);
	if (!file.is_open()) {
		return;
	}
	nlohmann::json j;
	file >> j;

	ComponentsDb::JsonToRegistry(j, reg);
}

void ECS_World::SaveToDisk(const fs::path& path, bool updateSrcPath)
{
	if (path.empty()) {
		if (srcPath.empty()) {
			LOG_ERROR("Attempting to save world without path, but world was not loaded from a file");
			return;
		}
		SaveToDisk(srcPath);
		return;
	}

	if (updateSrcPath) {
		srcPath = path;
	}

	nlohmann::json j;
	ComponentsDb::RegistryToJson(reg, j);

	std::ofstream file(srcPath);
	CLOG_ERROR(!file.is_open(), "Failed to open file: {} when saving world", srcPath);
	if (!file.is_open()) {
		return;
	}
	file << std::setw(2) << j;
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
