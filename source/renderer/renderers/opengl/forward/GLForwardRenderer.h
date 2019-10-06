#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "world/nodes/camera/CameraNode.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"

namespace OpenGL
{
	struct GLBasicSkybox;

	class GLForwardRenderer :  public GLRendererBase
	{
		MAKE_METADATA(GLForwardRenderer);

	protected:
		// TODO: should use the camera and window respectively
		int32 m_outWidth{}, m_outHeight{};
		
		// shaders
		GLShader* m_testShader;
		GLShader* m_screenQuadShader;

		// entities
		std::vector<GLBasicGeometry*> m_glGeometries;
		GLBasicDirectionalLight* m_glDirectionalLight;
		GLBasicSpotLight* m_glSpotLight;
		GLBasicSkybox* m_skybox;
		
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
		void RenderSpotLights();
		void RenderGeometries();
		void RenderSkybox();
		void RenderPostProcess();
		void RenderWindow();

		GLuint m_currentTexture;
		
	public:
		
		std::string m_previewModeString{};

		DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);

		GLForwardRenderer()
			: m_camera(nullptr),
			  m_msaaFbo(0),
			  m_msaaColorTexture(0),
		      m_msaaDepthStencilRbo(0),
			  m_outFbo(0),
			  m_outColorTexture(0),
		      m_previewMode(0)
		{
			m_resizeListener.BindMember(this, &GLForwardRenderer::WindowResize);
		}

		~GLForwardRenderer();

		bool InitScene() override;
		
		void WindowResize(int32 width, int32 height);
		void Render() override;

		void Update() override;

		void RecompileShaders();
	};
}
