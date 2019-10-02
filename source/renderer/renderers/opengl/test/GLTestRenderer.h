#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "world/nodes/camera/CameraNode.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicSkybox.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"

namespace OpenGL
{
	class GLTestRenderer :  public GLRendererBase
	{
		MAKE_METADATA(GLTestRenderer);

	protected:

		// shaders
		GLShader* m_testShader;
		GLShader* m_screenQuadShader;

		// entities
		std::vector<std::unique_ptr<GLBasicGeometry>> m_glGeometries;
		std::vector<std::unique_ptr<GLBasicDirectionalLight>> m_glDirectionalLights;
		std::unique_ptr<GLBasicSkybox> m_skybox;
		
		// raw nodes
		CameraNode* m_camera;

		// rendering
		GLuint m_msaaFbo;
		GLuint m_msaaColorTexture;
		GLuint m_msaaDepthStencilRbo;
		
		GLuint m_outFbo;
		GLuint m_outColorTexture;
		
		int32 m_previewMode;

		void RenderDirectionalLights();
		void RenderGeometries();
		void RenderSkybox();
		void RenderPostProcess();
		void RenderWindow();

		GLuint m_currentTexture;
		
	public:
		
		std::string m_previewModeString{};

		DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);

		GLTestRenderer()
			: m_camera(nullptr),
			  m_msaaFbo(0),
			  m_msaaColorTexture(0),
		      m_msaaDepthStencilRbo(0),
			  m_outFbo(0),
			  m_outColorTexture(0),
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
