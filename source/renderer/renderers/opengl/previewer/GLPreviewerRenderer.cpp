#include "pch/pch.h"

#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/previewer/GLPreviewerRenderer.h"
#include "world/World.h"
#include "world/nodes/camera/CameraNode.h"
#include "system/Input.h"
#include "platform/windows/Win32Window.h"
#include "renderer/renderers/opengl/GLUtil.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>


namespace ogl {

constexpr int32 textMaxWidth = 3840;
constexpr int32 textMaxHeight = 2160;
constexpr glm::vec2 invTextureSize = { 1.f / textMaxWidth, 1.f / textMaxHeight };

GLPreviewerRenderer::~GLPreviewerRenderer()
{
	glDeleteFramebuffers(1, &m_previewFbo);
	glDeleteTextures(1, &m_previewColorTexture);
	glDeleteTextures(1, &m_previewDepthTexture);
}

void GLPreviewerRenderer::PreviewTarget::Next()
{
	++target %= count;
	LOG_REPORT("Switched to '{}'", CurrentToString());
}

void GLPreviewerRenderer::PreviewTarget::Previous()
{
	--target;
	if (target < 0) {
		target = count - 1;
	}
	LOG_REPORT("Switched to '{}'", CurrentToString());
}

std::string GLPreviewerRenderer::MaterialPreviewTarget::CurrentToString()
{
	switch (target) {
		case ALBEDO: return "albedo";
		case METALLIC: return "metallic";
		case ROUGHNESS: return "roughness";
		case OCCLUSION: return "occlusion";
		case EMISSIVE: return "emissive";
		case NORMALMAP: return "normal map";
		case OPACITY: return "opacity";
		case ALPHACUTOFF: return "alpha cutoff";
		case ALPHAMODE: return "alpha mode";
		case DOUBLESIDED: return "double sidedness";
	}
	return "invalid";
}

std::string GLPreviewerRenderer::GeometryPreviewTarget::CurrentToString()
{
	switch (target) {
		case POSITIONS: return "positions";
		case NORMALS: return "normals";
		case TANGENTS: return "tangents";
		case BITANGENTS: return "bitangents";
		case FINALNORMALS: return "final normals";
		case TEXTCOORD0: return "textcoord 0";
		case TEXTCOORD1: return "textcoord 1";
	}
	return "invalid";
}

std::string GLPreviewerRenderer::TextCoordIndexPreviewTarget::CurrentToString()
{
	switch (target) {
		case ALBEDO: return "albedo map index";
		case METALLIC: return "metallic map index";
		case ROUGHNESS: return "roughness map index";
		case EMISSIVE: return "emissive map index";
		case NORMAL: return "normal map index";
		case OCCLUSION: return "occlusion map index";
	}
	return "invalid";
}

std::string GLPreviewerRenderer::FactorsPreviewTarget::CurrentToString()
{
	switch (target) {
		case ALBEDO: return "albedo factor";
		case METALLIC: return "metallic factor";
		case ROUGHNESS: return "roughness factor";
		case EMISSIVE: return "emissive factor";
		case NORMAL: return "normal scale";
		case OCCLUSION: return "occlusion strength";
		case OPACITY: return "opacity factor";
	}
	return "invalid";
}

void GLPreviewerRenderer::GetPreviousLight()
{
	auto lightCount = m_glDirectionalLights.size() + m_glSpotLights.size() + m_glPunctualLights.size();

	--m_lightPreviewIndex;
	if (m_lightPreviewIndex < 0) {
		m_lightPreviewIndex = m_lightPreviewIndex - 1;
	}
}

void GLPreviewerRenderer::GetNextLight()
{
	auto lightCount = m_glDirectionalLights.size() + m_glSpotLights.size() + m_glPunctualLights.size();
	++m_lightPreviewIndex %= lightCount;
}

void GLPreviewerRenderer::InitObservers()
{
	m_camera = Engine::GetWorld()->GetActiveCamera();
	CLOG_WARN(!m_camera, "Failed to find a camera.");

	RegisterObserverContainer_AutoLifetimes(m_glGeometries);
	RegisterObserverContainer_AutoLifetimes(m_glDirectionalLights);
	RegisterObserverContainer_AutoLifetimes(m_glSpotLights);
	RegisterObserverContainer_AutoLifetimes(m_glPunctualLights);
}

void GLPreviewerRenderer::InitShaders()
{
	m_previewShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/previewer/PreviewerBuffer.json");

	m_previewShader->StoreUniformLoc("mvp");
	m_previewShader->StoreUniformLoc("m");
	m_previewShader->StoreUniformLoc("normalMatrix");
	// material
	m_previewShader->StoreUniformLoc("material.baseColorFactor");
	m_previewShader->StoreUniformLoc("material.emissiveFactor");
	m_previewShader->StoreUniformLoc("material.metallicFactor");
	m_previewShader->StoreUniformLoc("material.roughnessFactor");
	m_previewShader->StoreUniformLoc("material.normalScale");
	m_previewShader->StoreUniformLoc("material.occlusionStrength");
	m_previewShader->StoreUniformLoc("material.baseColorTexcoordIndex");
	m_previewShader->StoreUniformLoc("material.baseColorSampler");
	m_previewShader->StoreUniformLoc("material.metallicRoughnessTexcoordIndex");
	m_previewShader->StoreUniformLoc("material.metallicRoughnessSampler");
	m_previewShader->StoreUniformLoc("material.emissiveTexcoordIndex");
	m_previewShader->StoreUniformLoc("material.emissiveSampler");
	m_previewShader->StoreUniformLoc("material.normalTexcoordIndex");
	m_previewShader->StoreUniformLoc("material.normalSampler");
	m_previewShader->StoreUniformLoc("material.occlusionTexcoordIndex");
	m_previewShader->StoreUniformLoc("material.occlusionSampler");
	m_previewShader->StoreUniformLoc("material.alphaCutoff");
	m_previewShader->StoreUniformLoc("material.alphaMode");
	m_previewShader->StoreUniformLoc("material.doubleSided");

	m_previewShader->StoreUniformLoc("previewMode");
	m_previewShader->StoreUniformLoc("previewTarget");

	m_cubemapPreviewShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/engine-data/glsl/previewer/CubemapPreviewer.json");
	m_cubemapPreviewShader->StoreUniformLoc("wcs_viewPos");
	m_cubemapPreviewShader->StoreUniformLoc("invTextureSize");
	m_cubemapPreviewShader->StoreUniformLoc("vp_inv");

	m_textureQuadShader = GetGLAssetManager()->GenerateFromPodPath<GLShader>(
		"/engine-data/glsl/general/QuadWriteTexture_InvTextureSize.json");
	m_textureQuadShader->StoreUniformLoc("invTextureSize");
}

void GLPreviewerRenderer::InitRenderBuffers()
{
	glGenFramebuffers(1, &m_previewFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_previewFbo);

	glGenTextures(1, &m_previewColorTexture);
	glBindTexture(GL_TEXTURE_2D, m_previewColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textMaxWidth, textMaxHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_previewColorTexture, 0);

	glGenTextures(1, &m_previewDepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_previewDepthTexture);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, textMaxWidth, textMaxHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_previewDepthTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG_FATAL("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
	}
}

void GLPreviewerRenderer::InitScene()
{
	InitObservers();

	InitShaders();

	InitRenderBuffers();
}

void GLPreviewerRenderer::RenderDirectionalLights()
{
	for (auto light : m_glDirectionalLights) {

		light->RenderShadowMap(m_glGeometries);
	}
}

void GLPreviewerRenderer::RenderPunctualLights()
{
	for (auto light : m_glPunctualLights) {

		light->RenderShadowMap(m_glGeometries);
	}
}

void GLPreviewerRenderer::RenderSpotLights()
{
	for (auto light : m_glSpotLights) {

		light->RenderShadowMap(m_glGeometries);
	}
}

void GLPreviewerRenderer::RenderPreviewBuffer()
{
	glViewport(0, 0, m_camera->GetWidth(), m_camera->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, m_previewFbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	auto ps = m_previewShader;
	glUseProgram(ps->programId);

	const auto vp = m_camera->GetViewProjectionMatrix();

	// render geometry (non-instanced)
	for (auto& geometry : m_glGeometries) {
		// view frustum culling
		if (!math::BoxFrustumCollision(geometry->node->GetAABB(), m_camera->GetFrustum())) {
			continue;
		}

		auto m = geometry->node->GetWorldMatrix();
		auto mvp = vp * m;

		ps->SendMat4("m", m);
		ps->SendMat4("mvp", mvp);
		ps->SendMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(m))));

		for (auto& glMesh : geometry->glModel->meshes) {
			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			ps->SendVec4("material.baseColorFactor", materialData->baseColorFactor);
			ps->SendVec3("material.emissiveFactor", materialData->emissiveFactor);
			ps->SendFloat("material.metallicFactor", materialData->metallicFactor);
			ps->SendFloat("material.roughnessFactor", materialData->roughnessFactor);
			ps->SendFloat("material.normalScale", materialData->normalScale);
			ps->SendFloat("material.occlusionStrength", materialData->occlusionStrength);
			ps->SendFloat("material.alphaCutoff", materialData->alphaCutoff);
			ps->SendInt("material.baseColorTexcoordIndex", materialData->baseColorTexCoordIndex);
			ps->SendInt("material.metallicRoughnessTexcoordIndex", materialData->metallicRoughnessTexCoordIndex);
			ps->SendInt("material.emissiveTexcoordIndex", materialData->emissiveTexCoordIndex);
			ps->SendInt("material.normalTexcoordIndex", materialData->normalTexCoordIndex);
			ps->SendInt("material.occlusionTexcoordIndex", materialData->occlusionTexCoordIndex);
			ps->SendTexture("material.baseColorSampler", glMaterial->baseColorTexture->id, 0);
			ps->SendTexture("material.metallicRoughnessSampler", glMaterial->metallicRoughnessTexture->id, 1);
			ps->SendTexture("material.emissiveSampler", glMaterial->emissiveTexture->id, 2);
			ps->SendTexture("material.normalSampler", glMaterial->normalTexture->id, 3);
			ps->SendTexture("material.occlusionSampler", glMaterial->occlusionTexture->id, 4);

			glm::vec3 alphaMode{};
			switch (materialData->alphaMode) {
				case MaterialPod::OPAQUE_: alphaMode = { 1.f, 1.f, 1.f }; break;
				case MaterialPod::MASK: alphaMode = { 0.f, 0.f, 0.f }; break;
				case MaterialPod::BLEND: alphaMode = { 0.4f, 0.4f, 0.4f }; break;
			}

			ps->SendVec3("material.alphaMode", alphaMode);

			glm::vec3 doubleSided = materialData->doubleSided ? glm::vec3{ 1.f, 1.f, 1.f } : glm::vec3{ 0.f, 0.f, 0.f };
			ps->SendVec3("material.doubleSided", doubleSided);

			m_previewShader->SendInt("previewMode", m_previewMode);
			m_previewShader->SendInt("previewTarget", m_previewTarget->target);

			glBindVertexArray(glMesh.vao);
			glDrawElements(GL_TRIANGLES, glMesh.indicesCount, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}
	glDisable(GL_DEPTH_TEST);
}

void GLPreviewerRenderer::RenderWindow()
{
	auto wnd = Engine::GetMainWindow();
	glViewport(0, 0, wnd->GetWidth(), wnd->GetHeight());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render
	switch (m_previewMode) {
		case PM_MATERIAL:
		case PM_FACTORS:
		case PM_GEOMETRY:
		case PM_TEXTCOORD:
			// preview pass
			glUseProgram(m_textureQuadShader->programId);
			m_textureQuadShader->SendVec2("invTextureSize", invTextureSize);
			m_textureQuadShader->SendTexture(m_previewColorTexture, 0);
			break;
		case PM_DEPTH:
			// preview pass
			glUseProgram(m_textureQuadShader->programId);
			m_textureQuadShader->SendVec2("invTextureSize", invTextureSize);
			m_textureQuadShader->SendTexture(m_previewDepthTexture, 0);
			break;
		case PM_LIGHTS:
			auto lightCount = m_glDirectionalLights.size() + m_glSpotLights.size() + m_glPunctualLights.size();
			m_lightPreviewIndex %= lightCount;

			if (m_lightPreviewIndex < m_glDirectionalLights.size()) {
				glUseProgram(m_textureQuadShader->programId);
				auto light = m_glDirectionalLights[m_lightPreviewIndex];
				glm::vec2 sinvTextureSize
					= { 1.f / light->node->GetShadowMapWidth(), 1.f / light->node->GetShadowMapHeight() };
				m_textureQuadShader->SendVec2("invTextureSize", sinvTextureSize);
				m_textureQuadShader->SendTexture(light->shadowMap, 0);
			}
			else if (m_lightPreviewIndex < m_glDirectionalLights.size() + m_glSpotLights.size()) {
				glUseProgram(m_textureQuadShader->programId);
				auto rIndex = m_lightPreviewIndex - m_glDirectionalLights.size();

				auto light = m_glSpotLights[rIndex];
				glm::vec2 sinvTextureSize
					= { 1.f / light->node->GetShadowMapWidth(), 1.f / light->node->GetShadowMapHeight() };
				m_textureQuadShader->SendVec2("invTextureSize", sinvTextureSize);
				m_textureQuadShader->SendTexture(light->shadowMap, 0);
			} // punctual
			else {
				glUseProgram(m_cubemapPreviewShader->programId);

				auto rIndex = m_lightPreviewIndex - m_glDirectionalLights.size() - m_glSpotLights.size();
				auto light = m_glPunctualLights[rIndex];
				m_cubemapPreviewShader->SendVec2("invTextureSize", invTextureSize);
				auto vpInv = glm::inverse(m_camera->GetViewProjectionMatrix());
				m_cubemapPreviewShader->SendMat4("vp_inv", vpInv);
				m_cubemapPreviewShader->SendVec3("wcs_viewPos", m_camera->GetWorldTranslation());

				m_cubemapPreviewShader->SendTexture(m_previewDepthTexture, 0);
				m_cubemapPreviewShader->SendCubeTexture(light->cubeShadowMap, 1);
				GLCheckError();
			}
			break;
	}

	// big triangle trick, no vao
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLPreviewerRenderer::Render()
{
	if (m_camera) {
		RenderPreviewBuffer();

		RenderDirectionalLights();
		RenderSpotLights();
		RenderPunctualLights();

		RenderWindow();
	}

	GLEditorRenderer::Render();
} // namespace ogl

void GLPreviewerRenderer::RecompileShaders()
{
	m_previewShader->Load();
	m_textureQuadShader->Load();
	m_cubemapPreviewShader->Load();
}

void GLPreviewerRenderer::Update()
{
	GLRendererBase::Update();

	// WIP:
	m_camera = Engine::GetWorld()->GetActiveCamera();

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::M)) {
		m_previewMode = PM_MATERIAL;
		m_previewTarget = &m_materialPreviewTarget;
		LOG_REPORT("Enabled material previewing");
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::F)) {
		m_previewMode = PM_FACTORS;
		m_previewTarget = &m_factorsPreviewTarget;
		LOG_REPORT("Enabled factors previewing");
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::G)) {
		m_previewMode = PM_GEOMETRY;
		m_previewTarget = &m_geometryPreviewTarget;
		LOG_REPORT("Enabled geometry previewing");
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::T)) {
		m_previewMode = PM_TEXTCOORD;
		m_previewTarget = &m_textCoordPreviewTarget;
		LOG_REPORT("Enabled texture coordinate index previewing");
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::B)) {
		m_previewMode = PM_DEPTH;
		LOG_REPORT("Enabled depth previewing");
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::L)) {
		m_previewMode = PM_LIGHTS;
		LOG_REPORT("Enabled shadow map previewing");
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K1)) {
		switch (m_previewMode) {
			case PM_MATERIAL:
			case PM_FACTORS:
			case PM_GEOMETRY:
			case PM_TEXTCOORD: m_previewTarget->Previous(); break;
			case PM_LIGHTS: GetPreviousLight(); break;
		}
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K2)) {
		switch (m_previewMode) {
			case PM_MATERIAL:
			case PM_FACTORS:
			case PM_GEOMETRY:
			case PM_TEXTCOORD: m_previewTarget->Next(); break;
			case PM_LIGHTS: GetNextLight(); break;
		}
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::R)) {
		RecompileShaders();
	}
}
} // namespace ogl
