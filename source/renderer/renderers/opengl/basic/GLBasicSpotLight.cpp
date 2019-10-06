#include "pch.h"

#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL
{
	GLBasicSpotLight::GLBasicSpotLight(SpotLightNode* node)
		: NodeObserver<SpotLightNode, GLRendererBase>(node)
	{
		const auto shaderAsset = AssetManager::GetOrCreate<ShaderPod>("depth_map.shader.json");
		shader = GetGLAssetManager(this)->GetOrMakeFromUri<GLShader>(Engine::GetAssetManager()->GetPodPath(shaderAsset));
		shader->AddUniform("mvp");
		shader->AddUniform("m");
		shader->AddUniform("source_pos");
		shader->AddUniform("far");

		glGenFramebuffers(1, &fbo);

		glGenTextures(1, &shadowMap);
		
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			node->GetShadowMapWidth(), node->GetShadowMapHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	GLBasicSpotLight::~GLBasicSpotLight()
	{
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &shadowMap);
	}

	void GLBasicSpotLight::RenderShadowMap(const std::vector<std::unique_ptr<GLBasicGeometry>>& geometries)
	{
		glViewport(0, 0, node->GetShadowMapWidth(), node->GetShadowMapHeight());

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		auto ar = static_cast<float>(node->GetShadowMapWidth()) / static_cast<float>(node->GetShadowMapHeight());
		const auto lightProjection = glm::perspective(glm::radians(45.0f), ar, node->m_near, node->m_far);

		// TODO check value of center
		const auto center = node->GetWorldTranslation() + node->GetFront();
		const auto lightView = glm::lookAt(node->GetWorldTranslation(), center, node->GetUp());

		// TODO - PERF: calculate this only from update from node not every frame
		lightSpaceMatrix = lightProjection * lightView;
	
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader->id);
		
		for (auto& geometry : geometries)
		{
			auto m = geometry->node->GetWorldMatrix();
			auto mvp = lightSpaceMatrix * m;

			glUniformMatrix4fv(shader->GetUniform("mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

			for (auto& glMesh : geometry->glModel->meshes)
			{
				glBindVertexArray(glMesh.vao);

				GLMaterial* glMaterial = glMesh.material;
				PodHandle<MaterialPod> materialData = glMaterial->m_materialPod;

				materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

				glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
			}
		}

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
	}
}
