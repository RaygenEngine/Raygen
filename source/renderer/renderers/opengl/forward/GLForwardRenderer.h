#pragma once

#include "renderer/renderers/opengl/GLEditorRenderer.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"


class CameraNode;

namespace ogl {
struct GLBasicSkybox;

class GLForwardRenderer : public GLEditorRenderer {

protected:
	// shaders
	GLShader* m_testShader{ nullptr };
	GLShader* m_simpleOutShader{ nullptr };
	GLShader* m_linearizeOutShader{ nullptr };
	GLShader* m_bBoxShader{ nullptr };
	bool m_isOutNonLinear{ false };

	// entities
	std::vector<GLBasicGeometry*> m_glGeometries;
	GLBasicDirectionalLight* m_glDirectionalLight{ nullptr };
	GLBasicSpotLight* m_glSpotLight{ nullptr };
	GLBasicSkybox* m_skybox{ nullptr };

	// raw nodes
	CameraNode* m_camera{ nullptr };

	// rendering
	GLuint m_msaaFbo{ 0 };
	GLuint m_msaaColorTexture{ 0 };
	GLuint m_msaaDepthStencilRbo{ 0 };

	GLuint m_outFbo{ 0 };
	GLuint m_outColorTexture{ 0 };

	// bounding boxes
	GLuint m_bbVao;
	GLuint m_bbVbo;

	void RenderDirectionalLights();
	void RenderSpotLights();
	void RenderGeometries();
	void RenderBoundingBoxes();
	void RenderSkybox();
	void RenderPostProcess();
	void RenderWindowSimple();
	void RenderWindowLinearized();

	GLuint m_currentTexture{};

public:
	~GLForwardRenderer();

	bool InitScene() override;

	void Render() override;

	void Update() override;

	void RecompileShaders();

	virtual void OnNodeAddedToWorld(Node* node)
	{
		if (dynamic_cast<GeometryNode*>(node)) {
			CreateObserver_AutoContained<GLBasicGeometry>(dynamic_cast<GeometryNode*>(node), m_glGeometries);
		}
	}
};
} // namespace ogl
