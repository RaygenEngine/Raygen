#include "pch/pch.h"

#include "renderer/renderers/opengl/forward/GLForwardRenderer.h"
#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/GLPreviewer.h"
#include "world/World.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/light/AmbientNode.h"
#include "world/nodes/RootNode.h"
#include "system/Input.h"
#include "renderer/renderers/opengl/GLUtil.h"
#include "platform/windows/Win32Window.h"

#include <glad/glad.h>
#include "editor/Editor.h"

namespace ogl {


constexpr int32 msaaSamples = 4;

GLForwardRenderer::~GLForwardRenderer()
{
	glDeleteFramebuffers(1, &m_msaaFbo);
	glDeleteTextures(1, &m_msaaColorTexture);
	glDeleteRenderbuffers(1, &m_msaaDepthStencilRbo);

	glDeleteFramebuffers(1, &m_intrFbo);
	glDeleteTextures(1, &m_intrColorTexture);
	glDeleteTextures(1, &m_intrDepthTexture);

	glDeleteFramebuffers(1, &m_outFbo);
	glDeleteTextures(1, &m_outColorTexture);

	glDeleteVertexArrays(1, &m_skyboxVao);
	glDeleteBuffers(1, &m_skyboxVbo);
}

void GLForwardRenderer::InitObservers()
{
	m_activeCamera = Engine::GetWorld()->GetActiveCamera();
	CLOG_WARN(!m_activeCamera, "Renderer found no camera.");

	const auto reload = [](GLBasicAmbient* ambObs) {
		ambObs->ReloadSkybox();
	};

	m_ambient = CreateTrackerObserver_AnyAvailableWithCallback<GLBasicAmbient>(reload);

	// Auto deduces observer type and node type
	RegisterObserverContainer_AutoLifetimes(m_glGeometries);
	RegisterObserverContainer_AutoLifetimes(m_glDirectionalLights);
	RegisterObserverContainer_AutoLifetimes(m_glSpotLights);
	RegisterObserverContainer_AutoLifetimes(m_glPunctualLights);
}

void GLForwardRenderer::InitShaders()
{
	m_depthPassShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/DepthMap_AlphaMask.json");
	m_depthPassShader->StoreUniformLoc("mvp");
	m_depthPassShader->StoreUniformLoc("baseColorFactor");
	m_depthPassShader->StoreUniformLoc("baseColorTexcoordIndex");
	m_depthPassShader->StoreUniformLoc("alphaCutoff");
	m_depthPassShader->StoreUniformLoc("mask");

	m_ambientLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/forward/FR_AmbientLight.json");
	m_ambientLightShader->StoreUniformLoc("mvp");
	m_ambientLightShader->StoreUniformLoc("ambient");

	// material
	m_ambientLightShader->StoreUniformLoc("material.baseColorFactor");
	m_ambientLightShader->StoreUniformLoc("material.emissiveFactor");
	m_ambientLightShader->StoreUniformLoc("material.occlusionStrength");
	m_ambientLightShader->StoreUniformLoc("material.baseColorTexcoordIndex");
	m_ambientLightShader->StoreUniformLoc("material.baseColorSampler");
	m_ambientLightShader->StoreUniformLoc("material.emissiveTexcoordIndex");
	m_ambientLightShader->StoreUniformLoc("material.emissiveSampler");
	m_ambientLightShader->StoreUniformLoc("material.occlusionTexcoordIndex");
	m_ambientLightShader->StoreUniformLoc("material.occlusionSampler");
	m_ambientLightShader->StoreUniformLoc("material.alphaCutoff");
	m_ambientLightShader->StoreUniformLoc("material.mask");


	m_forwardDirectionalLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/forward/FR_DirectionalLight.json");

	// general
	m_forwardDirectionalLightShader->StoreUniformLoc("mvp");
	m_forwardDirectionalLightShader->StoreUniformLoc("m");
	m_forwardDirectionalLightShader->StoreUniformLoc("normalMatrix");
	m_forwardDirectionalLightShader->StoreUniformLoc("wcs_viewPos");
	// directional light
	m_forwardDirectionalLightShader->StoreUniformLoc("directionalLight.wcs_dir");
	m_forwardDirectionalLightShader->StoreUniformLoc("directionalLight.color");
	m_forwardDirectionalLightShader->StoreUniformLoc("directionalLight.intensity");
	m_forwardDirectionalLightShader->StoreUniformLoc("directionalLight.mvpBiased");
	m_forwardDirectionalLightShader->StoreUniformLoc("directionalLight.maxShadowBias");
	m_forwardDirectionalLightShader->StoreUniformLoc("directionalLight.samples");
	m_forwardDirectionalLightShader->StoreUniformLoc("directionalLight.shadowMap");
	m_forwardDirectionalLightShader->StoreUniformLoc("directionalLight.castsShadow");
	// material
	m_forwardDirectionalLightShader->StoreUniformLoc("material.baseColorFactor");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.metallicFactor");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.roughnessFactor");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.normalScale");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.baseColorTexcoordIndex");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.baseColorSampler");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.metallicRoughnessTexcoordIndex");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.metallicRoughnessSampler");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.normalTexcoordIndex");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.normalSampler");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.alphaCutoff");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.mask");

	m_forwardSpotLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/forward/FR_Spotlight.json");

	// general
	m_forwardSpotLightShader->StoreUniformLoc("mvp");
	m_forwardSpotLightShader->StoreUniformLoc("m");
	m_forwardSpotLightShader->StoreUniformLoc("normalMatrix");
	m_forwardSpotLightShader->StoreUniformLoc("wcs_viewPos");
	// spot light
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.wcs_pos");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.wcs_dir");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.outerCutOff");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.innerCutOff");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.color");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.intensity");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.attenCoef");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.mvpBiased");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.samples");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.maxShadowBias");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.shadowMap");
	m_forwardSpotLightShader->StoreUniformLoc("spotLight.castsShadow");
	// material
	m_forwardSpotLightShader->StoreUniformLoc("material.baseColorFactor");
	m_forwardSpotLightShader->StoreUniformLoc("material.metallicFactor");
	m_forwardSpotLightShader->StoreUniformLoc("material.roughnessFactor");
	m_forwardSpotLightShader->StoreUniformLoc("material.normalScale");
	m_forwardSpotLightShader->StoreUniformLoc("material.baseColorTexcoordIndex");
	m_forwardSpotLightShader->StoreUniformLoc("material.baseColorSampler");
	m_forwardSpotLightShader->StoreUniformLoc("material.metallicRoughnessTexcoordIndex");
	m_forwardSpotLightShader->StoreUniformLoc("material.metallicRoughnessSampler");
	m_forwardSpotLightShader->StoreUniformLoc("material.normalTexcoordIndex");
	m_forwardSpotLightShader->StoreUniformLoc("material.normalSampler");
	m_forwardSpotLightShader->StoreUniformLoc("material.alphaCutoff");
	m_forwardSpotLightShader->StoreUniformLoc("material.mask");

	m_forwardPunctualLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/forward/FR_PunctualLight.json");

	// general
	m_forwardPunctualLightShader->StoreUniformLoc("mvp");
	m_forwardPunctualLightShader->StoreUniformLoc("m");
	m_forwardPunctualLightShader->StoreUniformLoc("normalMatrix");
	m_forwardPunctualLightShader->StoreUniformLoc("wcs_viewPos");
	// punctual light
	m_forwardPunctualLightShader->StoreUniformLoc("punctualLight.wcs_pos");
	m_forwardPunctualLightShader->StoreUniformLoc("punctualLight.color");
	m_forwardPunctualLightShader->StoreUniformLoc("punctualLight.intensity");
	m_forwardPunctualLightShader->StoreUniformLoc("punctualLight.far");
	m_forwardPunctualLightShader->StoreUniformLoc("punctualLight.attenCoef");
	m_forwardPunctualLightShader->StoreUniformLoc("punctualLight.samples");
	m_forwardPunctualLightShader->StoreUniformLoc("punctualLight.maxShadowBias");
	m_forwardPunctualLightShader->StoreUniformLoc("punctualLight.shadowCubemap");
	m_forwardPunctualLightShader->StoreUniformLoc("punctualLight.castsShadow");
	// material
	m_forwardPunctualLightShader->StoreUniformLoc("material.baseColorFactor");
	m_forwardPunctualLightShader->StoreUniformLoc("material.metallicFactor");
	m_forwardPunctualLightShader->StoreUniformLoc("material.roughnessFactor");
	m_forwardPunctualLightShader->StoreUniformLoc("material.normalScale");
	m_forwardPunctualLightShader->StoreUniformLoc("material.baseColorTexcoordIndex");
	m_forwardPunctualLightShader->StoreUniformLoc("material.baseColorSampler");
	m_forwardPunctualLightShader->StoreUniformLoc("material.metallicRoughnessTexcoordIndex");
	m_forwardPunctualLightShader->StoreUniformLoc("material.metallicRoughnessSampler");
	m_forwardPunctualLightShader->StoreUniformLoc("material.normalTexcoordIndex");
	m_forwardPunctualLightShader->StoreUniformLoc("material.normalSampler");
	m_forwardPunctualLightShader->StoreUniformLoc("material.alphaCutoff");
	m_forwardPunctualLightShader->StoreUniformLoc("material.mask");

	m_cubemapInfDistShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/Cubemap_InfDist.json");
	m_cubemapInfDistShader->StoreUniformLoc("vp");

	m_dummyPostProcShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/post-process/DummyPostProc.json");
	m_dummyPostProcShader->StoreUniformLoc("gamma");
	m_dummyPostProcShader->StoreUniformLoc("exposure");

	m_windowShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/QuadWriteTexture.json");
}
// TODO: default box model (json)
constexpr float skyboxVertices[] = {
	// positions
	-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
	-1.0f,

	-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
	1.0f,

	1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

	-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
	1.0f
};

void GLForwardRenderer::InitRenderBuffers()
{
	const auto width = m_activeCamera->GetWidth();
	const auto height = m_activeCamera->GetHeight();

	// msaa fbo
	glGenFramebuffers(1, &m_msaaFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

	glGenTextures(1, &m_msaaColorTexture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaaSamples, GL_RGBA16F, width, height, GL_TRUE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture, 0);

	glGenRenderbuffers(1, &m_msaaDepthStencilRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_msaaDepthStencilRbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaaSamples, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_msaaDepthStencilRbo);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete!");

	// intr fbo
	glGenFramebuffers(1, &m_intrFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_intrFbo);

	glGenTextures(1, &m_intrColorTexture);
	glBindTexture(GL_TEXTURE_2D, m_intrColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_intrColorTexture, 0);

	glGenTextures(1, &m_intrDepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_intrDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_intrDepthTexture, 0);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete!");

	// out fbo
	glGenFramebuffers(1, &m_outFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

	glGenTextures(1, &m_outColorTexture);
	glBindTexture(GL_TEXTURE_2D, m_outColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outColorTexture, 0);

	// skybox
	glGenVertexArrays(1, &m_skyboxVao);
	glGenBuffers(1, &m_skyboxVbo);

	glBindVertexArray(m_skyboxVao);

	glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);


	GetGLPreviewer()->AddPreview(m_intrColorTexture, "color");
	GetGLPreviewer()->AddPreview(m_intrDepthTexture, "depth");
	GetGLPreviewer()->AddPreview(m_outColorTexture, "out");
}

void GLForwardRenderer::InitScene()
{
	GLEditorRenderer::InitScene();

	InitObservers();

	InitShaders();

	InitRenderBuffers();
}

void GLForwardRenderer::ClearBuffers()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_intrFbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GLForwardRenderer::RenderEarlyDepthPass()
{
	glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(m_depthPassShader->programId);

	auto vp = m_activeCamera->GetViewProjectionMatrix();

	for (auto& geometry : m_glGeometries) {
		// view frustum culling
		if (!m_activeCamera->IsNodeInsideFrustum(geometry->node)) {
			continue;
		}

		auto m = geometry->node->GetWorldMatrix();
		auto mvp = vp * m;

		m_depthPassShader->SendMat4("mvp", mvp);

		for (auto& glMesh : geometry->glModel->meshes) {
			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			m_depthPassShader->SendTexture(glMaterial->baseColorTexture->id, 0);
			m_depthPassShader->SendFloat("alphaCutoff", materialData->alphaCutoff);
			m_depthPassShader->SendVec4("baseColorFactor", materialData->baseColorFactor);
			m_depthPassShader->SendInt("baseColorTexcoordIndex", materialData->baseColorTexCoordIndex);
			m_depthPassShader->SendInt("mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

			glBindVertexArray(glMesh.vao);
			glDrawElements(GL_TRIANGLES, glMesh.indicesCount, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLForwardRenderer::RenderAmbientLight()
{
	if (!m_ambient->node) {
		return;
	}

	glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

	glDepthMask(GL_FALSE); // disable depth map writes

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_EQUAL);

	glUseProgram(m_ambientLightShader->programId);

	m_ambientLightShader->SendVec3("ambient", m_ambient->node->GetAmbientTerm());

	const auto vp = m_activeCamera->GetViewProjectionMatrix();

	for (auto& geometry : m_glGeometries) {
		// view frustum culling
		if (!m_activeCamera->IsNodeInsideFrustum(geometry->node)) {
			continue;
		}


		auto m = geometry->node->GetWorldMatrix();
		auto mvp = vp * m;

		m_ambientLightShader->SendMat4("mvp", mvp);

		for (auto& glMesh : geometry->glModel->meshes) {
			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			// material
			m_ambientLightShader->SendVec4("material.baseColorFactor", materialData->baseColorFactor);
			m_ambientLightShader->SendVec3("material.emissiveFactor", materialData->emissiveFactor);
			m_ambientLightShader->SendFloat("material.occlusionStrength", materialData->occlusionStrength);
			m_ambientLightShader->SendFloat("material.alphaCutoff", materialData->alphaCutoff);
			m_ambientLightShader->SendInt(
				"material.mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

			m_ambientLightShader->SendInt("material.baseColorTexcoordIndex", materialData->baseColorTexCoordIndex);
			m_ambientLightShader->SendInt("material.emissiveTexcoordIndex", materialData->emissiveTexCoordIndex);
			m_ambientLightShader->SendInt("material.occlusionTexcoordIndex", materialData->occlusionTexCoordIndex);

			m_ambientLightShader->SendTexture("material.baseColorSampler", glMaterial->baseColorTexture->id, 0);
			m_ambientLightShader->SendTexture("material.emissiveSampler", glMaterial->emissiveTexture->id, 1);
			m_ambientLightShader->SendTexture("material.occlusionSampler", glMaterial->occlusionTexture->id, 2);

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

			glBindVertexArray(glMesh.vao);
			glDrawElements(GL_TRIANGLES, glMesh.indicesCount, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}
	glDepthMask(GL_TRUE); // renable depth map writes

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLForwardRenderer::RenderDirectionalLights()
{
	auto ls = m_forwardDirectionalLightShader;

	for (auto light : m_glDirectionalLights) {

		// light AABB camera frustum culling
		if (!m_activeCamera->IsNodeInsideFrustum(light->node)) {
			continue;
		}

		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

		glDepthMask(GL_FALSE); // disable depth map writes

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);

		glUseProgram(ls->programId);

		const auto vp = m_activeCamera->GetViewProjectionMatrix();

		// global uniforms
		ls->SendVec3("wcs_viewPos", m_activeCamera->GetWorldTranslation());

		// light
		ls->SendVec3("directionalLight.wcs_dir", light->node->GetWorldForward());
		ls->SendVec3("directionalLight.color", light->node->GetColor());
		ls->SendFloat("directionalLight.intensity", light->node->GetIntensity());
		ls->SendInt("directionalLight.samples", light->node->GetSamples());
		ls->SendInt("directionalLight.castsShadow", light->node->HasShadow() ? GL_TRUE : GL_FALSE);
		ls->SendFloat("directionalLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendTexture("directionalLight.shadowMap", light->shadowMap, 0);

		for (auto& geometry : m_glGeometries) {
			// view frustum culling
			if (!m_activeCamera->IsNodeInsideFrustum(geometry->node)) {
				continue;
			}

			auto m = geometry->node->GetWorldMatrix();

			constexpr glm::mat4 biasMatrix(
				0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);

			glm::mat4 mvpBiased = biasMatrix * light->node->GetViewProjectionMatrix() * m;
			ls->SendMat4("directionalLight.mvpBiased", mvpBiased);

			auto mvp = vp * m;


			// model
			ls->SendMat4("m", m);
			ls->SendMat4("mvp", mvp);
			ls->SendMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(m))));

			for (auto& glMesh : geometry->glModel->meshes) {
				GLMaterial* glMaterial = glMesh.material;
				const MaterialPod* materialData = glMaterial->LockData();

				// material
				ls->SendVec4("material.baseColorFactor", materialData->baseColorFactor);
				ls->SendFloat("material.metallicFactor", materialData->metallicFactor);
				ls->SendFloat("material.roughnessFactor", materialData->roughnessFactor);
				ls->SendFloat("material.normalScale", materialData->normalScale);
				ls->SendFloat("material.alphaCutoff", materialData->alphaCutoff);
				ls->SendInt("material.mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

				ls->SendInt("material.baseColorTexcoordIndex", materialData->baseColorTexCoordIndex);
				ls->SendInt("material.metallicRoughnessTexcoordIndex", materialData->metallicRoughnessTexCoordIndex);
				ls->SendInt("material.normalTexcoordIndex", materialData->normalTexCoordIndex);

				ls->SendTexture("material.baseColorSampler", glMaterial->baseColorTexture->id, 1);
				ls->SendTexture("material.metallicRoughnessSampler", glMaterial->metallicRoughnessTexture->id, 2);
				ls->SendTexture("material.normalSampler", glMaterial->normalTexture->id, 3);

				materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

				glBindVertexArray(glMesh.vao);
				glDrawElements(GL_TRIANGLES, glMesh.indicesCount, GL_UNSIGNED_INT, (GLvoid*)0);
			}
		}
		glDepthMask(GL_TRUE); // renable depth map writes

		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
	}
}

void GLForwardRenderer::RenderSpotLights()
{
	auto ls = m_forwardSpotLightShader;

	for (auto light : m_glSpotLights) {

		// light AABB camera frustum culling
		if (!m_activeCamera->IsNodeInsideFrustum(light->node)) {
			continue;
		}

		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

		glDepthMask(GL_FALSE); // disable depth map writes

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);

		glUseProgram(ls->programId);

		const auto vp = m_activeCamera->GetViewProjectionMatrix();

		// global uniforms
		ls->SendVec3("wcs_viewPos", m_activeCamera->GetWorldTranslation());

		// light
		ls->SendVec3("spotLight.wcs_pos", light->node->GetWorldTranslation());
		ls->SendVec3("spotLight.wcs_dir", light->node->GetWorldForward());
		ls->SendFloat("spotLight.outerCutOff", glm::cos(light->node->GetOuterAperture() / 2.f));
		ls->SendFloat("spotLight.innerCutOff", glm::cos(light->node->GetInnerAperture() / 2.f));
		ls->SendVec3("spotLight.color", light->node->GetColor());
		ls->SendFloat("spotLight.intensity", light->node->GetIntensity());
		ls->SendInt("spotLight.attenCoef", light->node->GetAttenuationMode());
		ls->SendInt("spotLight.samples", light->node->GetSamples());
		ls->SendInt("spotLight.castsShadow", light->node->HasShadow() ? GL_TRUE : GL_FALSE);
		ls->SendFloat("spotLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendTexture("spotLight.shadowMap", light->shadowMap, 0);


		for (auto& geometry : m_glGeometries) {
			// view frustum culling
			if (!m_activeCamera->IsNodeInsideFrustum(geometry->node)) {
				continue;
			}

			auto m = geometry->node->GetWorldMatrix();

			constexpr glm::mat4 biasMatrix(
				0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);

			glm::mat4 mvpBiased = biasMatrix * light->node->GetViewProjectionMatrix() * m;
			ls->SendMat4("spotLight.mvpBiased", mvpBiased);

			auto mvp = vp * m;

			// model
			ls->SendMat4("m", m);
			ls->SendMat4("mvp", mvp);
			ls->SendMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(m))));

			for (auto& glMesh : geometry->glModel->meshes) {
				GLMaterial* glMaterial = glMesh.material;
				const MaterialPod* materialData = glMaterial->LockData();

				// material
				ls->SendVec4("material.baseColorFactor", materialData->baseColorFactor);
				ls->SendFloat("material.metallicFactor", materialData->metallicFactor);
				ls->SendFloat("material.roughnessFactor", materialData->roughnessFactor);
				ls->SendFloat("material.normalScale", materialData->normalScale);
				ls->SendFloat("material.alphaCutoff", materialData->alphaCutoff);
				ls->SendInt("material.mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

				ls->SendInt("material.baseColorTexcoordIndex", materialData->baseColorTexCoordIndex);
				ls->SendInt("material.metallicRoughnessTexcoordIndex", materialData->metallicRoughnessTexCoordIndex);
				ls->SendInt("material.normalTexcoordIndex", materialData->normalTexCoordIndex);

				ls->SendTexture("material.baseColorSampler", glMaterial->baseColorTexture->id, 1);
				ls->SendTexture("material.metallicRoughnessSampler", glMaterial->metallicRoughnessTexture->id, 2);
				ls->SendTexture("material.normalSampler", glMaterial->normalTexture->id, 3);

				materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

				glBindVertexArray(glMesh.vao);
				glDrawElements(GL_TRIANGLES, glMesh.indicesCount, GL_UNSIGNED_INT, (GLvoid*)0);
			}
		}
		glDepthMask(GL_TRUE); // renable depth map writes

		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
	}
}

void GLForwardRenderer::RenderPunctualLights()
{
	auto ls = m_forwardPunctualLightShader;

	for (auto light : m_glPunctualLights) {
		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

		glDepthMask(GL_FALSE); // disable depth map writes

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);

		glUseProgram(ls->programId);

		const auto vp = m_activeCamera->GetViewProjectionMatrix();

		// global uniforms
		ls->SendVec3("wcs_viewPos", m_activeCamera->GetWorldTranslation());

		// light
		ls->SendVec3("punctualLight.wcs_pos", light->node->GetWorldTranslation());
		ls->SendVec3("punctualLight.color", light->node->GetColor());
		ls->SendFloat("punctualLight.intensity", light->node->GetIntensity());
		ls->SendFloat("punctualLight.far", light->node->GetFar());
		ls->SendInt("punctualLight.attenCoef", light->node->GetAttenuationMode());
		ls->SendInt("punctualLight.samples", light->node->GetSamples());
		ls->SendInt("punctualLight.castsShadow", light->node->HasShadow() ? GL_TRUE : GL_FALSE);
		ls->SendFloat("punctualLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendCubeTexture("punctualLight.shadowCubemap", light->cubeShadowMap, 0);

		for (auto& geometry : m_glGeometries) {
			// view frustum culling
			if (!m_activeCamera->IsNodeInsideFrustum(geometry->node)) {
				continue;
			}

			auto m = geometry->node->GetWorldMatrix();
			auto mvp = vp * m;

			// model
			ls->SendMat4("m", m);
			ls->SendMat4("mvp", mvp);
			ls->SendMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(m))));

			for (auto& glMesh : geometry->glModel->meshes) {
				GLMaterial* glMaterial = glMesh.material;
				const MaterialPod* materialData = glMaterial->LockData();

				// material
				ls->SendVec4("material.baseColorFactor", materialData->baseColorFactor);
				ls->SendFloat("material.metallicFactor", materialData->metallicFactor);
				ls->SendFloat("material.roughnessFactor", materialData->roughnessFactor);
				ls->SendFloat("material.normalScale", materialData->normalScale);
				ls->SendFloat("material.alphaCutoff", materialData->alphaCutoff);
				ls->SendInt("material.mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

				ls->SendInt("material.baseColorTexcoordIndex", materialData->baseColorTexCoordIndex);
				ls->SendInt("material.metallicRoughnessTexcoordIndex", materialData->metallicRoughnessTexCoordIndex);
				ls->SendInt("material.normalTexcoordIndex", materialData->normalTexCoordIndex);

				ls->SendTexture("material.baseColorSampler", glMaterial->baseColorTexture->id, 1);
				ls->SendTexture("material.metallicRoughnessSampler", glMaterial->metallicRoughnessTexture->id, 2);
				ls->SendTexture("material.normalSampler", glMaterial->normalTexture->id, 3);

				materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

				glBindVertexArray(glMesh.vao);
				glDrawElements(GL_TRIANGLES, glMesh.indicesCount, GL_UNSIGNED_INT, (GLvoid*)0);
			}
		}
		glDepthMask(GL_TRUE); // renable depth map writes

		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
	}
}

void GLForwardRenderer::RenderBoundingBoxes()
{
	// TODO: this way post process effect will be on top of the bounding boxes...
	glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLEditorRenderer::RenderBoundingBoxes();

	glDisable(GL_DEPTH_TEST);
}

void GLForwardRenderer::RenderSkybox()
{
	if (!m_ambient->node || !m_ambient->texture) {
		return;
	}

	glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glUseProgram(m_cubemapInfDistShader->programId);

	const auto vpNoTransformation
		= m_activeCamera->GetProjectionMatrix() * glm::mat4(glm::mat3(m_activeCamera->GetViewMatrix()));
	m_cubemapInfDistShader->SendMat4("vp", vpNoTransformation);
	m_cubemapInfDistShader->SendCubeTexture(m_ambient->texture->id, 0);

	glBindVertexArray(m_skyboxVao);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void GLForwardRenderer::BlitMSAAtoIntr()
{
	// blit msaa to out
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_intrFbo);

	glBlitFramebuffer(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight(), 0, 0, m_activeCamera->GetWidth(),
		m_activeCamera->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBlitFramebuffer(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight(), 0, 0, m_activeCamera->GetWidth(),
		m_activeCamera->GetHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void GLForwardRenderer::RenderPostProcess()
{
	// after the first post process, blend the rest on top of it
	// or copy the lightsFbo to the outFbo at the beginning of post processing

	// Dummy post process
	glViewport(0, 0, m_activeCamera->GetWidth(), m_activeCamera->GetHeight());

	// on m_outFbo
	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

	glUseProgram(m_dummyPostProcShader->programId);

	m_dummyPostProcShader->SendFloat("gamma", m_gamma);
	m_dummyPostProcShader->SendFloat("exposure", m_exposure);
	m_dummyPostProcShader->SendTexture(m_intrColorTexture, 0);

	// big triangle trick, no vao
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLForwardRenderer::RenderWindow()
{
	auto wnd = Engine::GetMainWindow();
	glViewport(0, 0, wnd->GetWidth(), wnd->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(m_windowShader->programId);

	m_windowShader->SendTexture(m_outColorTexture, 0);

	// big triangle trick, no vao
	glDrawArrays(GL_TRIANGLES, 0, 3);
}


void GLForwardRenderer::Render()
{
	ClearBuffers();

	if (m_activeCamera) {
		// perform first pass as depth pass
		RenderEarlyDepthPass();
		// ambient pass
		RenderAmbientLight();
		// render lights
		RenderDirectionalLights();
		RenderSpotLights();
		RenderPunctualLights();
		// render node bounding boxes
		RenderBoundingBoxes();
		// render skybox, seamless enabled (render last)
		RenderSkybox();
		// blit
		BlitMSAAtoIntr();
		// render post process
		RenderPostProcess();
		// write out texture of out fbo to window (big triangle trick)
		RenderWindow();
	}

	// ensure writing of editor on the back buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLEditorRenderer::Render();

	GLCheckError();
}

void GLForwardRenderer::RecompileShaders()
{
	m_depthPassShader->Reload();
	m_forwardSpotLightShader->Reload();
	m_forwardDirectionalLightShader->Reload();
	m_forwardPunctualLightShader->Reload();
	m_cubemapInfDistShader->Reload();
	m_dummyPostProcShader->Reload();
	m_windowShader->Reload();
}

void GLForwardRenderer::ActiveCameraResize()
{
	const auto width = m_activeCamera->GetWidth();
	const auto height = m_activeCamera->GetHeight();

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaaSamples, GL_RGBA16F, width, height, GL_TRUE);

	glBindRenderbuffer(GL_RENDERBUFFER, m_msaaDepthStencilRbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaaSamples, GL_DEPTH24_STENCIL8, width, height);

	glBindTexture(GL_TEXTURE_2D, m_intrColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, m_intrDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_2D, m_outColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

void GLForwardRenderer::Update()
{
	GLRendererBase::Update();

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::R)) {
		RecompileShaders();
	}
}
} // namespace ogl
