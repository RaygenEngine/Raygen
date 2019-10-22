#include "pch/pch.h"

#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/dovr/GLDOVRRenderer.h"
#include "world/World.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/vr/OVRNode.h"
#include "world/nodes/sky/SkyboxNode.h"
#include "system/Input.h"
#include "platform/windows/Win32Window.h"
#include "renderer/renderers/opengl/GLUtil.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <ovr/Extras/OVR_Math.h>
#include <ovr/OVR_CAPI_GL.h>


namespace ogl {

GLDOVRRenderer::Eye::Eye(ovrSession session, CameraNode* camera)
	: session(session)
	, camera(camera)
{
	const auto width = camera->GetWidth();
	const auto height = camera->GetHeight();

	ovrTextureSwapChainDesc desc = {};
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	// no MSAA textures.
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;
	{
		const ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &colorTextureChain);
		int32 length = 0;
		ovr_GetTextureSwapChainLength(session, colorTextureChain, &length);
		if (OVR_SUCCESS(result)) {
			for (int32 i = 0; i < length; ++i) {
				GLuint chainTexId;
				ovr_GetTextureSwapChainBufferGL(session, colorTextureChain, i, &chainTexId);
				glBindTexture(GL_TEXTURE_2D, chainTexId);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}
	desc.Format = OVR_FORMAT_D32_FLOAT;
	{
		const ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &depthTextureChain);

		int32 length = 0;
		ovr_GetTextureSwapChainLength(session, depthTextureChain, &length);

		if (OVR_SUCCESS(result)) {
			for (int i = 0; i < length; ++i) {
				GLuint chainTexId;
				ovr_GetTextureSwapChainBufferGL(session, depthTextureChain, i, &chainTexId);
				glBindTexture(GL_TEXTURE_2D, chainTexId);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}

	glGenFramebuffers(1, &outFbo);
}

GLDOVRRenderer::Eye::~Eye()
{
	ovr_DestroyTextureSwapChain(session, colorTextureChain);
	ovr_DestroyTextureSwapChain(session, depthTextureChain);
	glDeleteFramebuffers(1, &outFbo);
}

void GLDOVRRenderer::Eye::SwapTexture()
{
	GLuint curColorTexId;
	{
		int32 curIndex;
		ovr_GetTextureSwapChainCurrentIndex(session, colorTextureChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(session, colorTextureChain, curIndex, &curColorTexId);
	}

	GLuint curDepthTexId;
	{
		int32 curIndex;
		ovr_GetTextureSwapChainCurrentIndex(session, depthTextureChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(session, depthTextureChain, curIndex, &curDepthTexId);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, outFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curColorTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, curDepthTexId, 0);

	// clean outFbo for next frame
	glViewport(0, 0, camera->GetWidth(), camera->GetHeight());

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// glViewport(0, 0, texSize.w, texSize.h);
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// glEnable(GL_FRAMEBUFFER_SRGB);
}

void GLDOVRRenderer::Eye::Commit()
{
	ovr_CommitTextureSwapChain(session, colorTextureChain);
	ovr_CommitTextureSwapChain(session, depthTextureChain);
}

GLDOVRRenderer::GBuffer::~GBuffer()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &positionsAttachment);
	glDeleteTextures(1, &normalsAttachment);
	glDeleteTextures(1, &albedoOpacityAttachment);
	glDeleteTextures(1, &specularAttachment);
	glDeleteTextures(1, &emissiveAttachment);
	glDeleteTextures(1, &depthAttachment);
}

// WIP
GLDOVRRenderer::~GLDOVRRenderer()
{
	auto session = m_ovr->GetOVRSession();

	ovr_DestroyMirrorTexture(session, m_mirrorTexture);
	glDeleteFramebuffers(1, &m_mirrorFBO);
}

void GLDOVRRenderer::InitObservers()
{
	// TODO: will remove
	m_ovr = Engine::GetWorld()->GetAnyAvailableNode<OVRNode>();
	CLOG_ABORT(!m_ovr, "This renderer expects an ovr node to be present!");

	auto skyboxNode = Engine::GetWorld()->GetAnyAvailableNode<SkyboxNode>();
	CLOG_ABORT(!skyboxNode, "This renderer expects a skybox node to be present!");

	m_skyboxCubemap = GetGLAssetManager()->GpuGetOrCreate<GLTexture>(skyboxNode->GetSkyMap());

	for (auto geometryNode : Engine::GetWorld()->GetNodeMap<GeometryNode>()) {
		CreateObserver_AutoContained<GLBasicGeometry>(geometryNode, m_glGeometries);
	}

	for (auto lightNode : Engine::GetWorld()->GetNodeMap<DirectionalLightNode>()) {
		CreateObserver_AutoContained<GLBasicDirectionalLight>(lightNode, m_glDirectionalLights);
	}

	for (auto lightNode : Engine::GetWorld()->GetNodeMap<SpotLightNode>()) {
		CreateObserver_AutoContained<GLBasicSpotLight>(lightNode, m_glSpotLights);
	}

	for (auto lightNode : Engine::GetWorld()->GetNodeMap<PunctualLightNode>()) {
		CreateObserver_AutoContained<GLBasicPunctualLight>(lightNode, m_glPunctualLights);
	}
}

void GLDOVRRenderer::InitShaders()
{
	m_gBuffer.shader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_GBuffer_AlphaMask.json");

	m_gBuffer.shader->StoreUniformLoc("mvp");
	m_gBuffer.shader->StoreUniformLoc("m");
	m_gBuffer.shader->StoreUniformLoc("normalMatrix");
	// material
	m_gBuffer.shader->StoreUniformLoc("material.baseColorFactor");
	m_gBuffer.shader->StoreUniformLoc("material.emissiveFactor");
	m_gBuffer.shader->StoreUniformLoc("material.metallicFactor");
	m_gBuffer.shader->StoreUniformLoc("material.roughnessFactor");
	m_gBuffer.shader->StoreUniformLoc("material.normalScale");
	m_gBuffer.shader->StoreUniformLoc("material.occlusionStrength");
	m_gBuffer.shader->StoreUniformLoc("material.baseColorTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.baseColorSampler");
	m_gBuffer.shader->StoreUniformLoc("material.metallicRoughnessTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.metallicRoughnessSampler");
	m_gBuffer.shader->StoreUniformLoc("material.emissiveTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.emissiveSampler");
	m_gBuffer.shader->StoreUniformLoc("material.normalTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.normalSampler");
	m_gBuffer.shader->StoreUniformLoc("material.occlusionTexcoordIndex");
	m_gBuffer.shader->StoreUniformLoc("material.occlusionSampler");
	m_gBuffer.shader->StoreUniformLoc("material.alphaCutoff");
	m_gBuffer.shader->StoreUniformLoc("material.mask");

	m_deferredDirectionalLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_DirectionalLight.json");

	m_deferredDirectionalLightShader->StoreUniformLoc("wcs_viewPos");
	m_deferredDirectionalLightShader->StoreUniformLoc("invTextureSize");

	// directional light
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.wcs_dir");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.color");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.intensity");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.mvpBiased");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.maxShadowBias");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.samples");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.shadowMap");
	m_deferredDirectionalLightShader->StoreUniformLoc("directionalLight.castsShadow");

	// gBuffer
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.positionsSampler");
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.normalsSampler");
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.albedoOpacitySampler");
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.specularSampler");
	m_deferredDirectionalLightShader->StoreUniformLoc("gBuffer.emissiveSampler");

	m_deferredSpotLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_SpotLight.json");

	m_deferredSpotLightShader->StoreUniformLoc("wcs_viewPos");
	m_deferredSpotLightShader->StoreUniformLoc("invTextureSize");

	// spot light
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.wcs_pos");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.wcs_dir");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.outerCutOff");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.innerCutOff");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.color");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.intensity");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.attenCoef");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.mvpBiased");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.samples");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.maxShadowBias");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.shadowMap");
	m_deferredSpotLightShader->StoreUniformLoc("spotLight.castsShadow");

	// gBuffer
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.positionsSampler");
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.normalsSampler");
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.albedoOpacitySampler");
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.specularSampler");
	m_deferredSpotLightShader->StoreUniformLoc("gBuffer.emissiveSampler");

	m_deferredPunctualLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_PunctualLight.json");

	m_deferredPunctualLightShader->StoreUniformLoc("wcs_viewPos");
	m_deferredPunctualLightShader->StoreUniformLoc("invTextureSize");

	// punctual light
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.wcs_pos");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.color");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.intensity");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.far");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.attenCoef");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.samples");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.maxShadowBias");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.shadowCubemap");
	m_deferredPunctualLightShader->StoreUniformLoc("punctualLight.castsShadow");

	// gBuffer
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.positionsSampler");
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.normalsSampler");
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.albedoOpacitySampler");
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.specularSampler");
	m_deferredPunctualLightShader->StoreUniformLoc("gBuffer.emissiveSampler");


	m_ambientLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/deferred/DR_AmbientLight.json");

	m_ambientLightShader->StoreUniformLoc("wcs_viewPos");
	m_ambientLightShader->StoreUniformLoc("invTextureSize");
	m_ambientLightShader->StoreUniformLoc("vp_inv");

	m_windowShader = GetGLAssetManager()->GenerateFromPodPath<GLShader>(
		"/engine-data/glsl/general/QuadWriteTexture_InvTextureSize.json");
	m_windowShader->StoreUniformLoc("invTextureSize");
}

void GLDOVRRenderer::InitRenderBuffers()
{
	// eye buffers
	const auto session = m_ovr->GetOVRSession();

	m_eyes[0] = std::make_unique<Eye>(session, m_ovr->GetEyeCamera(0));
	m_eyes[1] = std::make_unique<Eye>(session, m_ovr->GetEyeCamera(1));

	// WIP
	const auto cam = m_eyes[0]->camera;
	auto width = cam->GetWidth();
	auto height = cam->GetHeight();

	glGenFramebuffers(1, &m_gBuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer.fbo);

	// - rgb: position
	glGenTextures(1, &m_gBuffer.positionsAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.positionsAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gBuffer.positionsAttachment, 0);

	// - rgb: normal
	glGenTextures(1, &m_gBuffer.normalsAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.normalsAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gBuffer.normalsAttachment, 0);

	// - rgb: albedo, a: opacity
	glGenTextures(1, &m_gBuffer.albedoOpacityAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.albedoOpacityAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gBuffer.albedoOpacityAttachment, 0);

	// - r: metallic, g: roughness, b: occlusion, a: occlusion strength
	glGenTextures(1, &m_gBuffer.specularAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.specularAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_gBuffer.specularAttachment, 0);

	// - rgb: emissive, a: <reserved>
	glGenTextures(1, &m_gBuffer.emissiveAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.emissiveAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_gBuffer.emissiveAttachment, 0);

	GLuint attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);

	glGenTextures(1, &m_gBuffer.depthAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.depthAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_gBuffer.depthAttachment, 0);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete!");

	auto wnd = Engine::GetMainWindow();

	// Mirror (for debug, def)
	ovrMirrorTextureDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = wnd->GetWidth();
	desc.Height = wnd->GetHeight();
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

	// desc.MirrorOptions = ovrMirrorOption_PostDistortion;
	// Create mirror texture and an FBO used to copy mirror texture to back buffer
	const ovrResult result = ovr_CreateMirrorTextureWithOptionsGL(session, &desc, &m_mirrorTexture);

	CLOG_ABORT(!OVR_SUCCESS(result), "Failed to construct mirror texture");

	// Configure the mirror read buffer
	GLuint texId;
	ovr_GetMirrorTextureBufferGL(session, m_mirrorTexture, &texId);

	glGenFramebuffers(1, &m_mirrorFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void GLDOVRRenderer::InitScene()
{
	InitObservers();

	InitShaders();

	InitRenderBuffers();
}

void GLDOVRRenderer::RenderGBuffer(Eye* eye)
{
	const auto cam = eye->camera;

	glViewport(0, 0, cam->GetWidth(), cam->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer.fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	auto gs = m_gBuffer.shader;
	glUseProgram(gs->programId);

	const auto vp = eye->camera->GetViewProjectionMatrix();

	// render geometry (non-instanced)
	for (auto& geometry : m_glGeometries) {
		// view frustum culling WIP
		// if (!math::BoxFrustumCollision(geometry->node->GetAABB(), eye->camera->GetFrustum())) {
		//	continue;
		//}

		auto m = geometry->node->GetWorldMatrix();
		auto mvp = vp * m;

		gs->SendMat4("m", m);
		gs->SendMat4("mvp", mvp);
		gs->SendMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(m))));

		for (auto& glMesh : geometry->glModel->meshes) {
			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			// material
			gs->SendVec4("material.baseColorFactor", materialData->baseColorFactor);
			gs->SendVec3("material.emissiveFactor", materialData->emissiveFactor);
			gs->SendFloat("material.metallicFactor", materialData->metallicFactor);
			gs->SendFloat("material.roughnessFactor", materialData->roughnessFactor);
			gs->SendFloat("material.normalScale", materialData->normalScale);
			gs->SendFloat("material.occlusionStrength", materialData->occlusionStrength);
			gs->SendFloat("material.alphaCutoff", materialData->alphaCutoff);
			gs->SendInt("material.mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);
			gs->SendInt("material.baseColorTexcoordIndex", materialData->baseColorTexCoordIndex);
			gs->SendInt("material.metallicRoughnessTexcoordIndex", materialData->metallicRoughnessTexCoordIndex);
			gs->SendInt("material.emissiveTexcoordIndex", materialData->emissiveTexCoordIndex);
			gs->SendInt("material.normalTexcoordIndex", materialData->normalTexCoordIndex);
			gs->SendInt("material.occlusionTexcoordIndex", materialData->occlusionTexCoordIndex);

			gs->SendTexture("material.baseColorSampler", glMaterial->baseColorTexture->id, 0);
			gs->SendTexture("material.metallicRoughnessSampler", glMaterial->metallicRoughnessTexture->id, 1);
			gs->SendTexture("material.emissiveSampler", glMaterial->emissiveTexture->id, 2);
			gs->SendTexture("material.normalSampler", glMaterial->normalTexture->id, 3);
			gs->SendTexture("material.occlusionSampler", glMaterial->occlusionTexture->id, 4);

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

			glBindVertexArray(glMesh.vao);
			glDrawElements(GL_TRIANGLES, glMesh.indicesCount, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLDOVRRenderer::RenderDirectionalLights(Eye* eye)
{
	auto ls = m_deferredDirectionalLightShader;

	for (auto light : m_glDirectionalLights) {

		// light AABB camera frustum culling WIP
		// if (!math::BoxFrustumCollision(light->node->GetFrustumAABB(), eye->camera->GetFrustum())) {
		//	continue;
		//}

		light->RenderShadowMap(m_glGeometries);

		const auto cam = eye->camera;
		const auto fbo = eye->outFbo;

		glViewport(0, 0, cam->GetWidth(), cam->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glUseProgram(ls->programId);

		// global uniforms
		ls->SendVec3("wcs_viewPos", eye->camera->GetWorldTranslation());
		const glm::vec2 invTextureSize = { 1.f / cam->GetWidth(), 1.f / cam->GetHeight() };
		ls->SendVec2("invTextureSize", invTextureSize);

		// light
		ls->SendVec3("directionalLight.wcs_dir", light->node->GetWorldForward());
		ls->SendVec3("directionalLight.color", light->node->GetColor());
		ls->SendFloat("directionalLight.intensity", light->node->GetIntensity());
		ls->SendInt("directionalLight.samples", light->node->GetSamples());
		ls->SendInt("directionalLight.castsShadow", light->node->CastsShadows() ? GL_TRUE : GL_FALSE);
		ls->SendFloat("directionalLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendTexture("directionalLight.shadowMap", light->shadowMap, 0);
		constexpr glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);
		glm::mat4 mvpBiased = biasMatrix * light->node->GetViewProjectionMatrix();
		ls->SendMat4("directionalLight.mvpBiased", mvpBiased);

		// gBuffer
		ls->SendTexture("gBuffer.positionsSampler", m_gBuffer.positionsAttachment, 1);
		ls->SendTexture("gBuffer.normalsSampler", m_gBuffer.normalsAttachment, 2);
		ls->SendTexture("gBuffer.albedoOpacitySampler", m_gBuffer.albedoOpacityAttachment, 3);
		ls->SendTexture("gBuffer.specularSampler", m_gBuffer.specularAttachment, 4);
		ls->SendTexture("gBuffer.emissiveSampler", m_gBuffer.emissiveAttachment, 5);

		// big triangle trick, no vao
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisable(GL_BLEND);
	}
}

void GLDOVRRenderer::RenderSpotLights(Eye* eye)
{
	auto ls = m_deferredSpotLightShader;

	for (auto light : m_glSpotLights) {

		// light AABB camera frustum culling WIP
		// if (!math::BoxFrustumCollision(light->node->GetFrustumAABB(), eye->camera->GetFrustum())) {
		//	continue;
		//}

		light->RenderShadowMap(m_glGeometries);

		const auto cam = eye->camera;
		const auto fbo = eye->outFbo;

		glViewport(0, 0, cam->GetWidth(), cam->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glUseProgram(ls->programId);

		// global uniforms
		ls->SendVec3("wcs_viewPos", eye->camera->GetWorldTranslation());
		const glm::vec2 invTextureSize = { 1.f / cam->GetWidth(), 1.f / cam->GetHeight() };
		ls->SendVec2("invTextureSize", invTextureSize);

		// light
		ls->SendVec3("spotLight.wcs_pos", light->node->GetWorldTranslation());
		ls->SendVec3("spotLight.wcs_dir", light->node->GetWorldForward());
		ls->SendFloat("spotLight.outerCutOff", glm::cos(glm::radians(light->node->GetOuterAperture() / 2.f)));
		ls->SendFloat("spotLight.innerCutOff", glm::cos(glm::radians(light->node->GetInnerAperture() / 2.f)));
		ls->SendVec3("spotLight.color", light->node->GetColor());
		ls->SendFloat("spotLight.intensity", light->node->GetIntensity());
		ls->SendInt("spotLight.attenCoef", light->node->GetAttenuationMode());
		ls->SendInt("spotLight.samples", light->node->GetSamples());
		ls->SendInt("spotLight.castsShadow", light->node->CastsShadows() ? GL_TRUE : GL_FALSE);
		ls->SendFloat("spotLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendTexture("spotLight.shadowMap", light->shadowMap, 0);
		constexpr glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);
		glm::mat4 mvpBiased = biasMatrix * light->node->GetViewProjectionMatrix();
		ls->SendMat4("spotLight.mvpBiased", mvpBiased);

		// gBuffer
		ls->SendTexture("gBuffer.positionsSampler", m_gBuffer.positionsAttachment, 1);
		ls->SendTexture("gBuffer.normalsSampler", m_gBuffer.normalsAttachment, 2);
		ls->SendTexture("gBuffer.albedoOpacitySampler", m_gBuffer.albedoOpacityAttachment, 3);
		ls->SendTexture("gBuffer.specularSampler", m_gBuffer.specularAttachment, 4);
		ls->SendTexture("gBuffer.emissiveSampler", m_gBuffer.emissiveAttachment, 5);

		// big triangle trick, no vao
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisable(GL_BLEND);
	}
}

void GLDOVRRenderer::RenderPunctualLights(Eye* eye)
{
	auto ls = m_deferredPunctualLightShader;

	for (auto light : m_glPunctualLights) {

		light->RenderShadowMap(m_glGeometries);

		const auto cam = eye->camera;
		const auto fbo = eye->outFbo;

		glViewport(0, 0, cam->GetWidth(), cam->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glUseProgram(ls->programId);

		// global uniforms
		ls->SendVec3("wcs_viewPos", eye->camera->GetWorldTranslation());
		const glm::vec2 invTextureSize = { 1.f / cam->GetWidth(), 1.f / cam->GetHeight() };
		ls->SendVec2("invTextureSize", invTextureSize);

		// light
		ls->SendVec3("punctualLight.wcs_pos", light->node->GetWorldTranslation());
		ls->SendVec3("punctualLight.color", light->node->GetColor());
		ls->SendFloat("punctualLight.intensity", light->node->GetIntensity());
		ls->SendFloat("punctualLight.far", light->node->GetFar());
		ls->SendInt("punctualLight.attenCoef", light->node->GetAttenuationMode());
		ls->SendInt("punctualLight.samples", light->node->GetSamples());
		ls->SendInt("punctualLight.castsShadow", light->node->CastsShadows() ? GL_TRUE : GL_FALSE);
		ls->SendFloat("punctualLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendCubeTexture("punctualLight.shadowCubemap", light->cubeShadowMap, 0);

		// gBuffer
		ls->SendTexture("gBuffer.positionsSampler", m_gBuffer.positionsAttachment, 1);
		ls->SendTexture("gBuffer.normalsSampler", m_gBuffer.normalsAttachment, 2);
		ls->SendTexture("gBuffer.albedoOpacitySampler", m_gBuffer.albedoOpacityAttachment, 3);
		ls->SendTexture("gBuffer.specularSampler", m_gBuffer.specularAttachment, 4);
		ls->SendTexture("gBuffer.emissiveSampler", m_gBuffer.emissiveAttachment, 5);

		// big triangle trick, no vao
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisable(GL_BLEND);
	}
}

void GLDOVRRenderer::RenderAmbientLight(Eye* eye)
{
	const auto cam = eye->camera;
	const auto fbo = eye->outFbo;

	glViewport(0, 0, cam->GetWidth(), cam->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glUseProgram(m_ambientLightShader->programId);

	auto vpInv = glm::inverse(eye->camera->GetViewProjectionMatrix());

	m_ambientLightShader->SendMat4("vp_inv", vpInv);
	m_ambientLightShader->SendVec3("wcs_viewPos", eye->camera->GetWorldTranslation());
	const glm::vec2 invTextureSize = { 1.f / cam->GetWidth(), 1.f / cam->GetHeight() };
	m_ambientLightShader->SendVec2("invTextureSize", invTextureSize);
	m_ambientLightShader->SendTexture(m_gBuffer.depthAttachment, 0);
	m_ambientLightShader->SendCubeTexture(m_skyboxCubemap->id, 1);

	// big triangle trick, no vao
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glDisable(GL_BLEND);
}

void GLDOVRRenderer::Render()
{
	const auto session = m_ovr->GetOVRSession();
	const auto info = m_ovr->GetRendererInfo();

	if (info.visible) {
		CLOG_ABORT(!OVR_SUCCESS(ovr_WaitToBeginFrame(session, info.frameIndex)), "Oculus Wait to begin frame failure");
		CLOG_ABORT(!OVR_SUCCESS(ovr_BeginFrame(session, info.frameIndex)), "Oculus Begin frame failure");


		for (auto& eye : m_eyes) {

			eye->SwapTexture();

			// geometry pass
			RenderGBuffer(eye.get());
			// light pass - blend lights on outFbo
			RenderDirectionalLights(eye.get());
			RenderSpotLights(eye.get());
			RenderPunctualLights(eye.get());
			// ambient pass
			RenderAmbientLight(eye.get());


			eye->Commit();
		}

		// Do distortion rendering, Present and flush/sync
		ovrLayerEyeFovDepth ld = {};
		ld.Header.Type = ovrLayerType_EyeFovDepth;
		ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft; // Because OpenGL.

		ld.ColorTexture[0] = m_eyes[0]->colorTextureChain;
		ld.DepthTexture[0] = m_eyes[0]->depthTextureChain;
		ld.ColorTexture[1] = m_eyes[1]->colorTextureChain;
		ld.DepthTexture[1] = m_eyes[1]->depthTextureChain;


		ld.ProjectionDesc = ovrTimewarpProjectionDesc_FromProjection(info.proj, ovrProjection_ClipRangeOpenGL);
		ld.SensorSampleTime = info.sensorSampleTime;

		ld.Viewport[0] = info.eyeViewports[0];
		ld.Fov[0] = info.eyeFovs[0];
		ld.RenderPose[0] = info.eyePoses[0];
		ld.Viewport[1] = info.eyeViewports[1];
		ld.Fov[1] = info.eyeFovs[1];
		ld.RenderPose[1] = info.eyePoses[1];


		ovrLayerHeader* layers = &ld.Header;
		CLOG_ABORT(
			!OVR_SUCCESS(ovr_EndFrame(session, info.frameIndex, nullptr, &layers, 1)), "Oculus End frame failure");

		m_ovr->IncrementFrameIndex();
	}
	auto wnd = Engine::GetMainWindow();
	// Blit mirror texture to back buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, wnd->GetHeight(), wnd->GetWidth(), 0, 0, 0, wnd->GetWidth(), wnd->GetHeight(),
		GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	GLEditorRenderer::Render();

	GLCheckError();
}

void GLDOVRRenderer::RecompileShaders()
{
	m_deferredDirectionalLightShader->Load();
	m_deferredSpotLightShader->Load();
	m_deferredPunctualLightShader->Load();
	m_ambientLightShader->Load();
	m_windowShader->Load();
	m_gBuffer.shader->Load();
}

void GLDOVRRenderer::Update()
{
	GLRendererBase::Update();

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::R)) {
		RecompileShaders();
	}
}
} // namespace ogl
