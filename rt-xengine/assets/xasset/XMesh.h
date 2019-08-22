

#ifndef XMESH_H
#define XMESH_H

#include "assets/DiskAssetPart.h"

#include "XMaterial.h"

#include "core/data/Vertex.h"
#include "core/auxiliary/SmartPtrAux.h"

#include "core/auxiliary/FileAux.h"

namespace Assets
{
	class XMesh : public DiskAssetPart
	{
		std::vector<Core::Vertex> m_vertices;

		// If indices are in material order (XMD supports that)
		// we can optimize the drawing in apis such as OpenGL (/ Optix 6.0 optix::GeometryTriangles)
		// that requires different draw calls (/ geometry instances) for each material (changing of uniforms)
		std::vector<glm::u32vec3> m_vertexIndices;

		std::vector<uint32> m_materialIndices;

	    // materialOffsets contain the material pairs (face begin, face end) 
		// to consecutive different materials in vertexIndices
		// If indices are in material order, we can minimize the draw calls
		// e.g.    0 0 0 0 1 1 1 2 2 = 3 draw calls using material offsets
		// whereas 0 0 1 0 1 2 1 2 0 = 8 draw calls using material offsets
		// One could split the triangle meshes and not use the material offsets, however this requires
		// more processing time and memory (also using material offsets we can process (memcpy / draw call)
		// XMesh data in its loaded serial form)
		void CalculateMaterialOffsets();
		std::vector<std::pair<uint32, uint32>> m_materialOffsets;

		std::vector<std::shared_ptr<XMaterial>> m_materials;

	public:
		XMesh(DiskAsset* parent);
		~XMesh() = default;

		bool Load(Core::XMDFileData& data);
		void Clear() override;


		const std::vector<Core::Vertex>& GetVertices() const { return m_vertices; }
		const std::vector<glm::u32vec3>& GetVertexIndices() const { return m_vertexIndices; }
		const std::vector<uint32>& GetMaterialIndices() const { return m_materialIndices; }
		const std::vector<std::pair<uint32, uint32>>& GetMaterialOffsets() const { return m_materialOffsets; }
		std::vector<XMaterial*> GetMaterials() const { return Core::GetRawPtrVector(m_materials); }

		std::vector<XMaterial*> GetMaterialsInOffsetOrder() const;



		void ToString(std::ostream& os) const override { os << "asset-type: XMesh, name: " << m_label; }
	};
}

#endif // XMESH_H
