#pragma once

// OVR Deferred renderer, pbr (direct), ambient pass

#include "renderer/renderers/opengl/GLEditorRenderer.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"
#include "renderer/renderers/opengl/basic/GLBasicGeometry.h"
#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"
#include "renderer/renderers/opengl/basic/GLBasicPunctualLight.h"

#include <ovr/OVR_CAPI.h>

class OVRNode;
class CameraNode;

namespace ogl {

class GLDOVRRenderer : public GLEditorRenderer {

	struct Eye {
		ovrSession session;
		CameraNode* camera;

		GLuint outFbo{ 0 };
		ovrTextureSwapChain colorTextureChain{ nullptr };
		ovrTextureSwapChain depthTextureChain{ nullptr };

		Eye(ovrSession session, CameraNode* camera);
		~Eye();

		void SwapTexture();
		void Commit();
	};


	// shaders
	GLShader* m_deferredDirectionalLightShader{ nullptr };
	GLShader* m_deferredSpotLightShader{ nullptr };
	GLShader* m_deferredPunctualLightShader{ nullptr };
	GLShader* m_ambientLightShader{ nullptr };
	GLShader* m_windowShader{ nullptr };

	// observers
	std::vector<GLBasicGeometry*> m_glGeometries;
	std::vector<GLBasicDirectionalLight*> m_glDirectionalLights;
	std::vector<GLBasicPunctualLight*> m_glPunctualLights;
	std::vector<GLBasicSpotLight*> m_glSpotLights;

	// raw nodes
	OVRNode* m_ovr{ nullptr };

	// rendering
	std::array<std::unique_ptr<Eye>, 2> m_eyes;

	// used to copy oculus output to window back buffer
	ovrMirrorTexture m_mirrorTexture{ nullptr };
	GLuint m_mirrorFBO{ 0 };

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
	void RenderGBuffer(int32 eyeIndex);
	void RenderDirectionalLights(int32 eyeIndex);
	void RenderSpotLights(int32 eyeIndex);
	void RenderPunctualLights(int32 eyeIndex);
	void RenderAmbientLight(int32 eyeIndex);

	// Update
	void RecompileShaders();

public:
	~GLDOVRRenderer();

	void InitScene() override;

	void Render() override;

	void Update() override;
};
} // namespace ogl
