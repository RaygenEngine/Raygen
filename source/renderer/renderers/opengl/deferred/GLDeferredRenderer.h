#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "world/nodes/camera/CameraNode.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSkybox.h"



namespace OpenGL
{
	class GLDeferredRenderer :  public GLRendererBase
	{
		MAKE_METADATA(GLDeferredRenderer);

	protected:

		// TODO: should use the camera and window respectively
		int32 m_outWidth{}, m_outHeight{};
		GLuint m_outFbo{};
		GLuint m_outTexture{};
		
		GLShader* m_deferredDirectionalLightShader{};
		GLShader* m_deferredSpotLightShader{};

		struct GBuffer
		{
			GLuint fbo{ 0 };
			GLuint positionsAttachment{ 0 };
			GLuint normalsAttachment{ 0 };
			GLuint albedoOpacityAttachment{ 0 };
			GLuint metallicRoughnessOcclusionOcclusionStengthAttachment{ 0 };
			GLuint emissiveAttachment{ 0 };
			GLuint depthAttachment{ 0 };
			GLShader* shader{ nullptr };
			GLShader* shaderAlphaMask{ nullptr };

			~GBuffer();
			
		} m_gBuffer;


		std::vector<GLBasicGeometry*> m_glGeometries;
		std::vector<GLBasicDirectionalLight*> m_glDirectionalLights;
		std::vector<GLBasicSpotLight*> m_glSpotLights;
		CameraNode* m_camera;

		void RenderGBuffer();
		
		void RenderDirectionalLights();
		void RenderSpotLights();
		
		void RenderWindow();

		GLuint m_currentTexture{};
		
	public:
		
		std::string m_previewModeString{};

		DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);

		GLDeferredRenderer()
			: m_camera(nullptr)
		{
			m_resizeListener.BindMember(this, &GLDeferredRenderer::WindowResize);
		}

		~GLDeferredRenderer();

		bool InitScene() override;
		
		void WindowResize(int32 width, int32 height);
		void Render() override;

		void Update() override;

		void RecompileShaders();
	};
}
