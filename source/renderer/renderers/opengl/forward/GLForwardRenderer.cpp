#include "pch/pch.h"

#include "renderer/renderers/opengl/forward/GLForwardRenderer.h"
#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "world/World.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/sky/SkyboxNode.h"
#include "world/nodes/RootNode.h"
#include "system/Input.h"
// WIP:
#include "platform/windows/Win32Window.h"

#include <glad/glad.h>

namespace ogl {
GLForwardRenderer::~GLForwardRenderer()
{
	glDeleteFramebuffers(1, &m_msaaFbo);
	glDeleteTextures(1, &m_msaaColorTexture);
	glDeleteRenderbuffers(1, &m_msaaDepthStencilRbo);

	glDeleteFramebuffers(1, &m_outFbo);
	glDeleteTextures(1, &m_outColorTexture);

	glDeleteVertexArrays(1, &m_bbVao);
	glDeleteBuffers(1, &m_bbVbo);

	glDeleteVertexArrays(1, &m_skyboxVao);
	glDeleteBuffers(1, &m_skyboxVbo);
}

void GLForwardRenderer::InitObservers()
{
	m_camera = Engine::GetWorld()->GetActiveCamera();
	CLOG_ABORT(!m_camera, "This renderer expects a camera node to be present!");

	auto skyboxNode = Engine::GetWorld()->GetAnyAvailableNode<SkyboxNode>();
	CLOG_ABORT(!skyboxNode, "This renderer expects a skybox node to be present!");

	m_skyboxCubemap = GetGLAssetManager()->GpuGetOrCreate<GLTexture>(skyboxNode->GetSkyMap());

	for (auto* geometryNode : Engine::GetWorld()->GetNodeMap<GeometryNode>()) {
		CreateObserver_AutoContained<GLBasicGeometry>(geometryNode, m_glGeometries);
	}

	for (auto* dirLightNode : Engine::GetWorld()->GetNodeMap<DirectionalLightNode>()) {
		CreateObserver_AutoContained<GLBasicDirectionalLight>(dirLightNode, m_glDirectionalLights);
	}

	for (auto* dirLightNode : Engine::GetWorld()->GetNodeMap<SpotLightNode>()) {
		CreateObserver_AutoContained<GLBasicSpotLight>(dirLightNode, m_glSpotLights);
	}

	for (auto* dirLightNode : Engine::GetWorld()->GetNodeMap<PunctualLightNode>()) {
		CreateObserver_AutoContained<GLBasicPunctualLight>(dirLightNode, m_glPunctualLights);
	}
}

void GLForwardRenderer::InitShaders()
{
	m_forwardSpotLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/forward/FR_Spotlight.json");

	m_forwardSpotLightShader->StoreUniformLoc("mvp");
	m_forwardSpotLightShader->StoreUniformLoc("m");
	m_forwardSpotLightShader->StoreUniformLoc("normal_matrix");
	m_forwardSpotLightShader->StoreUniformLoc("mode");
	m_forwardSpotLightShader->StoreUniformLoc("view_pos");
	m_forwardSpotLightShader->StoreUniformLoc("spot_light.world_pos");
	m_forwardSpotLightShader->StoreUniformLoc("spot_light.world_dir");
	m_forwardSpotLightShader->StoreUniformLoc("spot_light.color");
	m_forwardSpotLightShader->StoreUniformLoc("spot_light.intensity");
	m_forwardSpotLightShader->StoreUniformLoc("spot_light.vp");
	m_forwardSpotLightShader->StoreUniformLoc("spot_light.near");
	m_forwardSpotLightShader->StoreUniformLoc("spot_light.atten_coef");
	m_forwardSpotLightShader->StoreUniformLoc("spot_light.outer_cut_off");
	m_forwardSpotLightShader->StoreUniformLoc("spot_light.inner_cut_off");
	m_forwardSpotLightShader->StoreUniformLoc("base_color_factor");
	m_forwardSpotLightShader->StoreUniformLoc("emissive_factor");
	m_forwardSpotLightShader->StoreUniformLoc("ambient");
	m_forwardSpotLightShader->StoreUniformLoc("metallic_factor");
	m_forwardSpotLightShader->StoreUniformLoc("roughness_factor");
	m_forwardSpotLightShader->StoreUniformLoc("normal_scale");
	m_forwardSpotLightShader->StoreUniformLoc("occlusion_strength");
	m_forwardSpotLightShader->StoreUniformLoc("base_color_texcoord_index");
	m_forwardSpotLightShader->StoreUniformLoc("metallic_roughness_texcoord_index");
	m_forwardSpotLightShader->StoreUniformLoc("emissive_texcoord_index");
	m_forwardSpotLightShader->StoreUniformLoc("normal_texcoord_index");
	m_forwardSpotLightShader->StoreUniformLoc("occlusion_texcoord_index");
	m_forwardSpotLightShader->StoreUniformLoc("mask");
	m_forwardSpotLightShader->StoreUniformLoc("alpha_cutoff");

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
	// material
	m_forwardDirectionalLightShader->StoreUniformLoc("material.baseColorFactor");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.emissiveFactor");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.metallicFactor");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.roughnessFactor");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.normalScale");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.occlusionStrength");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.baseColorSampler.index");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.baseColorSampler.sampler");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.metallicRoughnessSampler.index");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.metallicRoughnessSampler.sampler");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.emissiveSampler.index");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.emissiveSampler.sampler");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.normalSampler.index");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.normalSampler.sampler");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.occlusionSampler.index");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.occlusionSampler.sampler");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.alphaCutoff");
	m_forwardDirectionalLightShader->StoreUniformLoc("material.mask");

	m_forwardPunctualLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/forward/FR_PunctualLight.json");

	m_forwardPunctualLightShader->StoreUniformLoc("mvp");
	m_forwardPunctualLightShader->StoreUniformLoc("m");
	m_forwardPunctualLightShader->StoreUniformLoc("normal_matrix");
	m_forwardPunctualLightShader->StoreUniformLoc("mode");
	m_forwardPunctualLightShader->StoreUniformLoc("view_pos");
	m_forwardPunctualLightShader->StoreUniformLoc("punctual_light.world_pos");
	m_forwardPunctualLightShader->StoreUniformLoc("punctual_light.color");
	m_forwardPunctualLightShader->StoreUniformLoc("punctual_light.intensity");
	m_forwardPunctualLightShader->StoreUniformLoc("punctual_light.far");
	m_forwardPunctualLightShader->StoreUniformLoc("punctual_light.atten_coef");
	m_forwardPunctualLightShader->StoreUniformLoc("base_color_factor");
	m_forwardPunctualLightShader->StoreUniformLoc("emissive_factor");
	m_forwardPunctualLightShader->StoreUniformLoc("ambient");
	m_forwardPunctualLightShader->StoreUniformLoc("metallic_factor");
	m_forwardPunctualLightShader->StoreUniformLoc("roughness_factor");
	m_forwardPunctualLightShader->StoreUniformLoc("normal_scale");
	m_forwardPunctualLightShader->StoreUniformLoc("occlusion_strength");
	m_forwardPunctualLightShader->StoreUniformLoc("base_color_texcoord_index");
	m_forwardPunctualLightShader->StoreUniformLoc("metallic_roughness_texcoord_index");
	m_forwardPunctualLightShader->StoreUniformLoc("emissive_texcoord_index");
	m_forwardPunctualLightShader->StoreUniformLoc("normal_texcoord_index");
	m_forwardPunctualLightShader->StoreUniformLoc("occlusion_texcoord_index");
	m_forwardPunctualLightShader->StoreUniformLoc("mask");
	m_forwardPunctualLightShader->StoreUniformLoc("alpha_cutoff");

	m_bBoxShader = GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/BBox.json");
	m_bBoxShader->StoreUniformLoc("vp");
	m_bBoxShader->StoreUniformLoc("color");

	m_cubemapInfDistShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/Cubemap_InfDist.json");
	m_cubemapInfDistShader->StoreUniformLoc("vp");

	m_depthPassShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/general/DepthMap_AlphaMask.json");
	m_depthPassShader->StoreUniformLoc("mvp");
	m_depthPassShader->StoreUniformLoc("base_color_factor");
	m_depthPassShader->StoreUniformLoc("base_color_texcoord_index");
	m_depthPassShader->StoreUniformLoc("alpha_cutoff");
	m_depthPassShader->StoreUniformLoc("mask");
}
// TODO: default box model (json)
float skyboxVertices[] = {
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
	const auto maxWidth = 3840;
	const auto maxHeight = 2160;
	const auto maxSamples = 4;

	// msaa fbo
	glGenFramebuffers(1, &m_msaaFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

	glGenTextures(1, &m_msaaColorTexture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, maxSamples, GL_RGB, maxWidth, maxHeight, GL_TRUE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture, 0);

	glGenRenderbuffers(1, &m_msaaDepthStencilRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_msaaDepthStencilRbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, maxSamples, GL_DEPTH24_STENCIL8, maxWidth, maxHeight);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_msaaDepthStencilRbo);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE,
		"ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

	// out fbo
	glGenFramebuffers(1, &m_outFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

	glGenTextures(1, &m_outColorTexture);
	glBindTexture(GL_TEXTURE_2D, m_outColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxWidth, maxHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outColorTexture, 0);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE,
		"ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

	// bounding boxes
	glCreateVertexArrays(1, &m_bbVao);

	glEnableVertexArrayAttrib(m_bbVao, 0);
	glVertexArrayAttribFormat(m_bbVao, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glCreateBuffers(1, &m_bbVbo);
	// TODO: (48)
	glNamedBufferData(m_bbVbo, 48 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

	glVertexArrayAttribBinding(m_bbVao, 0, 0);
	glVertexArrayVertexBuffer(m_bbVao, 0, m_bbVbo, 0, sizeof(float) * 3);

	// skybox
	glGenVertexArrays(1, &m_skyboxVao);
	glGenBuffers(1, &m_skyboxVbo);

	glBindVertexArray(m_skyboxVao);

	glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
}

bool GLForwardRenderer::InitScene()
{
	InitObservers();

	InitShaders();

	InitRenderBuffers();

	return true;
}

void GLForwardRenderer::RenderEarlyDepthPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(m_depthPassShader->programId);

	auto vp = m_camera->GetViewProjectionMatrix();

	for (auto& geometry : m_glGeometries) {
		auto m = geometry->node->GetWorldMatrix();
		auto mvp = vp * m;

		m_depthPassShader->SendMat4("mvp", mvp);

		for (auto& glMesh : geometry->glModel->meshes) {
			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			if (materialData->unlit)
				continue;

			glBindVertexArray(glMesh.vao);


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, glMaterial->baseColorTexture->id);

			m_depthPassShader->SendFloat("alpha_cutoff", materialData->alphaCutoff);
			m_depthPassShader->SendVec4("base_color_factor", materialData->baseColorFactor);
			m_depthPassShader->SendInt("base_color_texcoord_index", materialData->baseColorTexCoordIndex);
			m_depthPassShader->SendInt("mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

			glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLForwardRenderer::RenderDirectionalLights()
{
	auto ls = m_forwardDirectionalLightShader;

	for (auto light : m_glDirectionalLights) {
		// render lights
		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_camera->GetWidth(), m_camera->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

		glDepthMask(GL_FALSE); // disable depth map writes

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);

		glUseProgram(ls->programId);

		const auto root = Engine::GetWorld()->GetRoot();
		const auto vp = m_camera->GetViewProjectionMatrix();

		// global uniforms
		ls->SendVec3("wcs_viewPos", m_camera->GetWorldTranslation());

		// light
		ls->SendVec3("directionalLight.wcs_dir", light->node->GetFront());
		ls->SendVec3("directionalLight.color", light->node->GetColor());
		ls->SendFloat("directionalLight.intensity", light->node->GetIntensity());
		ls->SendInt("directionalLight.samples", light->node->GetSamples());
		ls->SendFloat("directionalLight.maxShadowBias", light->node->GetMaxShadowBias());
		ls->SendTexture("directionalLight.shadowMap", light->shadowMap, 0);

		for (auto& geometry : m_glGeometries) {
			auto m = geometry->node->GetWorldMatrix();

			static glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);

			glm::mat4 mvpBiased = biasMatrix * light->node->GetViewProjectionMatrix() * m;
			ls->SendMat4("directionalLight.mvpBiased", mvpBiased);

			auto mvp = vp * m;


			// model
			ls->SendMat4("m", m);
			ls->SendMat4("mvp", mvp);
			ls->SendMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(m))));

			for (auto& glMesh : geometry->glModel->meshes) {
				glBindVertexArray(glMesh.vao);

				GLMaterial* glMaterial = glMesh.material;
				const MaterialPod* materialData = glMaterial->LockData();

				// material
				ls->SendVec4("material.baseColorFactor", materialData->baseColorFactor);
				ls->SendVec3("material.emissiveFactor", materialData->emissiveFactor);
				ls->SendFloat("material.metallicFactor", materialData->metallicFactor);
				ls->SendFloat("material.roughnessFactor", materialData->roughnessFactor);
				ls->SendFloat("material.normalScale", materialData->normalScale);
				ls->SendFloat("material.occlusionStrength", materialData->occlusionStrength);
				ls->SendFloat("material.alphaCutoff", materialData->alphaCutoff);
				ls->SendInt("material.mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

				ls->SendInt("material.baseColorSampler.index", materialData->baseColorTexCoordIndex);
				ls->SendInt("material.metallicRoughnessSampler.index", materialData->metallicRoughnessTexCoordIndex);
				ls->SendInt("material.emissiveSampler.index", materialData->emissiveTexCoordIndex);
				ls->SendInt("material.normalSampler.index", materialData->normalTexCoordIndex);
				ls->SendInt("material.occlusionSampler.index", materialData->occlusionTexCoordIndex);

				ls->SendTexture("material.baseColorSampler.sampler", glMaterial->baseColorTexture->id, 1);
				ls->SendTexture(
					"material.metallicRoughnessSampler.sampler", glMaterial->metallicRoughnessTexture->id, 2);
				ls->SendTexture("material.emissiveSampler.sampler", glMaterial->emissiveTexture->id, 3);
				ls->SendTexture("material.normalSampler.sampler", glMaterial->normalTexture->id, 4);
				ls->SendTexture("material.occlusionSampler.sampler", glMaterial->occlusionTexture->id, 5);

				materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

				glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
			}
		}
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glDepthMask(GL_TRUE); // renable depth map writes
	}
}

void GLForwardRenderer::RenderSpotLights()
{
	for (auto light : m_glSpotLights) {
		// render lights
		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_camera->GetWidth(), m_camera->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

		glDepthMask(GL_FALSE); // disable depth map writes

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);

		glUseProgram(m_forwardSpotLightShader->programId);

		const auto root = Engine::GetWorld()->GetRoot();
		const auto vp = m_camera->GetViewProjectionMatrix();

		// global uniforms
		m_forwardSpotLightShader->SendVec3("ambient", root->GetAmbientColor());
		m_forwardSpotLightShader->SendVec3("view_pos", m_camera->GetWorldTranslation());

		// light
		m_forwardSpotLightShader->SendMat4("spot_light.vp", light->node->GetViewProjectionMatrix());
		m_forwardSpotLightShader->SendVec3("spot_light.world_pos", light->node->GetWorldTranslation());
		m_forwardSpotLightShader->SendVec3("spot_light.world_dir", light->node->GetFront());
		m_forwardSpotLightShader->SendVec3("spot_light.color", light->node->GetColor());
		m_forwardSpotLightShader->SendFloat("spot_light.intensity", light->node->GetIntensity());
		m_forwardSpotLightShader->SendFloat("spot_light.near", light->node->GetNear());
		m_forwardSpotLightShader->SendInt("spot_light.atten_coef", light->node->GetAttenuationMode());
		m_forwardSpotLightShader->SendFloat(
			"spot_light.outer_cut_off", glm::cos(glm::radians(light->node->GetOuterAperture() / 2.f)));
		m_forwardSpotLightShader->SendFloat(
			"spot_light.inner_cut_off", glm::cos(glm::radians(light->node->GetInnerAperture() / 2.f)));


		for (auto& geometry : m_glGeometries) {
			auto m = geometry->node->GetWorldMatrix();
			auto mvp = vp * m;

			// model
			m_forwardSpotLightShader->SendMat4("m", m);
			m_forwardSpotLightShader->SendMat4("mvp", mvp);
			m_forwardSpotLightShader->SendMat3("normal_matrix", glm::transpose(glm::inverse(glm::mat3(m))));

			for (auto& glMesh : geometry->glModel->meshes) {
				glBindVertexArray(glMesh.vao);

				GLMaterial* glMaterial = glMesh.material;
				const MaterialPod* materialData = glMaterial->LockData();

				// material
				m_forwardSpotLightShader->SendVec4("base_color_factor", materialData->baseColorFactor);
				m_forwardSpotLightShader->SendVec3("emissive_factor", materialData->emissiveFactor);
				m_forwardSpotLightShader->SendFloat("metallic_factor", materialData->metallicFactor);
				m_forwardSpotLightShader->SendFloat("roughness_factor", materialData->roughnessFactor);
				m_forwardSpotLightShader->SendFloat("normal_scale", materialData->normalScale);
				m_forwardSpotLightShader->SendFloat("occlusion_strength", materialData->occlusionStrength);
				m_forwardSpotLightShader->SendFloat("alpha_cutoff", materialData->alphaCutoff);
				m_forwardSpotLightShader->SendInt(
					"mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

				// uv index
				m_forwardSpotLightShader->SendInt("base_color_texcoord_index", materialData->baseColorTexCoordIndex);
				m_forwardSpotLightShader->SendInt(
					"metallic_roughness_texcoord_index", materialData->metallicRoughnessTexCoordIndex);
				m_forwardSpotLightShader->SendInt("emissive_texcoord_index", materialData->emissiveTexCoordIndex);
				m_forwardSpotLightShader->SendInt("normal_texcoord_index", materialData->normalTexCoordIndex);
				m_forwardSpotLightShader->SendInt("occlusion_texcoord_index", materialData->occlusionTexCoordIndex);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, glMaterial->baseColorTexture->id);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, glMaterial->metallicRoughnessTexture->id);

				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, glMaterial->emissiveTexture->id);

				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, glMaterial->normalTexture->id);

				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, glMaterial->occlusionTexture->id);

				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, light->shadowMap);

				materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

				glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
			}
		}
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glDepthMask(GL_TRUE); // renable depth map writes
	}
}

void GLForwardRenderer::RenderPunctualLights()
{
	for (auto light : m_glPunctualLights) {
		// render lights
		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_camera->GetWidth(), m_camera->GetHeight());

		glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

		glDepthMask(GL_FALSE); // disable depth map writes

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);

		glUseProgram(m_forwardPunctualLightShader->programId);

		const auto root = Engine::GetWorld()->GetRoot();
		const auto vp = m_camera->GetViewProjectionMatrix();

		// global uniforms
		m_forwardPunctualLightShader->SendVec3("ambient", root->GetAmbientColor());
		m_forwardPunctualLightShader->SendVec3("view_pos", m_camera->GetWorldTranslation());

		// light
		m_forwardPunctualLightShader->SendVec3("punctual_light.world_pos", light->node->GetWorldTranslation());
		m_forwardPunctualLightShader->SendVec3("punctual_light.color", light->node->GetColor());
		m_forwardPunctualLightShader->SendFloat("punctual_light.intensity", light->node->GetIntensity());
		m_forwardPunctualLightShader->SendFloat("punctual_light.far", light->node->GetFar());
		m_forwardPunctualLightShader->SendInt("punctual_light.atten_coef", light->node->GetAttenuationMode());

		for (auto& geometry : m_glGeometries) {
			auto m = geometry->node->GetWorldMatrix();
			auto mvp = vp * m;

			// model
			m_forwardPunctualLightShader->SendMat4("m", m);
			m_forwardPunctualLightShader->SendMat4("mvp", mvp);
			m_forwardPunctualLightShader->SendMat3("normal_matrix", glm::transpose(glm::inverse(glm::mat3(m))));

			for (auto& glMesh : geometry->glModel->meshes) {
				glBindVertexArray(glMesh.vao);

				GLMaterial* glMaterial = glMesh.material;
				const MaterialPod* materialData = glMaterial->LockData();

				// material
				m_forwardPunctualLightShader->SendVec4("base_color_factor", materialData->baseColorFactor);
				m_forwardPunctualLightShader->SendVec3("emissive_factor", materialData->emissiveFactor);
				m_forwardPunctualLightShader->SendFloat("metallic_factor", materialData->metallicFactor);
				m_forwardPunctualLightShader->SendFloat("roughness_factor", materialData->roughnessFactor);
				m_forwardPunctualLightShader->SendFloat("normal_scale", materialData->normalScale);
				m_forwardPunctualLightShader->SendFloat("occlusion_strength", materialData->occlusionStrength);
				m_forwardPunctualLightShader->SendFloat("alpha_cutoff", materialData->alphaCutoff);
				m_forwardPunctualLightShader->SendInt(
					"mask", materialData->alphaMode == MaterialPod::MASK ? GL_TRUE : GL_FALSE);

				// uv index
				m_forwardPunctualLightShader->SendInt(
					"base_color_texcoord_index", materialData->baseColorTexCoordIndex);
				m_forwardPunctualLightShader->SendInt(
					"metallic_roughness_texcoord_index", materialData->metallicRoughnessTexCoordIndex);
				m_forwardPunctualLightShader->SendInt("emissive_texcoord_index", materialData->emissiveTexCoordIndex);
				m_forwardPunctualLightShader->SendInt("normal_texcoord_index", materialData->normalTexCoordIndex);
				m_forwardPunctualLightShader->SendInt("occlusion_texcoord_index", materialData->occlusionTexCoordIndex);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, glMaterial->baseColorTexture->id);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, glMaterial->metallicRoughnessTexture->id);

				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, glMaterial->emissiveTexture->id);

				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, glMaterial->normalTexture->id);

				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, glMaterial->occlusionTexture->id);

				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_CUBE_MAP, light->cubeShadowMap);

				materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

				glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
			}
		}
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glDepthMask(GL_TRUE); // renable depth map writes
	}
}

void GLForwardRenderer::RenderBoundingBoxes()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	auto worldNodes = Engine::GetWorld()->GetNodeMap<Node>();

	glBindVertexArray(m_bbVao);

	auto RenderBox = [&](Box box, glm::vec4 color) {
		const GLfloat data[] = {
			box.min.x,
			box.min.y,
			box.min.z,

			box.max.x,
			box.min.y,
			box.min.z,

			box.max.x,
			box.max.y,
			box.min.z,

			box.min.x,
			box.max.y,
			box.min.z,
			//
			box.min.x,
			box.min.y,
			box.max.z,

			box.max.x,
			box.min.y,
			box.max.z,

			box.max.x,
			box.max.y,
			box.max.z,

			box.min.x,
			box.max.y,
			box.max.z,
			//
			box.min.x,
			box.min.y,
			box.min.z,

			box.min.x,
			box.min.y,
			box.max.z,

			box.max.x,
			box.min.y,
			box.min.z,

			box.max.x,
			box.min.y,
			box.max.z,
			//
			box.max.x,
			box.max.y,
			box.min.z,

			box.max.x,
			box.max.y,
			box.max.z,

			box.min.x,
			box.max.y,
			box.min.z,

			box.min.x,
			box.max.y,
			box.max.z,
		};

		glNamedBufferSubData(m_bbVbo, 0, 48 * sizeof(float), data);

		glUseProgram(m_bBoxShader->programId);
		const auto vp = m_camera->GetViewProjectionMatrix();
		m_bBoxShader->SendMat4("vp", vp);
		m_bBoxShader->SendVec4("color", color);

		// TODO: with fewer draw calls
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glDrawArrays(GL_LINE_LOOP, 4, 4);
		glDrawArrays(GL_LINES, 8, 8);
	};

	for (auto node : worldNodes) {
		RenderBox(node->m_aabb, { 1, 0, 0, 1 });
		// RenderBox(node->GetBBox(), { 1, 1, 1, 1 });
	}

	glDisable(GL_DEPTH_TEST);
}


void GLForwardRenderer::RenderSkybox()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	const auto vpNoTransformation = m_camera->GetProjectionMatrix() * glm::mat4(glm::mat3(m_camera->GetViewMatrix()));

	glUseProgram(m_cubemapInfDistShader->programId);

	m_cubemapInfDistShader->SendMat4("vp", vpNoTransformation);

	glBindVertexArray(m_skyboxVao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxCubemap->id);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void GLForwardRenderer::RenderPostProcess()
{
	// blit msaa to out
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_outFbo);

	glBlitFramebuffer(0, 0, m_camera->GetWidth(), m_camera->GetHeight(), 0, 0, m_camera->GetWidth(),
		m_camera->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// do post process here
}

void GLForwardRenderer::RenderWindowSimple()
{
	auto wnd = Engine::GetMainWindow();

	// blit out to window buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_outFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(0, 0, m_camera->GetWidth(), m_camera->GetHeight(), 0, 0, wnd->GetWidth(), wnd->GetHeight(),
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GLForwardRenderer::RenderWindowLinearized()
{
	// const auto window = Engine::GetMainWindow();

	// glViewport(0, 0, window->GetWidth(), window->GetHeight());

	//// write to window
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// glClear(GL_COLOR_BUFFER_BIT);

	// glUseProgram(m_linearizeOutShader->id);
	// glUniform1f(m_linearizeOutShader->GetUniform("near"), m_glSpotLight->node->GetNear());
	// glUniform1f(m_linearizeOutShader->GetUniform("far"), m_glSpotLight->node->GetFar());

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, m_currentTexture);

	//// big triangle trick, no vao
	// glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLForwardRenderer::Render()
{
	// perform first pass as depth pass
	RenderEarlyDepthPass();
	// render directional lights shadow maps
	RenderDirectionalLights();
	RenderSpotLights();
	RenderPunctualLights();
	// render node bounding boxes
	// RenderBoundingBoxes();
	// render skybox, seamless enabled (render last)
	RenderSkybox();
	// copy msaa to out fbo and render any post process on it
	RenderPostProcess();
	// write out texture of out fbo to window (big triangle trick)
	RenderWindowSimple();
	//! m_isOutNonLinear ? RenderWindowSimple() : RenderWindowLinearized();

	GLEditorRenderer::Render();
}

void GLForwardRenderer::RecompileShaders()
{
	m_forwardSpotLightShader->Load();
	m_forwardDirectionalLightShader->Load();
	m_forwardPunctualLightShader->Load();
	// m_simpleOutShader->Load();
	// m_linearizeOutShader->Load();
}

void GLForwardRenderer::Update()
{
	GLRendererBase::Update();
	m_camera = Engine::GetWorld()->GetActiveCamera();
	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::R)) {
		RecompileShaders();
	}
}
} // namespace ogl
