#include "pch/pch.h"

#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/deferred/GLDeferredRenderer.h"

#include "glad/glad.h"
#include "world/World.h"


namespace OpenGL {
GLDeferredRenderer::GBuffer::~GBuffer()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &positionsAttachment);
	glDeleteTextures(1, &normalsAttachment);
	glDeleteTextures(1, &albedoOpacityAttachment);
	glDeleteTextures(1, &metallicRoughnessOcclusionOcclusionStengthAttachment);
	glDeleteTextures(1, &emissiveAttachment);
	glDeleteRenderbuffers(1, &depthAttachment);
}

GLDeferredRenderer::~GLDeferredRenderer()
{
	glDeleteFramebuffers(1, &m_outFbo);
	glDeleteTextures(1, &m_outTexture);
}

bool GLDeferredRenderer::InitScene()
{
	m_camera = Engine::GetWorld()->GetActiveCamera();
	int32 width = m_camera->GetWidth();
	int32 height = m_camera->GetHeight();


	for (auto* geometryNode : Engine::GetWorld()->GetNodeMap<GeometryNode>()) {
		CreateObserver_AutoContained<GLBasicGeometry>(geometryNode, m_glGeometries);
	}

	for (auto* dirLightNode : Engine::GetWorld()->GetNodeMap<DirectionalLightNode>()) {
		CreateObserver_AutoContained<GLBasicDirectionalLight>(dirLightNode, m_glDirectionalLights);
	}

	for (auto* dirLightNode : Engine::GetWorld()->GetNodeMap<SpotLightNode>()) {
		CreateObserver_AutoContained<GLBasicSpotLight>(dirLightNode, m_glSpotLights);
	}

	m_gBuffer.shader = GetGLAssetManager()->GenerateFromPodPath<GLShader>("/shaders/glsl/deferred/DR_GBuffer.json");

	m_gBuffer.shader->AddUniform("mvp");
	m_gBuffer.shader->AddUniform("m");
	m_gBuffer.shader->AddUniform("normal_matrix");
	m_gBuffer.shader->AddUniform("base_color_factor");
	m_gBuffer.shader->AddUniform("emissive_factor");
	m_gBuffer.shader->AddUniform("metallic_factor");
	m_gBuffer.shader->AddUniform("roughness_factor");
	m_gBuffer.shader->AddUniform("normal_scale");
	m_gBuffer.shader->AddUniform("occlusion_strength");
	m_gBuffer.shader->AddUniform("base_color_texcoord_index");
	m_gBuffer.shader->AddUniform("metallic_roughness_texcoord_index");
	m_gBuffer.shader->AddUniform("emissive_texcoord_index");
	m_gBuffer.shader->AddUniform("normal_texcoord_index");
	m_gBuffer.shader->AddUniform("occlusion_texcoord_index");

	m_gBuffer.shaderAlphaMask
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/shaders/glsl/deferred/DR_GBuffer_AlphaMask.json");

	m_gBuffer.shaderAlphaMask->AddUniform("mvp");
	m_gBuffer.shaderAlphaMask->AddUniform("m");
	m_gBuffer.shaderAlphaMask->AddUniform("normal_matrix");
	m_gBuffer.shaderAlphaMask->AddUniform("base_color_factor");
	m_gBuffer.shaderAlphaMask->AddUniform("emissive_factor");
	m_gBuffer.shaderAlphaMask->AddUniform("metallic_factor");
	m_gBuffer.shaderAlphaMask->AddUniform("roughness_factor");
	m_gBuffer.shaderAlphaMask->AddUniform("normal_scale");
	m_gBuffer.shaderAlphaMask->AddUniform("occlusion_strength");
	m_gBuffer.shaderAlphaMask->AddUniform("alpha_cutoff");
	m_gBuffer.shaderAlphaMask->AddUniform("base_color_texcoord_index");
	m_gBuffer.shaderAlphaMask->AddUniform("metallic_roughness_texcoord_index");
	m_gBuffer.shaderAlphaMask->AddUniform("emissive_texcoord_index");
	m_gBuffer.shaderAlphaMask->AddUniform("normal_texcoord_index");
	m_gBuffer.shaderAlphaMask->AddUniform("occlusion_texcoord_index");

	m_deferredDirectionalLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/shaders/glsl/deferred/DR_DirectionalLight.json");

	m_deferredDirectionalLightShader->AddUniform("view_pos");
	m_deferredDirectionalLightShader->AddUniform("light_pos");
	m_deferredDirectionalLightShader->AddUniform("light_color");
	m_deferredDirectionalLightShader->AddUniform("light_intensity");
	m_deferredDirectionalLightShader->AddUniform("light_space_matrix");

	m_deferredSpotLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/shaders/glsl/deferred/DR_SpotLight.json");

	m_deferredSpotLightShader->AddUniform("view_pos");
	m_deferredSpotLightShader->AddUniform("light_pos");
	m_deferredSpotLightShader->AddUniform("light_color");
	m_deferredSpotLightShader->AddUniform("light_intensity");
	m_deferredSpotLightShader->AddUniform("light_space_matrix");

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
	glGenTextures(1, &m_gBuffer.metallicRoughnessOcclusionOcclusionStengthAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.metallicRoughnessOcclusionOcclusionStengthAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
		m_gBuffer.metallicRoughnessOcclusionOcclusionStengthAttachment, 0);

	// - rgb: emissive, a: <reserved>
	glGenTextures(1, &m_gBuffer.emissiveAttachment);
	glBindTexture(GL_TEXTURE_2D, m_gBuffer.emissiveAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_gBuffer.emissiveAttachment, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
	GLuint attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);

	glGenRenderbuffers(1, &m_gBuffer.depthAttachment);
	glBindRenderbuffer(GL_RENDERBUFFER, m_gBuffer.depthAttachment);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_gBuffer.depthAttachment);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG_FATAL("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
	}

	// out fbo
	glGenFramebuffers(1, &m_outFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

	glGenTextures(1, &m_outTexture);
	glBindTexture(GL_TEXTURE_2D, m_outTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG_FATAL("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
	}

	m_currentTexture = m_outTexture;

	m_outWidth = width;
	m_outHeight = height;

	return true;
}

// TODO: resize textures and stuff
void GLDeferredRenderer::WindowResize(int32 width, int32 height)
{
	m_outWidth = width;
	m_outHeight = height;
}

void GLDeferredRenderer::RenderGBuffer()
{
	glViewport(0, 0, m_outWidth, m_outHeight);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer.fbo);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT /*| GL_STENCIL_BUFFER_BIT*/);

	auto gBufferShader = m_gBuffer.shader;
	const auto vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();

	// render geometry (non-instanced)
	for (auto& geometry : m_glGeometries) {
		auto m = geometry->node->GetWorldMatrix();
		auto mvp = vp * m;

		for (auto& glMesh : geometry->glModel->meshes) {
			glBindVertexArray(glMesh.vao);

			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			switch (materialData->alphaMode) {
					// blend not handled
				case AM_BLEND:
				case AM_OPAQUE:
					gBufferShader = m_gBuffer.shader;
					glUseProgram(gBufferShader->id);
					break;
				case AM_MASK:
					gBufferShader = m_gBuffer.shaderAlphaMask;
					glUseProgram(gBufferShader->id);
					glUniform1f(m_gBuffer.shaderAlphaMask->GetUniform("alpha_cutoff"), materialData->alphaCutoff);
					break;

				case AM_INVALID: abort();
			}

			glUniformMatrix4fv(gBufferShader->GetUniform("mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
			glUniformMatrix4fv(gBufferShader->GetUniform("m"), 1, GL_FALSE, glm::value_ptr(m));
			glUniformMatrix3fv(gBufferShader->GetUniform("normal_matrix"), 1, GL_FALSE,
				glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m)))));

			glUniform4fv(
				gBufferShader->GetUniform("base_color_factor"), 1, glm::value_ptr(materialData->baseColorFactor));
			glUniform3fv(gBufferShader->GetUniform("emissive_factor"), 1, glm::value_ptr(materialData->emissiveFactor));
			glUniform1f(gBufferShader->GetUniform("metallic_factor"), materialData->metallicFactor);
			glUniform1f(gBufferShader->GetUniform("roughness_factor"), materialData->roughnessFactor);
			glUniform1f(gBufferShader->GetUniform("normal_scale"), materialData->normalScale);
			glUniform1f(gBufferShader->GetUniform("occlusion_strength"), materialData->occlusionStrength);

			glUniform1i(gBufferShader->GetUniform("base_color_texcoord_index"), materialData->baseColorTexCoordIndex);
			glUniform1i(gBufferShader->GetUniform("metallic_roughness_texcoord_index"),
				materialData->metallicRoughnessTexCoordIndex);
			glUniform1i(gBufferShader->GetUniform("emissive_texcoord_index"), materialData->emissiveTexCoordIndex);
			glUniform1i(gBufferShader->GetUniform("normal_texcoord_index"), materialData->normalTexCoordIndex);
			glUniform1i(gBufferShader->GetUniform("occlusion_texcoord_index"), materialData->occlusionTexCoordIndex);

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

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
			glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLDeferredRenderer::RenderDirectionalLights()
{
	for (auto& light : m_glDirectionalLights) {
		// render shadow map and get the light space matrix
		light->RenderShadowMap(m_glGeometries);

		// additive blend all directional lights
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glViewport(0, 0, m_outWidth, m_outHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

		glUseProgram(m_deferredDirectionalLightShader->id);

		glUniform3fv(m_deferredDirectionalLightShader->GetUniform("view_pos"), 1,
			glm::value_ptr(m_camera->GetWorldTranslation()));
		glUniform3fv(m_deferredDirectionalLightShader->GetUniform("light_pos"), 1,
			glm::value_ptr(light->node->GetWorldTranslation()));
		glUniform3fv(
			m_deferredDirectionalLightShader->GetUniform("light_color"), 1, glm::value_ptr(light->node->GetColor()));
		glUniform1f(m_deferredDirectionalLightShader->GetUniform("light_intensity"), light->node->GetIntensity());

		glUniformMatrix4fv(m_deferredDirectionalLightShader->GetUniform("light_space_matrix"), 1, GL_FALSE,
			glm::value_ptr(light->lightSpaceMatrix));

		// gBuffer
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.positionsAttachment);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.normalsAttachment);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.albedoOpacityAttachment);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.metallicRoughnessOcclusionOcclusionStengthAttachment);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.emissiveAttachment);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, light->shadowMap);

		// big triangle trick, no vao
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisable(GL_BLEND);
	}
}

void GLDeferredRenderer::RenderSpotLights()
{
	for (auto& light : m_glSpotLights) {
		light->RenderShadowMap(m_glGeometries);

		// additive blend all directional lights
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glViewport(0, 0, m_outWidth, m_outHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

		glUseProgram(m_deferredSpotLightShader->id);

		glUniform3fv(
			m_deferredSpotLightShader->GetUniform("view_pos"), 1, glm::value_ptr(m_camera->GetWorldTranslation()));
		glUniform3fv(
			m_deferredSpotLightShader->GetUniform("light_pos"), 1, glm::value_ptr(light->node->GetWorldTranslation()));
		glUniform3fv(m_deferredSpotLightShader->GetUniform("light_color"), 1, glm::value_ptr(light->node->GetColor()));
		glUniform1f(m_deferredSpotLightShader->GetUniform("light_intensity"), light->node->GetIntensity());

		glUniformMatrix4fv(m_deferredSpotLightShader->GetUniform("light_space_matrix"), 1, GL_FALSE,
			glm::value_ptr(light->lightSpaceMatrix));

		// gBuffer
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.positionsAttachment);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.normalsAttachment);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.albedoOpacityAttachment);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.metallicRoughnessOcclusionOcclusionStengthAttachment);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.emissiveAttachment);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, light->shadowMap);

		// big triangle trick, no vao
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisable(GL_BLEND);
	}
}

void GLDeferredRenderer::RenderWindow()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_outFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer

	// TODO: should be main window width and height
	glBlitFramebuffer(0, 0, m_outWidth, m_outHeight, 0, 0, m_outWidth, m_outHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GLDeferredRenderer::Render()
{
	glViewport(0, 0, m_outWidth, m_outHeight);

	// TODO: begining of rendering"?
	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// geometry pass
	RenderGBuffer();

	// light pass - blend lights on outFbo
	RenderDirectionalLights();
	RenderSpotLights();

	// post process - apply any to outFbo

	// render to window
	RenderWindow();
}

void GLDeferredRenderer::Update()
{
	GLRendererBase::Update();
	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K1)) {
		m_currentTexture = m_gBuffer.positionsAttachment;
	}
	else if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K2)) {
		m_currentTexture = m_gBuffer.normalsAttachment;
	}
	else if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K3)) {
		m_currentTexture = m_gBuffer.albedoOpacityAttachment;
	}
	else if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K4)) {
		m_currentTexture = m_gBuffer.metallicRoughnessOcclusionOcclusionStengthAttachment;
	}
	else if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K5)) {
		m_currentTexture = m_gBuffer.emissiveAttachment;
	}
	else if (Engine::GetInput()->IsKeyPressed(XVirtualKey::L)) {
		// m_currentTexture = m_glDirectionalLights[1]->shadowMap;
	}

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::R)) {
		RecompileShaders();
	}
}

void GLDeferredRenderer::RecompileShaders()
{
	m_gBuffer.shader->Load();
	m_deferredDirectionalLightShader->Load();
	m_deferredSpotLightShader->Load();
}
} // namespace OpenGL
