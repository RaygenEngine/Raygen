#include "pch.h"

#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

namespace Renderer::OpenGL
{
	GLModel::GLModel(GLAssetManager* glAssetManager, const std::string& name)
		: GLAsset(glAssetManager, name),
	      m_usage(GL_STATIC_DRAW)
	{
	}

	bool GLModel::Load(Assets::Model* data)
	{
		INIT_TIMER;

		START_TIMER;
		
		m_usage = GetGLUsage(data->GetUsage());

		for (auto& mesh : data->GetMeshes())
		{
			for (auto& geometryGroup : mesh->GetGeometryGroups())
			{
				GLMesh* ptr = new GLMesh(GetGLAssetManager(), this->GetName());
				ptr->Load(geometryGroup.get(), m_usage);
				m_meshes.emplace_back(ptr);
			}
		}
	
		STOP_TIMER("uploading");

		return true;
	}
}
