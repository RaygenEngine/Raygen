#pragma once

#include "renderer/renderers/opengl/GLRendererBase.h"
#include "world/nodes/camera/CameraNode.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"

namespace OpenGL {
struct GLBasicSkybox;

class GLForwardRenderer : public GLRendererBase {
	MAKE_METADATA(GLForwardRenderer);

protected:
	// shaders
	GLShader* m_testShader{nullptr};
	GLShader* m_simpleOutShader{nullptr};
	GLShader* m_linearizeOutShader{nullptr};
	bool m_isOutNonLinear{ false };

	// entities
	std::vector<GLBasicGeometry*> m_glGeometries;
	GLBasicDirectionalLight* m_glDirectionalLight{nullptr};
	GLBasicSpotLight* m_glSpotLight{nullptr};
	GLBasicSkybox* m_skybox{nullptr};

	// raw nodes
	CameraNode* m_camera{nullptr};

	// rendering
	GLuint m_msaaFbo{ 0 };
	GLuint m_msaaColorTexture{ 0 };
	GLuint m_msaaDepthStencilRbo{ 0 };

	GLuint m_outFbo{ 0 };
	GLuint m_outColorTexture{ 0 };

	int32 m_previewMode{ 0 };

	void RenderDirectionalLights();
	void RenderSpotLights();
	void RenderGeometries();
	void RenderSkybox();
	void RenderPostProcess();
	void RenderWindowSimple();
	void RenderWindowLinearized();

	GLuint m_currentTexture{};

public:
	std::string m_previewModeString{};

	~GLForwardRenderer();

	bool InitScene() override;

	void Render() override;

	void Update() override;

	void RecompileShaders();
};
} // namespace OpenGL
