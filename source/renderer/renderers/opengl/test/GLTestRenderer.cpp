#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "asset/AssetManager.h"
#include "system/Engine.h"
#include "renderer/renderers/opengl/assets/GLModel.h"
#include "renderer/renderers/opengl/assets/GLShader.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "world/nodes/sky/SkyCubeNode.h"

#include "glad/glad.h"

namespace OpenGL
{
	GLTestRenderer::~GLTestRenderer()
	{
		glDeleteFramebuffers(1, &m_msaaFbo);
		glDeleteTextures(1, &m_msaaColorTexture);
		glDeleteRenderbuffers(1, &m_msaaDepthStencilRbo);

		glDeleteFramebuffers(1, &m_outFbo);
		glDeleteTextures(1, &m_outColorTexture);
	}

	bool GLTestRenderer::InitScene(int32 width, int32 height)
	{
		auto am = Engine::GetAssetManager();

		// shaders
		auto shaderAsset = AssetManager::GetOrCreate<ShaderPod>("screen_quad.shader.json");
		m_screenQuadShader = GetGLAssetManager()->GetOrMakeFromUri<GLShader>(am->GetPodPath(shaderAsset));

		shaderAsset = AssetManager::GetOrCreate<ShaderPod>("test.shader.json");
		m_testShader = GetGLAssetManager()->GetOrMakeFromUri<GLShader>(am->GetPodPath(shaderAsset));
		
		auto& testShader = *m_testShader;
		testShader += "mvp";
		testShader += "m";
		testShader += "normal_matrix";
		testShader += "mode";
		testShader += "view_pos";
		testShader += "light_pos";
		testShader += "light_color";
		testShader += "light_intensity";
		testShader += "base_color_factor";
		testShader += "emissive_factor";
		testShader += "ambient";
		testShader += "metallic_factor";
		testShader += "roughness_factor";
		testShader += "normal_scale";
		testShader += "occlusion_strength";
		testShader += "alpha_mode";
		testShader += "alpha_cutoff";
		testShader += "double_sided";
	
		// world data
		auto* user = Engine::GetWorld()->GetAvailableNodeSpecificSubType<FreeformUserNode>();

		// TODO: better way to check world requirements
		if(!user)
		{
			LOG_FATAL("Missing freeform user node!" );
			return false;
		}

		m_camera = user->GetCamera();

		auto* sky = Engine::GetWorld()->GetAvailableNodeSpecificSubType<SkyCubeNode>();
		
		if(!sky)
		{
			LOG_FATAL("Missing sky node!");
			return false;
		}
		
		m_skybox = CreateObserver<GLBasicSkybox>(sky);

		for (auto* directionalLightNode : Engine::GetWorld()->GetNodeMap<DirectionalLightNode>())
			m_glDirectionalLights.push_back(CreateObserver<GLBasicDirectionalLight>(directionalLightNode));
		
		for (auto* geometryNode : Engine::GetWorld()->GetNodeMap<GeometryNode>())
			m_glGeometries.push_back(CreateObserver<GLBasicGeometry>(geometryNode));

		// msaa fbo
		glGenFramebuffers(1, &m_msaaFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

		// TODO:
		auto samples = 4;
		glGenTextures(1, &m_msaaColorTexture);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture, 0);

		glGenRenderbuffers(1, &m_msaaDepthStencilRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_msaaDepthStencilRbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_msaaDepthStencilRbo);
	
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG_FATAL("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
		
		// out fbo
		glGenFramebuffers(1, &m_outFbo);
		
		glBindFramebuffer(GL_FRAMEBUFFER, m_outFbo);
	
		glGenTextures(1, &m_outColorTexture);
		glBindTexture(GL_TEXTURE_2D, m_outColorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outColorTexture, 0);
		
		return true;
	}

	void GLTestRenderer::WindowResize(int32 width, int32 height)
	{
		glViewport(0, 0, width, height);
	}

	void GLTestRenderer::RenderGeometries()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);

		glClearColor(0, 0, 0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT /*| GL_STENCIL_BUFFER_BIT*/);
		
		// TODO change syntax
		auto& testShader = *m_testShader;

		glUseProgram(testShader.id);

		auto light = m_glDirectionalLights.at(0).get();

		// global uniforms
		glUniform3fv(testShader["view_pos"], 1, glm::value_ptr(m_camera->GetWorldTranslation()));
		glUniform3fv(testShader["light_pos"], 1, glm::value_ptr(light->GetNode()->GetWorldTranslation()));
		glUniform3fv(testShader["light_color"], 1, glm::value_ptr(light->GetNode()->GetColor()));
		glUniform1f(testShader["light_intensity"], light->GetNode()->GetIntensity());

		auto root = Engine::GetWorld()->GetRoot();
		const auto backgroundColor = root->GetBackgroundColor();
		glUniform3fv(testShader["ambient"], 1, glm::value_ptr(root->GetAmbientColor()));

		const auto vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();
		// render geometry (non-instanced)
		for (auto& geometry : m_glGeometries)
		{
			auto m = geometry->GetNode()->GetWorldMatrix();
			auto mvp = vp * m;

			glUniformMatrix4fv(testShader["mvp"], 1, GL_FALSE, glm::value_ptr(mvp));
			glUniformMatrix4fv(testShader["m"], 1, GL_FALSE, glm::value_ptr(m));
			glUniformMatrix3fv(testShader["normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m)))));

			for (auto& glMesh : geometry->glModel->meshes)
			{
				glBindVertexArray(glMesh.vao);

				GLMaterial* glMaterial = glMesh.material;
				PodHandle<MaterialPod> materialData = glMaterial->m_materialPod;

				glUniform4fv(testShader["base_color_factor"], 1, glm::value_ptr(materialData->baseColorFactor));
				glUniform3fv(testShader["emissive_factor"], 1, glm::value_ptr(materialData->emissiveFactor));
				glUniform1f(testShader["metallic_factor"], materialData->metallicFactor);
				glUniform1f(testShader["roughness_factor"], materialData->roughnessFactor);
				glUniform1f(testShader["normal_scale"], materialData->normalScale);
				glUniform1f(testShader["occlusion_strength"], materialData->occlusionStrength);
				glUniform1i(testShader["alpha_mode"], materialData->alphaMode);
				glUniform1f(testShader["alpha_cutoff"], materialData->alphaCutoff);
				glUniform1i(testShader["double_sided"], materialData->doubleSided);

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

	void GLTestRenderer::RenderSkybox()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_CULL_FACE);
		
		const auto vpNoTransformation = m_camera->GetProjectionMatrix() * glm::mat4(glm::mat3(m_camera->GetViewMatrix()));
		
		glUseProgram(m_skybox->shader->id);
		
		glUniformMatrix4fv((*m_skybox->shader)["vp"], 1, GL_FALSE, glm::value_ptr(vpNoTransformation));
		glBindVertexArray(m_skybox->vao);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox->cubemap->id);
		
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glDisable(GL_CULL_FACE);
	}

	void GLTestRenderer::RenderPostProcess()
	{
		// copy msaa to out
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_outFbo);
		// TODO this is wrong
		auto wnd = Engine::GetMainWindow();
		glBlitFramebuffer(0, 0, wnd->GetWidth(), wnd->GetHeight(), 0, 0, wnd->GetWidth(), wnd->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// do post process here
	}

	void GLTestRenderer::RenderWindow()
	{
		// write to window
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(m_screenQuadShader->id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_outColorTexture);

		// big triangle trick, no vao
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void GLTestRenderer::Render()
	{
		// forward msaa-ed pass
		RenderGeometries();
		// render skybox, seamless enabled (render last)
		RenderSkybox();
		// copy msaa to out fbo and render any post process on it
		RenderPostProcess();
		// write out texture of out fbo to window (big triangle trick)
		RenderWindow();
	}

	void GLTestRenderer::Update()
	{
		if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K1))
		{
			m_previewMode = m_previewMode - 1 < 0 ? PT_COUNT - 1 : m_previewMode - 1;
			m_previewModeString = utl::SurfacePreviewTargetModeString(m_previewMode) + std::string(" -> ") + std::to_string(m_previewMode);
			LOG_INFO("Preview Mode set to: {}({})", utl::SurfacePreviewTargetModeString(m_previewMode), m_previewMode);
		}
		else if (Engine::GetInput()->IsKeyPressed(XVirtualKey::K2))
		{
			++m_previewMode %= PT_COUNT;
			m_previewModeString = utl::SurfacePreviewTargetModeString(m_previewMode) + std::string(" -> ") + std::to_string(m_previewMode);
			LOG_INFO("Preview Mode set to: {}({})", utl::SurfacePreviewTargetModeString(m_previewMode), m_previewMode);
		}
	}
}
