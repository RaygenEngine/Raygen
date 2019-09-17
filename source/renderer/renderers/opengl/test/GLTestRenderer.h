#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "world/nodes/camera/CameraNode.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "world/nodes/light/PunctualLightNode.h"

namespace OpenGL
{
	struct GLTestGeometry;

	class GLTestRenderer :  public GLRendererBase
	{
		MAKE_METADATA(GLTestRenderer);

	protected:
		GLShader* m_testShader;
		GLShader* m_skyboxShader;
		GLShader* m_screenQuadShader;

		GLTexture* m_skyboxCubemap;
		
		std::vector<std::shared_ptr<GLTestGeometry>> m_geometryObservers;
		CameraNode* m_camera;
		PunctualLightNode* m_light;

		GLuint m_fbo;
		GLuint m_outTexture;
		GLuint m_depthStencilRbo;

		GLuint m_skyboxVAO;
		GLuint m_skyboxVBO;



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
