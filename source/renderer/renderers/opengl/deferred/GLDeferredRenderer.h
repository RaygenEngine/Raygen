#pragma once

// Deferred renderer, pbr (direct), ambient pass

#include "renderer/renderers/opengl/GLEditorRenderer.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"
#include "renderer/renderers/opengl/basic/GLBasicPunctualLight.h"

class CameraNode;

namespace ogl {

class GLDeferredRenderer : public GLEditorRenderer {

protected:
	// shaders
	GLShader* m_deferredDirectionalLightShader{ nullptr };
	GLShader* m_deferredSpotLightShader{ nullptr };
	GLShader* m_deferredPunctualLightShader{ nullptr };
	GLShader* m_ambientLightShader{ nullptr };
	GLShader* m_dummyPostProcShader{ nullptr };
	GLShader* m_windowShader{ nullptr };

	// observers
	std::vector<GLBasicGeometry*> m_glGeometries;
	std::vector<GLBasicDirectionalLight*> m_glDirectionalLights;
	std::vector<GLBasicPunctualLight*> m_glPunctualLights;
	std::vector<GLBasicSpotLight*> m_glSpotLights;

	// raw nodes
	CameraNode* m_camera{ nullptr };

	// rendering
	GLuint m_lightFbo{ 0 };
	GLuint m_lightTexture{ 0 };

	GLuint m_outFbo{ 0 };
	GLuint m_outTexture{ 0 };

	struct GBuffer {
		GLuint fbo{ 0 };
		GLuint positionsAttachment{ 0 };
		GLuint normalsAttachment{ 0 };
		GLuint albedoOpacityAttachment{ 0 };
		// r: metallic, g: roughness, b: occlusion, a: occlusion strength
		GLuint specularAttachment{ 0 };
		GLuint emissiveAttachment{ 0 };
		GLuint depthAttachment{ 0 };
		GLShader* shader{ nullptr };

		~GBuffer();

	} m_gBuffer;

	// skybox
	GLTexture* m_skyboxCubemap{ nullptr };

	// Init
	void InitObservers();
	void InitShaders();
	void InitRenderBuffers();

	// Render
	void ClearBuffers();
	void RenderGBuffer();
	void RenderDirectionalLights();
	void RenderSpotLights();
	void RenderPunctualLights();
	void RenderAmbientLight();
	void RenderPostProcess();
	void RenderWindow();

	// Update
	void RecompileShaders();

	void ActiveCameraResize() override;

public:
	~GLDeferredRenderer() override;

	void InitScene() override;

	void Render() override;

	void Update() override;
};
} // namespace ogl
