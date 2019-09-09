#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "world/nodes/camera/CameraNode.h"

namespace OpenGL
{
	struct GLTestGeometry;

	class GLTestRenderer :  public GLRendererBase
	{
		MAKE_METADATA(GLTestRenderer)

	protected:
		//std::shared_ptr<GLShader> m_instancedShader;
		std::shared_ptr<GLShader> m_nonInstancedShader;

		
		std::vector<std::shared_ptr<GLTestGeometry>> m_geometryObservers;

		//std::vector<std::shared_ptr<GLInstancedModel>> m_instancedGeometries;

		//std::shared_ptr<GLTexture> m_skyTexture;

		CameraNode* m_camera;

		int32 m_previewMode;

	public:
		DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);
		
		GLTestRenderer()
			: m_camera(nullptr), m_previewMode(0) 
		{
			m_resizeListener.BindMember(this, &GLTestRenderer::WindowResize);
		}

		~GLTestRenderer() = default;

		bool InitScene(int32 width, int32 height) override;
		
		void WindowResize(int32 width, int32 height);
		void Render() override;


		void Update() override;
	};
}
