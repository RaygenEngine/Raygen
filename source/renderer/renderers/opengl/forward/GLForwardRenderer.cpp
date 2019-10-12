#include "pch/pch.h"

#include "renderer/renderers/opengl/forward/GLForwardRenderer.h"
#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/basic/GLBasicSkybox.h"
#include "world/World.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/RootNode.h"
#include "system/Input.h"
// WIP:
#include "platform/windows/Win32Window.h"

#include <glad/glad.h>
#include <glm/ext.hpp>

namespace ogl {
GLForwardRenderer::~GLForwardRenderer()
{
	glDeleteFramebuffers(1, &m_msaaFbo);
	glDeleteTextures(1, &m_msaaColorTexture);
	glDeleteRenderbuffers(1, &m_msaaDepthStencilRbo);

	glDeleteFramebuffers(1, &m_outFbo);
	glDeleteTextures(1, &m_outColorTexture);

	glDeleteFramebuffers(1, &m_interFbo);
	glDeleteTextures(1, &m_interColorTexture);

	glDeleteVertexArrays(1, &m_bbVao);
	glDeleteBuffers(1, &m_bbVbo);
}

void GLForwardRenderer::InitObservers()
{
	m_camera = Engine::GetWorld()->GetActiveCamera();
	m_skybox = CreateObserver_AnyAvailable<GLBasicSkybox>();

	for (auto* geometryNode : Engine::GetWorld()->GetNodeMap<GeometryNode>()) {
		CreateObserver_AutoContained<GLBasicGeometry>(geometryNode, m_glGeometries);
	}

	for (auto* dirLightNode : Engine::GetWorld()->GetNodeMap<DirectionalLightNode>()) {
		CreateObserver_AutoContained<GLBasicDirectionalLight>(dirLightNode, m_glDirectionalLights);
	}

	for (auto* dirLightNode : Engine::GetWorld()->GetNodeMap<SpotLightNode>()) {
		CreateObserver_AutoContained<GLBasicSpotLight>(dirLightNode, m_glSpotLights);
	}

	CLOG_ABORT(!m_skybox, "This renderer expects a skybox node to be present!");
}

void GLForwardRenderer::InitShaders()
{
	m_simpleOutShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/shaders/glsl/general/QuadWriteTexture.json");

	m_linearizeOutShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/shaders/glsl/general/QuadWriteTexture_Linear.json");
	m_linearizeOutShader->AddUniform("near");
	m_linearizeOutShader->AddUniform("far");

	m_forwardSpotLightShader
		= GetGLAssetManager()->GenerateFromPodPath<GLShader>("/shaders/glsl/forward/FR_PBRtemp.json");

	m_forwardSpotLightShader->AddUniform("mvp");
	m_forwardSpotLightShader->AddUniform("m");
	m_forwardSpotLightShader->AddUniform("normal_matrix");
	m_forwardSpotLightShader->AddUniform("mode");
	m_forwardSpotLightShader->AddUniform("view_pos");
	m_forwardSpotLightShader->AddUniform("light_pos");
	m_forwardSpotLightShader->AddUniform("light_color");
	m_forwardSpotLightShader->AddUniform("light_intensity");
	m_forwardSpotLightShader->AddUniform("base_color_factor");
	m_forwardSpotLightShader->AddUniform("emissive_factor");
	m_forwardSpotLightShader->AddUniform("ambient");
	m_forwardSpotLightShader->AddUniform("metallic_factor");
	m_forwardSpotLightShader->AddUniform("roughness_factor");
	m_forwardSpotLightShader->AddUniform("normal_scale");
	m_forwardSpotLightShader->AddUniform("occlusion_strength");
	m_forwardSpotLightShader->AddUniform("alpha_mode");
	m_forwardSpotLightShader->AddUniform("alpha_cutoff");
	m_forwardSpotLightShader->AddUniform("double_sided");
	m_forwardSpotLightShader->AddUniform("light_space_matrix");
	m_forwardSpotLightShader->AddUniform("light_near");

	m_bBoxShader = GetGLAssetManager()->GenerateFromPodPath<GLShader>("/shaders/glsl/general/BBox.json");
	m_bBoxShader->AddUniform("vp");
	m_bBoxShader->AddUniform("color");
}

void GLForwardRenderer::InitOther()
{
	const int32 width = m_camera->GetWidth();
	const int32 height = m_camera->GetHeight();

	// msaa fbo
	glGenFramebuffers(1, &m_msaaFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

	// TODO:
	const auto samples = 4;
	glGenTextures(1, &m_msaaColorTexture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture, 0);

	glGenRenderbuffers(1, &m_msaaDepthStencilRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_msaaDepthStencilRbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_msaaDepthStencilRbo);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE,
		"ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

	// out fbo
	glGenFramebuffers(1, &m_outFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

	glGenTextures(1, &m_outColorTexture);
	glBindTexture(GL_TEXTURE_2D, m_outColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outColorTexture, 0);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE,
		"ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

	// inter fbo
	glGenFramebuffers(1, &m_interFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_interFbo);

	glGenTextures(1, &m_interColorTexture);
	glBindTexture(GL_TEXTURE_2D, m_interColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_interColorTexture, 0);

	CLOG_ABORT(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE,
		"ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

	// bounding boxes
	glCreateVertexArrays(1, &m_bbVao);

	glEnableVertexArrayAttrib(m_bbVao, 0);
	glVertexArrayAttribFormat(m_bbVao, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glCreateBuffers(1, &m_bbVbo);
	// TODO:
	glNamedBufferData(m_bbVbo, 48 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

	glVertexArrayAttribBinding(m_bbVao, 0, 0);
	glVertexArrayVertexBuffer(m_bbVao, 0, m_bbVbo, 0, sizeof(float) * 3);
}

bool GLForwardRenderer::InitScene()
{
	InitObservers();

	InitShaders();

	InitOther();

	return true;
}

void GLForwardRenderer::RenderDirectionalLights()
{
	// m_glSpotLight->RenderShadowMap(m_glGeometries);
}

void GLForwardRenderer::RenderSpotLights()
{
	for (auto light : m_glSpotLights) {
		// render lights
		light->RenderShadowMap(m_glGeometries);

		glViewport(0, 0, m_camera->GetWidth(), m_camera->GetHeight());

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);
		glClearColor(0, 0, 0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT /*| GL_STENCIL_BUFFER_BIT*/);

		glUseProgram(m_forwardSpotLightShader->id);

		// global uniforms
		glUniform3fv(
			m_forwardSpotLightShader->GetUniform("view_pos"), 1, glm::value_ptr(m_camera->GetWorldTranslation()));
		glUniform3fv(
			m_forwardSpotLightShader->GetUniform("light_pos"), 1, glm::value_ptr(light->node->GetWorldTranslation()));
		glUniform3fv(m_forwardSpotLightShader->GetUniform("light_color"), 1, glm::value_ptr(light->node->GetColor()));
		glUniform1f(m_forwardSpotLightShader->GetUniform("light_intensity"), light->node->GetIntensity());

		const auto root = Engine::GetWorld()->GetRoot();
		glUniform3fv(m_forwardSpotLightShader->GetUniform("ambient"), 1, glm::value_ptr(root->GetAmbientColor()));

		const auto vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();

		for (auto& geometry : m_glGeometries) {
			auto m = geometry->node->GetWorldMatrix();
			auto mvp = vp * m;

			glUniformMatrix4fv(m_forwardSpotLightShader->GetUniform("mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
			glUniformMatrix4fv(m_forwardSpotLightShader->GetUniform("m"), 1, GL_FALSE, glm::value_ptr(m));
			glUniformMatrix3fv(m_forwardSpotLightShader->GetUniform("normal_matrix"), 1, GL_FALSE,
				glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m)))));

			glUniformMatrix4fv(m_forwardSpotLightShader->GetUniform("light_space_matrix"), 1, GL_FALSE,
				glm::value_ptr(light->lightSpaceMatrix));

			glUniform1f(m_forwardSpotLightShader->GetUniform("roughness_factor"), light->node->GetNear());

			for (auto& glMesh : geometry->glModel->meshes) {
				glBindVertexArray(glMesh.vao);

				GLMaterial* glMaterial = glMesh.material;
				const MaterialPod* materialData = glMaterial->LockData();

				glUniform4fv(m_forwardSpotLightShader->GetUniform("base_color_factor"), 1,
					glm::value_ptr(materialData->baseColorFactor));
				glUniform3fv(m_forwardSpotLightShader->GetUniform("emissive_factor"), 1,
					glm::value_ptr(materialData->emissiveFactor));
				glUniform1f(m_forwardSpotLightShader->GetUniform("metallic_factor"), materialData->metallicFactor);
				glUniform1f(m_forwardSpotLightShader->GetUniform("roughness_factor"), materialData->roughnessFactor);
				glUniform1f(m_forwardSpotLightShader->GetUniform("normal_scale"), materialData->normalScale);
				glUniform1f(
					m_forwardSpotLightShader->GetUniform("occlusion_strength"), materialData->occlusionStrength);
				glUniform1i(m_forwardSpotLightShader->GetUniform("alpha_mode"), materialData->alphaMode);
				glUniform1f(m_forwardSpotLightShader->GetUniform("alpha_cutoff"), materialData->alphaCutoff);
				glUniform1i(m_forwardSpotLightShader->GetUniform("double_sided"), materialData->doubleSided);

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
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		// copy to intermediate
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_interFbo);

		glBlitFramebuffer(0, 0, m_camera->GetWidth(), m_camera->GetHeight(), 0, 0, m_camera->GetWidth(),
			m_camera->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);


		// additive blend all directional lights
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		// write to window
		glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);

		glUseProgram(m_simpleOutShader->id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_interColorTexture);

		// big triangle trick, no vao
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisable(GL_BLEND);
	}
}

void GLForwardRenderer::RenderBoundingBoxes()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	auto worldNodes = Engine::GetWorld()->GetNodeMap<GeometryNode>();

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

		glUseProgram(m_bBoxShader->id);
		const auto vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();
		glUniformMatrix4fv(m_bBoxShader->GetUniform("vp"), 1, GL_FALSE, glm::value_ptr(vp));
		glUniform4fv(m_bBoxShader->GetUniform("color"), 1, glm::value_ptr(color));

		// TODO: with fewer draw calls
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glDrawArrays(GL_LINE_LOOP, 4, 4);
		glDrawArrays(GL_LINES, 8, 8);
	};

	for (auto node : worldNodes) {
		RenderBox(node->m_aabb, { 1, 0, 0, 1 });
		// RenderBox(node->GetBBox(), { 1, 1, 1, 1 });
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLForwardRenderer::RenderSkybox()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_CULL_FACE);

	const auto vpNoTransformation = m_camera->GetProjectionMatrix() * glm::mat4(glm::mat3(m_camera->GetViewMatrix()));

	glUseProgram(m_skybox->shader->id);

	glUniformMatrix4fv(m_skybox->shader->GetUniform("vp"), 1, GL_FALSE, glm::value_ptr(vpNoTransformation));
	glBindVertexArray(m_skybox->vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox->cubemap->id);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glDisable(GL_CULL_FACE);
}

void GLForwardRenderer::RenderPostProcess()
{
	// copy msaa to out
	// glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFbo);
	// glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_outFbo);

	// glBlitFramebuffer(0, 0, m_camera->GetWidth(), m_camera->GetHeight(), 0, 0, m_camera->GetWidth(),
	//	m_camera->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// do post process here
}

void GLForwardRenderer::RenderWindowSimple()
{
	const auto window = Engine::GetMainWindow();

	glViewport(0, 0, window->GetWidth(), window->GetHeight());

	// write to window
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_simpleOutShader->id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_outColorTexture);

	// big triangle trick, no vao
	glDrawArrays(GL_TRIANGLES, 0, 3);
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
	glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_interFbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// render directional lights shadow maps
	// RenderDirectionalLights();
	RenderSpotLights();
	// forward msaa-ed pass
	// RenderBoundingBoxes();
	// render skybox, seamless enabled (render last)
	// RenderSkybox();
	// copy msaa to out fbo and render any post process on it
	// RenderPostProcess();
	// write out texture of out fbo to window (big triangle trick)
	RenderWindowSimple();
	//! m_isOutNonLinear ? RenderWindowSimple() : RenderWindowLinearized();

	GLEditorRenderer::Render();
}

void GLForwardRenderer::Update()
{
	GLRendererBase::Update();

	if (Engine::GetInput()->IsKeyPressed(XVirtualKey::R)) {
		RecompileShaders();
	}
}

void GLForwardRenderer::RecompileShaders()
{
	m_forwardSpotLightShader->Load();
	m_simpleOutShader->Load();
	m_linearizeOutShader->Load();
}
} // namespace ogl
