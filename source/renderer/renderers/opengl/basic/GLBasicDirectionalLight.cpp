#include "pch/pch.h"

#include "renderer/renderers/opengl/basic/GLBasicDirectionalLight.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "asset/AssetManager.h"

#include <glm/gtc/type_ptr.hpp>

namespace ogl {
GLBasicDirectionalLight::GLBasicDirectionalLight(DirectionalLightNode* node)
	: NodeObserver<DirectionalLightNode, GLRendererBase>(node)
{
	depthMap = GetGLAssetManager(this)->GenerateFromPodPath<GLShader>("/shaders/glsl/general/DepthMap.json");
	depthMap->AddUniform("mvp");

	depthMapAlphaMask
		= GetGLAssetManager(this)->GenerateFromPodPath<GLShader>("/shaders/glsl/general/DepthMap_AlphaMask.json");
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
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	lightSpaceMatrix = node->GetProjectionMatrix() * node->GetViewMatrix();
}

GLBasicDirectionalLight::~GLBasicDirectionalLight()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &shadowMap);
}

void GLBasicDirectionalLight::RenderShadowMap(const std::vector<GLBasicGeometry*>& geometries)
{
	glViewport(0, 0, node->GetShadowMapWidth(), node->GetShadowMapHeight());

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClear(GL_DEPTH_BUFFER_BIT);

	auto shadowMapShader = depthMap;

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
					shadowMapShader = depthMap;
					glUseProgram(shadowMapShader->id);
					break;
				case AM_MASK:
					shadowMapShader = depthMapAlphaMask;
					glUseProgram(shadowMapShader->id);

					shadowMapShader->UploadFloat("alpha_cutoff", materialData->alphaCutoff);
					shadowMapShader->UploadVec4("base_color_factor", materialData->baseColorFactor);
					shadowMapShader->UploadInt("base_color_texcoord_index", materialData->baseColorTexCoordIndex);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, glMaterial->baseColorTexture->id);
					break;
			}

			shadowMapShader->UploadMat4("mvp", mvp);

			glBindVertexArray(glMesh.vao);

			materialData->doubleSided ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

			glDrawElements(GL_TRIANGLES, glMesh.count, GL_UNSIGNED_INT, (GLvoid*)0);
		}
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void GLBasicDirectionalLight::DirtyNodeUpdate(DirtyFlagset nodeDirtyFlagset)
{
	if (nodeDirtyFlagset[DirectionalLightNode::DF::ResizeShadows]) {
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, node->GetShadowMapWidth(), node->GetShadowMapHeight(), 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	if (nodeDirtyFlagset[DirectionalLightNode::DF::Projection] || nodeDirtyFlagset[Node::DF::TRS]) {
		lightSpaceMatrix = node->GetProjectionMatrix() * node->GetViewMatrix();
	}
}
} // namespace ogl
