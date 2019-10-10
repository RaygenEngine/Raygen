#include "pch/pch.h"

#include "renderer/renderers/opengl/basic/GLBasicSpotLight.h"
#include "renderer/renderers/opengl/GLAssetManager.h"

namespace OpenGL {
GLBasicSpotLight::GLBasicSpotLight(SpotLightNode* node)
	: NodeObserver<SpotLightNode, GLRendererBase>(node)
{
	auto shaderAsset = AssetManager::GetOrCreate<ShaderPod>("/shaders/glsl/general/DepthMap.json");
	depthMap = GetGLAssetManager(this)->GpuGetOrCreate<GLShader>(shaderAsset);
	depthMap->AddUniform("mvp");

	shaderAsset = AssetManager::GetOrCreate<ShaderPod>("/shaders/glsl/general/DepthMap_AlphaMask.json");
	depthMapAlphaMask = GetGLAssetManager(this)->GpuGetOrCreate<GLShader>(shaderAsset);
	depthMapAlphaMask->AddUniform("mvp");
	depthMapAlphaMask->AddUniform("base_color_factor");
	depthMapAlphaMask->AddUniform("base_color_texcoord_index");
	depthMapAlphaMask->AddUniform("alpha_cutoff");

	glGenFramebuffers(1, &fbo);

	glGenTextures(1, &shadowMap);

	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, node->GetShadowMapWidth(), node->GetShadowMapHeight(), 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	lightSpaceMatrix = node->GetProjectionMatrix() * node->GetViewMatrix();
}

GLBasicSpotLight::~GLBasicSpotLight()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &shadowMap);
}

void GLBasicSpotLight::RenderShadowMap(const std::vector<GLBasicGeometry*>& geometries)
{
	glViewport(0, 0, node->GetShadowMapWidth(), node->GetShadowMapHeight());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClear(GL_DEPTH_BUFFER_BIT);

	auto gBufferShader = depthMap;

	for (auto& geometry : geometries) {
		auto m = geometry->node->GetWorldMatrix();
		auto mvp = lightSpaceMatrix * m;

		for (auto& glMesh : geometry->glModel->meshes) {
			GLMaterial* glMaterial = glMesh.material;
			const MaterialPod* materialData = glMaterial->LockData();

			switch (materialData->alphaMode) {
					// blend not handled
				case AM_BLEND:
				case AM_OPAQUE:
					gBufferShader = depthMap;
					glUseProgram(gBufferShader->id);
					break;
				case AM_MASK:
					gBufferShader = depthMapAlphaMask;
					glUseProgram(gBufferShader->id);
					glUniform1f(gBufferShader->GetUniform("alpha_cutoff"), materialData->alphaCutoff);
					glUniform4fv(gBufferShader->GetUniform("base_color_factor"), 1,
						glm::value_ptr(materialData->baseColorFactor));
					glUniform1i(
						gBufferShader->GetUniform("base_color_texcoord_index"), materialData->baseColorTexCoordIndex);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, glMaterial->baseColorTexture->id);
					break;

				case AM_INVALID: abort();
			}

			glUniformMatrix4fv(gBufferShader->GetUniform("mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

			glBindVertexArray(glMesh.vao);

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

			glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLBasicSpotLight::DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset)
{
	if (nodeDirtyFlagset[SpotLightNode::DF::ResizeShadows]) {
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, node->GetShadowMapWidth(), node->GetShadowMapHeight(), 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	if (nodeDirtyFlagset[SpotLightNode::DF::Projection] || nodeDirtyFlagset[Node::DF::TRS]) {
		lightSpaceMatrix = node->GetProjectionMatrix() * node->GetViewMatrix();
	}
}
} // namespace OpenGL
