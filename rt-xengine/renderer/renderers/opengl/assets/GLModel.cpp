#include "pch.h"

#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "renderer/renderers/opengl/GLRendererBase.h"

namespace Renderer::OpenGL
{
	GLModel::GLModel(GLRendererBase* renderer, const std::string& name)
		: GLAsset(renderer, name),
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
				std::unique_ptr<GLMesh> me = std::make_unique<GLMesh>(GetGLRenderer(), this->GetName());
				me->Load(geometryGroup.get(), m_usage);
				m_meshes.emplace_back(std::move(me));
			}
		}
	
		STOP_TIMER("uploading");

		return true;
	}
}
