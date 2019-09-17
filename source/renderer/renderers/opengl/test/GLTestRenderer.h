#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "world/nodes/camera/CameraNode.h"
#include "renderer/renderers/opengl/assets/GLShader.h"

namespace OpenGL
{
	struct GLTestGeometry;

	class GLTestRenderer :  public GLRendererBase
	{
		MAKE_METADATA(GLTestRenderer);

	protected:
		GLShader* m_testShader;
		GLShader* m_screenQuadShader;

		
		std::vector<std::shared_ptr<GLTestGeometry>> m_geometryObservers;
		CameraNode* m_camera;

		GLuint m_fbo;
		GLuint m_outTexture;
		GLuint m_depthStencilRbo;

		int32 m_previewMode;
	public:
		std::string m_previewModeString{};

		DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);

		GLTestRenderer()
			: m_camera(nullptr),
			  m_fbo(0),
			  m_outTexture(0),
		      m_depthStencilRbo(0),
		      m_previewMode(0)
		{
			m_resizeListener.BindMember(this, &GLTestRenderer::WindowResize);
		}

		~GLTestRenderer();

		bool InitScene(int32 width, int32 height) override;
		
		void WindowResize(int32 width, int32 height);
		void Render() override;

		void Update() override;
	};
}
