#include "pch.h"

#include "assets/model/Model.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tinygltf/tiny_gltf.h"
#include "assets/PathSystem.h"

namespace Assets
{
	Model::Model(EngineObject* pObject, const std::string& path)
		: DiskAsset(pObject, path)
	{
	}

	bool Model::Load(const std::string& path, GeometryUsage usage)
	{
		m_usage = usage;
		
		// TODO Disk asset base
		tinygltf::Model gltfModel;

		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		auto ext = PathSystem::GetExtension(path);

		INIT_TIMER;

		START_TIMER;
		
		bool ret = false;
		if (Core::CaseInsensitiveCompare(ext, ".gltf"))
			ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path);
		else if (Core::CaseInsensitiveCompare(ext, ".glb"))
			ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path);

		STOP_TIMER("loading");
		
		if (!warn.empty())
		{
			RT_XENGINE_LOG_WARN(warn.c_str());
		}

		if (!err.empty())
		{
			RT_XENGINE_LOG_ERROR(err.c_str());
		}

		if (!ret) return false;

		START_TIMER;

		m_info.version = gltfModel.asset.version;
		m_info.generator = gltfModel.asset.generator;
		m_info.minVersion = gltfModel.asset.minVersion;
		m_info.copyright = gltfModel.asset.copyright;

		for (auto& gltfMesh : gltfModel.meshes)
		{
			Mesh mesh{this, Core::UnnamedDescription(gltfMesh.name) };
			mesh.LoadFromGltfData(gltfModel, gltfMesh);

			m_meshes.emplace_back(mesh);
		}

		STOP_TIMER("copying");

		return true;
	}

	void Model::Clear()
	{
		m_meshes.clear();
	}
}
