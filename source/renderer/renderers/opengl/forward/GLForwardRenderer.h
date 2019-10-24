#pragma once

// Forward renderer, msaa, pbr (direct), skybox

#include "renderer/renderers/opengl/GLEditorRenderer.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"
#include "renderer/renderers/opengl/basic/GLBasicPunctualLight.h"

class CameraNode;
class SkyboxNode;

namespace ogl {
class GLForwardRenderer : public GLEditorRenderer {

protected:
	// shaders
	GLShader* m_depthPassShader{ nullptr };
	GLShader* m_ambientLightShader{ nullptr };
	GLShader* m_forwardSpotLightShader{ nullptr };
	GLShader* m_forwardDirectionalLightShader{ nullptr };
	GLShader* m_forwardPunctualLightShader{ nullptr };
	GLShader* m_cubemapInfDistShader{ nullptr };
	GLShader* m_bBoxShader{ nullptr };
	GLShader* m_dummyPostProcShader{ nullptr };
	GLShader* m_windowShader{ nullptr };

	// observers
	std::vector<GLBasicGeometry*> m_glGeometries;
	std::vector<GLBasicDirectionalLight*> m_glDirectionalLights;
	std::vector<GLBasicPunctualLight*> m_glPunctualLights;
	std::vector<GLBasicSpotLight*> m_glSpotLights;

	// rendering
	GLuint m_msaaFbo{ 0u };
	GLuint m_msaaColorTexture{ 0u };
	GLuint m_msaaDepthStencilRbo{ 0u };

	GLuint m_intrFbo{ 0u };
	GLuint m_intrColorTexture{ 0u };
	GLuint m_intrDepthTexture{ 0u };

	GLuint m_outFbo{ 0u };
	GLuint m_outColorTexture{ 0u };

	// bounding boxes
	GLuint m_bbVao{ 0u };
	GLuint m_bbVbo{ 0u };

	// skybox
	GLTexture* m_skyboxCubemap{ nullptr };
	GLuint m_skyboxVao{ 0u };
	GLuint m_skyboxVbo{ 0u };

	// Init
	void InitObservers();
	void InitShaders();
	void InitRenderBuffers();

	// Render
	void ClearBuffers();
	void RenderEarlyDepthPass();
	void RenderAmbientLight();
	void RenderDirectionalLights();
	void RenderSpotLights();
	void RenderPunctualLights();
	void RenderBoundingBoxes();
	void RenderSkybox();
	void BlitMSAAtoIntr();
	void RenderPostProcess();
	void RenderWindow();

	// Update
	void RecompileShaders();

	void ActiveCameraResize() override;

public:
	~GLForwardRenderer();

	void InitScene() override;

	void Render() override;

	void Update() override;
};
} // namespace ogl
