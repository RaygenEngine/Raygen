#pragma once

#include "renderer/renderers/opengl/GLEditorRenderer.h"
#include "world/nodes/camera/CameraNode.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"


namespace ogl {
class GLDeferredRenderer : public GLEditorRenderer {

protected:
	// TODO: should use the camera and window respectively
	int32 m_outWidth{ 0 }, m_outHeight{ 0 };
	GLuint m_outFbo{ 0 };
	GLuint m_outTexture{ 0 };

	GLShader* m_deferredDirectionalLightShader{ nullptr };
	GLShader* m_deferredSpotLightShader{ nullptr };

	struct GBuffer {
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
	CameraNode* m_camera{ nullptr };

	void RenderGBuffer();

	void RenderDirectionalLights();
	void RenderSpotLights();

	void RenderWindow();

	GLuint m_currentTexture{ 0 };

public:
	std::string m_previewModeString{};

	// TODO:
	DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);

	GLDeferredRenderer()
		: m_camera(nullptr)
	{
		m_resizeListener.BindMember(this, &GLDeferredRenderer::WindowResize);
	}

	~GLDeferredRenderer() override;

	bool InitScene() override;

	void WindowResize(int32 width, int32 height);
	void Render() override;

	void Update() override;

	void RecompileShaders();
};
} // namespace ogl
