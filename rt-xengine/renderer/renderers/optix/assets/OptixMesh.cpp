#include "pch.h"
#include "OptixMesh.h"
#include "OptixProgram.h"


namespace Renderer::Optix
{

	OptixMesh::OptixMesh(OptixRendererBase* renderer)
		: OptixAsset(renderer)
	{
	}

	bool OptixMesh::Load(Assets::XMesh* data, uint32 closestHitRayType, Assets::StringFile* closestHitProgramSource,
		std::string closestHitProgramName, uint32 anyHitRayType, Assets::StringFile* anyHitProgramSource,
		std::string anyHitProgramName)
	{
		SetIdentificationFromAssociatedDiskAssetIdentification(data->GetLabel());

		m_triGeomVarPtx = GetDiskAssetManager()->LoadFileAsset<Assets::StringFile>("optixGeometryTriangles.ptx");
		m_triGeomVarProgram = GetRenderer()->RequestOptixProgram(m_triGeomVarPtx.get(), "triangle_attributes");

		auto vertices = data->GetVertices();
		const auto vertexCount = static_cast<uint32>(vertices.size());

		auto indices = data->GetVertexIndices();

		// we create copy the vertexBuffer only once
		optix::Buffer vertexBuffer = GetOptixContext()->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_DISCARD_HOST_MEMORY, RT_FORMAT_USER, vertexCount);
		vertexBuffer->setElementSize(sizeof(Core::Vertex));
		memcpy(vertexBuffer->map(), vertices.data(), sizeof(Core::Vertex) * vertexCount);
		vertexBuffer->unmap();

		auto materialOffsets = data->GetMaterialOffsets();
		auto materialsInOffsetOrder = data->GetMaterialsInOffsetOrder();

		RT_XENGINE_ASSERT(materialOffsets.size() == materialsInOffsetOrder.size());

		auto matOffset = materialOffsets.begin();

		auto material = materialsInOffsetOrder.begin();

		// get materials in offset order (parallel to each corresponding group)
		for (; matOffset != materialOffsets.end() && material != materialsInOffsetOrder.end(); ++matOffset, ++material)
		{
			const auto begin = matOffset->first;
			const auto end = matOffset->second;
			const auto size = end - begin + 1;

			optix::Buffer indexBuffer = GetOptixContext()->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_DISCARD_HOST_MEMORY, RT_FORMAT_UNSIGNED_INT3, size);
			// copy from mat begin to end (size)
			memcpy(indexBuffer->map(), &indices[begin], sizeof(glm::u32vec3) * size);
			indexBuffer->unmap();

			optix::GeometryTriangles triGeom = GetOptixContext()->createGeometryTriangles();

			triGeom->setPrimitiveCount(size);
			triGeom->setTriangleIndices(indexBuffer, RT_FORMAT_UNSIGNED_INT3);
			// stride, we need only positions
			triGeom->setVertices(vertexCount, vertexBuffer, 0, sizeof(Core::Vertex), RT_FORMAT_FLOAT3);

			triGeom->setBuildFlags(RTgeometrybuildflags(0));

			triGeom["vertex_buffer"]->setBuffer(vertexBuffer);
			triGeom["index_buffer"]->setBuffer(indexBuffer);

			triGeom->setAttributeProgram(m_triGeomVarProgram->GetOptixHandle());



			OptixMeshGeometryGroup omg;

			omg.group = triGeom;
			omg.material = GetRenderer()->RequestOptixMaterial((*material),
				closestHitRayType, closestHitProgramSource, closestHitProgramName,
				anyHitRayType, anyHitProgramSource, anyHitProgramName);

			m_groups.push_back(omg);
		}

		return true;
	}
}

