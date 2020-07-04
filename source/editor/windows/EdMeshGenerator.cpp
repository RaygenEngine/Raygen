#include "pch.h"
#include "EdMeshGenerator.h"

#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiImpl.h"
#include "editor/imgui/ImAssetSlot.h"
#include "engine/console/Console.h"
#include "engine/console/ConsoleVariable.h"
#include "assets/PodEditor.h"
#include "assets/pods/MaterialInstance.h"


#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

namespace {

void GeneratePlane(GeometrySlot& mesh, const ed::MeshGenerator::PlaneSettings& settings)
{
	// 4 triangles per quad (for better deformations in vertex shaders)


	int32 N = settings.quadSide;
	int32 quads = N * N;
	int32 triangles = quads * 4;

	float quadScale = settings.worldSize / N;

	mesh.indices.resize(triangles * 6);

	// vertices = 2 * total quads

	// Each Quad "owns" v1 and c, the others (v2,v3,v4) are owned by the adjacent quads
	// Plus the final vertices
	// Vertices = 2 * quads + (2*N-1)
	int32 vertices = 2 * quads + (2 * (N + 1) - 1);
	mesh.vertices.resize(vertices);


	// PERF: Implement better indexing for GPU performance

	// v1 for each quad
	for (int x = 0; x <= N; ++x) {
		for (int z = 0; z <= N; ++z) {
			auto& vert = mesh.vertices[x + z * (N + 1)];
			vert.position.x = x * quadScale;
			vert.position.y = 0;
			vert.position.z = z * quadScale;

			vert.uv.x = (float)x / N;
			vert.uv.y = (float)z / N;

			vert.normal.y = 1.0f;
			vert.tangent.x = 1.0f;
		}
	}

	int32 centerBeginOffset = (N + 1) * (N + 1);

	// c for each quad (that has one)
	for (int x = 0; x < N; ++x) {
		for (int z = 0; z < N; ++z) {
			auto& vert = mesh.vertices[centerBeginOffset + (x + z * N)];
			vert.position.x = (x * quadScale) + quadScale / 2;
			vert.position.y = 0;
			vert.position.z = (z * quadScale) + quadScale / 2;

			vert.uv.x = ((float)x + 0.5f) / N;
			vert.uv.y = ((float)z + 0.5f) / N;

			vert.normal.y = 1.0f;
			vert.tangent.x = 1.0f;
		}
	}

	// indicies
	uint32 indiciesIndex = 0;
	for (int x = 0; x < N; ++x) {
		for (int z = 0; z < N; ++z) {
			uint32 v1 = x + z * (N + 1);
			uint32 v2 = v1 + 1;
			uint32 v3 = v1 + (N + 1);
			uint32 v4 = v3 + 1;

			uint32 cc = centerBeginOffset + x + z * N;


			mesh.indices[indiciesIndex++] = v2;
			mesh.indices[indiciesIndex++] = v1;
			mesh.indices[indiciesIndex++] = cc;


			mesh.indices[indiciesIndex++] = v3;
			mesh.indices[indiciesIndex++] = v4;
			mesh.indices[indiciesIndex++] = cc;

			mesh.indices[indiciesIndex++] = v2;
			mesh.indices[indiciesIndex++] = cc;
			mesh.indices[indiciesIndex++] = v4;

			mesh.indices[indiciesIndex++] = v3;
			mesh.indices[indiciesIndex++] = cc;
			mesh.indices[indiciesIndex++] = v1;
		}
	}
}

} // namespace


namespace ed {

void MeshGenerator::ImguiDraw()
{
	ImEd::AssetSlot<Mesh>("Mesh to write", mesh);

	if (mesh.IsDefault()) {
		ImGui::Text("Cannot edit default mesh.");
		return;
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Separator();


	ImEd::EnumDropDown("Generation Type", genType);


	switch (genType) {
		case MeshGenerationType::Box: ImGui::Text("TODO: "); break;
		case MeshGenerationType::Plane: PlaneGenDraw(); break;
		case MeshGenerationType::Sphere: ImGui::Text("TODO: "); break;
	}
}


void MeshGenerator::PlaneGenDraw()
{
	ImGui::InputFloat("World Size", &planeSettings.worldSize);
	planeSettings.worldSize = std::max(planeSettings.worldSize, 1.0f);

	ImGui::InputInt("Quads per side(N)", &planeSettings.quadSide);
	planeSettings.quadSide = std::max(planeSettings.quadSide, 1);

	TEXT_TOOLTIP("Actual quads will be N*N");


	if (ImEd::Button("Generate!")) {
		PodEditor pod(mesh);

		PodHandle<MaterialInstance> prevMaterial;
		if (!pod->materials.empty()) {
			prevMaterial = pod->materials[0];
		}
		pod->materials.clear();
		pod->geometrySlots.clear();


		auto& geom = pod->geometrySlots.emplace_back(GeometrySlot{});

		GeneratePlane(geom, planeSettings);

		pod->materials.emplace_back(prevMaterial);
	}
}
} // namespace ed
