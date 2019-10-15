#pragma once

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
	GLShader* m_forwardSpotLightShader{ nullptr };
	GLShader* m_forwardDirectionalLightShader{ nullptr };
	GLShader* m_forwardPunctualLightShader{ nullptr };
	GLShader* m_cubemapInfDistShader{ nullptr };
	// GLShader* m_simpleOutShader{ nullptr };
	// GLShader* m_linearizeOutShader{ nullptr };
	GLShader* m_bBoxShader{ nullptr };

	// entities
	std::vector<GLBasicGeometry*> m_glGeometries;
	std::vector<GLBasicDirectionalLight*> m_glDirectionalLights;
	std::vector<GLBasicPunctualLight*> m_glPunctualLights;
	std::vector<GLBasicSpotLight*> m_glSpotLights;

	// raw nodes
	CameraNode* m_camera{ nullptr };

	// rendering
	GLuint m_msaaFbo{ 0u };
	GLuint m_msaaColorTexture{ 0u };
	GLuint m_msaaDepthStencilRbo{ 0u };

	GLuint m_outFbo{ 0u };
	GLuint m_outColorTexture{ 0u };

	// bounding boxes
	GLuint m_bbVao{ 0u };
	GLuint m_bbVbo{ 0u };

	// skybox
	GLTexture* m_skyboxCubemap;
	GLuint m_skyboxVao{ 0u };
	GLuint m_skyboxVbo{ 0u };

	// Init
	void InitObservers();
	void InitShaders();
	void InitRenderBuffers();

	// Render
	void RenderEarlyDepthPass();
	void RenderDirectionalLights();
	void RenderSpotLights();
	void RenderPunctualLights();
	void RenderBoundingBoxes();
	void RenderSkybox();
	void RenderPostProcess();
	void RenderWindowSimple();
	void RenderWindowLinearized();

	// Update
	void RecompileShaders();

public:
	~GLForwardRenderer();

	bool InitScene() override;

	void Render() override;

	void Update() override;
};
} // namespace ogl
