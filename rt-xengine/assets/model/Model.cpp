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
		{
			RT_XENGINE_ASSERT(false, "glb data is not handled yet");
			ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path);
		}

		STOP_TIMER("loading");
		
		RT_XENGINE_CLOG_WARN(warn.empty(), warn.c_str());
		RT_XENGINE_CLOG_ERROR(err.empty(), err.c_str());
		
		if (!ret) return false;

		START_TIMER;

		m_info.version = gltfModel.asset.version;
		m_info.generator = gltfModel.asset.generator;
		m_info.minVersion = gltfModel.asset.minVersion;
		m_info.copyright = gltfModel.asset.copyright;

		// loads meshes in default scene (scene = model)
		auto& defaultScene = gltfModel.scenes.at(gltfModel.defaultScene);

		std::function<bool(const std::vector<int>&, glm::mat4)> RecurseChildren;
		RecurseChildren = [&] (const std::vector<int>& childrenIndices, glm::mat4 parentTransformMat)
		{
			for (auto& nodeIndex : childrenIndices)
			{		
				auto& childNode = gltfModel.nodes.at(nodeIndex);
				
				glm::mat4 localTransformMat = glm::mat4(1.f);
				
				// When matrix is defined, it must be decomposable to TRS.
				if (!childNode.matrix.empty())
				{
					for (int32 row = 0; row < 4; ++row)
						for (int32 column = 0; column < 4; ++column)
							localTransformMat[row][column] = static_cast<float>(childNode.matrix[column + 4 * row]);
				}
				else
				{
					glm::vec3 translation = glm::vec3(0.f);
					glm::quat orientation = { 1.f, 0.f, 0.f, 0.f };
					glm::vec3 scale = glm::vec3(1.f);

					if(!childNode.translation.empty())
					{
						translation[0] = static_cast<float>(childNode.translation[0]);
						translation[1] = static_cast<float>(childNode.translation[1]);
						translation[2] = static_cast<float>(childNode.translation[2]);
					}

					if (!childNode.rotation.empty())
					{
						orientation[0] = static_cast<float>(childNode.rotation[0]);
						orientation[1] = static_cast<float>(childNode.rotation[1]);
						orientation[2] = static_cast<float>(childNode.rotation[2]);
						orientation[3] = static_cast<float>(childNode.rotation[3]);
					}

					if (!childNode.scale.empty())
					{
						scale[0] = static_cast<float>(childNode.scale[0]);
						scale[1] = static_cast<float>(childNode.scale[1]);
						scale[2] = static_cast<float>(childNode.scale[2]);
					}
					
					localTransformMat = Core::GetTransformMat(translation, orientation, scale);
				}

				localTransformMat = parentTransformMat * localTransformMat;
				
				// load mesh if exists
				if (childNode.mesh != -1)
				{
					auto& gltfMesh = gltfModel.meshes.at(childNode.mesh);
					
					std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(this, Core::UnnamedDescription(gltfMesh.name));

					if (!mesh->Load(gltfModel, gltfMesh, localTransformMat))
					{
						RT_XENGINE_LOG_ERROR("Failed to load mesh, {}", mesh);
						return false;
					}

					m_meshes.emplace_back(std::move(mesh));
				}
				
				//load child's children
				if(!childNode.children.empty())
					if (!RecurseChildren(childNode.children, localTransformMat))
						return false;
			}
			
			return true;
		};
		
		auto res = RecurseChildren(defaultScene.nodes, glm::mat4(1.f));
		// TODO use scope timer
		STOP_TIMER("copying");

		return res;
	}

	void Model::Clear()
	{
		m_meshes.clear();
	}
}
