#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/test/GLTestGeometry.h"
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
	// TODO: default skybox model (json) 
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	
	GLTestRenderer::~GLTestRenderer()
	{
		glDeleteFramebuffers(1, &m_fbo);
		glDeleteTextures(1, &m_outTexture);
		glDeleteRenderbuffers(1, &m_depthStencilRbo);
	
		glDeleteVertexArrays(1, &m_skyboxVAO);
		glDeleteBuffers(1, &m_skyboxVBO);
	}

	bool GLTestRenderer::InitScene(int32 width, int32 height)
	{
		auto am = Engine::GetAssetManager();

		// shaders
		auto shaderAsset = AssetManager::GetOrCreate<ShaderPod>("screen_quad.shader.json");
		m_screenQuadShader = GetGLAssetManager()->GetOrMakeFromUri<GLShader>(am->GetPodPath(shaderAsset));

		shaderAsset = AssetManager::GetOrCreate<ShaderPod>("skybox.shader.json");
		m_skyboxShader = GetGLAssetManager()->GetOrMakeFromUri<GLShader>(am->GetPodPath(shaderAsset));

		auto& skyboxShader = *m_skyboxShader;
		skyboxShader += "vp";
		
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
		m_skyboxCubemap = GetGLAssetManager()->GetOrMakeFromUri<GLTexture>(am->GetPodPath(sky->GetSkyMap()));

		m_light = Engine::GetWorld()->GetAvailableNodeSpecificSubType<PunctualLightNode>();
		
		// TODO: better way to check world requirements
		if (!m_light)
		{
			LOG_FATAL("Missing light node!");
			return false;
		}

		for (auto* geometryNode : Engine::GetWorld()->GetNodeMap<GeometryNode>())
			m_geometryObservers.emplace_back(CreateObserver<GLTestRenderer, GLTestGeometry>(this, geometryNode));

		// buffers
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		
		glGenTextures(1, &m_outTexture);
		glBindTexture(GL_TEXTURE_2D, m_outTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outTexture, 0);

		glGenRenderbuffers(1, &m_depthStencilRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG_FATAL("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// skybox
		glGenVertexArrays(1, &m_skyboxVAO);
		glGenBuffers(1, &m_skyboxVBO);
		glBindVertexArray(m_skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
		
		return true;
	}

	void GLTestRenderer::WindowResize(int32 width, int32 height)
	{

		glViewport(0, 0, width, height);
	}

	void GLTestRenderer::Render()
	{
		// first pass
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		auto root = Engine::GetWorld()->GetRoot();
		const auto backgroundColor = root->GetBackgroundColor();
		glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT /*| GL_STENCIL_BUFFER_BIT*/);

		glEnable(GL_DEPTH_TEST);
		
		auto& testShader = *m_testShader;

		glUseProgram(testShader.id);

		glDepthFunc(GL_LESS);

		// global uniforms
		glUniform3fv(testShader["view_pos"], 1, glm::value_ptr(m_camera->GetWorldTranslation()));
		glUniform3fv(testShader["light_pos"], 1, glm::value_ptr(m_light->GetWorldTranslation()));
		glUniform3fv(testShader["light_color"], 1, glm::value_ptr(m_light->GetColor()));
		glUniform1f(testShader["light_intensity"], m_light->GetIntensity());
		glUniform3fv(testShader["ambient"], 1, glm::value_ptr(root->GetAmbientColor()));
		glUniform1i(testShader["mode"], m_previewMode);

		const auto vp = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();
		// render geometry (non-instanced)
		for (auto& geometry : m_geometryObservers)
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

		const auto vpNoTransformation = m_camera->GetProjectionMatrix() * glm::mat4(glm::mat3(m_camera->GetViewMatrix()));
		
		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		// TODO: where to put this
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		
		glUseProgram(m_skyboxShader->id);
		glUniformMatrix4fv((*m_skyboxShader)["vp"], 1, GL_FALSE, glm::value_ptr(vpNoTransformation));
		glBindVertexArray(m_skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxCubemap->id);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		
		// second pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(m_screenQuadShader->id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_outTexture);

		// big triangle trick, no vao
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindTexture(GL_TEXTURE_2D, 0);
		
		glUseProgram(0);
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
